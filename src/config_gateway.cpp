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
#if defined(GATEWAY)

#include <config.hpp>
#include <main.hpp>

GravmonGatewayConfig::GravmonGatewayConfig(String baseMDNS, String fileName)
    : BrewingConfig(baseMDNS, fileName) {}

void GravmonGatewayConfig::createJson(JsonObject& doc) {
  BrewingConfig::createJson(doc);

  // Call base class functions
  createJsonBase(doc);
  createJsonWifi(doc);
  createJsonPush(doc);

  doc[CONFIG_GRAVITY_UNIT] = String(getGravityUnit());
  doc[CONFIG_TIMEZONE] = getTimezone();
  doc[CONFIG_BLE_ACTIVE_SCAN] = getBleActiveScan();
  doc[CONFIG_BLE_SCAN_TIME] = getBleScanTime();
  doc[CONFIG_PUSH_RESEND_TIME] = getPushResendTime();
  doc[CONFIG_PRESSURE_UNIT] = getPressureUnit();

  doc[CONFIG_HTTP_POST_GRAVITY_ENABLE] = isHttpPostGravityEnable();
  doc[CONFIG_HTTP_POST_PRESSURE_ENABLE] = isHttpPostPressureEnable();
  doc[CONFIG_HTTP_POST2_GRAVITY_ENABLE] = isHttpPost2GravityEnable();
  doc[CONFIG_HTTP_POST2_PRESSURE_ENABLE] = isHttpPost2PressureEnable();
  doc[CONFIG_HTTP_GET_GRAVITY_ENABLE] = isHttpGetGravityEnable();
  doc[CONFIG_HTTP_GET_PRESSURE_ENABLE] = isHttpGetPressureEnable();
  doc[CONFIG_INFLUXDB2_GRAVITY_ENABLE] = isInfluxdb2GravityEnable();
  doc[CONFIG_INFLUXDB2_PRESSURE_ENABLE] = isInfluxdb2PressureEnable();
  doc[CONFIG_MQTT_GRAVITY_ENABLE] = isMqttGravityEnable();
  doc[CONFIG_MQTT_PRESSURE_ENABLE] = isMqttPressureEnable();
}

void GravmonGatewayConfig::parseJson(JsonObject& doc) {
  BrewingConfig::parseJson(doc);

  // Call base class functions
  parseJsonBase(doc);
  parseJsonWifi(doc);
  parseJsonPush(doc);

  if (!doc[CONFIG_GRAVITY_UNIT].isNull()) {
    String s = doc[CONFIG_GRAVITY_UNIT];
    setGravityUnit(s.charAt(0));
  }
  if (!doc["gravity_format"].isNull()) { // Legacy support for gravity_format
    String s = doc["gravity_format"];
    setGravityUnit(s.charAt(0));
  }

  if (!doc[CONFIG_TIMEZONE].isNull()) setTimezone(doc[CONFIG_TIMEZONE]);
  if (!doc[CONFIG_BLE_ACTIVE_SCAN].isNull())
    setBleActiveScan(doc[CONFIG_BLE_ACTIVE_SCAN].as<bool>());
  if (!doc[CONFIG_BLE_SCAN_TIME].isNull())
    setBleScanTime(doc[CONFIG_BLE_SCAN_TIME].as<int>());
  if (!doc[CONFIG_PUSH_RESEND_TIME].isNull())
    setPushResendTime(doc[CONFIG_PUSH_RESEND_TIME].as<int>());
  if (!doc[CONFIG_PRESSURE_UNIT].isNull())
    setPressureUnit(doc[CONFIG_PRESSURE_UNIT].as<String>());

  if (!doc[CONFIG_HTTP_POST_GRAVITY_ENABLE].isNull())
    setHttpPostGravityEnable(doc[CONFIG_HTTP_POST_GRAVITY_ENABLE].as<bool>());
  if (!doc[CONFIG_HTTP_POST_PRESSURE_ENABLE].isNull())
    setHttpPostPressureEnable(doc[CONFIG_HTTP_POST_PRESSURE_ENABLE].as<bool>());

  if (!doc[CONFIG_HTTP_POST2_GRAVITY_ENABLE].isNull())
    setHttpPost2GravityEnable(doc[CONFIG_HTTP_POST2_GRAVITY_ENABLE].as<bool>());
  if (!doc[CONFIG_HTTP_POST2_PRESSURE_ENABLE].isNull())
    setHttpPost2PressureEnable(
        doc[CONFIG_HTTP_POST2_PRESSURE_ENABLE].as<bool>());

  if (!doc[CONFIG_HTTP_GET_GRAVITY_ENABLE].isNull())
    setHttpGetGravityEnable(doc[CONFIG_HTTP_GET_GRAVITY_ENABLE].as<bool>());
  if (!doc[CONFIG_HTTP_GET_PRESSURE_ENABLE].isNull())
    setHttpGetPressureEnable(doc[CONFIG_HTTP_GET_PRESSURE_ENABLE].as<bool>());
  if (!doc[CONFIG_INFLUXDB2_GRAVITY_ENABLE].isNull())
    setInfluxdb2GravityEnable(doc[CONFIG_INFLUXDB2_GRAVITY_ENABLE].as<bool>());
  if (!doc[CONFIG_INFLUXDB2_PRESSURE_ENABLE].isNull())
    setInfluxdb2PressureEnable(
        doc[CONFIG_INFLUXDB2_PRESSURE_ENABLE].as<bool>());
  if (!doc[CONFIG_MQTT_GRAVITY_ENABLE].isNull())
    setMqttGravityEnable(doc[CONFIG_MQTT_GRAVITY_ENABLE].as<bool>());
  if (!doc[CONFIG_MQTT_PRESSURE_ENABLE].isNull())
    setMqttPressureEnable(doc[CONFIG_MQTT_PRESSURE_ENABLE].as<bool>());
}

#endif  // GATEWAY

// EOF
