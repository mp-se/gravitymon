/*
  Copyright (c) 2021-2026 Magnus
  https://github.com/mp-se/gravitymon

  Licensed under the GNU General Public License v3.0 (GPL v3) for open source use,
  or a Commercial License for proprietary use. See LICENSE and LICENSE_COMMERCIAL.
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
  uint8_t _addr;

  void debug();
  void applyCalibration();

 public:
  static bool isDeviceDetected(uint8_t& addr);

  explicit MPU6050Gyro(uint8_t addr, GyroConfigInterface* gyroConfig)
      : GyroSensorInterface(gyroConfig) {
    _accelgyro = MPU6050(addr);
    _addr = addr;
  }
  bool setup(GyroMode mode, bool force);
  bool calibrateSensor();
  GyroMode enterSleep(GyroMode mode);
  GyroResultData readSensor(GyroMode mode);
  const char* getGyroFamily();
  void getGyroTestResult(JsonObject& doc);
  uint8_t getGyroID() { return _accelgyro.getDeviceID(); }
  bool needCalibration() { return true; }
};

#endif  // GRAVITYMON

#endif  // SRC_MPU6050_GYRO_HPP_

// EOF
