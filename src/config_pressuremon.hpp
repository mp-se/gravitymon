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

enum PressureSensorType {
  SensorNone = 0,

  SensorXidibeiXDB401_I2C_KPa_200 = 1,    // 0-0.2 MPa
  SensorXidibeiXDB401_I2C_KPa_400 = 2,    // 0-0.4 MPa
  SensorXidibeiXDB401_I2C_KPa_500 = 3,    // 0-0.5 MPa
  SensorXidibeiXDB401_I2C_KPa_600 = 4,    // 0-0.6 MPa
  SensorXidibeiXDB401_I2C_KPa_800 = 5,    // 0-0.8 MPa
  SensorXidibeiXDB401_I2C_KPa_1000 = 6,   // 0-1 MPa
  SensorXidibeiXDB401_I2C_KPa_1200 = 7,   // 0-1.2 MPa
  SensorXidibeiXDB401_I2C_KPa_1500 = 8,   // 0-1.5 MPa
  SensorXidibeiXDB401_I2C_KPa_1600 = 9,   // 0-1.6 MPa
  SensorXidibeiXDB401_I2C_KPa_2000 = 10,  // 0-2 MPa
  SensorXidibeiXDB401_I2C_KPa_2500 = 11,  // 0-2.5 MPa
  SensorXidibeiXDB401_I2C_KPa_3000 = 12,  // 0-3 MPa
  SensorXidibeiXDB401_I2C_KPa_3500 = 13,  // 0-3.5 MPa
  SensorXidibeiXDB401_I2C_KPa_4000 = 14,  // 0-4 MPa

  SensorXidibeiXDB401_Analog_KPa_200 = 101,   // 0-0.2 MPa
  SensorXidibeiXDB401_Analog_KPa_400 = 102,   // 0-0.4 MPa
  SensorXidibeiXDB401_Analog_KPa_500 = 103,   // 0-0.5 MPa
  SensorXidibeiXDB401_Analog_KPa_600 = 104,   // 0-0.6 MPa
  SensorXidibeiXDB401_Analog_KPa_800 = 105,   // 0-0.8 MPa
  SensorXidibeiXDB401_Analog_KPa_1000 = 106,  // 0-1 MPa
  SensorXidibeiXDB401_Analog_KPa_1200 = 107,  // 0-1.2 MPa
  SensorXidibeiXDB401_Analog_KPa_1500 = 108,  // 0-1.5 MPa
  SensorXidibeiXDB401_Analog_KPa_1600 = 109,  // 0-1.6 MPa
  SensorXidibeiXDB401_Analog_KPa_2000 = 110,  // 0-2 MPa
  SensorXidibeiXDB401_Analog_KPa_2500 = 111,  // 0-2.5 MPa
  SensorXidibeiXDB401_Analog_KPa_3000 = 112,  // 0-3 MPa
  SensorXidibeiXDB401_Analog_KPa_3500 = 113,  // 0-3.5 MPa
  SensorXidibeiXDB401_Analog_KPa_4000 = 114,  // 0-4 MPa
};

constexpr auto PRESSURE_KPA = "kPa";
constexpr auto PRESSURE_BAR = "Bar";
constexpr auto PRESSURE_PSI = "PSI";

constexpr auto MAX_SENSOR_DEVICES = 2;

constexpr auto CONFIG_SENSOR_TYPE = "sensor_type";
constexpr auto CONFIG_SENSOR1_TYPE = "sensor1_type";
constexpr auto CONFIG_PRESSURE_ADJUSTMENT = "pressure_adjustment";
constexpr auto CONFIG_PRESSURE1_ADJUSTMENT = "pressure1_adjustment";
// constexpr auto CONFIG_TEMPERATURE_ADJUSTMENT = "temp_adjustment";
// constexpr auto CONFIG_TEMPERATURE1_ADJUSTMENT = "temp1_adjustment";

constexpr auto CONFIG_PRESSURE_UNIT = "pressure_unit";

class PressuremonConfig : public BrewingConfig {
 private:
  PressuremonBleFormat _pressuremonBleFormat =
      PressuremonBleFormat::BLE_DISABLED;
  PressureSensorType _pressureSensor[MAX_SENSOR_DEVICES] = {
      PressureSensorType::SensorNone, PressureSensorType::SensorNone};

  float _pressureSensorCorrection[MAX_SENSOR_DEVICES] = {0, 0};
  // float _temperatureSensorCorrection[MAX_SENSOR_DEVICES] = {0, 0};

  bool _batterySaving = true;

  String _pressureUnit = PRESSURE_PSI;

 public:
  PressuremonConfig(String baseMDNS, String fileName);

  bool isPressureBar() { return _pressureUnit == PRESSURE_BAR; }
  bool isPressureKpa() { return _pressureUnit == PRESSURE_KPA; }
  bool isPressurePsi() { return _pressureUnit == PRESSURE_PSI; }

  const char* getPressureUnit() { return _pressureUnit.c_str(); }
  void setPressureUnit(String s) {
    _pressureUnit = s;
    _saveNeeded = true;
  }

  PressureSensorType getPressureSensorType(int idx) {
    return _pressureSensor[idx];
  }
  int getPressureSensorTypeAsInt(int idx) {
    return static_cast<int>(_pressureSensor[idx]);
  }
  void setPressureSensorType(PressureSensorType s, int idx) {
    _pressureSensor[idx] = s;
    _saveNeeded = true;
  }
  void setPressureSensorType(int s, int idx) {
    setPressureSensorType((PressureSensorType)s, idx);
  }

  float getPressureSensorCorrection(int idx) {
    return _pressureSensorCorrection[idx];
  }
  void setPressureSensorCorrection(float v, int idx) {
    _pressureSensorCorrection[idx] = v;
    _saveNeeded = true;
  }

  float getTemperatureSensorCorrection(int idx) {
    return 0;
  }
  // float getTemperatureSensorCorrection(int idx) {
  //   return _temperatureSensorCorrection[idx];
  // }
  // void setTemperatureSensorCorrection(float v, int idx) {
  //   _temperatureSensorCorrection[idx] = v;
  //   _saveNeeded = true;
  // }

  bool isBleActive() {
    return (_pressuremonBleFormat != PressuremonBleFormat::BLE_DISABLED);
  }

  const PressuremonBleFormat getPressuremonBleFormat() {
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
