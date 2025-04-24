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

#include <ICM42670P_gyro.hpp>
#include <MPU6050_gyro.hpp>
#include <gyro.hpp>
#include <log.hpp>
#include <main.hpp>

#define GYRO_USE_INTERRUPT  // Use interrupt to detect when new sample is ready
#define GYRO_SHOW_MINMAX    // Will calculate the min/max values when doing
                            // calibration

bool GyroSensor::setup() {
  int clock = 400000;

#if defined(FLOATY)
  pinMode(PIN_VCC, OUTPUT);
  pinMode(PIN_GND, OUTPUT_OPEN_DRAIN);
  digitalWrite(PIN_VCC, HIGH);
  digitalWrite(PIN_GND, LOW);
  delay(10);  // Wait for the pins to settle or we will fail to connect
#endif

#if LOG_LEVEL == 6
  Log.verbose(F("GYRO: Setting up hardware." CR));
#endif

  Wire.begin(PIN_SDA, PIN_SCL);
  Wire.setClock(clock);  // 400kHz I2C clock.

  if (MPU6050Gyro::isDeviceDetected()) {
    Log.notice(F("GYRO: Detected MPU6050/MPU6000." CR));
    _impl.reset(new MPU6050Gyro(_gyroConfig));
    _impl->setup();
  }

  if (ICM42670pGyro::isDeviceDetected()) {
    Log.notice(F("GYRO: Detected ICM42670-p." CR));
    _impl.reset(new ICM42670pGyro(_gyroConfig));
    _impl->setup();
  }

  // Add code for new gyro implementations here

  return _impl ? _impl->isConnected() : false;
}

bool GyroSensor::read() {
  if (!_impl) return false;

  GyroResultData resultData = _impl->readSensor();

  _valid = resultData.valid;

  if (resultData.valid) {
    _angle = resultData.angle;
    _temp = resultData.temp;

    if (_initialSensorTemp == INVALID_TEMPERATURE) {
      _initialSensorTemp = resultData.temp;
    }
  }

  return _valid;
}

void GyroSensor::enterSleep() {
#if defined(FLOATY)
  digitalWrite(PIN_VCC, LOW);
#else
  if (_impl) _impl->enterSleep();
#endif
}

float GyroSensorInterface::calculateAngle(float ax, float ay, float az) {
#if LOG_LEVEL == 6
  Log.verbose(F("GYRO: Calculating the angle." CR));
#endif

  // Source: https://www.nxp.com/docs/en/application-note/AN3461.pdf
  float vY = (acos(abs(ay) / sqrt(ax * ax + ay * ay + az * az)) * 180.0 / PI);
  // float vZ = (acos(abs(az) / sqrt(ax * ax + ay * ay + az * az)) * 180.0 /
  // PI);
  float vX = (acos(abs(ax) / sqrt(ax * ax + ay * ay + az * az)) * 180.0 / PI);

#if LOG_LEVEL == 6
  // Log.notice(F("GYRO: angleX= %F." CR), vX);
  Log.notice(F("GYRO: angleY= %F." CR), vY);
  // Log.notice(F("GYRO: angleZ= %F." CR), vZ);
#endif

  return _gyroConfig->isGyroSwapXY() ? vX : vY;
}

bool GyroSensorInterface::isSensorMoving(int16_t gx, int16_t gy, int16_t gz) {
#if LOG_LEVEL == 6
  Log.verbose(F("GYRO: Checking for sensor movement." CR));
#endif

  int x = abs(gx), y = abs(gy), z = abs(gz);
  int threashold = _gyroConfig->getGyroSensorMovingThreashold();
  _sensorMoving = false;

  if (x > threashold || y > threashold || z > threashold) {
    Log.notice(F("GYRO: Movement detected (%d)\t%d\t%d\t%d." CR), threashold, x,
               y, z);
    _sensorMoving = true;
  }

  return _sensorMoving;
}

#endif  // GRAVITYMON

// EOF
