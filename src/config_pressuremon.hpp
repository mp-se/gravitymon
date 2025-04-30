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
#include <pressure.hpp>

enum PressuremonBleFormat {
  BLE_DISABLED = 0,
  BLE_PRESSUREMON_IBEACON = 1,
  // BLE_PRESSUREMON_EDDYSTONE = 2
};

constexpr auto MAX_SENSOR_DEVICES = 2;

constexpr auto PRESSURE_KPA = "kPa";
constexpr auto PRESSURE_BAR = "Bar";
constexpr auto PRESSURE_PSI = "PSI";

constexpr auto CONFIG_SENSOR_TYPE = "sensor_type";
constexpr auto CONFIG_SENSOR1_TYPE = "sensor1_type";
constexpr auto CONFIG_PRESSURE_ADJUSTMENT = "pressure_adjustment";
constexpr auto CONFIG_PRESSURE1_ADJUSTMENT = "pressure1_adjustment";

constexpr auto CONFIG_PRESSURE_UNIT = "pressure_unit";

class PressuremonConfig : public BrewingConfig, public PressureConfigInterface {
 private:
  PressuremonBleFormat _pressuremonBleFormat =
      PressuremonBleFormat::BLE_DISABLED;
  PressureSensorType _pressureSensor[MAX_SENSOR_DEVICES] = {
      PressureSensorType::SensorNone, PressureSensorType::SensorNone};

  float _pressureSensorCorrection[MAX_SENSOR_DEVICES] = {0, 0};

  bool _batterySaving = true;

  String _pressureUnit = PRESSURE_PSI;

 public:
  PressuremonConfig(String baseMDNS, String fileName);

  bool isPressureBar() const { return _pressureUnit == PRESSURE_BAR; }
  bool isPressureKpa() const { return _pressureUnit == PRESSURE_KPA; }
  bool isPressurePsi() const { return _pressureUnit == PRESSURE_PSI; }

  const char* getPressureUnit() const { return _pressureUnit.c_str(); }
  void setPressureUnit(String s) {
    _pressureUnit = s;
    _saveNeeded = true;
  }

  PressureSensorType getPressureSensorType(int idx) const {
    return _pressureSensor[idx];
  }
  int getPressureSensorTypeAsInt(int idx) const {
    return static_cast<int>(_pressureSensor[idx]);
  }
  void setPressureSensorType(PressureSensorType t, int idx) {
    _pressureSensor[idx] = t;
    _saveNeeded = true;
  }
  void setPressureSensorType(int t, int idx) {
    setPressureSensorType((PressureSensorType)t, idx);
  }
  bool hasPressureSensorConfigured() const {
    return (_pressureSensor[0] != PressureSensorType::SensorNone ||
            _pressureSensor[1] != PressureSensorType::SensorNone);
  }

  float getPressureSensorCorrection(int idx) const {
    return _pressureSensorCorrection[idx];
  }
  void setPressureSensorCorrection(float v, int idx) {
    _pressureSensorCorrection[idx] = v;
    _saveNeeded = true;
  }

  float getTemperatureSensorCorrection(int idx) const { return 0; }

  bool isBleActive() const {
    return (_pressuremonBleFormat != PressuremonBleFormat::BLE_DISABLED);
  }

  const PressuremonBleFormat getPressuremonBleFormat() const {
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

  const bool isBatterySaving() const { return _batterySaving; }
  void setBatterySaving(bool b) {
    _batterySaving = b;
    _saveNeeded = true;
  }

  void createJson(JsonObject& doc) const;
  void parseJson(JsonObject& doc);

  // Wrappers for PressureConfig
  char getTempUnit() const { return BaseConfig::getTempUnit(); }
};

extern PressuremonConfig myConfig;

#endif  // GRAVITYMON

#endif  // SRC_CONFIG_PRESSUREMON_HPP_

// EOF
