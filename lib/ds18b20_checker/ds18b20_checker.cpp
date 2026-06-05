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
 */

#include <ds18b20_checker.hpp>

static const uint16_t kConvTimeout_ms = 3000;
static const uint16_t kConv750_ms = 750;

// ---------------------------------------------------------------------------
// Public
// ---------------------------------------------------------------------------

const char* ds18b20TypeName(DS18B20Type t) {
  switch (t) {
    case DS18B20Type::GENUINE:              return "Genuine";
    case DS18B20Type::CLONE_FAMILY_A1:      return "Clone-A1";
    case DS18B20Type::CLONE_FAMILY_A2:      return "Clone-A2";
    case DS18B20Type::CLONE_FAMILY_A3:      return "Clone-A3";
    case DS18B20Type::CLONE_FAMILY_B1:      return "Clone-B1";
    case DS18B20Type::CLONE_FAMILY_B2:      return "Clone-B2";
    case DS18B20Type::CLONE_FAMILY_C:       return "Clone-C";
    case DS18B20Type::CLONE_FAMILY_D1:      return "Clone-D1";
    case DS18B20Type::CLONE_FAMILY_D2:      return "Clone-D2";
    case DS18B20Type::CLONE_FAMILY_E:       return "Clone-E";
    case DS18B20Type::CLONE_FAMILY_F:       return "Clone-F";
    case DS18B20Type::CLONE_FAMILY_G:       return "Clone-G";
    case DS18B20Type::CLONE_FAMILY_H:       return "Clone-H";
    case DS18B20Type::CLONE_FAMILY_UNKNOWN: return "Clone-Unknown";
    default:                                return "Unknown";
  }
}

// Safe test using only documented commands.
// Returns number of deviations from genuine DS18B20 behaviour (0 = likely genuine).
int DS18B20Checker::discover(uint8_t* addr) {
  uint8_t buffer0[9], buffer1[9], buffer2[9], buffer3[9];
  int fake_flags = 0;

  if (0 != OneWire::crc8(addr, 8))
    fake_flags++;

  if ((addr[6] != 0) || (addr[5] != 0) || (addr[0] != 0x28)) {
    fake_flags++;
  } else if ((addr[4] == 0) && (addr[6] == 0) && (addr[5] == 0) && (addr[0] == 0x28)) {
    fake_flags++;
  }

  if (!readScratchpad(addr, buffer0)) readScratchpad(addr, buffer0);
  if (0 != OneWire::crc8(buffer0, 9))
    return fake_flags + 1;  // bad connection — bail out

  if (buffer0[5] != 0xff) fake_flags++;

  bool byte6_ok =
      !((buffer0[6] == 0x00) || (buffer0[6] > 0x10)) &&
      (((buffer0[0] == 0x50) && (buffer0[1] == 0x05)) ||
       ((buffer0[0] == 0xff) && (buffer0[1] == 0x07)) ||
       (buffer0[6] == (0x10 - (buffer0[0] & 0x0f))));
  if (!byte6_ok) fake_flags++;

  if (buffer0[7] != 0x10) fake_flags++;

  // Write 5 bytes into scratchpad and check reserved bytes are preserved.
  _ds->reset();
  _ds->select(addr);
  _ds->write(0x4E);
  _ds->write(buffer0[2]);
  _ds->write(buffer0[3]);
  _ds->write(0x7F);
  _ds->write(0x11);
  _ds->write(0x22);
  _ds->reset();
  if (!readScratchpad(addr, buffer1)) readScratchpad(addr, buffer1);
  if ((buffer1[5] != 0xFF) || (buffer1[6] != buffer0[6]) || (buffer1[7] != 0x10))
    fake_flags++;

  // Check config register clamps to valid values.
  _ds->reset(); _ds->select(addr); _ds->write(0x4E);
  _ds->write(buffer0[2]); _ds->write(buffer0[3]); _ds->write(0x00);
  _ds->reset();
  if (!readScratchpad(addr, buffer1)) readScratchpad(addr, buffer1);
  if (buffer1[4] != 0x1F) fake_flags++;

  _ds->reset(); _ds->select(addr); _ds->write(0x4E);
  _ds->write(buffer0[2]); _ds->write(buffer0[3]); _ds->write(0xFF);
  _ds->reset();
  if (!readScratchpad(addr, buffer1)) readScratchpad(addr, buffer1);
  if (buffer1[4] != 0x7F) fake_flags++;

  // Set 10-bit resolution, modify alarm registers, check preserved bytes.
  _ds->reset(); _ds->select(addr); _ds->write(0x4E);
  _ds->write(buffer0[2] ^ 0xff); _ds->write(buffer0[3] ^ 0xff); _ds->write(0x3F);
  _ds->reset();
  if (!readScratchpad(addr, buffer1)) readScratchpad(addr, buffer1);
  if ((buffer1[2] != (buffer0[2] ^ 0xff)) || (buffer1[3] != (buffer0[3] ^ 0xff)))
    fake_flags++;
  if (buffer1[4] != 0x3f) fake_flags++;
  if ((buffer1[5] != buffer0[5]) || (buffer1[6] != buffer0[6]) || (buffer1[7] != buffer0[7]))
    fake_flags++;

  // Set 12-bit resolution, check preserved bytes.
  _ds->reset(); _ds->select(addr); _ds->write(0x4E);
  _ds->write(buffer0[2]); _ds->write(buffer0[3]); _ds->write(0x7f);
  _ds->reset();
  if (!readScratchpad(addr, buffer2)) readScratchpad(addr, buffer2);
  if (buffer2[4] != 0x7f) fake_flags++;
  if ((buffer2[5] != buffer1[5]) || (buffer2[6] != buffer1[6]) || (buffer2[7] != buffer1[7]))
    fake_flags++;

  // Check byte 6 consistency across several temperature conversions.
  {
    int count = 5;
    do {
      count--;
      if (count < 0) break;
      _ds->reset(); _ds->select(addr); _ds->write(0x44, 1);
      delay(kConv750_ms);
      if (!readScratchpad(addr, buffer3)) readScratchpad(addr, buffer3);
    } while (((buffer3[0] == 0x50) && (buffer3[1] == 0x05)) ||
             ((buffer3[0] == 0xff) && (buffer3[1] == 0x07)) ||
             ((buffer3[6] == 0x0c) && (((buffer3[0] + buffer3[6]) & 0x0f) == 0x00)));
    if (count >= 0) {
      bool ok = (buffer3[6] != 0x0c) &&
                (buffer3[6] == (0x10 - (buffer3[0] & 0x0F)));
      if (!ok) fake_flags++;
    }
  }

  // Check conversion-in-progress polling.
  bool reportsProgress = true;
  {
    _ds->reset(); _ds->select(addr); _ds->write(0x44);
    delay(3);
    uint8_t afterShort = _ds->read_bit();
    delay(kConv750_ms - 3);
    uint8_t after750 = _ds->read_bit();
    if ((afterShort != 0) || (after750 == 0)) {
      fake_flags++;
      reportsProgress = false;
    }
  }

  if (reportsProgress) {
    // Check polling is valid immediately after issuing convert command.
    const int n = 4;
    uint8_t okCount = 0;
    for (int i = 0; i < n; i++) {
      _ds->reset(); _ds->select(addr); _ds->write(0x44);
      if (_ds->read_bit() == 0) okCount++;
      delay(kConv750_ms);
    }
    if (okCount != n) fake_flags++;

    // Check conversion time scales correctly with resolution.
    uint8_t bitSetOk = 0;
    uint32_t convTime[4];
    for (int idx = 0; idx < 4; idx++) {
      _ds->reset(); _ds->select(addr); _ds->write(0x4E);
      _ds->write(buffer0[2]); _ds->write(buffer0[3]); _ds->write(idx * 32 + 0x1f);
      delay(1);
      if (!readScratchpad(addr, buffer2)) readScratchpad(addr, buffer2);
      if ((buffer2[4] / 32) == idx) bitSetOk++;
      _ds->reset(); _ds->select(addr); _ds->write(0x44);
      uint32_t start = millis();
      delay(1);
      while ((_ds->read_bit() == 0) && (millis() - start < kConvTimeout_ms));
      convTime[idx] = millis() - start;
    }
    if (bitSetOk != 4) {
      fake_flags++;
    } else {
      float r1 = (float)convTime[1] / max(static_cast<uint32_t>(1), convTime[0]);
      float r2 = (float)convTime[2] / max(static_cast<uint32_t>(1), convTime[1]);
      float r3 = (float)convTime[3] / max(static_cast<uint32_t>(1), convTime[2]);
      float rAvg = (r1 + r2 + r3) / 3.0f;
      if (!(fabs(rAvg - 2.0f) < 0.3f)) fake_flags++;
    }
    if (convTime[3] < 550 || convTime[3] > 660) fake_flags++;
  } else {
    fake_flags++;
  }

  return fake_flags;
}

// Classify the sensor family using undocumented commands.
DS18B20Type DS18B20Checker::classify(uint8_t* addr) {
  int identified = 0;
  DS18B20Type result = DS18B20Type::CLONE_FAMILY_UNKNOWN;

  // Family A
  {
    uint8_t r68 = oneByteReturn(addr, 0x68);
    if (r68 != 0xff) {
      int opr = offsetParamRange(addr);
      int cpp = curveParamProp(addr);
      if ((cpp == 1) && opr) {
        result = DS18B20Type::GENUINE;
        identified++;
      } else if (cpp == 2) {
        result = DS18B20Type::CLONE_FAMILY_A2;
        identified++;
      } else if ((cpp == 1) && !opr) {
        result = DS18B20Type::CLONE_FAMILY_A3;
        identified++;
      } else if (cpp == -1) {
        result = DS18B20Type::CLONE_FAMILY_A1;
        identified++;
      }
    }
    sendReset(addr);
  }

  // Family B
  if (!identified) {
    uint8_t r97 = oneByteReturn(addr, 0x97);
    if (r97 != 0xFF) {
      result = (r97 == 0x31) ? DS18B20Type::CLONE_FAMILY_B2 : DS18B20Type::CLONE_FAMILY_B1;
      identified++;
    }
  }

  // Family C — config register is constant regardless of what is written
  if (!identified) {
    uint8_t buff[9];
    uint8_t cfg1, cfg2;
    bool err = false;
    _ds->reset(); _ds->select(addr); _ds->write(0x4E);
    _ds->write(0xaa); _ds->write(0x55); _ds->write(0x00);
    if (!readScratchpad(addr, buff)) readScratchpad(addr, buff);
    if (isAll00(buff, 9) || (0 != OneWire::crc8(buff, 9))) { err = true; }
    else cfg1 = buff[4];
    if (!err) {
      _ds->reset(); _ds->select(addr); _ds->write(0x4E);
      _ds->write(0xaa); _ds->write(0x55); _ds->write(0xff);
      if (!readScratchpad(addr, buff)) readScratchpad(addr, buff);
      if (isAll00(buff, 9) || (0 != OneWire::crc8(buff, 9))) { err = true; }
      else cfg2 = buff[4];
    }
    _ds->reset(); _ds->select(addr); _ds->write(0x64); delay(10);
    if (!err && (cfg1 == cfg2)) {
      result = DS18B20Type::CLONE_FAMILY_C;
      identified++;
    }
  }

  // Family D
  if (!identified) {
    uint8_t r8B = oneByteReturn(addr, 0x8B);
    if (r8B != 0xFF) {
      result = (r8B == 0x00) ? DS18B20Type::CLONE_FAMILY_D2 : DS18B20Type::CLONE_FAMILY_D1;
      identified++;
    }
  }

  // Family E — 2-byte secondary scratchpad at 0xDE / 0x2E
  if (!identified) {
    uint8_t sp1, sp2, sp1b, sp2b;
    _ds->reset(); _ds->select(addr); _ds->write(0xDE);
    sp1 = _ds->read(); sp2 = _ds->read();
    _ds->reset(); _ds->select(addr); _ds->write(0x2E);
    _ds->write(~sp1); _ds->write(~sp2);
    _ds->reset(); _ds->select(addr); _ds->write(0xDE);
    sp1b = _ds->read(); sp2b = _ds->read();
    if ((sp1b == (uint8_t)~sp1) && (sp2b == (uint8_t)~sp2)) {
      result = DS18B20Type::CLONE_FAMILY_E;
      identified++;
    }
  }

  // Family F — returns 3 bytes on 0x19
  if (!identified) {
    uint8_t buffer[3];
    nByteReturn(addr, 0x19, 3, buffer);
    if ((buffer[0] != 0xFF) || (buffer[1] != 0xFF) || (buffer[2] != 0xFF)) {
      result = DS18B20Type::CLONE_FAMILY_F;
      identified++;
    }
  }

  // Family G — responds to 0x8E, has bug in byte 6
  if (!identified) {
    uint8_t r8E = oneByteReturn(addr, 0x8E);
    if (r8E != 0xFF) {
      uint8_t buffer[9];
      _ds->reset(); _ds->select(addr); _ds->write(0x44, 1);
      delay(kConv750_ms + 50);
      if (!readScratchpad(addr, buffer)) readScratchpad(addr, buffer);
      if (buffer[6] == 0x20 - (buffer[0] & 0x0f)) {
        result = DS18B20Type::CLONE_FAMILY_G;
        identified++;
      }
    }
  }

  // Family H — conversion time scales by sqrt(2) rather than 2
  if (!identified) {
    uint8_t buffer0[9], buffer1[9];
    if (!readScratchpad(addr, buffer0)) readScratchpad(addr, buffer0);
    bool done = false;
    for (int run = 0; run < 1 && !done; run++) {
      _ds->reset(); _ds->select(addr); _ds->write(0x4E);
      _ds->write(buffer0[2]); _ds->write(buffer0[3]); _ds->write(0x1f);
      delay(1);
      if (!readScratchpad(addr, buffer1)) readScratchpad(addr, buffer1);
      if ((buffer1[4] / 32) != 0) break;
      float t9 = max(1.0f, (float)timeConversion(addr));
      _ds->reset(); _ds->select(addr); _ds->write(0x4E);
      _ds->write(buffer0[2]); _ds->write(buffer0[3]); _ds->write(0x3f);
      delay(1);
      if (!readScratchpad(addr, buffer1)) readScratchpad(addr, buffer1);
      if ((buffer1[4] / 32) != 1) break;
      float t10 = max(1.0f, (float)timeConversion(addr));
      if (fabs(t10 / t9 - 1.41f) <= 0.2f) {
        result = DS18B20Type::CLONE_FAMILY_H;
        identified++;
      }
      done = true;
    }
    // restore 12-bit
    _ds->reset(); _ds->select(addr); _ds->write(0x4E);
    _ds->write(buffer0[2]); _ds->write(buffer0[3]); _ds->write(0x7f);
  }

  if (!identified)
    result = DS18B20Type::CLONE_FAMILY_UNKNOWN;

  return result;
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

bool DS18B20Checker::readScratchpad(uint8_t* addr, uint8_t* buff9) {
  _ds->reset();
  _ds->select(addr);
  _ds->write(0xBE);
  for (int i = 0; i < 9; i++) buff9[i] = _ds->read();
  return 0 == OneWire::crc8(buff9, 9);
}

int16_t DS18B20Checker::timeConversion(uint8_t* addr) {
  _ds->reset(); _ds->select(addr); _ds->write(0x44);
  uint32_t start = millis();
  delay(1);
  while ((_ds->read_bit() == 0) && (millis() - start < kConvTimeout_ms));
  uint32_t elapsed = millis() - start;
  if (elapsed >= kConvTimeout_ms) return -1;
  return (int16_t)elapsed;
}

void DS18B20Checker::sendReset(uint8_t* addr) {
  _ds->reset(); _ds->select(addr); _ds->write(0x64);
  delay(10);
}

uint8_t DS18B20Checker::oneByteReturn(uint8_t* addr, uint8_t cmd) {
  _ds->reset(); _ds->select(addr); _ds->write(cmd);
  return _ds->read();
}

void DS18B20Checker::nByteReturn(uint8_t* addr, uint8_t cmd, int count, uint8_t* buffer) {
  _ds->reset(); _ds->select(addr); _ds->write(cmd);
  for (int i = 0; i < count; i++) buffer[i] = _ds->read();
}

uint8_t DS18B20Checker::bitInvert(uint8_t a) {
  uint8_t b = 0;
  for (int i = 0; i < 8; i++) { b *= 2; b += (a & 0x01); a /= 2; }
  return b;
}

void DS18B20Checker::param2trim(uint16_t offset11, uint8_t curve5, uint8_t* t1, uint8_t* t2) {
  *t1 = bitInvert(offset11 & 0x0ff);
  *t2 = curve5 * 8 + offset11 / 256;
}

void DS18B20Checker::trim2param(uint8_t t1, uint8_t t2, uint16_t* offset11, uint8_t* curve5) {
  *offset11 = bitInvert(t1) + ((uint16_t)(t2 & 0x07)) * 256;
  *curve5 = t2 / 8;
}

void DS18B20Checker::getTrimA(uint8_t* addr, uint8_t* t1, uint8_t* t2) {
  _ds->reset();
  if (addr) _ds->select(addr); else _ds->write(0xCC);
  _ds->write(0x93); *t1 = _ds->read();
  _ds->reset();
  if (addr) _ds->select(addr); else _ds->write(0xCC);
  _ds->write(0x68); *t2 = _ds->read();
}

void DS18B20Checker::setTrimA(uint8_t* addr, uint8_t t1, uint8_t t2) {
  _ds->reset(); _ds->select(addr); _ds->write(0x95); _ds->write(t1);
  _ds->reset(); _ds->select(addr); _ds->write(0x63); _ds->write(t2);
}

void DS18B20Checker::getTrimParamsA(uint8_t* addr, uint16_t* offset11, uint8_t* curve5) {
  uint8_t t1, t2;
  getTrimA(addr, &t1, &t2);
  trim2param(t1, t2, offset11, curve5);
}

void DS18B20Checker::setTrimParamsA(uint8_t* addr, uint16_t offset11, uint8_t curve5) {
  uint8_t t1, t2;
  param2trim(offset11, curve5, &t1, &t2);
  setTrimA(addr, t1, t2);
}

bool DS18B20Checker::isValidAScratchpad(uint8_t* buff) {
  if ((buff[4] != 0x7f) && (buff[4] != 0x5f) && (buff[4] != 0x3f) && (buff[4] != 0x1f)) return false;
  if ((buff[0] == 0x50) && (buff[1] == 0x05) && (buff[6] == 0x0C)) return true;
  if ((buff[0] == 0xff) && (buff[1] == 0x07) && (buff[6] == 0x0C)) return true;
  return buff[6] == (0x10 - (buff[0] & 0x0f));
}

bool DS18B20Checker::isAll00(uint8_t* buff, int n) {
  for (int i = 0; i < n; i++) if (buff[i] != 0x00) return false;
  return true;
}

void DS18B20Checker::triggerConvert(uint8_t* addr, uint8_t conf, uint16_t wait_ms) {
  _ds->reset(); _ds->select(addr); _ds->write(0x4E);
  _ds->write(0xaf); _ds->write(0xfe); _ds->write(conf);
  _ds->reset(); _ds->select(addr); _ds->write(0x44, 1);
  delay(wait_ms);
  _ds->depower();
}

int DS18B20Checker::offsetParamRange(uint8_t* addr) {
  uint16_t offset0, offset1;
  uint8_t curve0, curve1;
  uint8_t buff[9];
  getTrimParamsA(addr, &offset0, &curve0);
  const uint16_t test0 = 0x300 + ((offset0 ^ 0x18) & 0xFF);
  const uint16_t test1 = test0 + 0x100;
  setTrimParamsA(addr, test0, curve0);
  getTrimParamsA(addr, &offset1, &curve1);
  if (offset1 != test0) return 0;
  triggerConvert(addr, 0x7f, kConv750_ms + 50);
  if (!readScratchpad(addr, buff)) readScratchpad(addr, buff);
  int16_t t0 = buff[0] + 256 * buff[1];
  setTrimParamsA(addr, test1, curve0);
  triggerConvert(addr, 0x7f, kConv750_ms + 50);
  if (!readScratchpad(addr, buff)) readScratchpad(addr, buff);
  setTrimParamsA(addr, offset0, curve0);
  int16_t t1 = buff[0] + 256 * buff[1];
  return (t1 - t0 > 128) ? 1 : 0;
}

int DS18B20Checker::curveParamProp(uint8_t* addr) {
  uint8_t buff[9];
  const uint16_t off = 0x3fe;
  setTrimParamsA(addr, off, 0x0f);
  triggerConvert(addr, 0x7f, kConv750_ms + 50);
  if (!readScratchpad(addr, buff)) readScratchpad(addr, buff);
  if (0 != OneWire::crc8(buff, 9)) return -3;
  if (!isValidAScratchpad(buff)) return -2;
  int16_t r0f = buff[0] + 256 * buff[1];
  { uint16_t o; uint8_t c; getTrimParamsA(addr, &o, &c); if ((o != off) || (c != 0x0f)) return 0; }
  setTrimParamsA(addr, off, 0x00);
  triggerConvert(addr, 0x7f, kConv750_ms + 50);
  if (!readScratchpad(addr, buff)) readScratchpad(addr, buff);
  if (0 != OneWire::crc8(buff, 9)) return -3;
  if (!isValidAScratchpad(buff)) return -2;
  int16_t r00 = buff[0] + 256 * buff[1];
  setTrimParamsA(addr, off, 0x1f);
  triggerConvert(addr, 0x7f, kConv750_ms + 50);
  if (!readScratchpad(addr, buff)) readScratchpad(addr, buff);
  if (0 != OneWire::crc8(buff, 9)) return -3;
  if (!isValidAScratchpad(buff)) return -2;
  int16_t r1f = buff[0] + 256 * buff[1];
  setTrimParamsA(addr, off, 0x10);
  triggerConvert(addr, 0x7f, kConv750_ms + 50);
  if (!readScratchpad(addr, buff)) readScratchpad(addr, buff);
  if (0 != OneWire::crc8(buff, 9)) return -3;
  if (!isValidAScratchpad(buff)) return -2;
  int16_t r10 = buff[0] + 256 * buff[1];
  int16_t mini = min(r00, min(r0f, min(r10, r1f)));
  int16_t maxi = max(r00, max(r0f, max(r10, r1f)));
  bool is_signed   = (r0f - r10 > r1f - r00);
  bool is_unsigned = (r0f - r10 < r1f - r00);
  if (is_signed   && (maxi - mini > 20 * 16)) return 2;
  if (is_unsigned && (maxi - mini > 1 * 16) && (maxi - mini < 6 * 16)) return 1;
  return -1;
}
