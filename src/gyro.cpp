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
#include <MPU6050.h>

#include <gyro.hpp>
#include <log.hpp>
#include <main.hpp>

GyroSensor myGyro;
MPU6050 accelgyro;

#define GYRO_USE_INTERRUPT // Use interrupt to detect when new sample is ready
#define GYRO_SHOW_MINMAX   // Will calculate the min/max values when doing
                           // calibration

uint8_t GyroSensor::getGyroID() { return accelgyro.getDeviceID(); }

bool GyroSensor::setup()
{
  int clock = 400000;
#if defined(FLOATY)
  pinMode(PIN_VCC, OUTPUT);
  pinMode(PIN_GND, OUTPUT_OPEN_DRAIN);
  digitalWrite(PIN_VCC, HIGH);
  digitalWrite(PIN_GND, LOW);
  delay(10); // Wait for the pins to settle or we will fail to connect
#else
#endif
  /* For testing pin config of new boards with led.
  pinMode(PIN_SDA, OUTPUT);
  pinMode(PIN_SCL, OUTPUT);
  for(int i = 0, j = LOW, k = LOW; i < 100; i++) {

    digitalWrite(PIN_SDA, k);
    digitalWrite(PIN_SCL, j);
    k = !k;
    delay(300);
    digitalWrite(PIN_SDA, k);
    k = !k;
    j = !j;
    delay(300);
  }*/

#if LOG_LEVEL == 6
  Log.verbose(F("GYRO: Setting up hardware." CR));
#endif
  Wire.begin(PIN_SDA, PIN_SCL);
  Wire.setClock(clock); // 400kHz I2C clock.

  uint8_t id = accelgyro.getDeviceID();

  if (id != 0x34 && id != 0x38)
  { // Allow both MPU6050 and MPU6500
    writeErrorLog("GYRO: Failed to connect to gyro, is it connected?");
    _sensorConnected = false;
  }
  else
  {
#if LOG_LEVEL == 6
    Log.notice(F("GYRO: Connected to MPU6050 (gyro)." CR));
#endif
    accelgyro.initialize();
    _sensorConnected = true;

    // Configure the sensor
    accelgyro.setTempSensorEnabled(true);
    accelgyro.setDLPFMode(MPU6050_DLPF_BW_5);
#if defined(GYRO_USE_INTERRUPT)
    // Alternative method to read data, let the MPU signal when sampling is
    // done.
    accelgyro.setRate(17);
    accelgyro.setInterruptDrive(1);
    accelgyro.setInterruptMode(1);
    accelgyro.setInterruptLatch(0);
    accelgyro.setIntDataReadyEnabled(true);
#endif

    // Once we have calibration values stored we just apply them from the
    // config.
    _calibrationOffset = myConfig.getGyroCalibration();
    applyCalibration();
  }
  return _sensorConnected;
}

void GyroSensor::enterSleep()
{
#if LOG_LEVEL == 6
  Log.verbose(F("GYRO: Setting up hardware." CR));
#endif
#if defined(FLOATY)
  digitalWrite(PIN_VCC, LOW);
#else
  accelgyro.setSleepEnabled(true);
#endif
}

void GyroSensor::readSensor(RawGyroData &raw, const int noIterations,
                            const int delayTime)
{
  RawGyroDataL average = {0, 0, 0, 0, 0, 0};

#if LOG_LEVEL == 6
  Log.verbose(F("GYRO: Reading sensor with %d iterations %d us delay." CR),
              noIterations, delayTime);
#endif

  // Set some initial values
#if defined(GYRO_SHOW_MINMAX)
  RawGyroData min = {0, 0, 0};
  RawGyroData max = {0, 0, 0};
  accelgyro.getAcceleration(&min.ax, &min.ay, &min.az);
  min.temp = accelgyro.getTemperature();
  max = min;
#endif
  for (int cnt = 0; cnt < noIterations; cnt++)
  {
#if defined(GYRO_USE_INTERRUPT)
    while (accelgyro.getIntDataReadyStatus() == 0)
    {
      delayMicroseconds(1);
    }
#endif

    accelgyro.getMotion6(&raw.ax, &raw.ay, &raw.az, &raw.gx, &raw.gy, &raw.gz);
    raw.temp = accelgyro.getTemperature();

    average.ax += raw.ax;
    average.ay += raw.ay;
    average.az += raw.az;
    average.gx += raw.gx;
    average.gy += raw.gy;
    average.gz += raw.gz;
    average.temp += raw.temp;

    // Log what the minium value is
#if defined(GYRO_SHOW_MINMAX)
    if (raw.ax < min.ax)
      min.ax = raw.ax;
    if (raw.ay < min.ay)
      min.ay = raw.ay;
    if (raw.az < min.az)
      min.az = raw.az;
    if (raw.gx < min.gx)
      min.gx = raw.gx;
    if (raw.gy < min.gy)
      min.gy = raw.gy;
    if (raw.gz < min.gz)
      min.gz = raw.gz;
    if (raw.temp < min.temp)
      min.temp = raw.temp;

    // Log what the maximum value is
    if (raw.ax > max.ax)
      max.ax = raw.ax;
    if (raw.ay > max.ay)
      max.ay = raw.ay;
    if (raw.az > max.az)
      max.az = raw.az;
    if (raw.gx > max.gx)
      max.gx = raw.gx;
    if (raw.gy > max.gy)
      max.gy = raw.gy;
    if (raw.gz > max.gz)
      max.gz = raw.gz;
    if (raw.temp > max.temp)
      max.temp = raw.temp;
#endif

#if !defined(GYRO_USE_INTERRUPT)
    delayMicroseconds(delayTime);
#endif
  }

  raw.ax = average.ax / noIterations;
  raw.ay = average.ay / noIterations;
  raw.az = average.az / noIterations;
  raw.gx = average.gx / noIterations;
  raw.gy = average.gy / noIterations;
  raw.gz = average.gz / noIterations;
  raw.temp = average.temp / noIterations;

#if LOG_LEVEL == 6
#if defined(GYRO_SHOW_MINMAX)
  Log.verbose(F("GYRO: Min    \t%d\t%d\t%d\t%d\t%d\t%d\t%d." CR), min.ax,
              min.ay, min.az, min.gx, min.gy, min.gz, min.temp);
  Log.verbose(F("GYRO: Max    \t%d\t%d\t%d\t%d\t%d\t%d\t%d." CR), max.ax,
              max.ay, max.az, max.gx, max.gy, max.gz, max.temp);
#endif
  Log.verbose(F("GYRO: Average\t%d\t%d\t%d\t%d\t%d\t%d\t%d." CR), raw.ax,
              raw.ay, raw.az, raw.gx, raw.gy, raw.gz, raw.temp);
  // Log.verbose(F("GYRO: Result \t%d\t%d\t%d\t%d\t%d\t%d." CR),
  // average.ax/noIterations, average.ay/noIterations, average.az/noIterations,
  //                                                             average.gx/noIterations,
  //                                                             average.gy/noIterations,
  //                                                             average.gz/noIterations
  //                                                             );
#endif
}

float GyroSensor::calculateAngle(RawGyroData &raw)
{
#if LOG_LEVEL == 6
  Log.verbose(F("GYRO: Calculating the angle." CR));
#endif

  // Smooth out the readings to we can have a more stable angle/tilt.
  // ------------------------------------------------------------------------------------------------------------
  // Accelerometer full scale range of +/- 2g with Sensitivity Scale Factor of
  // 16,384 LSB(Count)/g. Gyroscope full scale range of +/- 250 °/s with
  // Sensitivity Scale Factor of 131 LSB (Count)/°/s.
  float ax = (static_cast<float>(raw.ax)) / 16384,
        ay = (static_cast<float>(raw.ay)) / 16384,
        az = (static_cast<float>(raw.az)) / 16384;

  // Source: https://www.nxp.com/docs/en/application-note/AN3461.pdf
  float vY = (acos(abs(ay) / sqrt(ax * ax + ay * ay + az * az)) * 180.0 / PI);
  // float vZ = (acos(abs(az) / sqrt(ax * ax + ay * ay + az * az)) * 180.0 /
  // PI); float vX = (acos(abs(ax) / sqrt(ax * ax + ay * ay + az * az)) * 180.0
  // / PI);
#if LOG_LEVEL == 6
  // Log.notice(F("GYRO: angleX= %F." CR), vX);
  Log.notice(F("GYRO: angleY= %F." CR), vY);
  // Log.notice(F("GYRO: angleZ= %F." CR), vZ);
#endif
  return vY;
}

bool GyroSensor::isSensorMoving(RawGyroData &raw)
{
#if LOG_LEVEL == 6
  Log.verbose(F("GYRO: Checking for sensor movement." CR));
#endif

  int x = abs(raw.gx), y = abs(raw.gy), z = abs(raw.gz);
  int threashold = myConfig.getGyroSensorMovingThreashold();
  _sensorMoving = false;

  if (x > threashold || y > threashold || z > threashold)
  {
    Log.notice(F("GYRO: Movement detected (%d)\t%d\t%d\t%d." CR), threashold, x,
               y, z);
    _sensorMoving = true;
  }

  return _sensorMoving;
}

bool GyroSensor::read()
{
#if LOG_LEVEL == 6
  Log.verbose(F("GYRO: Getting new gyro position." CR));
#endif

  if (!_sensorConnected)
    return false;

  readSensor(_lastGyroData, myConfig.getGyroReadCount(),
             myConfig.getGyroReadDelay()); // Last param is unused if
                                           // GYRO_USE_INTERRUPT is defined.

  // If the sensor is unstable we return false to signal we dont have valid
  // value
  if (isSensorMoving(_lastGyroData))
  {
#if LOG_LEVEL == 6
    Log.notice(F("GYRO: Sensor is moving." CR));
#endif
    _validValue = false;
  }
  else
  {
    _validValue = true;
    _angle = calculateAngle(_lastGyroData);
#if LOG_LEVEL == 6
    Log.verbose(F("GYRO: Sensor values %d,%d,%d\t%F" CR), _lastGyroData.ax,
                _lastGyroData.ay, _lastGyroData.az, _angle);
#endif
  }

  _sensorTemp = (static_cast<float>(_lastGyroData.temp)) / 340 + 36.53;

  // The first read value is close to the DS18 value according to my tests, if
  // more reads are done then the gyro temp will increase to much
  if (_initialSensorTemp == INVALID_TEMPERATURE)
    _initialSensorTemp = _sensorTemp;

  return _validValue;
}

void GyroSensor::dumpCalibration()
{
#if LOG_LEVEL == 6
  Log.verbose(F("GYRO: Accel offset\t%d\t%d\t%d" CR), _calibrationOffset.ax,
              _calibrationOffset.ay, _calibrationOffset.az);
  Log.verbose(F("GYRO: Gyro offset \t%d\t%d\t%d" CR), _calibrationOffset.gx,
              _calibrationOffset.gy, _calibrationOffset.gz);
#endif
}

void GyroSensor::applyCalibration()
{
#if LOG_LEVEL == 6
  Log.verbose(F("GYRO: Applying calibration offsets to sensor." CR));
#endif

  if ((_calibrationOffset.ax + _calibrationOffset.ay + _calibrationOffset.az +
       _calibrationOffset.gx + _calibrationOffset.gy + _calibrationOffset.gz) ==
      0)
  {
    writeErrorLog(
        "GYRO: No valid calibration values, please calibrate the device.");
    return;
  }

  accelgyro.setXAccelOffset(_calibrationOffset.ax);
  accelgyro.setYAccelOffset(_calibrationOffset.ay);
  accelgyro.setZAccelOffset(_calibrationOffset.az);
  accelgyro.setXGyroOffset(_calibrationOffset.gx);
  accelgyro.setYGyroOffset(_calibrationOffset.gy);
  accelgyro.setZGyroOffset(_calibrationOffset.gz);
}

void GyroSensor::calibrateSensor()
{
#if LOG_LEVEL == 6
  Log.verbose(F("GYRO: Calibrating sensor" CR));
#endif
  // accelgyro.PrintActiveOffsets();
  // EspSerial.print( CR );

  accelgyro.setDLPFMode(MPU6050_DLPF_BW_5);
  accelgyro.CalibrateAccel(6); // 6 = 600 readings
  accelgyro.CalibrateGyro(6);

  accelgyro.PrintActiveOffsets();
  EspSerial.print(CR);

  _calibrationOffset.ax = accelgyro.getXAccelOffset();
  _calibrationOffset.ay = accelgyro.getYAccelOffset();
  _calibrationOffset.az = accelgyro.getZAccelOffset();
  _calibrationOffset.gx = accelgyro.getXGyroOffset();
  _calibrationOffset.gy = accelgyro.getYGyroOffset();
  _calibrationOffset.gz = accelgyro.getZGyroOffset();

  myConfig.setGyroCalibration(_calibrationOffset);
  myConfig.saveFile();
}

void GyroSensor::debug()
{
#if LOG_LEVEL == 6
  Log.verbose(F("GYRO: Debug - Clock src   %d." CR),
              accelgyro.getClockSource());
  Log.verbose(F("GYRO: Debug - Device ID   %d." CR), accelgyro.getDeviceID());
  Log.verbose(F("GYRO: Debug - DHPF Mode   %d." CR), accelgyro.getDHPFMode());
  Log.verbose(F("GYRO: Debug - DMP on      %s." CR),
              accelgyro.getDMPEnabled() ? "on" : "off");
  Log.verbose(F("GYRO: Debug - Acc range   %d." CR),
              accelgyro.getFullScaleAccelRange());
  Log.verbose(F("GYRO: Debug - Gyr range   %d." CR),
              accelgyro.getFullScaleGyroRange());
  Log.verbose(F("GYRO: Debug - Int         %s." CR),
              accelgyro.getIntEnabled() ? "on" : "off");
  Log.verbose(F("GYRO: Debug - Clock       %d." CR),
              accelgyro.getMasterClockSpeed());
  Log.verbose(F("GYRO: Debug - Rate        %d." CR), accelgyro.getRate());
  Log.verbose(F("GYRO: Debug - Gyro range  %d." CR),
              accelgyro.getFullScaleGyroRange());
  Log.verbose(F("GYRO: Debug - Acc FactX   %d." CR),
              accelgyro.getAccelXSelfTestFactoryTrim());
  Log.verbose(F("GYRO: Debug - Acc FactY   %d." CR),
              accelgyro.getAccelYSelfTestFactoryTrim());
  Log.verbose(F("GYRO: Debug - Acc FactZ   %d." CR),
              accelgyro.getAccelZSelfTestFactoryTrim());
  Log.verbose(F("GYRO: Debug - Gyr FactX   %d." CR),
              accelgyro.getGyroXSelfTestFactoryTrim());
  Log.verbose(F("GYRO: Debug - Gyr FactY   %d." CR),
              accelgyro.getGyroYSelfTestFactoryTrim());
  Log.verbose(F("GYRO: Debug - Gyr FactZ   %d." CR),
              accelgyro.getGyroZSelfTestFactoryTrim());

  switch (accelgyro.getFullScaleAccelRange())
  {
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
              accelgyro.getXAccelOffset(), _calibrationOffset.az);
  Log.verbose(F("GYRO: Debug - Acc OffY    %d\t%d." CR),
              accelgyro.getYAccelOffset(), _calibrationOffset.ay);
  Log.verbose(F("GYRO: Debug - Acc OffZ    %d\t%d." CR),
              accelgyro.getZAccelOffset(), _calibrationOffset.az);
  Log.verbose(F("GYRO: Debug - Gyr OffX    %d\t%d." CR),
              accelgyro.getXGyroOffset(), _calibrationOffset.gx);
  Log.verbose(F("GYRO: Debug - Gyr OffY    %d\t%d." CR),
              accelgyro.getYGyroOffset(), _calibrationOffset.gy);
  Log.verbose(F("GYRO: Debug - Gyr OffZ    %d\t%d." CR),
              accelgyro.getZGyroOffset(), _calibrationOffset.gz);
#endif
}

// EOF
