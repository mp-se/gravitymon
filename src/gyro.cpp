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
#include <main_gravitymon.hpp>

#define GYRO_USE_INTERRUPT  // Use interrupt to detect when new sample is ready
// #define GYRO_SHOW_MINMAX    // Will calculate the min/max values when doing
// calibration

#if defined(ESP32) && defined(ENABLE_RTCMEM)
RTC_DATA_ATTR RtcGyroData myRtcGyroData;
RTC_DATA_ATTR FilterData myRtcFilterData;
#endif

GyroType GyroSensor::detectGyro() {
  Log.notice(F("GYRO: Detecting installed gyro." CR));
  int clock = 400000;
  uint8_t addr;

  Wire.begin(PIN_SDA, PIN_SCL);
  Wire.setClock(clock);  // 400kHz I2C clock.

  if( MPU6050Gyro::isDeviceDetected(addr) ) {
    Log.notice(F("GYRO: MPU6050 detected." CR));
    return GyroType::GYRO_MPU6050;
  }

  else if( ICM42670pGyro::isDeviceDetected(addr) ) {
    Log.notice(F("GYRO: ICM42670P detected." CR));
    return GyroType::GYRO_ICM42670P;
  }

  return GyroType::GYRO_NONE;
}

bool GyroSensor::setup(GyroMode mode, bool force) {
  uint8_t addr = 0;
  int clock = 400000;

  if (_currentMode == GyroMode::GYRO_UNCONFIGURED || !_impl) {
    Log.notice(F("GYRO: Setting up hardware." CR));

    switch(_gyroConfig->getGyroType()) {
      case GyroType::GYRO_MPU6050: {
        Wire.begin(PIN_SDA, PIN_SCL);
        Wire.setClock(clock);  // 400kHz I2C clock.

#if defined(ESP32) && defined(ENABLE_RTCMEM)
        if (myRtcGyroData.IsDataAvailable == GYRO_RTC_DATA_AVAILABLE) {
          Log.notice(F("GYRO: Using MPU6050/MPU6500 %x." CR), myRtcGyroData.Address);

          _impl.reset(new MPU6050Gyro(myRtcGyroData.Address, _gyroConfig));
          _currentMode = GyroMode::GYRO_RUN;
        } else 
#endif
        if (MPU6050Gyro::isDeviceDetected(addr)) {
          Log.notice(F("GYRO: Detected MPU6050/MPU6500 %x." CR), addr);
          _impl.reset(new MPU6050Gyro(addr, _gyroConfig));
#if defined(ESP32) && defined(ENABLE_RTCMEM)
          myRtcGyroData = {.Address = addr,
                          .IsDataAvailable = GYRO_RTC_DATA_AVAILABLE };
#endif
        }
      }
      break;
      
      case GyroType::GYRO_ICM42670P: {
        Wire.begin(PIN_SDA, PIN_SCL);
        Wire.setClock(clock);  // 400kHz I2C clock.

#if defined(ESP32) && defined(ENABLE_RTCMEM)
        if (myRtcGyroData.IsDataAvailable == GYRO_RTC_DATA_AVAILABLE) {
          Log.notice(F("GYRO: Using ICM42670-p %x." CR), myRtcGyroData.Address);
          _impl.reset(new ICM42670pGyro(myRtcGyroData.Address, _gyroConfig));
          _currentMode = GyroMode::GYRO_RUN;
        } else 
#endif
        if (ICM42670pGyro::isDeviceDetected(addr)) {
          Log.notice(F("GYRO: Detected ICM42670-p %x." CR), addr);
          _impl.reset(new ICM42670pGyro(addr, _gyroConfig));
#if defined(ESP32) && defined(ENABLE_RTCMEM)
          myRtcGyroData = {.Address = addr,
                          .IsDataAvailable = GYRO_RTC_DATA_AVAILABLE};
#endif
        }
      }
      break;
    }
  }

  if (_impl) {
    if (_currentMode == mode && !force) {
      return true;  // already correctly setup
    }
    if (_impl->setup(mode, force)) {
      _currentMode = mode;
    }
  } else {
    _currentMode = GyroMode::GYRO_UNCONFIGURED;
  }

  return _currentMode != GyroMode::GYRO_UNCONFIGURED;
}

bool GyroSensor::read() {
  if (!_impl) return false;

  GyroResultData resultData = _impl->readSensor(_currentMode);

  _valid = resultData.valid;

  if (resultData.valid) {
    _angle = resultData.angle;
    _temp = resultData.temp;

    if (_initialSensorTemp == INVALID_TEMPERATURE) {
      _initialSensorTemp = resultData.temp;
    }
#if defined(ESP32) && defined(ENABLE_RTCMEM)
    _filteredAngle = _filter->filter(_angle);
#else
    _filteredAngle = _angle;
#endif
  }

  return _valid;
}

void GyroSensor::enterSleep() {
  if (_impl) _currentMode = _impl->enterSleep(_currentMode);
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
