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
