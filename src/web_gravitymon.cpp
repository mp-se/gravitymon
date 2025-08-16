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
constexpr auto PARAM_TEMP_SENSOR = "temp_sensor";

constexpr auto PARAM_FEATURE_BLE_SUPPORTED = "ble";
constexpr auto PARAM_FEATURE_FILTER_SUPPORTED = "filter";
constexpr auto PARAM_FEATURE_VELOCITY_SUPPORTED = "velocity";
constexpr auto PARAM_FEATURE_CHARGING_SUPPORTED = "charging";

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

void GravitymonWebServer::doWebFeature(JsonObject &obj) {
  obj[PARAM_FIRMWARE_FILE] = CFG_FILENAMEBIN;
#if defined(ENABLE_BLE) && defined(ESP32)
  obj[PARAM_FEATURE_BLE_SUPPORTED] = true;
#else
  obj[PARAM_FEATURE_BLE_SUPPORTED] = false;
#endif
#if defined(ENABLE_RTCMEM) && defined(ESP32)
  obj[PARAM_FEATURE_VELOCITY_SUPPORTED] = true;
  obj[PARAM_FEATURE_FILTER_SUPPORTED] = true;
#else
  obj[PARAM_FEATURE_VELOCITY_SUPPORTED] = false;
  obj[PARAM_FEATURE_FILTER_SUPPORTED] = false;
#endif
#if defined(PIN_CHARGING) && defined(ESP32)
  obj[PARAM_FEATURE_CHARGING_SUPPORTED] = true;
#else
  obj[PARAM_FEATURE_CHARGING_SUPPORTED] = false;
#endif
  obj[PARAM_HARDWARE] = "ispindel";
}

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
  obj[PARAM_GYRO_FAMILY] = myGyro.getGyroFamily();

#if defined(ESP8266)
  obj[PARAM_ISPINDEL_CONFIG] = LittleFS.exists("/config.json");
#else
  obj[PARAM_ISPINDEL_CONFIG] = false;
#endif

  obj[PARAM_SELF][PARAM_SELF_TEMP_CONNECTED] = myTempSensor.isSensorAttached();
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

  if (!_pushTestTarget.compareTo(PARAM_FORMAT_POST_GRAVITY) &&
      _gravConfig->hasTargetHttpPost()) {
    String tpl = push.getTemplate(BrewingPush::GRAVITY_TEMPLATE_HTTP1);
    String doc = engine.create(tpl.c_str());

    if (_gravConfig->isHttpPostSSL() && _gravConfig->isSkipSslOnTest())
      Log.notice(F("PUSH: SSL enabled, skip run when not in gravity mode." CR));
    else
      push.sendHttpPost(doc);
    _pushTestEnabled = true;
  } else if (!_pushTestTarget.compareTo(PARAM_FORMAT_POST2_GRAVITY) &&
             _gravConfig->hasTargetHttpPost2()) {
    String tpl = push.getTemplate(BrewingPush::GRAVITY_TEMPLATE_HTTP2);
    String doc = engine.create(tpl.c_str());
    if (_gravConfig->isHttpPost2SSL() && _gravConfig->isSkipSslOnTest())
      Log.notice(F("PUSH: SSL enabled, skip run when not in gravity mode." CR));
    else
      push.sendHttpPost2(doc);
    _pushTestEnabled = true;
  } else if (!_pushTestTarget.compareTo(PARAM_FORMAT_GET_GRAVITY) &&
             _gravConfig->hasTargetHttpGet()) {
    String tpl = push.getTemplate(BrewingPush::GRAVITY_TEMPLATE_HTTP3);
    String doc = engine.create(tpl.c_str());
    if (_gravConfig->isHttpGetSSL() && _gravConfig->isSkipSslOnTest())
      Log.notice(F("PUSH: SSL enabled, skip run when not in gravity mode." CR));
    else
      push.sendHttpGet(doc);
    _pushTestEnabled = true;
  } else if (!_pushTestTarget.compareTo(PARAM_FORMAT_INFLUXDB_GRAVITY) &&
             _gravConfig->hasTargetInfluxDb2()) {
    String tpl = push.getTemplate(BrewingPush::GRAVITY_TEMPLATE_INFLUX);
    String doc = engine.create(tpl.c_str());
    if (_gravConfig->isHttpInfluxDb2SSL() && _gravConfig->isSkipSslOnTest())
      Log.notice(F("PUSH: SSL enabled, skip run when not in gravity mode." CR));
    else
      push.sendInfluxDb2(doc);
    _pushTestEnabled = true;
  } else if (!_pushTestTarget.compareTo(PARAM_FORMAT_MQTT_GRAVITY) &&
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
#if defined(ESP8266)
  JsonArray onew = obj[PARAM_ONEWIRE].to<JsonArray>();

  const uint8_t owPins[] = {/*D0,*/ D1, D2, D3, D4,
                            D5,         D6, D7, D8 /*, RX, TX*/};

  for (uint8_t i = 0, j = 0; i < sizeof(owPins); i++) {
    // Log.notice(F("WEB : Scanning onewire pin %d." CR), owPins[i]);
    uint8_t addr[8];
    OneWire ds(owPins[i]);

    if (ds.search(&addr[0])) {
      Log.notice(F("WEB : Found onewire on pin %d." CR), owPins[i]);
      onew[j] = owPins[i];
    }
  }
#endif

  JsonArray sensor = obj[PARAM_TEMP_SENSOR].to<JsonArray>();

  for (int i = 0, j = 0; i < myTempSensor.getSensorCount(); i++) {
    DeviceAddress adr;
    myTempSensor.getSensorAddress(&adr[0], i);
    Log.notice(F("WEB : Found onewire device %d." CR), i);
    sensor[j][PARAM_ADRESS] = String(adr[0], 16) + String(adr[1], 16) +
                              String(adr[2], 16) + String(adr[3], 16) +
                              String(adr[4], 16) + String(adr[5], 16) +
                              String(adr[6], 16) + String(adr[7], 16);
    switch (adr[0]) {
      case DS18S20MODEL:
        sensor[j][PARAM_FAMILY] = "DS18S20";
        break;
      case DS18B20MODEL:
        sensor[j][PARAM_FAMILY] = "DS18B20";
        break;
      case DS1822MODEL:
        sensor[j][PARAM_FAMILY] = "DS1822";
        break;
      case DS1825MODEL:
        sensor[j][PARAM_FAMILY] = "DS1825";
        break;
      case DS28EA00MODEL:
        sensor[j][PARAM_FAMILY] = "DS28EA00";
        break;
    }
    sensor[j][PARAM_RESOLUTION] = myTempSensor.getSensorResolution();
    j++;
  }

  // Test the gyro
  obj[PARAM_GYRO][PARAM_FAMILY] = myGyro.getGyroFamily();
}

void GravitymonWebServer::webHandleGyro(AsyncWebServerRequest *request) {
  Log.notice(F("WEB : webServer callback for /api/gyro(get)." CR));

  AsyncJsonResponse *response = new AsyncJsonResponse(false);
  JsonObject obj = response->getRoot().as<JsonObject>();

  double angle = 0;  // Indicate we have no valid gyro value

  if (myGyro.hasValue()) angle = myGyro.getAngle();

  obj[PARAM_ANGLE] = serialized(String(angle, DECIMALS_TILT));

  response->setLength();
  request->send(response);
}

bool GravitymonWebServer::setupWebServer(const char *serviceName) {
  BrewingWebServer::setupWebServer(serviceName);

  _server->on("/api/gyro", HTTP_GET,
              std::bind(&GravitymonWebServer::webHandleGyro, this,
                        std::placeholders::_1));
  return true;
}

#endif  // GRAVITYMON

// EOF
