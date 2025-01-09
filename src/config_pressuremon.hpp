/*
MIT License

Copyright (c) 2022-2025 Magnus

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
#ifndef SRC_CONFIG_PRESSUREMON_HPP_
#define SRC_CONFIG_PRESSUREMON_HPP_

#if defined(PRESSUREMON)

#include <config_brewing.hpp>

enum PressuremonBleFormat { BLE_DISABLED = 0 };

class PressuremonConfig : public BrewingConfig {
 private:
  PressuremonBleFormat _pressuremonBleFormat =
      PressuremonBleFormat::BLE_DISABLED;
  bool _batterySaving = true;

  char _pressureFormat = 'B';

 public:
  PressuremonConfig(String baseMDNS, String fileName);

  bool isPressureBar() { return _pressureFormat == 'B'; }
  bool isPressureHpa() { return _pressureFormat == 'H'; }
  bool isPressurePsi() { return _pressureFormat == 'P'; }

  bool isBleActive() {
    return (_pressuremonBleFormat != PressuremonBleFormat::BLE_DISABLED);
  }

  const PressuremonBleFormat getGravitymonBleFormat() {
    return _pressuremonBleFormat;
  }
  void setPressuremonBleFormat(int b) {
    _pressuremonBleFormat = (PressuremonBleFormat)b;
    _saveNeeded = true;
  }
  void setPressuremonBleFormat(PressuremonBleFormat b) {
    _pressuremonBleFormat = b;
    _saveNeeded = true;
  }

  const bool isBatterySaving() { return _batterySaving; }
  void setBatterySaving(bool b) {
    _batterySaving = b;
    _saveNeeded = true;
  }

  void createJson(JsonObject& doc);
  void parseJson(JsonObject& doc);
};

extern PressuremonConfig myConfig;

#endif  // GRAVITYMON

#endif  // SRC_CONFIG_PRESSUREMON_HPP_

// EOF
