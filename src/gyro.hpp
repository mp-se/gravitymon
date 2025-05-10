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
#ifndef SRC_GYRO_HPP_
#define SRC_GYRO_HPP_

#if defined(GRAVITYMON)

#define I2CDEV_IMPLEMENTATION I2CDEV_ARDUINO_WIRE
// #define I2CDEV_IMPLEMENTATION I2CDEV_BUILTIN_SBWIRE

#include <Arduino.h>
#include <lowpass.hpp>
#include <tempsensor.hpp>
#include <memory>

#if defined(ESP32)

#include <esp_attr.h>

#define GYRO_RTC_DATA_AVAILABLE \
  static_cast<uint8_t>(105)  // Unique number to flag resume data is available

// Represents the type of the gyro
enum GyroType {
  GYRO_NONE,
  GYRO_MPU6050,
  GYRO_ICM42670P,
};

// Used for the data stored in RTC memory
struct RtcGyroData {
  GyroType Type;
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
  virtual void calibrateSensor();
  virtual const char* getGyroFamily();
  virtual uint8_t getGyroID();
  virtual GyroMode enterSleep(GyroMode mode);
  virtual bool needCalibration();

  bool isSensorMoving() { return _sensorMoving; }
};

#define INVALID_TEMPERATURE -273

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
  float _initialSensorTemp = INVALID_TEMPERATURE;
  bool _valid = false;
  GyroMode _currentMode = GyroMode::GYRO_UNCONFIGURED;

  void debug();
  void applyCalibration();
  void dumpCalibration();

 public:
  explicit GyroSensor(GyroConfigInterface* gyroConfig) {
    _gyroConfig = gyroConfig;
#if defined(ESP32)
    _filter.reset(new TrimmedMovingAverageFilter(&myRtcFilterData));
    // _filter.reset(new MovingAverageFilter(&myRtcFilterData));
#endif
  }

  bool setup(GyroMode mode, bool force);
  bool read();
  void calibrateSensor() {
    if (_impl) _impl->calibrateSensor();
  }
  virtual const char* getGyroFamily() {
    return _impl ? _impl->getGyroFamily() : "";
  }
  uint8_t getGyroID() { return _impl ? _impl->getGyroID() : 0; }
  bool isSensorMoving() { return _impl ? _impl->isSensorMoving() : 0; }

  const RawGyroData& getLastGyroData() const { return _lastGyroData; }
  float getAngle() const { return _angle; }
  float getFilteredAngle() const { return _filteredAngle; }
  float getSensorTempC() const { return _temp; }
  float getInitialSensorTempC() const { return _initialSensorTemp; }
  bool isConnected() const { return _currentMode != GyroMode::GYRO_UNCONFIGURED; }
  GyroMode getCurrentGyroMode() { return _currentMode; }
  bool hasValue() const { return _valid; }
  bool needCalibration() { return _impl ? _impl->needCalibration() : false; }
  void enterSleep();
};

extern GyroSensor myGyro;

#endif  // GRAVITYMON

#endif  // SRC_GYRO_HPP_

// EOF
