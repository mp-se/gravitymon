/*
MIT License

Copyright (c) 2021-2026 Magnus

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
#include <cstdio>
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

void MPU6050Gyro::getGyroTestResult(JsonObject &doc) {
  EspSerial.printf("Accel full scale range: %d\n",
                   _accelgyro.getFullScaleAccelRange());
  EspSerial.printf("Gyro full scale range: %d\n",
                   _accelgyro.getFullScaleGyroRange());
  // int16_t temp = _accelgyro.getTemperature();
  // EspSerial.printf("Temperature: %.2f C\n", temp / 100.0);

  // doc["full_accel_range"] = _accelgyro.getFullScaleAccelRange();
  // doc["full_gyro_range"] = _accelgyro.getFullScaleGyroRange();
  doc["gyro_temp"] =
      (static_cast<float>(_accelgyro.getTemperature())) / 340 + 36.53;

  // Read config registers to check self-test enable
  uint8_t gyroConfig, accelConfig;
  I2Cdev::readByte(_addr, MPU6050_RA_GYRO_CONFIG, &gyroConfig);
  I2Cdev::readByte(_addr, MPU6050_RA_ACCEL_CONFIG, &accelConfig);
  EspSerial.printf("Gyro Config: 0x%02X (XG_ST: %d, YG_ST: %d, ZG_ST: %d)\n",
                   gyroConfig, (gyroConfig >> 7) & 1, (gyroConfig >> 6) & 1,
                   (gyroConfig >> 5) & 1);
  EspSerial.printf("Accel Config: 0x%02X (XA_ST: %d, YA_ST: %d, ZA_ST: %d)\n",
                   accelConfig, (accelConfig >> 7) & 1, (accelConfig >> 6) & 1,
                   (accelConfig >> 5) & 1);

  // doc["gyro_config"]["xg_st"] = (gyroConfig >> 7) & 1;
  // doc["gyro_config"]["yg_st"] = (gyroConfig >> 6) & 1;
  // doc["gyro_config"]["zg_st"] = (gyroConfig >> 5) & 1;
  // doc["accel_config"]["xa_st"] = (accelConfig >> 7) & 1;
  // doc["accel_config"]["ya_st"] = (accelConfig >> 6) & 1;
  // doc["accel_config"]["za_st"] = (accelConfig >> 5) & 1;

  // Read self-test registers
  uint8_t selfTestX, selfTestY, selfTestZ, selfTestA;
  I2Cdev::readByte(_addr, MPU6050_RA_SELF_TEST_X, &selfTestX);
  I2Cdev::readByte(_addr, MPU6050_RA_SELF_TEST_Y, &selfTestY);
  I2Cdev::readByte(_addr, MPU6050_RA_SELF_TEST_Z, &selfTestZ);
  I2Cdev::readByte(_addr, MPU6050_RA_SELF_TEST_A, &selfTestA);

  // Extract and print accel and gyro self-test values separately
  uint8_t xa_high = (selfTestX >> 5) & 0x07;
  uint8_t xa_low = (selfTestA >> 4) & 0x03;
  uint8_t xa_test = (xa_high << 2) | xa_low;
  uint8_t xg_test = selfTestX & 0x1F;

  uint8_t ya_high = (selfTestY >> 5) & 0x07;
  uint8_t ya_low = (selfTestA >> 2) & 0x03;
  uint8_t ya_test = (ya_high << 2) | ya_low;
  uint8_t yg_test = selfTestY & 0x1F;

  uint8_t za_high = (selfTestZ >> 5) & 0x07;
  uint8_t za_low = selfTestA & 0x03;
  uint8_t za_test = (za_high << 2) | za_low;
  uint8_t zg_test = selfTestZ & 0x1F;

  EspSerial.printf("Accel Self-Test: X=%d, Y=%d, Z=%d\n", xa_test, ya_test,
                   za_test);
  EspSerial.printf("Gyro Self-Test: X=%d, Y=%d, Z=%d\n", xg_test, yg_test,
                   zg_test);

  // doc["self_test"]["accel"]["x"] = xa_test;
  // doc["self_test"]["accel"]["y"] = ya_test;
  // doc["self_test"]["accel"]["z"] = za_test;
  // doc["self_test"]["gyro"]["x"] = xg_test;
  // doc["self_test"]["gyro"]["y"] = yg_test;
  // doc["self_test"]["gyro"]["z"] = zg_test;

  // Perform self-test
  EspSerial.printf("Performing MPU6050 Self-Test...\n");

  // Enable self-test for all axes
  I2Cdev::writeByte(_addr, MPU6050_RA_GYRO_CONFIG,
                    0xE0);  // XG_ST=1, YG_ST=1, ZG_ST=1 (bits 7,6,5)
  I2Cdev::writeByte(_addr, MPU6050_RA_ACCEL_CONFIG,
                    0xE0);  // XA_ST=1, YA_ST=1, ZA_ST=1 (bits 7,6,5)
  delay(100);               // Allow settling

  // Read average with self-test enabled
  int16_t ax_st = 0, ay_st = 0, az_st = 0, gx_st = 0, gy_st = 0, gz_st = 0;
  int16_t ax_temp, ay_temp, az_temp, gx_temp, gy_temp, gz_temp;
  for (int i = 0; i < 10; i++) {
    _accelgyro.getMotion6(&ax_temp, &ay_temp, &az_temp, &gx_temp, &gy_temp,
                          &gz_temp);
    ax_st += ax_temp;
    ay_st += ay_temp;
    az_st += az_temp;
    gx_st += gx_temp;
    gy_st += gy_temp;
    gz_st += gz_temp;
    delay(10);
  }
  ax_st /= 10;
  ay_st /= 10;
  az_st /= 10;
  gx_st /= 10;
  gy_st /= 10;
  gz_st /= 10;

  // Disable self-test
  I2Cdev::writeByte(_addr, MPU6050_RA_GYRO_CONFIG, 0x00);
  I2Cdev::writeByte(_addr, MPU6050_RA_ACCEL_CONFIG, 0x00);
  delay(100);

  // Read average with self-test disabled
  int16_t ax_norm = 0, ay_norm = 0, az_norm = 0, gx_norm = 0, gy_norm = 0,
          gz_norm = 0;
  for (int i = 0; i < 10; i++) {
    _accelgyro.getMotion6(&ax_temp, &ay_temp, &az_temp, &gx_temp, &gy_temp,
                          &gz_temp);
    ax_norm += ax_temp;
    ay_norm += ay_temp;
    az_norm += az_temp;
    gx_norm += gx_temp;
    gy_norm += gy_temp;
    gz_norm += gz_temp;
    delay(10);
  }
  ax_norm /= 10;
  ay_norm /= 10;
  az_norm /= 10;
  gx_norm /= 10;
  gy_norm /= 10;
  gz_norm /= 10;

  // Calculate self-test response (change)
  int16_t ax_delta = ax_st - ax_norm;
  int16_t ay_delta = ay_st - ay_norm;
  int16_t az_delta = az_st - az_norm;
  int16_t gx_delta = gx_st - gx_norm;
  int16_t gy_delta = gy_st - gy_norm;
  int16_t gz_delta = gz_st - gz_norm;

  EspSerial.printf("Self-Test Results:\n");
  EspSerial.printf("Accel Delta: X=%d, Y=%d, Z=%d\n", ax_delta, ay_delta,
                   az_delta);
  EspSerial.printf("Gyro Delta: X=%d, Y=%d, Z=%d\n", gx_delta, gy_delta,
                   gz_delta);

  doc["self_test"]["accel"]["x_delta"] = ax_delta;
  doc["self_test"]["accel"]["y_delta"] = ay_delta;
  doc["self_test"]["accel"]["z_delta"] = az_delta;
  doc["self_test"]["gyro"]["x_delta"] = gx_delta;
  doc["self_test"]["gyro"]["y_delta"] = gy_delta;
  doc["self_test"]["gyro"]["z_delta"] = gz_delta;

  // Check if deltas are within reasonable range (rough check, adjust as needed)
  bool accel_pass =
      (abs(ax_delta) > 500 && abs(ay_delta) > 500 && abs(az_delta) > 500);
  bool gyro_pass =
      (abs(gx_delta) > 500 && abs(gy_delta) > 500 && abs(gz_delta) > 500);
  EspSerial.printf("Self-Test Pass: Accel=%s, Gyro=%s\n",
                   accel_pass ? "YES" : "NO", gyro_pass ? "YES" : "NO");

  doc["self_test"]["accel"]["result"] = accel_pass ? "passed" : "failed";
  doc["self_test"]["gyro"]["result"] = gyro_pass ? "passed" : "failed";
}

bool MPU6050Gyro::calibrateSensor() {
#if LOG_LEVEL == 6
  Log.verbose(F("GYRO: Calibrating sensor" CR));
#endif

  // Start of calibration sequence
  _accelgyro.setDLPFMode(MPU6050_DLPF_BW_5);
  if (!_accelgyro.CalibrateAccel(6)) {  // 6 = 600 readings
    return false;
  }

  if (!_accelgyro.CalibrateGyro(6)) {  // 6 = 600 readings
    return false;
  }

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
  return true;
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
  const uint8_t who = _accelgyro.getDeviceID();
  switch (who) {
    case 0x34:
      return "MPU6050";
    case 0x38:
      return "MPU6500";
    default: {
      static char buffer[16];
      snprintf(buffer, sizeof(buffer), "0x%02X", who);
      return buffer;
    }
  }
}

#endif  // GRAVITYMON

// EOF
