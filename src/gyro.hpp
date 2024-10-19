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
#ifndef SRC_GYRO_HPP_
#define SRC_GYRO_HPP_

#include <config.hpp>

struct RawGyroDataL {  // Used for average multiple readings
  int32_t ax;          // Raw Acceleration
  int32_t ay;
  int32_t az;

  int32_t gx;  // Raw Position
  int32_t gy;
  int32_t gz;

  int32_t temp;  // Only for information (temperature of chip)
};

#define INVALID_TEMPERATURE -273

class GyroSensor {
private:
  RawGyroData _lastGyroData;

protected:
  bool _sensorConnected = false;
  bool _validValue = false;
  float _angle = 0;
  float _sensorTemp = 0;
  float _initialSensorTemp = INVALID_TEMPERATURE;
  RawGyroData _calibrationOffset;

  void dumpCalibration();
  bool isSensorMoving(RawGyroData &raw);
  float calculateAngle(RawGyroData &raw);

//virtual to be implemented by gyro
  virtual void debug() = 0;
  virtual void applyCalibration() = 0;
  virtual void readSensor(RawGyroData &raw, const int noIterations = 100,
                  const int delayTime = 1) = 0;

public:
  bool read();
  float getAngle() { return _angle; }
  float getSensorTempC() { return _sensorTemp; }
  float getInitialSensorTempC() { return _initialSensorTemp; }
  bool isConnected() { return _sensorConnected; }
  bool hasValue() { return _validValue; }

//virtual to be implemented by gyro
  virtual bool isOnline() = 0;
  virtual bool setup() = 0;
  virtual void calibrateSensor() = 0;
  virtual String getGyroFamily() = 0;
  virtual void enterSleep() = 0;
};

#include <MPU6050Gyro.hpp>

extern GyroSensor* myGyro;

#endif  // SRC_GYRO_HPP_

// EOF
