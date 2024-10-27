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

GyroSensor* myGyro = &mpuGyro;

float GyroSensor::calculateAngleRaw(RawGyroData &raw) {
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

  float vY = calculateAngle(ax, ay, az);
#if LOG_LEVEL == 6
  // Log.notice(F("GYRO: angleX= %F." CR), vX);
  Log.notice(F("GYRO: angleY= %F." CR), vY);
  // Log.notice(F("GYRO: angleZ= %F." CR), vZ);
#endif
  return vY;
}

float GyroSensor::calculateAngle(float ax, float ay, float az) {
  // Source: https://www.nxp.com/docs/en/application-note/AN3461.pdf
  float vY = (acos(abs(ay) / sqrt(ax * ax + ay * ay + az * az)) * 180.0 / PI);
  // float vZ = (acos(abs(az) / sqrt(ax * ax + ay * ay + az * az)) * 180.0 /
  // PI); float vX = (acos(abs(ax) / sqrt(ax * ax + ay * ay + az * az)) * 180.0
  // / PI);
  return vY;
}

bool GyroSensor::isSensorMovingRaw(RawGyroData &raw) {
#if LOG_LEVEL == 6
  Log.verbose(F("GYRO: Checking for sensor movement." CR));
#endif

  int x = abs(raw.gx), y = abs(raw.gy), z = abs(raw.gz);
  int threashold = myConfig.getGyroSensorMovingThreashold();

  if (x > threashold || y > threashold || z > threashold) {
    Log.notice(F("GYRO: Movement detected (%d)\t%d\t%d\t%d." CR), threashold, x,
               y, z);
    return true;
  }

  return false;
}

bool GyroSensor::isSensorMoving(int16_t gx, int16_t gy, int16_t gz) {
  int x = abs(gx), y = abs(gy), z = abs(gz);
  int threashold = myConfig.getGyroSensorMovingThreashold();
    // Log.notice(F("GYRO: Movement (%d)\t%d\t%d\t%d." CR), threashold, x,
    //            y, z);
  return x > threashold || y > threashold || z > threashold;
}

bool GyroSensor::read() {
#if LOG_LEVEL == 6
  Log.verbose(F("GYRO: Getting new gyro position." CR));
#endif

  if (!_sensorConnected) return false;

#if LOG_LEVEL == 6
  Log.verbose(F("GYRO: Reading sensor with %d iterations %d us delay." CR),
              noIterations, delayTime);
#endif

  auto result = readSensor();

#if LOG_LEVEL == 6
  Log.verbose(F("GYRO: Average\t%d\t%d\t%d\t%d\t%d\t%d\t%d." CR), _lastGyroData.ax,
              _lastGyroData.ay, _lastGyroData.az, _lastGyroData.gx, _lastGyroData.gy,
              _lastGyroData.gz, _lastGyroData.temp);
#endif

  // If the sensor is unstable we return false to signal we dont have valid
  // value
  if (result.isValid) {
    _validValue = true;
    _angle = result.angle;
#if LOG_LEVEL == 6
    Log.verbose(F("GYRO: Sensor values %d,%d,%d\t%F" CR), _lastGyroData.ax,
                _lastGyroData.ay, _lastGyroData.az, _angle);
#endif
  } else {
#if LOG_LEVEL == 6
    Log.notice(F("GYRO: Sensor is moving." CR));
#endif
    _validValue = false;
  }

  _sensorTemp = result.temp;

  // The first read value is close to the DS18 value according to my tests, if
  // more reads are done then the gyro temp will increase to much
  if (_initialSensorTemp == INVALID_TEMPERATURE)
    _initialSensorTemp = _sensorTemp;

  return _validValue;
}

void GyroSensor::dumpCalibration() {
#if LOG_LEVEL == 6
  Log.verbose(F("GYRO: Accel offset\t%d\t%d\t%d" CR), _calibrationOffset.ax,
              _calibrationOffset.ay, _calibrationOffset.az);
  Log.verbose(F("GYRO: Gyro offset \t%d\t%d\t%d" CR), _calibrationOffset.gx,
              _calibrationOffset.gy, _calibrationOffset.gz);
#endif
}

// EOF
