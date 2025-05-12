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
#if defined(GRAVITYMON)

#include <calc.hpp>
#include <config_gravitymon.hpp>
#include <gyro.hpp>
#include <main.hpp>
#include <push_gravitymon.hpp>
#include <tempsensor.hpp>
#include <web_gravitymon.hpp>

constexpr auto PARAM_ANGLE = "angle";
constexpr auto PARAM_TEMP = "temp";
constexpr auto PARAM_ISPINDEL_CONFIG = "ispindel_config";
constexpr auto PARAM_GRAVITYMON1_CONFIG = "gravitymon1_config";
constexpr auto PARAM_SELF_GYRO_CONNECTED = "gyro_connected";
constexpr auto PARAM_SELF_GYRO_MOVING = "gyro_moving";
constexpr auto PARAM_SELF_GYRO_CALIBRATION = "gyro_calibration";
constexpr auto PARAM_SELF_GRAVITY_FORMULA = "gravity_formula";
constexpr auto PARAM_SELF_TEMP_CONNECTED = "temp_connected";
constexpr auto PARAM_GYRO = "gyro";
constexpr auto PARAM_GYRO_FAMILY = "gyro_family";
constexpr auto PARAM_ONEWIRE = "onewire";
constexpr auto PARAM_BLE_SUPPORTED = "ble_supported";

void GravitymonWebServer::doWebCalibrateStatus(JsonObject &obj) {
  if (myGyro.isConnected()) {
    obj[PARAM_SUCCESS] = true;
    obj[PARAM_MESSAGE] = "Calibration completed";
  } else {
    obj[PARAM_SUCCESS] = false;
    obj[PARAM_MESSAGE] = "Calibration failed, no gyro connected";
  }
}

void GravitymonWebServer::doWebConfigWrite() {}

void GravitymonWebServer::doWebStatus(JsonObject &obj) {
  obj[PARAM_SELF][PARAM_SELF_PUSH_TARGET] =
      _gravConfig->isBleActive() || _gravConfig->hasTargetHttpPost() ||
              _gravConfig->hasTargetHttpPost2() ||
              _gravConfig->hasTargetHttpGet() || _gravConfig->hasTargetMqtt() ||
              _gravConfig->hasTargetInfluxDb2()
          ? true
          : false;

  double angle = 0;  // Indicate we have no valid gyro value

  if (myGyro.hasValue()) angle = myGyro.getAngle();

  double tempC = myTempSensor.getTempC();
  double gravity =
      calculateGravity(_gravConfig->getGravityFormula(), angle, tempC);

  obj[CONFIG_GRAVITY_UNIT] = String(_gravConfig->getGravityUnit());

  obj[PARAM_ANGLE] = serialized(String(angle, DECIMALS_TILT));

  if (_gravConfig->isGravityTempAdj()) {
    gravity = gravityTemperatureCorrectionC(
        gravity, tempC, _gravConfig->getDefaultCalibrationTemp());
  }
  if (_gravConfig->isGravityPlato()) {
    obj[CONFIG_GRAVITY] =
        serialized(String(convertToPlato(gravity), DECIMALS_PLATO));
  } else {
    obj[CONFIG_GRAVITY] = serialized(String(gravity, DECIMALS_SG));
  }

  if (_gravConfig->isTempFormatC()) {
    obj[PARAM_TEMP] = serialized(String(tempC, DECIMALS_TEMP));
  } else {
    obj[PARAM_TEMP] = serialized(String(convertCtoF(tempC), DECIMALS_TEMP));
  }

  obj[PARAM_GRAVITYMON1_CONFIG] = LittleFS.exists("/gravitymon.json");
  obj[PARAM_SELF][PARAM_SELF_TEMP_CONNECTED] = myTempSensor.isSensorAttached();

#if defined(ENABLE_BLE)
  obj[PARAM_BLE_SUPPORTED] = true;
#else
  obj[PARAM_BLE_SUPPORTED] = false;
#endif

  obj[PARAM_GYRO_FAMILY] = myGyro.getGyroFamily();

#if defined(ESP8266)
  obj[PARAM_ISPINDEL_CONFIG] = LittleFS.exists("/config.json");
#else
  obj[PARAM_ISPINDEL_CONFIG] = false;
#endif

#if defined(ESP32LITE)
  obj[PARAM_HARDWARE] = "floaty";
  obj[PARAM_SELF][PARAM_SELF_BATTERY_LEVEL] =
      true;  // No hardware support for these
  obj[PARAM_SELF][PARAM_SELF_TEMP_CONNECTED] = true;
#else
  obj[PARAM_HARDWARE] = "ispindel";
  obj[PARAM_SELF][PARAM_SELF_TEMP_CONNECTED] = myTempSensor.isSensorAttached();
#endif

  obj[PARAM_SELF][PARAM_SELF_GRAVITY_FORMULA] =
      strlen(_gravConfig->getGravityFormula()) > 0 ? true : false;
  obj[PARAM_SELF][PARAM_SELF_GYRO_CALIBRATION] =
      _gravConfig->hasGyroCalibration() || !myGyro.needCalibration();
  obj[PARAM_SELF][PARAM_SELF_GYRO_CONNECTED] = myGyro.isConnected();
  obj[PARAM_SELF][PARAM_SELF_GYRO_MOVING] = myGyro.isSensorMoving();
}

void GravitymonWebServer::doTaskSensorCalibration() {
  if (myGyro.isConnected()) {
    myGyro.calibrateSensor();
  } else {
    Log.error(F("WEB : No gyro connected, skipping calibration" CR));
  }
}

void GravitymonWebServer::doTaskPushTestSetup(TemplatingEngine &engine,
                                              BrewingPush &push) {
  // When runnning in configuration mode we dont apply the filter on the angle
  float angle = myGyro.getAngle();
  float tempC = myTempSensor.getTempC();
  float gravitySG =
      calculateGravity(_gravConfig->getGravityFormula(), angle, tempC);
  float corrGravitySG = gravityTemperatureCorrectionC(
      gravitySG, tempC, _gravConfig->getDefaultCalibrationTemp());

  if (_gravConfig->isGravityTempAdj()) {  // Apply if flag is set
    gravitySG = corrGravitySG;
  }

  setupTemplateEngineGravity(_gravConfig, engine, angle, 0, gravitySG,
                             corrGravitySG, tempC, 1.0,
                             myBatteryVoltage.getVoltage());

  Log.notice(F("WEB : Running scheduled push test for %s" CR),
             _pushTestTarget.c_str());

  if (!_pushTestTarget.compareTo(PARAM_FORMAT_POST) &&
      _gravConfig->hasTargetHttpPost()) {
    String tpl = push.getTemplate(BrewingPush::GRAVITY_TEMPLATE_HTTP1);
    String doc = engine.create(tpl.c_str());

    if (_gravConfig->isHttpPostSSL() && _gravConfig->isSkipSslOnTest())
      Log.notice(F("PUSH: SSL enabled, skip run when not in gravity mode." CR));
    else
      push.sendHttpPost(doc);
    _pushTestEnabled = true;
  } else if (!_pushTestTarget.compareTo(PARAM_FORMAT_POST2) &&
             _gravConfig->hasTargetHttpPost2()) {
    String tpl = push.getTemplate(BrewingPush::GRAVITY_TEMPLATE_HTTP2);
    String doc = engine.create(tpl.c_str());
    if (_gravConfig->isHttpPost2SSL() && _gravConfig->isSkipSslOnTest())
      Log.notice(F("PUSH: SSL enabled, skip run when not in gravity mode." CR));
    else
      push.sendHttpPost2(doc);
    _pushTestEnabled = true;
  } else if (!_pushTestTarget.compareTo(PARAM_FORMAT_GET) &&
             _gravConfig->hasTargetHttpGet()) {
    String tpl = push.getTemplate(BrewingPush::GRAVITY_TEMPLATE_HTTP3);
    String doc = engine.create(tpl.c_str());
    if (_gravConfig->isHttpGetSSL() && _gravConfig->isSkipSslOnTest())
      Log.notice(F("PUSH: SSL enabled, skip run when not in gravity mode." CR));
    else
      push.sendHttpGet(doc);
    _pushTestEnabled = true;
  } else if (!_pushTestTarget.compareTo(PARAM_FORMAT_INFLUXDB) &&
             _gravConfig->hasTargetInfluxDb2()) {
    String tpl = push.getTemplate(BrewingPush::GRAVITY_TEMPLATE_INFLUX);
    String doc = engine.create(tpl.c_str());
    if (_gravConfig->isHttpInfluxDb2SSL() && _gravConfig->isSkipSslOnTest())
      Log.notice(F("PUSH: SSL enabled, skip run when not in gravity mode." CR));
    else
      push.sendInfluxDb2(doc);
    _pushTestEnabled = true;
  } else if (!_pushTestTarget.compareTo(PARAM_FORMAT_MQTT) &&
             _gravConfig->hasTargetMqtt()) {
    String tpl = push.getTemplate(BrewingPush::GRAVITY_TEMPLATE_MQTT);
    String doc = engine.create(tpl.c_str());
    if (_gravConfig->isMqttSSL() && _gravConfig->isSkipSslOnTest())
      Log.notice(F("PUSH: SSL enabled, skip run when not in gravity mode." CR));
    else
      push.sendMqtt(doc);
    _pushTestEnabled = true;
  }

  engine.freeMemory();
  push.clearTemplate();
}

void GravitymonWebServer::doTaskHardwareScanning(JsonObject &obj) {
  JsonArray onew = obj[PARAM_ONEWIRE].to<JsonArray>();

  for (int i = 0, j = 0; i < myTempSensor.getSensorCount(); i++) {
    DeviceAddress adr;
    myTempSensor.getSensorAddress(&adr[0], i);
    Log.notice(F("WEB : Found onewire device %d." CR), i);
    onew[j][PARAM_ADRESS] = String(adr[0], 16) + String(adr[1], 16) +
                            String(adr[2], 16) + String(adr[3], 16) +
                            String(adr[4], 16) + String(adr[5], 16) +
                            String(adr[6], 16) + String(adr[7], 16);
    switch (adr[0]) {
      case DS18S20MODEL:
        onew[j][PARAM_FAMILY] = "DS18S20";
        break;
      case DS18B20MODEL:
        onew[j][PARAM_FAMILY] = "DS18B20";
        break;
      case DS1822MODEL:
        onew[j][PARAM_FAMILY] = "DS1822";
        break;
      case DS1825MODEL:
        onew[j][PARAM_FAMILY] = "DS1825";
        break;
      case DS28EA00MODEL:
        onew[j][PARAM_FAMILY] = "DS28EA00";
        break;
    }
    onew[j][PARAM_RESOLUTION] = myTempSensor.getSensorResolution();
    j++;
  }

  // Test the gyro
  obj[PARAM_GYRO][PARAM_FAMILY] = myGyro.getGyroFamily();
}

#endif  // GRAVITYMON

// EOF
