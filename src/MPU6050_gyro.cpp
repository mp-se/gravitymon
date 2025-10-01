/*
MIT License

Copyright (c) 2021-2025 Magnus

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

#include <MPU6050_gyro.hpp>
#include <log.hpp>
#include <main.hpp>

#define GYRO_USE_INTERRUPT  // Use interrupt to detect when new sample is ready
#define GYRO_SHOW_MINMAX    // Will calculate the min/max values when doing
                            // calibration

bool MPU6050Gyro::isDeviceDetected(uint8_t &addr) {
  uint8_t whoami;
  if (I2Cdev::readBits(MPU6050_ADDRESS_AD0_LOW, MPU6050_RA_WHO_AM_I,
                       MPU6050_WHO_AM_I_BIT, MPU6050_WHO_AM_I_LENGTH,
                       &whoami) == 1 &&
      (whoami == 0x34 || whoami == 0x38)) {
    addr = MPU6050_ADDRESS_AD0_LOW;
    return true;
  }
  if (I2Cdev::readBits(MPU6050_ADDRESS_AD0_HIGH, MPU6050_RA_WHO_AM_I,
                       MPU6050_WHO_AM_I_BIT, MPU6050_WHO_AM_I_LENGTH,
                       &whoami) == 1 &&
      (whoami == 0x34 || whoami == 0x38)) {
    addr = MPU6050_ADDRESS_AD0_HIGH;
    return true;
  }
  return false;
}

bool MPU6050Gyro::setup(GyroMode mode, bool force) {
  _accelgyro.initialize();

  // Configure the sensor
  _accelgyro.setTempSensorEnabled(true);
  _accelgyro.setDLPFMode(MPU6050_DLPF_BW_5);
#if defined(GYRO_USE_INTERRUPT)
  // Alternative method to read data, let the MPU signal when sampling is
  // done.
  _accelgyro.setRate(17);
  _accelgyro.setInterruptDrive(1);
  _accelgyro.setInterruptMode(1);
  _accelgyro.setInterruptLatch(0);
  _accelgyro.setIntDataReadyEnabled(true);
#endif

  // Once we have calibration values stored we just apply them from the
  // config.
  _calibrationOffset = _gyroConfig->getGyroCalibration();
  applyCalibration();
  return true;
}

GyroMode MPU6050Gyro::enterSleep(GyroMode mode) {
  _accelgyro.setSleepEnabled(true);
  return GyroMode::GYRO_SLEEP;
}

GyroResultData MPU6050Gyro::readSensor(GyroMode mode) {
  int noIterations = _gyroConfig->getGyroReadCount();
#if !defined(GYRO_USE_INTERRUPT)
  int delayTime = myConfig.getGyroReadDelay();
#endif
  RawGyroDataL average = {0, 0, 0, 0, 0, 0};

  // Set some initial values
#if defined(GYRO_SHOW_MINMAX)
  RawGyroData min = {0, 0, 0};
  RawGyroData max = {0, 0, 0};
  _accelgyro.getAcceleration(&min.ax, &min.ay, &min.az);
  min.temp = _accelgyro.getTemperature();
  max = min;
#endif
  for (int cnt = 0; cnt < noIterations; cnt++) {
#if defined(GYRO_USE_INTERRUPT)
    while (_accelgyro.getIntDataReadyStatus() == 0) {
      delayMicroseconds(1);
    }
#endif

    _accelgyro.getMotion6(&raw.ax, &raw.ay, &raw.az, &raw.gx, &raw.gy, &raw.gz);
    raw.temp = _accelgyro.getTemperature();

    average.ax += raw.ax;
    average.ay += raw.ay;
    average.az += raw.az;
    average.gx += raw.gx;
    average.gy += raw.gy;
    average.gz += raw.gz;
    average.temp += raw.temp;

    // Log what the minium value is
#if defined(GYRO_SHOW_MINMAX)
    if (raw.ax < min.ax) min.ax = raw.ax;
    if (raw.ay < min.ay) min.ay = raw.ay;
    if (raw.az < min.az) min.az = raw.az;
    if (raw.gx < min.gx) min.gx = raw.gx;
    if (raw.gy < min.gy) min.gy = raw.gy;
    if (raw.gz < min.gz) min.gz = raw.gz;
    if (raw.temp < min.temp) min.temp = raw.temp;

    // Log what the maximum value is
    if (raw.ax > max.ax) max.ax = raw.ax;
    if (raw.ay > max.ay) max.ay = raw.ay;
    if (raw.az > max.az) max.az = raw.az;
    if (raw.gx > max.gx) max.gx = raw.gx;
    if (raw.gy > max.gy) max.gy = raw.gy;
    if (raw.gz > max.gz) max.gz = raw.gz;
    if (raw.temp > max.temp) max.temp = raw.temp;
#endif

#if !defined(GYRO_USE_INTERRUPT)
    delayMicroseconds(delayTime);
#endif
  }

  // Protect against division by zero
  if (noIterations > 0) {
    raw.ax = average.ax / noIterations;
    raw.ay = average.ay / noIterations;
    raw.az = average.az / noIterations;
    raw.gx = average.gx / noIterations;
    raw.gy = average.gy / noIterations;
    raw.gz = average.gz / noIterations;
    raw.temp = average.temp / noIterations;
  } else {
    Log.error(F("GYRO: No iterations performed, using zero values." CR));
    raw.ax = raw.ay = raw.az = raw.gx = raw.gy = raw.gz = raw.temp = 0;
  }

#if defined(GYRO_SHOW_MINMAX) && LOG_LEVEL == 6
  Log.verbose(F("GYRO: Min    \t%d\t%d\t%d\t%d\t%d\t%d\t%d." CR), min.ax,
              min.ay, min.az, min.gx, min.gy, min.gz, min.temp);
  Log.verbose(F("GYRO: Max    \t%d\t%d\t%d\t%d\t%d\t%d\t%d." CR), max.ax,
              max.ay, max.az, max.gx, max.gy, max.gz, max.temp);
#endif
  GyroResultData result;
  result.valid = !isSensorMoving(raw.gx, raw.gy, raw.gz);

  if (result.valid) {
    // Smooth out the readings to we can have a more stable angle/tilt.
    // ------------------------------------------------------------------------------------------------------------
    // Accelerometer full scale range of +/- 2g with Sensitivity Scale Factor of
    // 16,384 LSB(Count)/g. Gyroscope full scale range of +/- 250 °/s with
    // Sensitivity Scale Factor of 131 LSB (Count)/°/s.
    float ax = (static_cast<float>(raw.ax)) / 16384,
          ay = (static_cast<float>(raw.ay)) / 16384,
          az = (static_cast<float>(raw.az)) / 16384;

    result.angle = calculateAngle(ax, ay, az);
  }

  result.temp = (static_cast<float>(raw.temp)) / 340 + 36.53;
  return result;
}

void MPU6050Gyro::applyCalibration() {
#if LOG_LEVEL == 6
  Log.verbose(F("GYRO: Applying calibration offsets to sensor." CR));
#endif

  if ((_calibrationOffset.ax + _calibrationOffset.ay + _calibrationOffset.az +
       _calibrationOffset.gx + _calibrationOffset.gy + _calibrationOffset.gz) ==
      0) {
    writeErrorLog(
        "GYRO: No valid calibration values, please calibrate the device.");
    return;
  }

  _accelgyro.setXAccelOffset(_calibrationOffset.ax);
  _accelgyro.setYAccelOffset(_calibrationOffset.ay);
  _accelgyro.setZAccelOffset(_calibrationOffset.az);
  _accelgyro.setXGyroOffset(_calibrationOffset.gx);
  _accelgyro.setYGyroOffset(_calibrationOffset.gy);
  _accelgyro.setZGyroOffset(_calibrationOffset.gz);
}

void MPU6050Gyro::calibrateSensor() {
#if LOG_LEVEL == 6
  Log.verbose(F("GYRO: Calibrating sensor" CR));
#endif
  // _accelgyro.PrintActiveOffsets();
  // EspSerial.print( CR );

  _accelgyro.setDLPFMode(MPU6050_DLPF_BW_5);
  _accelgyro.CalibrateAccel(6);  // 6 = 600 readings
  _accelgyro.CalibrateGyro(6);

  _accelgyro.PrintActiveOffsets();
  EspSerial.print(CR);

  _calibrationOffset.ax = _accelgyro.getXAccelOffset();
  _calibrationOffset.ay = _accelgyro.getYAccelOffset();
  _calibrationOffset.az = _accelgyro.getZAccelOffset();
  _calibrationOffset.gx = _accelgyro.getXGyroOffset();
  _calibrationOffset.gy = _accelgyro.getYGyroOffset();
  _calibrationOffset.gz = _accelgyro.getZGyroOffset();

  _gyroConfig->setGyroCalibration(_calibrationOffset);
  _gyroConfig->saveFile();
}

void MPU6050Gyro::debug() {
#if LOG_LEVEL == 6
  Log.verbose(F("GYRO: Debug - Clock src   %d." CR),
              _accelgyro.getClockSource());
  Log.verbose(F("GYRO: Debug - Device ID   %d." CR), _accelgyro.getDeviceID());
  Log.verbose(F("GYRO: Debug - DHPF Mode   %d." CR), _accelgyro.getDHPFMode());
  Log.verbose(F("GYRO: Debug - DMP on      %s." CR),
              _accelgyro.getDMPEnabled() ? "on" : "off");
  Log.verbose(F("GYRO: Debug - Acc range   %d." CR),
              _accelgyro.getFullScaleAccelRange());
  Log.verbose(F("GYRO: Debug - Gyr range   %d." CR),
              _accelgyro.getFullScaleGyroRange());
  Log.verbose(F("GYRO: Debug - Int         %s." CR),
              _accelgyro.getIntEnabled() ? "on" : "off");
  Log.verbose(F("GYRO: Debug - Clock       %d." CR),
              _accelgyro.getMasterClockSpeed());
  Log.verbose(F("GYRO: Debug - Rate        %d." CR), _accelgyro.getRate());
  Log.verbose(F("GYRO: Debug - Gyro range  %d." CR),
              _accelgyro.getFullScaleGyroRange());
  Log.verbose(F("GYRO: Debug - Acc FactX   %d." CR),
              _accelgyro.getAccelXSelfTestFactoryTrim());
  Log.verbose(F("GYRO: Debug - Acc FactY   %d." CR),
              _accelgyro.getAccelYSelfTestFactoryTrim());
  Log.verbose(F("GYRO: Debug - Acc FactZ   %d." CR),
              _accelgyro.getAccelZSelfTestFactoryTrim());
  Log.verbose(F("GYRO: Debug - Gyr FactX   %d." CR),
              _accelgyro.getGyroXSelfTestFactoryTrim());
  Log.verbose(F("GYRO: Debug - Gyr FactY   %d." CR),
              _accelgyro.getGyroYSelfTestFactoryTrim());
  Log.verbose(F("GYRO: Debug - Gyr FactZ   %d." CR),
              _accelgyro.getGyroZSelfTestFactoryTrim());

  switch (_accelgyro.getFullScaleAccelRange()) {
    case 0:
      Log.verbose(F("GYRO: Debug - Accel range +/- 2g." CR));
      break;
    case 1:
      Log.verbose(F("GYRO: Debug - Accel range +/- 4g." CR));
      break;
    case 2:
      Log.verbose(F("GYRO: Debug - Accel range +/- 8g." CR));
      break;
    case 3:
      Log.verbose(F("GYRO: Debug - Accel range +/- 16g." CR));
      break;
  }

  Log.verbose(F("GYRO: Debug - Acc OffX    %d\t%d." CR),
              _accelgyro.getXAccelOffset(), _calibrationOffset.az);
  Log.verbose(F("GYRO: Debug - Acc OffY    %d\t%d." CR),
              _accelgyro.getYAccelOffset(), _calibrationOffset.ay);
  Log.verbose(F("GYRO: Debug - Acc OffZ    %d\t%d." CR),
              _accelgyro.getZAccelOffset(), _calibrationOffset.az);
  Log.verbose(F("GYRO: Debug - Gyr OffX    %d\t%d." CR),
              _accelgyro.getXGyroOffset(), _calibrationOffset.gx);
  Log.verbose(F("GYRO: Debug - Gyr OffY    %d\t%d." CR),
              _accelgyro.getYGyroOffset(), _calibrationOffset.gy);
  Log.verbose(F("GYRO: Debug - Gyr OffZ    %d\t%d." CR),
              _accelgyro.getZGyroOffset(), _calibrationOffset.gz);
#endif
}

const char *MPU6050Gyro::getGyroFamily() {
  switch (_accelgyro.getDeviceID()) {
    case 0x34:
      return "MPU6050";
    case 0x38:
      return "MPU6500";
    default:
      return "";
  }
}

#endif  // GRAVITYMON

// EOF
