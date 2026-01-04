/*
MIT License

Copyright (c) 2025-2026 Magnus, Levi--G

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */
#if defined(GRAVITYMON)

#include <I2Cdev.h>

#include <ICM42670P_gyro.hpp>
#include <algorithm>
#include <log.hpp>
#include <main.hpp>

#define INT16_FROM_BUFFER(high, low)                                  \
  static_cast<int16_t>(((static_cast<int16_t>(_buffer[high])) << 8) | \
                       _buffer[low])
#define UINT16_FROM_BUFFER(high, low)                                   \
  static_cast<uint16_t>(((static_cast<uint16_t>(_buffer[high])) << 8) | \
                        _buffer[low])

#define ICM42670_PRIMARY_ADDRESS 0x68
#define ICM42670_SECONDARY_ADDRESS 0x69
#define ICM42670_WHOAMI_REGISTER 0x75
#define ICM42670_WHOAMI_VALUE 0x67
#define ICM42670_FLUSH_REGISTER 0x75
#define ICM42670_FLUSH_VALUE 0x67
#define ICM42670_PWR_MGMT0_REGISTER 0x1F
#define ICM42670_FIFO_CONFIG1_REGISTER 0x28

bool ICM42670pGyro::isDeviceDetected(uint8_t &addr) {
  uint8_t whoami = 0;

  if (I2Cdev::readByte(ICM42670_PRIMARY_ADDRESS, ICM42670_WHOAMI_REGISTER,
                       &whoami) == 1 &&
      whoami == ICM42670_WHOAMI_VALUE) {
    addr = ICM42670_PRIMARY_ADDRESS;
    return true;
  }
  if (I2Cdev::readByte(ICM42670_SECONDARY_ADDRESS, ICM42670_WHOAMI_REGISTER,
                       &whoami) == 1 &&
      whoami == ICM42670_WHOAMI_VALUE) {
    addr = ICM42670_SECONDARY_ADDRESS;
    return true;
  }
  return false;
}

bool ICM42670pGyro::writeMBank1(uint8_t reg, uint8_t value) {
  bool status = true;
  status &= I2Cdev::writeByte(_addr, 0x79, 0);
  status &= I2Cdev::writeByte(_addr, 0x7A, reg);
  status &= I2Cdev::writeByte(_addr, 0x7B, value);
  delayMicroseconds(10);
  return status;
}

bool ICM42670pGyro::readMBank1(uint8_t reg) {
  bool status = true;
  status &= I2Cdev::writeByte(_addr, 0x7C, 0);
  status &= I2Cdev::writeByte(_addr, 0x7D, reg);
  delayMicroseconds(10);
  status &= I2Cdev::readByte(_addr, 0x7E, _buffer);
  delayMicroseconds(10);
  return status;
}

bool ICM42670pGyro::writeMBank1AndVerify(uint8_t reg, uint8_t value) {
  int i = 0;
  _buffer[0] = 0;
  while (i < 5) {
    auto status = true;
    status &= writeMBank1(reg, value);
    status &= readMBank1(reg);
    if (!status || _buffer[0] != value) {
      // the only reason this occurs is if the IC is still booting after reset
      // or poweron and comms are up but the ic is not actually running
      // correctly only way to fix is no comms for 1s and try again
      delay(1000);
      i++;
    } else {
      return true;
    }
  }
  return false;
}

uint8_t getNextPowerOf2(int n) {
  int k = 0;
  while (n > (1 << k) && k < 8) k++;
  return k;
}

uint8_t getDecimationValue(uint8_t p) {
  // 0 = /1
  // 8 = /2
  // 9 = /4
  //...
  // 15 = /256
  if (p == 0) {
    return 0;
  }
  return min(7 + p, 15);
}

bool ICM42670pGyro::setup(GyroMode mode, bool force) {
#if LOG_LEVEL == 6
  Log.verbose(F("ICM : Starting ICM setup" CR));
#endif
  if (!force && mode == GyroMode::GYRO_RUN &&
      I2Cdev::readByte(_addr, ICM42670_PWR_MGMT0_REGISTER, _buffer) == 1 &&
      (_buffer[0] & 0x0F) == 0x0F &&
      I2Cdev::readByte(_addr, ICM42670_FIFO_CONFIG1_REGISTER, _buffer) == 1 &&
      (_buffer[0] & 0x03) == 0x00) {
    // ICM is already configured = OK
#if LOG_LEVEL == 6
    Log.info(F("ICM : Setup OK" CR));
#endif
    return true;
  }

  // first time setup or comm faillure
  // reset ICM
  if (!I2Cdev::writeByte(_addr, 0x02, 0x14)) {
    Log.error(F("ICM : Reset failed" CR));
    return false;
  }
  while (!I2Cdev::readByte(_addr, 0x02, _buffer) ||
         (_buffer[0] & 0x10) == 0x10) {
#if LOG_LEVEL == 6
    Log.verbose(F("ICM : Waiting for reset" CR));
#endif
  }
  // mandatory wait
  delay(3);

  if (mode == GYRO_RUN) {
    // start oscillator
    if (!I2Cdev::writeByte(_addr, ICM42670_PWR_MGMT0_REGISTER, 0x10)) {
      Log.error(F("ICM : Start OSC failed" CR));
      return false;
    }
    // wait until oscillator is ok, this takes only 1 or 2 comms
    while (I2Cdev::readByte(_addr, 0x00, _buffer) && _buffer[0] == 0) {
    }
    // disable apex
    if (!writeMBank1AndVerify(0x06, 0x40)) {
      Log.error(F("ICM : Disable APEX failed" CR));
      return false;
    }
    // calculate the Hz/fifo
    auto sleep = _gyroConfig->getSleepInterval();
    // 80ms 2.25kB 2250B / 16B - 2 = 138 available packets
    // 80m * 138=11s recording time without decimation
    // select best decimation:
    auto decimation = sleep / 11;
    // round up to nearest possible value
    auto power = getNextPowerOf2(decimation);
    // get actual reg value
    auto reg = getDecimationValue(power);
#if LOG_LEVEL == 6
    Log.verbose(F("ICM : Configuring decimation for %d" CR), sleep);
    Log.verbose(F("ICM : decimation= %d" CR), decimation);
    Log.verbose(F("ICM : power= %d" CR), power);
    Log.verbose(F("ICM : reg= %d" CR), reg);
#endif
    // set fifo rate lower
    if (!writeMBank1AndVerify(0x66, reg)) {
      Log.error(F("ICM : Fifo rate set failed" CR));
      return false;
    }
    // enable gyro+accel to go to fifo
    if (!writeMBank1AndVerify(0x01, 0x03)) {
      Log.error(F("ICM : Fifo output set failed" CR));
      return false;
    }
    delayMicroseconds(10);
    // disable watermark
    I2Cdev::writeByte(_addr, 0x29, 255);
    I2Cdev::writeByte(_addr, 0x2A, 255);
    // set INTF_CONFIG0
    if (!I2Cdev::writeByte(_addr, 0x35, 0x70)) {
      Log.error(F("ICM : Set INTF_CONFIG0 failed" CR));
      return false;
    }
    // enable sensors
    if (!I2Cdev::writeByte(_addr, ICM42670_PWR_MGMT0_REGISTER, 0x0F)) {
      Log.error(F("ICM : Sensor enable failed" CR));
      return false;
    }
    delayMicroseconds(200);  // mandatory per datasheet
    // accel + gyro config @12.5 Hz
    uint8_t config[5] = {0x6C, 0x6C, 0x70, 0x37, 0x47};
    if (!I2Cdev::writeBytes(_addr, 0x20, 5, config)) {
      Log.error(F("ICM : Sensor config failed" CR));
    }
    // enable fifo
    if (!I2Cdev::writeByte(_addr, ICM42670_FIFO_CONFIG1_REGISTER, 0x00)) {
      Log.error(F("ICM : Fifo enable failed" CR));
    }
  } else if (mode == GyroMode::GYRO_CONTINUOUS) {
    // GYRO=LN ACCEL=LN
    if (!I2Cdev::writeByte(_addr, ICM42670_PWR_MGMT0_REGISTER, 0x0F)) {
      return false;
    }
    _configStart = millis();
    delayMicroseconds(200);  // mandatory per datasheet
    // GYRO=250dps GYRO=800Hz
    // ACCEL=2g ACCEL=800Hz
    // TEMP=4HzBW
    // GYRO=16HzBW
    // ACCEL=16HzBW
    uint8_t config[5] = {0x66, 0x66, 0x70, 0x37, 0x47};
    if (!I2Cdev::writeBytes(_addr, 0x20, 5, config)) {
      return false;
    }
  } else {
    return false;
  }
  return true;
}

GyroMode ICM42670pGyro::enterSleep(GyroMode mode) {
  if (mode == GyroMode::GYRO_RUN) {
    return mode;
  }
  return setup(GyroMode::GYRO_RUN, false) ? GyroMode::GYRO_RUN
                                          : GyroMode::GYRO_UNCONFIGURED;
}

uint8_t ICM42670pGyro::ReadFIFOPackets(const uint16_t &count,
                                       RawGyroDataL &data) {
  uint8_t success = 0;
  Wire.beginTransmission(_addr);
  Wire.write(0x3F);
  if (Wire.endTransmission() == 0) {
    uint8_t req = 0;
    uint16_t total = 0;
    while (count > total) {
      req = static_cast<uint8_t>(min(count - total, 8));
      if (Wire.requestFrom(_addr, static_cast<size_t>(req * 16),
                           (req == (count - total))) == req * 16) {
        while (Wire.available() >= 16) {
          Wire.readBytes(_buffer, 16);

          if ((_buffer[0] & 0b11111100) == 0b01101000 &&
              !isSensorMoving(INT16_FROM_BUFFER(7, 8), INT16_FROM_BUFFER(9, 10),
                              INT16_FROM_BUFFER(11, 12))) {
            data.ax += INT16_FROM_BUFFER(1, 2);
            data.ay += INT16_FROM_BUFFER(3, 4);
            data.az += INT16_FROM_BUFFER(5, 6);
            data.temp += _buffer[13];
            success++;
          }
        }
      }
      total += req;
    }
  }
  return success;
}

GyroResultData ICM42670pGyro::readSensor(GyroMode mode) {
  RawGyroDataL average = {0, 0, 0, 0, 0, 0};
  GyroResultData _result = {false, 0, 0};
  if (mode == GyroMode::GYRO_RUN) {
    _buffer[0] = 0;
    _buffer[1] = 0;
    I2Cdev::readBytes(_addr, 0x3D, 2, _buffer);
    uint16_t count = UINT16_FROM_BUFFER(0, 1);
    count = min(count, static_cast<uint16_t>(138));
#if LOG_LEVEL == 6
    Log.verbose(F("ICM : available packets= %d" CR), count);
#endif
    if (count > 0) {
      count = ReadFIFOPackets(count, average);

      if (count) {
        average.ax /= count;
        average.ay /= count;
        average.az /= count;
        float ax = (static_cast<float>(average.ax)) / 16384,
              ay = (static_cast<float>(average.ay)) / 16384,
              az = (static_cast<float>(average.az)) / 16384;
        _result.valid = true;
        _result.angle = calculateAngle(ax, ay, az);
        _result.temp = (static_cast<float>(average.temp)) / count / 2 + 25;
        Log.notice(F("ICM : angle=%F, temp=%F" CR), _result.angle,
                   _result.temp);
      }
    }
  } else if (mode == GyroMode::GYRO_CONTINUOUS) {
    int noIterations = _gyroConfig->getGyroReadCount();
    RawGyroData raw;
    auto end = millis();
    auto dur = end - _configStart;
    if (dur < 45) {
      delay(45 - dur);
    }

    for (int cnt = 0; cnt < noIterations; cnt++) {
      // INT_STATUS_DRDY
      while (I2Cdev::readByte(_addr, 0x36, _buffer) == 0 ||
             (_buffer[0] & 1) == 0) {
#if LOG_LEVEL == 6
        Log.verbose(F("INT_STATUS_DRDY: %X" CR), _buffer[0]);
#endif
      }

      I2Cdev::readBytes(_addr, 0x09, 14, _buffer);
      raw.temp = ((static_cast<int16_t>(_buffer[0])) << 8) | _buffer[1];
      raw.ax = ((static_cast<int16_t>(_buffer[2])) << 8) | _buffer[3];
      raw.ay = ((static_cast<int16_t>(_buffer[4])) << 8) | _buffer[5];
      raw.az = ((static_cast<int16_t>(_buffer[6])) << 8) | _buffer[7];
      raw.gx = ((static_cast<int16_t>(_buffer[8])) << 8) | _buffer[9];
      raw.gy = ((static_cast<int16_t>(_buffer[10])) << 8) | _buffer[11];
      raw.gz = ((static_cast<int16_t>(_buffer[12])) << 8) | _buffer[13];
      // Log.verbose(F("Buffer: %X %X %X %X %X %X %X" CR), raw.temp, raw.ax,
      // raw.ay, raw.az, raw.gx, raw.gy, raw.gz);

      average.ax += raw.ax;
      average.ay += raw.ay;
      average.az += raw.az;
      average.gx += raw.gx;
      average.gy += raw.gy;
      average.gz += raw.gz;
      average.temp += raw.temp;
    }

    raw.ax = average.ax / noIterations;
    raw.ay = average.ay / noIterations;
    raw.az = average.az / noIterations;
    raw.gx = average.gx / noIterations;
    raw.gy = average.gy / noIterations;
    raw.gz = average.gz / noIterations;
    raw.temp = average.temp / noIterations;

    Log.verbose(F("Results: %d\t%d\t%d\t%d\t%d\t%d\t%d" CR), raw.ax, raw.ay,
                raw.az, raw.gx, raw.gy, raw.gz, raw.temp);

    _result.valid = !isSensorMoving(raw.gx, raw.gy, raw.gz);
    if (_result.valid) {
      // Smooth out the readings to we can have a more stable angle/tilt.
      // ------------------------------------------------------------------------------------------------------------
      // Accelerometer full scale range of +/- 2g with Sensitivity Scale Factor
      // of 16,384 LSB(Count)/g. Gyroscope full scale range of +/- 250 °/s with
      // Sensitivity Scale Factor of 131 LSB (Count)/°/s.
      float ax = (static_cast<float>(raw.ax)) / 16384,
            ay = (static_cast<float>(raw.ay)) / 16384,
            az = (static_cast<float>(raw.az)) / 16384;

      _result.angle = calculateAngle(ax, ay, az);
    }
    _result.temp = (static_cast<float>(raw.temp)) / 340 + 36.53;
  }
  return _result;
}

bool ICM42670pGyro::calibrateSensor() {
  // don't for now, these should be properly factory calibrated, any slight
  return true;
}

void ICM42670pGyro::debug() {
  // TODO: Implement debug method for ICM gyro
}

const char *ICM42670pGyro::getGyroFamily() { return "ICM42670-p"; }

#endif  // GRAVITYMON

// EOF
