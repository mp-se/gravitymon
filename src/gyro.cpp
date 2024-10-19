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

float GyroSensor::calculateAngle(RawGyroData &raw) {
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

bool GyroSensor::isSensorMoving(RawGyroData &raw) {
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

bool GyroSensor::read() {
#if LOG_LEVEL == 6
  Log.verbose(F("GYRO: Getting new gyro position." CR));
#endif

  if (!_sensorConnected) return false;

  readSensor(_lastGyroData, myConfig.getGyroReadCount(),
             myConfig.getGyroReadDelay());  // Last param is unused if
                                            // GYRO_USE_INTERRUPT is defined.

  // If the sensor is unstable we return false to signal we dont have valid
  // value
  if (isSensorMoving(_lastGyroData)) {
#if LOG_LEVEL == 6
    Log.notice(F("GYRO: Sensor is moving." CR));
#endif
    _validValue = false;
  } else {
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

void GyroSensor::dumpCalibration() {
#if LOG_LEVEL == 6
  Log.verbose(F("GYRO: Accel offset\t%d\t%d\t%d" CR), _calibrationOffset.ax,
              _calibrationOffset.ay, _calibrationOffset.az);
  Log.verbose(F("GYRO: Gyro offset \t%d\t%d\t%d" CR), _calibrationOffset.gx,
              _calibrationOffset.gy, _calibrationOffset.gz);
#endif
}

// EOF
