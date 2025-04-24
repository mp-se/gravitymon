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

#include <memory>

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

// Inteface towards config class for gyro related settings
class GyroConfigInterface {
 public:
  virtual bool saveFile() = 0;

  // Common methods for all gyros
  virtual bool isGyroSwapXY() const;
  virtual int getGyroSensorMovingThreashold() const;

  // Methods for ICM42670p
  virtual int getSleepInterval() const;

  // Methods for MPU6050/MPU6500
  virtual const RawGyroData& getGyroCalibration() const;
  virtual void setGyroCalibration(const RawGyroData& r);
  virtual bool hasGyroCalibration() const;
  virtual int getGyroReadCount() const;
  virtual int getGyroReadDelay() const;
};

class GyroSensorInterface {
 protected:
  GyroConfigInterface* _gyroConfig;
  bool _sensorConnected = false;
  bool _sensorMoving = false;

  virtual void debug() = 0;
  virtual void applyCalibration();

  bool isSensorMoving(int16_t gx, int16_t gy, int16_t gz);
  float calculateAngle(float ax, float ay, float az);

 public:
  explicit GyroSensorInterface(GyroConfigInterface* gyroConfig) {
    _gyroConfig = gyroConfig;
  }
  virtual bool setup();
  virtual GyroResultData readSensor();
  virtual void calibrateSensor();
  virtual const char* getGyroFamily();
  virtual uint8_t getGyroID();
  virtual void enterSleep();
  virtual bool needCalibration();

  bool isConnected() { return _sensorConnected; }
  bool isSensorMoving() { return _sensorMoving; }
};

#define INVALID_TEMPERATURE -273

class GyroSensor {
 private:
  GyroConfigInterface* _gyroConfig;
  std::unique_ptr<GyroSensorInterface> _impl;

  RawGyroData _lastGyroData;
  float _angle = 0;
  float _temp = 0;
  float _initialSensorTemp = INVALID_TEMPERATURE;
  bool _valid = false;

  void debug();
  void applyCalibration();
  void dumpCalibration();

 public:
  explicit GyroSensor(GyroConfigInterface* gyroConfig) {
    _gyroConfig = gyroConfig;
  }

  bool setup();
  bool read();
  void calibrateSensor() {
    if (_impl) _impl->calibrateSensor();
  }
  virtual const char* getGyroFamily() {
    return _impl ? _impl->getGyroFamily() : "";
  }
  uint8_t getGyroID() { return _impl ? _impl->getGyroID() : 0; }
  bool isSensorMoving() { return _impl ? _impl->isSensorMoving() : 0; }

  const RawGyroData& getLastGyroData() { return _lastGyroData; }
  float getAngle() { return _angle; }
  float getSensorTempC() { return _temp; }
  float getInitialSensorTempC() { return _initialSensorTemp; }
  bool isConnected() { return _impl ? _impl->isConnected() : false; }
  bool hasValue() { return _valid; }
  bool needCalibration() { return _impl ? _impl->needCalibration() : false; }
  void enterSleep();
};

extern GyroSensor myGyro;

#endif  // GRAVITYMON

#endif  // SRC_GYRO_HPP_

// EOF
