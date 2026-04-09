/*
 * GravityMon
 * Copyright (c) 2021-2026 Magnus
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Alternatively, this software may be used under the terms of a
 * commercial license. See LICENSE_COMMERCIAL for details.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */


#ifndef SRC_GYRO_HPP_
#define SRC_GYRO_HPP_

#if defined(GRAVITYMON)

#define I2CDEV_IMPLEMENTATION I2CDEV_ARDUINO_WIRE
// #define I2CDEV_IMPLEMENTATION I2CDEV_BUILTIN_SBWIRE

#include <Arduino.h>
#include <ArduinoJson.h>

#include <lowpass.hpp>
#include <memory>
#include <tempsensor.hpp>

enum GyroType {
  GYRO_NONE = 0,
  GYRO_MPU6050 = 1,
  GYRO_ICM42670P = 2,
};

#if defined(ESP32) && defined(ENABLE_RTCMEM)

#include <esp_attr.h>

#define GYRO_RTC_DATA_AVAILABLE \
  static_cast<uint8_t>(105)  // Unique number to flag resume data is available

// Used for the data stored in RTC memory
struct RtcGyroData {
  uint8_t Address;
  uint8_t IsDataAvailable;
};

extern RTC_DATA_ATTR RtcGyroData myRtcGyroData;
extern RTC_DATA_ATTR FilterData myRtcFilterData;

#endif  // ESP32

// Used for holding sensordata or sensoroffsets
struct RawGyroData {
  int16_t ax;  // Raw Acceleration
  int16_t ay;
  int16_t az;

  int16_t gx;  // Raw Position
  int16_t gy;
  int16_t gz;

  int16_t temp;  // Only for information (temperature of chip)
};

// Used for average multiple readings
struct RawGyroDataL {
  int32_t ax;  // Raw Acceleration
  int32_t ay;
  int32_t az;

  int32_t gx;  // Raw Position
  int32_t gy;
  int32_t gz;

  int32_t temp;  // Only for information (temperature of chip)
};

// Return data from gyro implementation
struct GyroResultData {
  bool valid;
  float angle;
  float temp;
};

// Represents the mode of the gyro
enum GyroMode {
  GYRO_UNCONFIGURED,  // gyro is not configured
  GYRO_RUN,           // run in normal (sleep) mode, fifo if configured
  GYRO_CONTINUOUS,    // run in continuous mode, no fifo
  GYRO_SLEEP          // gyro is sleeping and not enabled
};

// Inteface towards config class for gyro related settings
class GyroConfigInterface {
 public:
  virtual bool saveFile() = 0;

  // Common methods for all gyros
  virtual bool isGyroFilter() const = 0;
  virtual bool isGyroSwapXY() const = 0;
  virtual int getGyroSensorMovingThreashold() const = 0;
  virtual GyroType getGyroType() const = 0;

  // Methods for ICM42670p
  virtual int getSleepInterval() const = 0;

  // Methods for MPU6050/MPU6500
  virtual const RawGyroData& getGyroCalibration() const = 0;
  virtual void setGyroCalibration(const RawGyroData& r) = 0;
  virtual bool hasGyroCalibration() const = 0;
  virtual int getGyroReadCount() const = 0;
  virtual int getGyroReadDelay() const = 0;
};

class GyroSensorInterface {
 protected:
  GyroConfigInterface* _gyroConfig;
  bool _sensorMoving = false;

  virtual void debug() = 0;

  bool isSensorMoving(int16_t gx, int16_t gy, int16_t gz);
  float calculateAngle(float ax, float ay, float az);

 public:
  explicit GyroSensorInterface(GyroConfigInterface* gyroConfig) {
    _gyroConfig = gyroConfig;
  }
  /// @brief set up the gyro, or reconfigure if set up before
  /// @param mode the GyroMode the gyro needs to run in
  /// @param force force the update even if its already configured
  /// @return true if the setup is successful, otherwise the sensor is not found
  /// or an issue occured
  virtual bool setup(GyroMode mode, bool force);
  virtual GyroResultData readSensor(GyroMode mode);
  virtual bool calibrateSensor();
  virtual const char* getGyroFamily();
  virtual void getGyroTestResult(JsonObject& doc);
  virtual uint8_t getGyroID();
  virtual GyroMode enterSleep(GyroMode mode);
  virtual bool needCalibration();

  bool isSensorMoving() { return _sensorMoving; }
};

class GyroSensor : public SecondayTempSensorInterface {
 private:
  GyroConfigInterface* _gyroConfig;
  std::unique_ptr<GyroSensorInterface> _impl;
#if defined(ESP32)
  std::unique_ptr<FilterBase> _filter;
#endif

  RawGyroData _lastGyroData = {0};
  float _angle = 0;
  float _filteredAngle = 0;
  float _temp = 0;
  float _initialSensorTemp = NAN;
  bool _valid = false;
  GyroMode _currentMode = GyroMode::GYRO_UNCONFIGURED;

  void debug();
  void applyCalibration();
  void dumpCalibration();

 public:
  explicit GyroSensor(GyroConfigInterface* gyroConfig) {
    _gyroConfig = gyroConfig;
#if defined(ESP32) && defined(ENABLE_RTCMEM)
    _filter.reset(new TrimmedMovingAverageFilter(&myRtcFilterData));
    // _filter.reset(new MovingAverageFilter(&myRtcFilterData));
#endif
  }

  GyroType detectGyro();

  bool setup(GyroMode mode, bool force);
  bool read();
  bool calibrateSensor() {
    if (_impl) return _impl->calibrateSensor();
    return false;
  }
  const char* getGyroFamily() { return _impl ? _impl->getGyroFamily() : ""; }
  void getGyroTestResult(JsonObject& doc) {
    if (_impl) _impl->getGyroTestResult(doc);
  }
  uint8_t getGyroID() { return _impl ? _impl->getGyroID() : 0; }
  bool isSensorMoving() { return _impl ? _impl->isSensorMoving() : 0; }

  const RawGyroData& getLastGyroData() const { return _lastGyroData; }
  float getAngle() const { return _angle; }
  float getFilteredAngle() const { return _filteredAngle; }
  float getSensorTempC() const { return _temp; }
  float getInitialSensorTempC() const { return _initialSensorTemp; }
  bool isConnected() const {
    return _currentMode != GyroMode::GYRO_UNCONFIGURED;
  }
  GyroMode getCurrentGyroMode() { return _currentMode; }
  bool hasValue() const { return _valid; }
  bool needCalibration() { return _impl ? _impl->needCalibration() : false; }
  void enterSleep();
};

extern GyroSensor myGyro;

#endif  // GRAVITYMON

#endif  // SRC_GYRO_HPP_

// EOF
