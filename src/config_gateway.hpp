/*
MIT License

Copyright (c) 2021-2025 Magnus

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
#ifndef SRC_CONFIG_GATEWAY_HPP_
#define SRC_CONFIG_GATEWAY_HPP_

#if defined(GATEWAY)

#include <config_brewing.hpp>

constexpr auto CONFIG_GRAVITY_UNIT = "gravity_unit";
constexpr auto CONFIG_BLE_ENABLE = "ble_enable";
constexpr auto CONFIG_BLE_ACTIVE_SCAN = "ble_active_scan";
constexpr auto CONFIG_BLE_SCAN_TIME = "ble_scan_time";
constexpr auto CONFIG_PUSH_RESEND_TIME = "push_resend_time";
constexpr auto CONFIG_TIMEZONE = "timezone";
constexpr auto CONFIG_PRESSURE_UNIT = "pressure_unit";

constexpr auto CONFIG_HTTP_POST_GRAVITY_ENABLE = "http_post_gravity";
constexpr auto CONFIG_HTTP_POST_PRESSURE_ENABLE = "http_post_pressure";
constexpr auto CONFIG_HTTP_POST2_GRAVITY_ENABLE = "http_post2_gravity";
constexpr auto CONFIG_HTTP_POST2_PRESSURE_ENABLE = "http_post2_pressure";
constexpr auto CONFIG_HTTP_GET_GRAVITY_ENABLE = "http_get_gravity";
constexpr auto CONFIG_HTTP_GET_PRESSURE_ENABLE = "http_get_pressure";
constexpr auto CONFIG_INFLUXDB2_GRAVITY_ENABLE = "influxdb2_gravity";
constexpr auto CONFIG_INFLUXDB2_PRESSURE_ENABLE = "influxdb2_pressure";
constexpr auto CONFIG_MQTT_GRAVITY_ENABLE = "mqtt_gravity";
constexpr auto CONFIG_MQTT_PRESSURE_ENABLE = "mqtt_pressure";

constexpr auto PRESSURE_KPA = "kPa";
constexpr auto PRESSURE_BAR = "Bar";
constexpr auto PRESSURE_PSI = "PSI";

class GravmonGatewayConfig : public BrewingConfig {
 private:
  char _gravityUnit = 'G';

  String _pressureUnit = PRESSURE_PSI;
  String _timezone = "CET-1CEST,M3.5.0,M10.5.0/3";

  int _bleScanTime = 5;
  int _pushResendTime = 300;

  bool _bleEnable = true;
  bool _bleActiveScan = false;
  bool _httpPostGravityEnable = true;
  bool _httpPostPressureEnable = true;
  bool _httpPost2GravityEnable = true;
  bool _httpPost2PressureEnable = true;
  bool _httpGetGravityEnable = true;
  bool _httpGetPressureEnable = true;
  bool _influxdb2GravityEnable = true;
  bool _influxdb2PressureEnable = true;
  bool _mqttGravityEnable = true;
  bool _mqttPressureEnable = true;

 public:
  GravmonGatewayConfig(String baseMDNS, String fileName);

  int getVoltagePin() const { return 0; }

  const char* getTimezone() const { return _timezone.c_str(); }
  void setTimezone(String s) {
    _timezone = s;
    _saveNeeded = true;
  }

  bool isBleActive() const {
    return true;
  }  // Used in common code for reporting if push targets are defined.

  bool isBleEnable() const { return _bleEnable; }
  void setBleEnable(bool f) {
    _bleEnable = f;
    _saveNeeded = true;
  }

  bool isPressureBar() const { return _pressureUnit == PRESSURE_BAR; }
  bool isPressureKpa() const { return _pressureUnit == PRESSURE_KPA; }
  bool isPressurePsi() const { return _pressureUnit == PRESSURE_PSI; }

  const char* getPressureUnit() const { return _pressureUnit.c_str(); }
  void setPressureUnit(String s) {
    _pressureUnit = s;
    _saveNeeded = true;
  }

  bool isHttpPostGravityEnable() const { return _httpPostGravityEnable; }
  void setHttpPostGravityEnable(bool f) {
    _httpPostGravityEnable = f;
    _saveNeeded = true;
  }

  bool isHttpPostPressureEnable() const { return _httpPostPressureEnable; }
  void setHttpPostPressureEnable(bool f) {
    _httpPostPressureEnable = f;
    _saveNeeded = true;
  }

  bool isHttpPost2GravityEnable() const { return _httpPost2GravityEnable; }
  void setHttpPost2GravityEnable(bool f) {
    _httpPost2GravityEnable = f;
    _saveNeeded = true;
  }

  bool isHttpPost2PressureEnable() const { return _httpPost2PressureEnable; }
  void setHttpPost2PressureEnable(bool f) {
    _httpPost2PressureEnable = f;
    _saveNeeded = true;
  }

  bool isHttpGetGravityEnable() const { return _httpGetGravityEnable; }
  void setHttpGetGravityEnable(bool f) {
    _httpGetGravityEnable = f;
    _saveNeeded = true;
  }

  bool isHttpGetPressureEnable() const { return _httpGetPressureEnable; }
  void setHttpGetPressureEnable(bool f) {
    _httpGetPressureEnable = f;
    _saveNeeded = true;
  }

  bool isInfluxdb2GravityEnable() const { return _influxdb2GravityEnable; }
  void setInfluxdb2GravityEnable(bool f) {
    _influxdb2GravityEnable = f;
    _saveNeeded = true;
  }

  bool isInfluxdb2PressureEnable() const { return _influxdb2PressureEnable; }
  void setInfluxdb2PressureEnable(bool f) {
    _influxdb2PressureEnable = f;
    _saveNeeded = true;
  }

  bool isMqttGravityEnable() const { return _mqttGravityEnable; }
  void setMqttGravityEnable(bool f) {
    _mqttGravityEnable = f;
    _saveNeeded = true;
  }

  bool isMqttPressureEnable() const { return _mqttPressureEnable; }
  void setMqttPressureEnable(bool f) {
    _mqttPressureEnable = f;
    _saveNeeded = true;
  }

  int getBleScanTime() const { return _bleScanTime; }
  void setBleScanTime(int v) {
    _bleScanTime = v;
    _saveNeeded = true;
  }

  int getPushResendTime() const { return _pushResendTime; }
  void setPushResendTime(int t) {
    _pushResendTime = t;
    _saveNeeded = true;
  }

  bool getBleActiveScan() const { return _bleActiveScan; }
  void setBleActiveScan(bool b) {
    _bleActiveScan = b;
    _saveNeeded = true;
  }

  char getGravityUnit() const { return _gravityUnit; }
  void setGravityUnit(char c) {
    if (c == 'G' || c == 'P') {
      _gravityUnit = c;
      _saveNeeded = true;
    }
  }
  bool isGravitySG() const { return _gravityUnit == 'G'; }
  bool isGravityPlato() const { return _gravityUnit == 'P'; }

  bool isWifiPushActive() const {
    return (hasTargetHttpPost() || hasTargetHttpPost2() || hasTargetHttpGet() ||
            hasTargetInfluxDb2() || hasTargetMqtt())
               ? true
               : false;
  }

  // IO functions
  void createJson(JsonObject& doc) const;
  void parseJson(JsonObject& doc);
};

extern GravmonGatewayConfig myConfig;

#endif  // GATEWAY

#endif  // SRC_CONFIG_GATEWAY_HPP_

// EOF
