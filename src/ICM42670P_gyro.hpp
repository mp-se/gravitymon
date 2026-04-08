/*
  Copyright (c) 2021-2026 Magnus
  https://github.com/mp-se/gravitymon

  Licensed under the GNU General Public License v3.0 (GPL v3) for open source use,
  or a Commercial License for proprietary use. See LICENSE and LICENSE_COMMERCIAL.
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
