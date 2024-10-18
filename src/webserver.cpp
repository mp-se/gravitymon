/*
MIT License

Copyright (c) 2021-2024 Magnus

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
#include <Wire.h>

#include <battery.hpp>
#include <calc.hpp>
#include <config.hpp>
#include <gyro.hpp>
#include <helper.hpp>
#include <history.hpp>
#include <main.hpp>
#include <perf.hpp>
#include <pushtarget.hpp>
#include <resources.hpp>
#include <templating.hpp>
#include <tempsensor.hpp>
#include <webserver.hpp>

#if !defined(ESP8266)
#include <esp_int_wdt.h>
#include <esp_task_wdt.h>
#endif

extern bool sleepModeActive;
extern bool sleepModeAlwaysSkip;

GravmonWebServer::GravmonWebServer(WebConfig *config) : BaseWebServer(config) {}

void GravmonWebServer::webHandleConfigRead(AsyncWebServerRequest *request) {
  if (!isAuthenticated(request)) {
    return;
  }

  PERF_BEGIN("webserver-api-config-read");
  Log.notice(F("WEB : webServer callback for /api/config(read)." CR));
  AsyncJsonResponse *response =
      new AsyncJsonResponse(false, JSON_BUFFER_SIZE_L);
  JsonObject obj = response->getRoot().as<JsonObject>();
  myConfig.createJson(obj);
  response->setLength();
  request->send(response);
  PERF_END("webserver-api-config-read");
}

void GravmonWebServer::webHandleConfigWrite(AsyncWebServerRequest *request,
                                            JsonVariant &json) {
  if (!isAuthenticated(request)) {
    return;
  }

  PERF_BEGIN("webserver-api-config-write");
  Log.notice(F("WEB : webServer callback for /api/config(write)." CR));
  JsonObject obj = json.as<JsonObject>();
  myConfig.parseJson(obj);
  obj.clear();
  myConfig.saveFile();
  myBatteryVoltage.read();

  AsyncJsonResponse *response =
      new AsyncJsonResponse(false, JSON_BUFFER_SIZE_S);
  obj = response->getRoot().as<JsonObject>();
  obj[PARAM_SUCCESS] = true;
  obj[PARAM_MESSAGE] = "Configuration updated";
  response->setLength();
  request->send(response);
  PERF_END("webserver-api-config-write");
}

void GravmonWebServer::webHandleCalibrate(AsyncWebServerRequest *request) {
  if (!isAuthenticated(request)) {
    return;
  }

  PERF_BEGIN("webserver-api-calibrate");
  Log.notice(F("WEB : webServer callback for /api/calibrate." CR));
  _sensorCalibrationTask = true;
  AsyncJsonResponse *response =
      new AsyncJsonResponse(false, JSON_BUFFER_SIZE_S);
  JsonObject obj = response->getRoot().as<JsonObject>();
  obj[PARAM_SUCCESS] = true;
  obj[PARAM_MESSAGE] = "Scheduled device calibration";
  response->setLength();
  request->send(response);
  PERF_END("webserver-api-calibrate");
}

void GravmonWebServer::webHandleCalibrateStatus(
    AsyncWebServerRequest *request) {
  if (!isAuthenticated(request)) {
    return;
  }

  PERF_BEGIN("webserver-api-calibrate-status");
  Log.notice(F("WEB : webServer callback for /api/calibrate/status." CR));
  AsyncJsonResponse *response =
      new AsyncJsonResponse(false, JSON_BUFFER_SIZE_S);
  JsonObject obj = response->getRoot().as<JsonObject>();
  obj[PARAM_STATUS] = static_cast<bool>(_sensorCalibrationTask);
  obj[PARAM_SUCCESS] = false;
  obj[PARAM_MESSAGE] = "Calibration running";

  if (!_sensorCalibrationTask) {
    if (myGyro.isConnected()) {
      obj[PARAM_SUCCESS] = true;
      obj[PARAM_MESSAGE] = "Calibration completed";
    } else {
      obj[PARAM_SUCCESS] = false;
      obj[PARAM_MESSAGE] = "Calibration failed, no gyro connected";
    }
  }

  response->setLength();
  request->send(response);
  PERF_END("webserver-api-calibrate-status");
}

void GravmonWebServer::webHandleFactoryDefaults(
    AsyncWebServerRequest *request) {
  if (!isAuthenticated(request)) {
    return;
  }

  Log.notice(F("WEB : webServer callback for /api/factory." CR));
  myConfig.saveFileWifiOnly();
  LittleFS.remove(ERR_FILENAME);
  LittleFS.remove(RUNTIME_FILENAME);
  LittleFS.remove(TPL_FNAME_POST);
  LittleFS.remove(TPL_FNAME_POST2);
  LittleFS.remove(TPL_FNAME_INFLUXDB);
  LittleFS.remove(TPL_FNAME_MQTT);
  LittleFS.end();

  Log.notice(F("WEB : Deleted files in filesystem, rebooting." CR));
  AsyncJsonResponse *response =
      new AsyncJsonResponse(false, JSON_BUFFER_SIZE_S);
  JsonObject obj = response->getRoot().as<JsonObject>();
  obj[PARAM_SUCCESS] = true;
  obj[PARAM_MESSAGE] = "Factory reset completed, rebooting";
  response->setLength();
  request->send(response);

  Log.notice(F("WEB : Scheduling reboot." CR));
  _rebootTimer = millis();
  _rebootTask = true;
}

void GravmonWebServer::webHandleStatus(AsyncWebServerRequest *request) {
  PERF_BEGIN("webserver-api-status");
  Log.notice(F("WEB : webServer callback for /api/status(get)." CR));

  // Fallback since sometimes the loop() does not always run after firmware
  // update...
  if (_rebootTask) {
    Log.notice(F("WEB : Rebooting using fallback..." CR));
    delay(500);
    ESP_RESET();
  }

  AsyncJsonResponse *response =
      new AsyncJsonResponse(false, JSON_BUFFER_SIZE_L);
  JsonObject obj = response->getRoot().as<JsonObject>();

  double angle = 0;  // Indicate we have no valid gyro value

  if (myGyro.hasValue()) angle = myGyro.getAngle();

  double tempC = myTempSensor.getTempC();
  double gravity = calculateGravity(angle, tempC);

  obj[PARAM_ID] = myConfig.getID();
  obj[PARAM_TEMP_FORMAT] = String(myConfig.getTempFormat());
  obj[PARAM_GRAVITY_FORMAT] = String(myConfig.getGravityFormat());
  obj[PARAM_APP_VER] = CFG_APPVER;
  obj[PARAM_APP_BUILD] = CFG_GITREV;
  obj[PARAM_MDNS] = myConfig.getMDNS();
#if defined(ESP8266)
  obj[PARAM_PLATFORM] = "esp8266";
  obj[PARAM_HARDWARE] = "ispindel";
#elif defined(ESP32C3)
  obj[PARAM_PLATFORM] = "esp32c3";
  obj[PARAM_HARDWARE] = "ispindel";
#elif defined(ESP32S2)
  obj[PARAM_PLATFORM] = "esp32s2";
  obj[PARAM_HARDWARE] = "ispindel";
#elif defined(ESP32S3)
  obj[PARAM_PLATFORM] = "esp32s3";
  obj[PARAM_HARDWARE] = "ispindel";
#elif defined(ESP32LITE)
  obj[PARAM_PLATFORM] = "esp32lite";
  obj[PARAM_HARDWARE] = "floaty";
#else  // esp32 mini
  obj[PARAM_PLATFORM] = "esp32";
  obj[PARAM_HARDWARE] = "ispindel";
#endif

  if (myGyro.isConnected()) {
    obj[PARAM_ANGLE] = serialized(String(angle, DECIMALS_TILT));
  } else {
    obj[PARAM_ANGLE] = -1;  // Indicate that there is no connection to gyro
  }

  if (myConfig.isGravityTempAdj()) {
    gravity = gravityTemperatureCorrectionC(
        gravity, tempC, myConfig.getDefaultCalibrationTemp());
  }
  if (myConfig.isGravityPlato()) {
    obj[PARAM_GRAVITY] =
        serialized(String(convertToPlato(gravity), DECIMALS_PLATO));
  } else {
    obj[PARAM_GRAVITY] = serialized(String(gravity, DECIMALS_SG));
  }

  if (myConfig.isTempFormatC()) {
    obj[PARAM_TEMP] = serialized(String(tempC, DECIMALS_TEMP));
  } else {
    obj[PARAM_TEMP] = serialized(String(convertCtoF(tempC), DECIMALS_TEMP));
  }

  obj[PARAM_BATTERY] =
      serialized(String(myBatteryVoltage.getVoltage(), DECIMALS_BATTERY));
  obj[PARAM_SLEEP_MODE] = sleepModeAlwaysSkip;
  obj[PARAM_RSSI] = WiFi.RSSI();
  obj[PARAM_SSID] = WiFi.SSID();

#if defined(ESP8266)
  obj[PARAM_ISPINDEL_CONFIG] = LittleFS.exists("/config.json");
  obj[PARAM_TOTAL_HEAP] = 81920;
  obj[PARAM_FREE_HEAP] = ESP.getFreeHeap();
  obj[PARAM_IP] = WiFi.localIP().toString();
#else
  obj[PARAM_ISPINDEL_CONFIG] = false;
  obj[PARAM_TOTAL_HEAP] = ESP.getHeapSize();
  obj[PARAM_FREE_HEAP] = ESP.getFreeHeap();
  obj[PARAM_IP] = WiFi.localIP().toString();
#endif
  obj[PARAM_WIFI_SETUP] = (runMode == RunMode::wifiSetupMode) ? true : false;
  obj[PARAM_GRAVITYMON1_CONFIG] = LittleFS.exists("/gravitymon.json");

  FloatHistoryLog runLog(RUNTIME_FILENAME);
  obj[PARAM_RUNTIME_AVERAGE] = serialized(String(
      runLog.getAverage() ? runLog.getAverage() / 1000 : 0, DECIMALS_RUNTIME));

  JsonObject self = obj.createNestedObject(PARAM_SELF);
  float v = myBatteryVoltage.getVoltage();

#if defined(ESP32LITE)
  self[PARAM_SELF_BATTERY_LEVEL] = true;  // No hardware support for these
  self[PARAM_SELF_TEMP_CONNECTED] = true;
#else
  self[PARAM_SELF_BATTERY_LEVEL] = v < 3.2 || v > 5.1 ? false : true;
  self[PARAM_SELF_TEMP_CONNECTED] = myTempSensor.isSensorAttached();
#endif
  self[PARAM_SELF_GRAVITY_FORMULA] =
      strlen(myConfig.getGravityFormula()) > 0 ? true : false;
  self[PARAM_SELF_GYRO_CALIBRATION] = myConfig.hasGyroCalibration();
  self[PARAM_SELF_GYRO_CONNECTED] = myGyro.isConnected();
  self[PARAM_SELF_PUSH_TARGET] =
      myConfig.isBleActive() || myConfig.hasTargetHttpPost() ||
              myConfig.hasTargetHttpPost() || myConfig.hasTargetHttpGet() ||
              myConfig.hasTargetMqtt() || myConfig.hasTargetInfluxDb2()
          ? true
          : false;

  response->setLength();
  request->send(response);
  PERF_END("webserver-api-status");
}

void GravmonWebServer::webHandleSleepmode(AsyncWebServerRequest *request,
                                          JsonVariant &json) {
  if (!isAuthenticated(request)) {
    return;
  }

  PERF_BEGIN("webserver-api-sleepmode");
  Log.notice(F("WEB : webServer callback for /api/sleepmode." CR));
  JsonObject obj = json.as<JsonObject>();
  sleepModeAlwaysSkip = obj[PARAM_SLEEP_MODE].as<bool>();

  AsyncJsonResponse *response =
      new AsyncJsonResponse(false, JSON_BUFFER_SIZE_S);
  obj = response->getRoot().as<JsonObject>();
  obj[PARAM_SLEEP_MODE] = sleepModeAlwaysSkip;
  response->setLength();
  request->send(response);
  PERF_END("webserver-api-sleepmode");
}

void GravmonWebServer::webHandleConfigFormatWrite(
    AsyncWebServerRequest *request, JsonVariant &json) {
  if (!isAuthenticated(request)) {
    return;
  }

  PERF_BEGIN("webserver-api-config-format-write");
  Log.notice(F("WEB : webServer callback for /api/config/format(post)." CR));

  JsonObject obj = json.as<JsonObject>();
  int success = 0;

  if (!obj[PARAM_FORMAT_POST].isNull()) {
    success += writeFile(TPL_FNAME_POST, obj[PARAM_FORMAT_POST]) ? 1 : 0;
  }
  if (!obj[PARAM_FORMAT_POST2].isNull()) {
    success += writeFile(TPL_FNAME_POST2, obj[PARAM_FORMAT_POST2]) ? 1 : 0;
  }
  if (!obj[PARAM_FORMAT_GET].isNull()) {
    success += writeFile(TPL_FNAME_GET, obj[PARAM_FORMAT_GET]) ? 1 : 0;
  }
  if (!obj[PARAM_FORMAT_INFLUXDB].isNull()) {
    success +=
        writeFile(TPL_FNAME_INFLUXDB, obj[PARAM_FORMAT_INFLUXDB]) ? 1 : 0;
  }
  if (!obj[PARAM_FORMAT_MQTT].isNull()) {
    success += writeFile(TPL_FNAME_MQTT, obj[PARAM_FORMAT_MQTT]) ? 1 : 0;
  }

  AsyncJsonResponse *response =
      new AsyncJsonResponse(false, JSON_BUFFER_SIZE_S);
  obj = response->getRoot().as<JsonObject>();
  obj[PARAM_SUCCESS] = success > 0 ? true : false;
  obj[PARAM_MESSAGE] = success > 0 ? "Format template stored"
                                   : "Failed to store format template";
  response->setLength();
  request->send(response);
  PERF_END("webserver-api-config-format-write");
}

void GravmonWebServer::webHandleTestPush(AsyncWebServerRequest *request,
                                         JsonVariant &json) {
  if (!isAuthenticated(request)) {
    return;
  }

  PERF_BEGIN("webserver-api-test-push");
  Log.notice(F("WEB : webServer callback for /api/test/push." CR));
  JsonObject obj = json.as<JsonObject>();
  _pushTestTarget = obj[PARAM_PUSH_FORMAT].as<String>();
  _pushTestTask = true;
  _pushTestEnabled = false;
  _pushTestLastSuccess = false;
  _pushTestLastCode = 0;
  AsyncJsonResponse *response =
      new AsyncJsonResponse(false, JSON_BUFFER_SIZE_S);
  obj = response->getRoot().as<JsonObject>();
  obj[PARAM_SUCCESS] = true;
  obj[PARAM_MESSAGE] = "Scheduled test for " + _pushTestTarget;
  response->setLength();
  request->send(response);
  PERF_END("webserver-api-test-push");
}

void GravmonWebServer::webHandleTestPushStatus(AsyncWebServerRequest *request) {
  PERF_BEGIN("webserver-api-test-push-status");
  Log.notice(F("WEB : webServer callback for /api/test/push/status." CR));
  AsyncJsonResponse *response =
      new AsyncJsonResponse(false, JSON_BUFFER_SIZE_S);
  JsonObject obj = response->getRoot().as<JsonObject>();
  String s;

  if (_pushTestTask)
    s = "Running push tests for " + _pushTestTarget;
  else if (!_pushTestTask && _pushTestLastSuccess == 0)
    s = "No push test has been started";
  else
    s = "Push test for " + _pushTestTarget + " is complete";

  obj[PARAM_STATUS] = static_cast<bool>(_pushTestTask);
  obj[PARAM_SUCCESS] = _pushTestLastSuccess;
  obj[PARAM_MESSAGE] = s;
  obj[PARAM_PUSH_ENABLED] = _pushTestEnabled;
  obj[PARAM_PUSH_RETURN_CODE] = _pushTestLastCode;
  response->setLength();
  request->send(response);
  PERF_END("webserver-api-test-push-status");
}

bool GravmonWebServer::writeFile(String fname, String data) {
  if (data.length()) {
    data = urldecode(data);
    File file = LittleFS.open(fname, "w");
    if (file) {
      Log.notice(F("WEB : Storing template data in %s." CR), fname.c_str());
#if defined(ESP8266)
      file.write(data.c_str());
#else  // defined (ESP32)
      file.write((unsigned char *)data.c_str(), data.length());
#endif
      file.close();
      return true;
    }
  } else {
    Log.notice(
        F("WEB : No template data to store in %s, reverting to default." CR),
        fname.c_str());
    LittleFS.remove(fname);
    return true;
  }

  return false;
}

void GravmonWebServer::webHandleHardwareScan(AsyncWebServerRequest *request) {
  if (!isAuthenticated(request)) {
    return;
  }

  Log.notice(F("WEB : webServer callback for /api/hardware." CR));
  _hardwareScanTask = true;
  _hardwareScanData = "";
  AsyncJsonResponse *response =
      new AsyncJsonResponse(false, JSON_BUFFER_SIZE_S);
  JsonObject obj = response->getRoot().as<JsonObject>();
  obj[PARAM_SUCCESS] = true;
  obj[PARAM_MESSAGE] = "Scheduled hardware scanning";
  response->setLength();
  request->send(response);
}

void GravmonWebServer::webHandleHardwareScanStatus(
    AsyncWebServerRequest *request) {
  if (!isAuthenticated(request)) {
    return;
  }

  Log.notice(F("WEB : webServer callback for /api/hardware/status." CR));

  if (_hardwareScanTask || !_hardwareScanData.length()) {
    AsyncJsonResponse *response =
        new AsyncJsonResponse(false, JSON_BUFFER_SIZE_L);
    JsonObject obj = response->getRoot().as<JsonObject>();
    obj[PARAM_STATUS] = static_cast<bool>(_hardwareScanTask);
    obj[PARAM_SUCCESS] = false;
    obj[PARAM_MESSAGE] =
        _hardwareScanTask ? "Hardware scanning running" : "No scanning running";
    response->setLength();
    request->send(response);
  } else {
    request->send(200, "application/json", _hardwareScanData);
  }
}

String GravmonWebServer::readFile(String fname) {
  File file = LittleFS.open(fname, "r");
  if (file) {
    char buf[file.size() + 1];
    memset(&buf[0], 0, file.size() + 1);
    file.readBytes(&buf[0], file.size());
    file.close();
    Log.notice(F("WEB : Read template data from %s." CR), fname.c_str());
    return String(&buf[0]);
  }
  return "";
}

void GravmonWebServer::webHandleConfigFormatRead(
    AsyncWebServerRequest *request) {
  if (!isAuthenticated(request)) {
    return;
  }

  PERF_BEGIN("webserver-api-config-format-read");
  Log.notice(F("WEB : webServer callback for /api/config/format(read)." CR));

  AsyncJsonResponse *response =
      new AsyncJsonResponse(false, JSON_BUFFER_SIZE_XL);
  JsonObject obj = response->getRoot().as<JsonObject>();
  String s;

  s = readFile(TPL_FNAME_POST);
  obj[PARAM_FORMAT_POST] =
      s.length() ? urlencode(s) : urlencode(String(&iSpindleFormat[0]));
  s = readFile(TPL_FNAME_POST2);
  obj[PARAM_FORMAT_POST2] =
      s.length() ? urlencode(s) : urlencode(String(&iSpindleFormat[0]));
  s = readFile(TPL_FNAME_GET);
  obj[PARAM_FORMAT_GET] =
      s.length() ? urlencode(s) : urlencode(String(&iHttpGetFormat[0]));
  s = readFile(TPL_FNAME_INFLUXDB);
  obj[PARAM_FORMAT_INFLUXDB] =
      s.length() ? urlencode(s) : urlencode(String(&influxDbFormat[0]));
  s = readFile(TPL_FNAME_MQTT);
  obj[PARAM_FORMAT_MQTT] =
      s.length() ? urlencode(s) : urlencode(String(&mqttFormat[0]));

  response->setLength();
  request->send(response);
  PERF_END("webserver-api-config-format-read");
}

bool GravmonWebServer::setupWebServer() {
  Log.notice(F("WEB : Configuring web server." CR));

  BaseWebServer::setupWebServer();
  MDNS.addService("gravitymon", "tcp", 80);

  // Static content
  Log.notice(F("WEB : Setting up handlers for gravmon web server." CR));

  AsyncCallbackJsonWebHandler *handler;
  _server->on("/api/format", HTTP_GET,
              std::bind(&GravmonWebServer::webHandleConfigFormatRead, this,
                        std::placeholders::_1));
  handler = new AsyncCallbackJsonWebHandler(
      "/api/format",
      std::bind(&GravmonWebServer::webHandleConfigFormatWrite, this,
                std::placeholders::_1, std::placeholders::_2),
      JSON_BUFFER_SIZE_L);
  _server->addHandler(handler);
  handler = new AsyncCallbackJsonWebHandler(
      "/api/sleepmode",
      std::bind(&GravmonWebServer::webHandleSleepmode, this,
                std::placeholders::_1, std::placeholders::_2),
      JSON_BUFFER_SIZE_S);
  _server->addHandler(handler);
  handler = new AsyncCallbackJsonWebHandler(
      "/api/config",
      std::bind(&GravmonWebServer::webHandleConfigWrite, this,
                std::placeholders::_1, std::placeholders::_2),
      JSON_BUFFER_SIZE_L);
  _server->addHandler(handler);
  _server->on("/api/config", HTTP_GET,
              std::bind(&GravmonWebServer::webHandleConfigRead, this,
                        std::placeholders::_1));
  _server->on("/api/calibrate/status", HTTP_GET,
              std::bind(&GravmonWebServer::webHandleCalibrateStatus, this,
                        std::placeholders::_1));
  _server->on("/api/calibrate", HTTP_GET,
              std::bind(&GravmonWebServer::webHandleCalibrate, this,
                        std::placeholders::_1));
  _server->on("/api/hardware/status", HTTP_GET,
              std::bind(&GravmonWebServer::webHandleHardwareScanStatus, this,
                        std::placeholders::_1));
  _server->on("/api/hardware", HTTP_GET,
              std::bind(&GravmonWebServer::webHandleHardwareScan, this,
                        std::placeholders::_1));
  _server->on("/api/factory", HTTP_GET,
              std::bind(&GravmonWebServer::webHandleFactoryDefaults, this,
                        std::placeholders::_1));
  _server->on("/api/status", HTTP_GET,
              std::bind(&GravmonWebServer::webHandleStatus, this,
                        std::placeholders::_1));
  _server->on("/api/push/status", HTTP_GET,
              std::bind(&GravmonWebServer::webHandleTestPushStatus, this,
                        std::placeholders::_1));
  handler = new AsyncCallbackJsonWebHandler(
      "/api/push",
      std::bind(&GravmonWebServer::webHandleTestPush, this,
                std::placeholders::_1, std::placeholders::_2),
      JSON_BUFFER_SIZE_S);
  _server->addHandler(handler);

  Log.notice(F("WEB : Web server started." CR));
  return true;
}

void GravmonWebServer::loop() {
#if defined(ESP8266)
  MDNS.update();
#endif
  BaseWebServer::loop();

  if (_sensorCalibrationTask) {
    if (myGyro.isConnected()) {
      myGyro.calibrateSensor();
    } else {
      Log.error(F("WEB : No gyro connected, skipping calibration" CR));
    }

    _sensorCalibrationTask = false;
  }

  if (_pushTestTask) {
    float angle = myGyro.getAngle();
    float tempC = myTempSensor.getTempC();
    float gravitySG = calculateGravity(angle, tempC);
    float corrGravitySG = gravityTemperatureCorrectionC(
        gravitySG, tempC, myConfig.getDefaultCalibrationTemp());

    Log.notice(F("WEB : Running scheduled push test for %s" CR),
               _pushTestTarget.c_str());

    TemplatingEngine engine;
    GravmonPush push(&myConfig);
    push.setupTemplateEngine(engine, angle, gravitySG, corrGravitySG, tempC,
                             1.0, myBatteryVoltage.getVoltage());

    if (!_pushTestTarget.compareTo(PARAM_FORMAT_POST) &&
        myConfig.hasTargetHttpPost()) {
      String tpl = push.getTemplate(GravmonPush::TEMPLATE_HTTP1);
      String doc = engine.create(tpl.c_str());
      push.sendHttpPost(doc);
      _pushTestEnabled = true;
    } else if (!_pushTestTarget.compareTo(PARAM_FORMAT_POST2) &&
               myConfig.hasTargetHttpPost2()) {
      String tpl = push.getTemplate(GravmonPush::TEMPLATE_HTTP2);
      String doc = engine.create(tpl.c_str());
      push.sendHttpPost2(doc);
      _pushTestEnabled = true;
    } else if (!_pushTestTarget.compareTo(PARAM_FORMAT_GET) &&
               myConfig.hasTargetHttpGet()) {
      String tpl = push.getTemplate(GravmonPush::TEMPLATE_HTTP3);
      String doc = engine.create(tpl.c_str());
      push.sendHttpGet(doc);
      _pushTestEnabled = true;
    } else if (!_pushTestTarget.compareTo(PARAM_FORMAT_INFLUXDB) &&
               myConfig.hasTargetInfluxDb2()) {
      String tpl = push.getTemplate(GravmonPush::TEMPLATE_INFLUX);
      String doc = engine.create(tpl.c_str());
      push.sendInfluxDb2(doc);
      _pushTestEnabled = true;
    } else if (!_pushTestTarget.compareTo(PARAM_FORMAT_MQTT) &&
               myConfig.hasTargetMqtt()) {
      String tpl = push.getTemplate(GravmonPush::TEMPLATE_MQTT);
      String doc = engine.create(tpl.c_str());
      push.sendMqtt(doc);
      _pushTestEnabled = true;
    }

    engine.freeMemory();
    push.clearTemplate();
    _pushTestLastSuccess = push.getLastSuccess();
    _pushTestLastCode = push.getLastCode();
    if (_pushTestEnabled)
      Log.notice(
          F("WEB : Scheduled push test %s completed, success=%d, code=%d" CR),
          _pushTestTarget.c_str(), _pushTestLastSuccess, _pushTestLastCode);
    else
      Log.notice(F("WEB : Scheduled push test %s failed, not enabled" CR),
                 _pushTestTarget.c_str());
    _pushTestTask = false;
  }

  if (_hardwareScanTask) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE_L);
    JsonObject obj = doc.createNestedObject();
    obj[PARAM_STATUS] = false;
    obj[PARAM_SUCCESS] = true;
    obj[PARAM_MESSAGE] = "";
    Log.notice(F("WEB : Scanning hardware." CR));

    // Scan the i2c bus for devices
    // Wire.begin(PIN_SDA, PIN_SCL); // Should already have been done in
    // gyro.cpp
    JsonArray i2c = obj.createNestedArray(PARAM_I2C);

    for (int i = 1; i < 127; i++) {
      // The i2c_scanner uses the return value of
      // the Write.endTransmisstion to see if
      // a device did acknowledge to the address.
      Wire.beginTransmission(i);
      int err = Wire.endTransmission();

      if (err == 0) {
        JsonObject sensor = i2c.createNestedObject();
        sensor[PARAM_ADRESS] = "0x" + String(i, 16);
      }
    }

    // Scan onewire
    JsonArray onew = obj.createNestedArray(PARAM_ONEWIRE);

    for (int i = 0; i < mySensors.getDS18Count(); i++) {
      DeviceAddress adr;
      JsonObject sensor = onew.createNestedObject();
      mySensors.getAddress(&adr[0], i);
      sensor[PARAM_ADRESS] = String(adr[0], 16) + String(adr[1], 16) +
                             String(adr[2], 16) + String(adr[3], 16) +
                             String(adr[4], 16) + String(adr[5], 16) +
                             String(adr[6], 16) + String(adr[7], 16);
      switch (adr[0]) {
        case DS18S20MODEL:
          sensor[PARAM_FAMILY] = "DS18S20";
          break;
        case DS18B20MODEL:
          sensor[PARAM_FAMILY] = "DS18B20";
          break;
        case DS1822MODEL:
          sensor[PARAM_FAMILY] = "DS1822";
          break;
        case DS1825MODEL:
          sensor[PARAM_FAMILY] = "DS1825";
          break;
        case DS28EA00MODEL:
          sensor[PARAM_FAMILY] = "DS28EA00";
          break;
      }
      sensor[PARAM_RESOLUTION] = mySensors.getResolution();
    }

    // TODO: Test the gyro
    JsonObject gyro = obj.createNestedObject(PARAM_GYRO);
    switch (myGyro.getGyroID()) {
      case 0x34:
        gyro[PARAM_FAMILY] = "MPU6050";
        break;
      case 0x38:
        gyro[PARAM_FAMILY] = "MPU6500";
        break;
      default:
        gyro[PARAM_FAMILY] = "0x" + String(myGyro.getGyroID(), 16);
        break;
    }

    // TODO: Test GPIO

    JsonObject cpu = obj.createNestedObject(PARAM_CHIP);

#if defined(ESP8266)
    cpu[PARAM_FAMILY] = "ESP8266";
#else
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    cpu[PARAM_REVISION] = chip_info.revision;
    cpu[PARAM_CORES] = chip_info.cores;

    JsonArray feature = cpu.createNestedArray(PARAM_FEATURES);

    if (chip_info.features & CHIP_FEATURE_EMB_FLASH)
      feature.add("embedded flash");
    if (chip_info.features & CHIP_FEATURE_WIFI_BGN)
      feature.add("Embedded Flash");
    if (chip_info.features & CHIP_FEATURE_EMB_FLASH) feature.add("2.4Ghz WIFI");
    if (chip_info.features & CHIP_FEATURE_BLE) feature.add("Bluetooth LE");
    if (chip_info.features & CHIP_FEATURE_BT) feature.add("Bluetooth Classic");
    if (chip_info.features & CHIP_FEATURE_IEEE802154)
      feature.add("IEEE 802.15.4/LR-WPAN");
    if (chip_info.features & CHIP_FEATURE_EMB_PSRAM)
      feature.add("Embedded PSRAM");

    switch (chip_info.model) {
      case CHIP_ESP32:
        cpu[PARAM_FAMILY] = "ESP32";
        break;
      case CHIP_ESP32S2:
        cpu[PARAM_FAMILY] = "ESP32S2";
        break;
      case CHIP_ESP32S3:
        cpu[PARAM_FAMILY] = "ESP32S3";
        break;
      case CHIP_ESP32C3:
        cpu[PARAM_FAMILY] = "ESP32C3";
        break;
      case CHIP_ESP32H2:
        cpu[PARAM_FAMILY] = "ESP32H2";
        break;
      default:
        cpu[PARAM_FAMILY] = String(chip_info.model);
        break;
    }
#endif

    serializeJson(obj, _hardwareScanData);
    Log.notice(F("WEB : Scan complete %s." CR), _hardwareScanData.c_str());
    _hardwareScanTask = false;
  }
}

// EOF
