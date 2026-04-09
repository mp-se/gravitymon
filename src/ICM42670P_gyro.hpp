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


#ifndef SRC_ICM42670P_GYRO_HPP_
#define SRC_ICM42670P_GYRO_HPP_

#if defined(GRAVITYMON)

#include <gyro.hpp>

class ICM42670pGyro : public GyroSensorInterface {
 private:
  uint8_t _addr;
  uint8_t _buffer[16] = {0};
  uint32_t _configStart = 0;

  void debug();

  bool writeMBank1(uint8_t reg, uint8_t value);
  bool writeMBank1AndVerify(uint8_t reg, uint8_t value);
  bool readMBank1(uint8_t reg);
  uint8_t ReadFIFOPackets(const uint16_t &count, RawGyroDataL &data);

 public:
  static bool isDeviceDetected(uint8_t &addr);

  explicit ICM42670pGyro(uint8_t addr, GyroConfigInterface *gyroConfig)
      : GyroSensorInterface(gyroConfig) {
    _addr = addr;
  }
  bool setup(GyroMode mode, bool force);
  bool calibrateSensor();
  GyroMode enterSleep(GyroMode mode);
  GyroResultData readSensor(GyroMode mode);
  const char *getGyroFamily();
  void getGyroTestResult(JsonObject &doc) {}
  uint8_t getGyroID() { return 0x67; }
  bool needCalibration() { return false; }
};

#endif  // GRAVITYMON

#endif  // SRC_ICM42670P_GYRO_HPP_

// EOF
