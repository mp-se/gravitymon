/*
MIT License

Copyright (c) 2021-2024 Magnus

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
#include <ICM42670pGyro.hpp>
#include <I2Cdev.h>
#include <log.hpp>
#include <main.hpp>

ICM42670pGyro icmGyro;

#define USE_FIFO_MODE true

#define INT16_FROM_BUFFER(high, low) (int16_t)((((int16_t)buffer[high]) << 8) | buffer[low])
#define UINT16_FROM_BUFFER(high, low) (uint16_t)((((uint16_t)buffer[high]) << 8) | buffer[low])

#define ICM42670_PRIMARY_ADDRESS 0x68
#define ICM42670_SECONDARY_ADDRESS 0x69
#define ICM42670_WHOAMI_REGISTER 0x75
#define ICM42670_WHOAMI_VALUE 0x67
#define ICM42670_FLUSH_REGISTER 0x75
#define ICM42670_FLUSH_VALUE 0x67
#define ICM42670_PWR_MGMT0_REGISTER 0x1F

bool ICM42670pGyro::isOnline()
{
  if (I2Cdev::readByte(ICM42670_PRIMARY_ADDRESS, ICM42670_WHOAMI_REGISTER, buffer) == 1 && buffer[0] == ICM42670_WHOAMI_VALUE)
  {
    addr = ICM42670_PRIMARY_ADDRESS;
    return true;
  }
  if (I2Cdev::readByte(ICM42670_SECONDARY_ADDRESS, ICM42670_WHOAMI_REGISTER, buffer) == 1 && buffer[0] == ICM42670_WHOAMI_VALUE)
  {
    addr = ICM42670_SECONDARY_ADDRESS;
    return true;
  }
  return false;
}

bool ICM42670pGyro::writeMBank1(uint8_t reg, uint8_t value)
{
  bool status = true;
  status &= I2Cdev::writeByte(addr, 0x79, 0);
  status &= I2Cdev::writeByte(addr, 0x7A, reg);
  status &= I2Cdev::writeByte(addr, 0x7B, value);
  delayMicroseconds(10);
  return status;
}

bool ICM42670pGyro::readMBank1(uint8_t reg)
{
  bool status = true;
  status &= I2Cdev::writeByte(addr, 0x7C, 0);
  status &= I2Cdev::writeByte(addr, 0x7D, reg);
  delayMicroseconds(10);
  status &= I2Cdev::readByte(addr, 0x7E, buffer);
  delayMicroseconds(10);
  return status;
}

bool ICM42670pGyro::writeMBank1AndVerify(uint8_t reg, uint8_t value)
{
  int i = 0;
  buffer[0] = 0;
  while (i < 5)
  {
    auto status = true;
    status &= writeMBank1(reg, value);
    status &= readMBank1(reg);
    if (!status || buffer[0] != value)
    {
      // the only reason this occurs is if the IC is still booting after reset or poweron
      // and comms are up but the ic is not actually running correctly
      // only way to fix is no comms for 1s and try again
      delay(1000);
      i++;
    }
    else
    {
      return true;
    }
  }
  return false;
}

uint8_t getNextPowerOf2(int n)
{
  int k = 0;
  while (n > (1 << k) && k < 8)
    k++;
  return k;
}

uint8_t getDecimationValue(uint8_t p)
{
  // 0 = /1
  // 8 = /2
  // 9 = /4
  //...
  // 15 = /256
  if (p == 0)
  {
    return 0;
  }
  return min(7 + p, 15);
}

bool ICM42670pGyro::setup()
{
#if LOG_LEVEL == 6
  Log.verbose(F("ICM : Starting ICM setup" CR));
#endif
#if USE_FIFO_MODE
  if (I2Cdev::readByte(addr, ICM42670_PWR_MGMT0_REGISTER, buffer) == 1 && buffer[0] == 0x0F)
  {
    // ICM is already configured = OK
#if LOG_LEVEL == 6
    Log.info(F("ICM : Setup OK" CR));
#endif
    _sensorConnected = true;
    return true;
  }
  else
  {
    // first time setup or comm faillure
    // start oscillator
    if (!I2Cdev::writeByte(addr, 0x1F, 0x10))
    {
      Log.error(F("ICM : Start OSC failed" CR));
      return false;
    }
    // wait until oscillator is ok, this takes only 1 or 2 comms
    while (I2Cdev::readByte(addr, 0x00, buffer) && buffer[0] == 0)
    {
    }
    // disable apex
    if (!writeMBank1AndVerify(0x06, 0x40))
    {
      Log.error(F("ICM : Disable APEX failed" CR));
      return false;
    }
    // calculate the Hz/fifo
    auto sleep = myConfig.getSleepInterval();
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
    if (!writeMBank1AndVerify(0x66, reg))
    {
      Log.error(F("ICM : Fifo rate set failed" CR));
      return false;
    }
    // enable gyro+accel to go to fifo
    if (!writeMBank1AndVerify(0x01, 0x03))
    {
      Log.error(F("ICM : Fifo output set failed" CR));
      return false;
    }
    delayMicroseconds(10);
    // disable watermark
    I2Cdev::writeByte(addr, 0x29, 255);
    I2Cdev::writeByte(addr, 0x2A, 255);
    // set INTF_CONFIG0
    if (!I2Cdev::writeByte(addr, 0x35, 0x70))
    {
      Log.error(F("ICM : Set INTF_CONFIG0 failed" CR));
      return false;
    }
    // enable sensors
    if (!I2Cdev::writeByte(addr, 0x1F, 0x0F))
    {
      Log.error(F("ICM : Sensor enable failed" CR));
      return false;
    }
    delayMicroseconds(200); // mandatory per datasheet
    // accel + gyro config @12.5 Hz
    uint8_t config[5] = {0x6C, 0x6C, 0x70, 0x37, 0x47};
    if (!I2Cdev::writeBytes(addr, 0x20, 5, config))
    {
      Log.error(F("ICM : Sensor config failed" CR));
    }
    // enable fifo
    if (!I2Cdev::writeByte(addr, 0x28, 0x00))
    {
      Log.error(F("ICM : Fifo enable failed" CR));
    }
  }
#else
  // GYRO=LN ACCEL=LN
  if (!I2Cdev::writeByte(addr, ICM42670_PWR_MGMT0_REGISTER, 0x0F))
  {
    return false;
  }
  configStart = millis();
  delayMicroseconds(200); // mandatory per datasheet
  // Original setup:
  // GYRO=250dps GYRO=800Hz
  // I2Cdev::writeByte(addr, 0x20, 0x66)
  // ACCEL=2g ACCEL=800Hz
  // I2Cdev::writeByte(addr, 0x21, 0x66)
  // TEMP=4HzBW
  // I2Cdev::writeByte(addr, 0x22, 0x70)
  // GYRO=16HzBW
  // I2Cdev::writeByte(addr, 0x23, 0x37)
  // ACCEL=16HzBW
  // I2Cdev::writeByte(addr, 0x24, 0x47)
  uint8_t config[5] = {0x66, 0x66, 0x70, 0x37, 0x47};
  if (
      !I2Cdev::writeBytes(addr, 0x20, 5, config))
  {
    return false;
  }
#endif

  _sensorConnected = true;
  // we will not do calibration for now
  // _calibrationOffset = myConfig.getGyroCalibration();
  // applyCalibration();
  return true;
}

void ICM42670pGyro::enterSleep()
{
#if !USE_FIFO_MODE
  I2Cdev::writeByte(addr, ICM42670_PWR_MGMT0_REGISTER, 0x00);
#endif
}

uint8_t ICM42670pGyro::ReadFIFOPackets(const uint16_t &count, RawGyroDataL &data)
{
  uint8_t success = 0;
  Wire.beginTransmission(addr);
  Wire.write(0x3F);
  if (Wire.endTransmission() == 0)
  {
    uint8_t req = 0;
    uint16_t total = 0;
    while (count > total)
    {
      req = (uint8_t)min(count - total, 8);
      if (Wire.requestFrom(addr, (size_t)(req * 16), (req == (count - total))) == req * 16)
      {
        while (Wire.available() >= 16)
        {
          Wire.readBytes(buffer, 16);

          if ((buffer[0] & 0b11111100) == 0b01101000 && !isSensorMoving(INT16_FROM_BUFFER(7, 8), INT16_FROM_BUFFER(9, 10), INT16_FROM_BUFFER(11, 12)))
          {
            data.ax += INT16_FROM_BUFFER(1, 2);
            data.ay += INT16_FROM_BUFFER(3, 4);
            data.az += INT16_FROM_BUFFER(5, 6);
            data.temp += (int8_t)buffer[13];
            success++;
          }
        }
      }
      total += req;
    }
  }
  return success;
}

GyroResultData ICM42670pGyro::readSensor()
{
  RawGyroDataL average = {0, 0, 0, 0, 0, 0};

#if USE_FIFO_MODE
  buffer[0] = 0;
  buffer[1] = 0;
  I2Cdev::readBytes(addr, 0x3D, 2, buffer);
  uint16_t count = UINT16_FROM_BUFFER(0, 1);
  count = min(count, (uint16_t)138);
#if LOG_LEVEL == 6
  Log.verbose(F("ICM : available packets= %d" CR), count);
#endif
  GyroResultData result = {false, _angle, _sensorTemp};
  if (count > 0)
  {
    uint16_t valid = ReadFIFOPackets(count, average);

    if (valid)
    {
      average.ax /= valid;
      average.ay /= valid;
      average.az /= valid;
      float ax = (static_cast<float>(average.ax)) / 16384,
            ay = (static_cast<float>(average.ay)) / 16384,
            az = (static_cast<float>(average.az)) / 16384;
      result.isValid = true;
      if (result.isValid)
      {
        result.angle = calculateAngle(ax, ay, az);
      }
      result.temp = (static_cast<float>(average.temp)) / valid / 2 + 25;
    }
  }
  return result;
#else
  int noIterations = myConfig.getGyroReadCount();
  RawGyroData raw;
  auto end = millis();
  auto dur = end - configStart;
  if (dur < 45)
  {
    delay(45 - dur);
  }

  for (int cnt = 0; cnt < noIterations; cnt++)
  {
    // INT_STATUS_DRDY
    while (I2Cdev::readByte(addr, 0x36, buffer) == 0 || buffer[0] == 0)
    {
    }

    I2Cdev::readBytes(addr, 0x09, 14, buffer);
    raw.temp = (((int16_t)buffer[0]) << 8) | buffer[1];
    raw.ax = (((int16_t)buffer[2]) << 8) | buffer[3];
    raw.ay = (((int16_t)buffer[4]) << 8) | buffer[5];
    raw.az = (((int16_t)buffer[6]) << 8) | buffer[7];
    raw.gx = (((int16_t)buffer[8]) << 8) | buffer[9];
    raw.gy = (((int16_t)buffer[10]) << 8) | buffer[11];
    raw.gz = (((int16_t)buffer[12]) << 8) | buffer[13];

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
  GyroResultData result;
  result.isValid = !isSensorMovingRaw(raw);
  if (result.isValid)
  {
    result.angle = calculateAngleRaw(raw);
  }
  result.temp = (static_cast<float>(raw.temp)) / 340 + 36.53;
  return result;
#endif
}

void ICM42670pGyro::applyCalibration()
{
  // don't for now, these should be properly factory calibrated, any slight error during calibration will introduce more error
}

void ICM42670pGyro::calibrateSensor()
{
  // don't for now, these should be properly factory calibrated, any slight error during calibration will introduce more error
  // fake calibration to get rid of error
  _calibrationOffset.ax = 1;
  _calibrationOffset.ay = 0;
  _calibrationOffset.az = 0;
  _calibrationOffset.gx = 0;
  _calibrationOffset.gy = 0;
  _calibrationOffset.gz = 0;

  myConfig.setGyroCalibration(_calibrationOffset);
  myConfig.saveFile();
}

void ICM42670pGyro::debug()
{
  // todo
}

String ICM42670pGyro::getGyroFamily()
{
  return "ICM42670-p";
}

// EOF
