/*
MIT License

Copyright (c) 2025 Magnus, Levi--G

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
  void calibrateSensor();
  GyroMode enterSleep(GyroMode mode);
  GyroResultData readSensor(GyroMode mode);
  const char *getGyroFamily();
  void getGyroTestResult(JsonObject& doc) {}
  uint8_t getGyroID() { return 0x67; }
  bool needCalibration() { return false; }
};

#endif  // GRAVITYMON

#endif  // SRC_ICM42670P_GYRO_HPP_

// EOF
