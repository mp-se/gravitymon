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
#ifndef SRC_MPU6050_GYRO_HPP_
#define SRC_MPU6050_GYRO_HPP_

#if defined(GRAVITYMON)

#include <MPU6050.h>

#include <gyro.hpp>

class MPU6050Gyro : public GyroSensorInterface {
 private:
  MPU6050 _accelgyro;
  RawGyroData raw;
  RawGyroData _calibrationOffset;

  void debug();
  void applyCalibration();

 public:
  static bool isDeviceDetected(uint8_t& addr);

  explicit MPU6050Gyro(uint8_t addr, GyroConfigInterface* gyroConfig)
      : GyroSensorInterface(gyroConfig) {
    _accelgyro = MPU6050(addr);
  }
  bool setup(GyroMode mode, bool force);
  void calibrateSensor();
  GyroMode enterSleep(GyroMode mode);
  GyroResultData readSensor(GyroMode mode);
  const char* getGyroFamily();
  uint8_t getGyroID() { return _accelgyro.getDeviceID(); }
  bool needCalibration() { return true; }
};

#endif  // GRAVITYMON

#endif  // SRC_MPU6050_GYRO_HPP_

// EOF
