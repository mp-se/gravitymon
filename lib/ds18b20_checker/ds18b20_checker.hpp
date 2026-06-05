/*
 * Copyright Chris Petrich, 2024
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *   Source:  https://github.com/cpetrich/counterfeit_DS18B20/
 *   Ported to C++ class for GravityMon by Magnus, 2026
 *
 * Tests DS18B20 sensors for differences from genuine Dallas/Maxim/Analog parts.
 * Tests 0–2 use only documented commands and are safe.
 * Test 3 (classify) sends undocumented commands — only use on sensors dedicated to testing.
 */

#ifndef LIB_DS18B20_CHECKER_HPP_
#define LIB_DS18B20_CHECKER_HPP_

#include <Arduino.h>
#include <OneWire.h>

enum class DS18B20Type {
  GENUINE,          // Passed all safe tests — responds like an authentic DS18B20
  CLONE_FAMILY_A1,  // Genuine die, older ROM pattern
  CLONE_FAMILY_A2,
  CLONE_FAMILY_A3,
  CLONE_FAMILY_B1,
  CLONE_FAMILY_B2,
  CLONE_FAMILY_C,
  CLONE_FAMILY_D1,
  CLONE_FAMILY_D2,
  CLONE_FAMILY_E,
  CLONE_FAMILY_F,
  CLONE_FAMILY_G,
  CLONE_FAMILY_H,
  CLONE_FAMILY_UNKNOWN,  // Failed safe test but family not identified
  UNKNOWN                // Could not determine (CRC error / connection issue)
};

const char* ds18b20TypeName(DS18B20Type t);

class DS18B20Checker {
 public:
  explicit DS18B20Checker(OneWire* ow) : _ds(ow) {}

  // Safe test using only documented commands. Returns number of failed checks.
  // fake_flags > 0 means the sensor deviates from the genuine DS18B20 spec.
  int discover(uint8_t* addr);

  // Aggressive classification using undocumented commands.
  // May affect calibration of clone sensors — use only on dedicated test sensors.
  DS18B20Type classify(uint8_t* addr);

 private:
  OneWire* _ds;

  bool readScratchpad(uint8_t* addr, uint8_t* buff9);
  int16_t timeConversion(uint8_t* addr);
  void sendReset(uint8_t* addr);
  uint8_t oneByteReturn(uint8_t* addr, uint8_t cmd);
  void nByteReturn(uint8_t* addr, uint8_t cmd, int count, uint8_t* buffer);
  uint8_t bitInvert(uint8_t a);
  void param2trim(uint16_t offset11, uint8_t curve5, uint8_t* t1, uint8_t* t2);
  void trim2param(uint8_t t1, uint8_t t2, uint16_t* offset11, uint8_t* curve5);
  void getTrimA(uint8_t* addr, uint8_t* t1, uint8_t* t2);
  void setTrimA(uint8_t* addr, uint8_t t1, uint8_t t2);
  void getTrimParamsA(uint8_t* addr, uint16_t* offset11, uint8_t* curve5);
  void setTrimParamsA(uint8_t* addr, uint16_t offset11, uint8_t curve5);
  bool isValidAScratchpad(uint8_t* buff);
  bool isAll00(uint8_t* buff, int n);
  void triggerConvert(uint8_t* addr, uint8_t conf, uint16_t wait_ms);
  int offsetParamRange(uint8_t* addr);
  int curveParamProp(uint8_t* addr);
};

#endif  // LIB_DS18B20_CHECKER_HPP_
