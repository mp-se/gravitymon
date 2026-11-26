/*
 * GravityMon
 * Copyright (c) 2021-2026 Magnus
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Alternatively, this software may be used under the terms of a
 * commercial license. See LICENSE_COMMERCIAL for details.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */


#if defined(GRAVITYMON)

#include <calc.hpp>
#include <config_gravitymon.hpp>
#include <gyro.hpp>
#include <hardware_scan.hpp>
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
constexpr auto PARAM_GYRO_FAMILY = "gyro_family";

constexpr auto PARAM_FEATURE_BLE_SUPPORTED = "ble";
constexpr auto PARAM_FEATURE_FILTER_SUPPORTED = "filter";
constexpr auto PARAM_FEATURE_VELOCITY_SUPPORTED = "velocity";
constexpr auto PARAM_FEATURE_CHARGING_SUPPORTED = "charging";

void GravitymonWebServer::doWebCalibrateStatus(JsonObject &obj) {
  if (myGyro.isConnected()) {
    if (_gyroCalibrationSuccess) {
      obj[PARAM_SUCCESS] = true;
      obj[PARAM_MESSAGE] = "Calibration completed";
      obj[PARAM_MESSAGE_CODE] = ESPFWK_WEB_ERR("CALIBRATION_COMPLETE");
    } else {
      obj[PARAM_SUCCESS] = false;
      obj[PARAM_MESSAGE] =
          "Calibration failed, faulty gyro or movement during calibration";
      obj[PARAM_MESSAGE_CODE] = ESPFWK_WEB_ERR("CALIBRATION_FAILED");
    }
  } else {
    obj[PARAM_SUCCESS] = false;
    obj[PARAM_MESSAGE] = "Calibration failed, no gyro connected";
    obj[PARAM_MESSAGE_CODE] = ESPFWK_WEB_ERR("CALIBRATION_NO_GYRO");
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
  obj[PARAM_HARDWARE] = CFG_PCB;
  obj[CONFIG_REGISTERED] = _gravConfig->isRegistered();
}

void GravitymonWebServer::doWebStatus(JsonObject &obj) {
  obj[PARAM_SELF][PARAM_SELF_PUSH_TARGET] =
      _gravConfig->isBleActive() || _gravConfig->hasTargetHttpPost() ||
              _gravConfig->hasTargetHttpPost2() ||
              _gravConfig->hasTargetHttpGet() || _gravConfig->hasTargetMqtt() ||
              _gravConfig->hasTargetInfluxDb2() || _gravConfig->isWifiDirect()
          ? true
          : false;

  double angle = 0;  // Indicate we have no valid gyro value

  if (myGyro.hasValue()) angle = myGyro.getAngle();

  double tempC = myTempSensor.getTempC();
  double gravity =
      calculateGravity(_gravConfig->getGravityFormula(), angle, tempC);

  obj[CONFIG_GRAVITY_UNIT] = String(_gravConfig->getGravityUnit());

  if (!isnan(angle)) {
    obj[PARAM_ANGLE] = serialized(String(angle, DECIMALS_TILT));
  }

  if (_gravConfig->isGravityTempAdj()) {
    gravity = gravityTemperatureCorrectionC(
        gravity, tempC, _gravConfig->getDefaultCalibrationTemp());
  }

  if (!isnan(gravity)) {
    if (_gravConfig->isGravityPlato()) {
      obj[CONFIG_GRAVITY] =
          serialized(String(convertToPlato(gravity), DECIMALS_PLATO));
    } else {
      obj[CONFIG_GRAVITY] = serialized(String(gravity, DECIMALS_SG));
    }
  }

  if (!isnan(tempC)) {
    if (_gravConfig->isTempFormatC()) {
      obj[PARAM_TEMP] = serialized(String(tempC, DECIMALS_TEMP));
    } else {
      obj[PARAM_TEMP] = serialized(String(convertCtoF(tempC), DECIMALS_TEMP));
    }
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
  _gyroCalibrationSuccess = false;

  if (myGyro.isConnected()) {
    _gyroCalibrationSuccess = myGyro.calibrateSensor();
    Log.info(F("WEB : Gyro calibration %s" CR),
             _gyroCalibrationSuccess ? "succeeded" : "failed");
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
  scanHardwareGravitymon(obj);
}

void GravitymonWebServer::webHandleGyro(AsyncWebServerRequest *request) {
  Log.notice(F("WEB : webServer callback for /api/gyro(get)." CR));

  AsyncJsonResponse *response = new AsyncJsonResponse(false);
  JsonObject obj = response->getRoot().as<JsonObject>();

  double angle = 0;  // Indicate we have no valid gyro value

  if (myGyro.hasValue()) angle = myGyro.getAngle();

  obj[PARAM_ANGLE] = serialized(String(angle, DECIMALS_TILT));

  // Acceleration in g, used by the UI to do the axis calibration. Only valid
  // when the device is not moving.
  obj["valid"] = myGyro.hasValue();
  obj["ax"] = serialized(String(myGyro.getAccelX(), 5));
  obj["ay"] = serialized(String(myGyro.getAccelY(), 5));
  obj["az"] = serialized(String(myGyro.getAccelZ(), 5));

  response->setLength();
  request->send(response);
}

bool GravitymonWebServer::setupWebServer(const char *serviceName) {
  BrewingWebServer::setupWebServer(serviceName);

  _server->on(
      "/api/gyro", (WebRequestMethodComposite)HTTP_GET,
      [this](AsyncWebServerRequest *request) { webHandleGyro(request); });
  return true;
}

#endif  // GRAVITYMON

// EOF
