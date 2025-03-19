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
#include <Wire.h>

#include <config.hpp>
#include <helper.hpp>
#include <history.hpp>
#include <main.hpp>
#include <perf.hpp>
#include <pushtarget.hpp>
#include <webserver.hpp>

#if !defined(ESP8266)
#include <esp_int_wdt.h>
#include <esp_task_wdt.h>
#endif

// #include <tempsensor.hpp>

extern bool sleepModeActive;
extern bool sleepModeAlwaysSkip;

BrewingWebServer::BrewingWebServer(WebConfig *config) : BaseWebServer(config) {}

void BrewingWebServer::webHandleConfigRead(AsyncWebServerRequest *request) {
  if (!isAuthenticated(request)) {
    return;
  }

  PERF_BEGIN("webserver-api-config-read");
  Log.notice(F("WEB : webServer callback for /api/config(read)." CR));
  AsyncJsonResponse *response = new AsyncJsonResponse(false);
  JsonObject obj = response->getRoot().as<JsonObject>();
  myConfig.createJson(obj);
  response->setLength();
  request->send(response);
  PERF_END("webserver-api-config-read");
}

void BrewingWebServer::webHandleConfigWrite(AsyncWebServerRequest *request,
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

  doWebConfigWrite();

  AsyncJsonResponse *response = new AsyncJsonResponse(false);
  obj = response->getRoot().as<JsonObject>();
  obj[PARAM_SUCCESS] = true;
  obj[PARAM_MESSAGE] = "Configuration updated";
  response->setLength();
  request->send(response);
  PERF_END("webserver-api-config-write");
}

void BrewingWebServer::webHandleCalibrate(AsyncWebServerRequest *request) {
  if (!isAuthenticated(request)) {
    return;
  }

  PERF_BEGIN("webserver-api-calibrate");
  Log.notice(F("WEB : webServer callback for /api/calibrate." CR));
  _sensorCalibrationTask = true;
  AsyncJsonResponse *response = new AsyncJsonResponse(false);
  JsonObject obj = response->getRoot().as<JsonObject>();
  obj[PARAM_SUCCESS] = true;
  obj[PARAM_MESSAGE] = "Scheduled device calibration";
  response->setLength();
  request->send(response);
  PERF_END("webserver-api-calibrate");
}

void BrewingWebServer::webHandleFactoryDefaults(
    AsyncWebServerRequest *request) {
  if (!isAuthenticated(request)) {
    return;
  }

  Log.notice(F("WEB : webServer callback for /api/factory." CR));
  myConfig.saveFileWifiOnly();
  LittleFS.remove(ERR_FILENAME);
  LittleFS.remove(RUNTIME_FILENAME);

  LittleFS.remove(TPL_GRAVITY_FNAME_POST);
  LittleFS.remove(TPL_GRAVITY_FNAME_POST2);
  LittleFS.remove(TPL_GRAVITY_FNAME_GET);
  LittleFS.remove(TPL_GRAVITY_FNAME_INFLUXDB);
  LittleFS.remove(TPL_GRAVITY_FNAME_MQTT);

  LittleFS.remove(TPL_PRESSURE_FNAME_POST);
  LittleFS.remove(TPL_PRESSURE_FNAME_POST2);
  LittleFS.remove(TPL_PRESSURE_FNAME_GET);
  LittleFS.remove(TPL_PRESSURE_FNAME_INFLUXDB);
  LittleFS.remove(TPL_PRESSURE_FNAME_MQTT);
  LittleFS.end();

  Log.notice(F("WEB : Deleted files in filesystem, rebooting." CR));
  AsyncJsonResponse *response = new AsyncJsonResponse(false);
  JsonObject obj = response->getRoot().as<JsonObject>();
  obj[PARAM_SUCCESS] = true;
  obj[PARAM_MESSAGE] = "Factory reset completed, rebooting";
  response->setLength();
  request->send(response);

  Log.notice(F("WEB : Scheduling reboot." CR));
  _rebootTimer = millis();
  _rebootTask = true;
}

void BrewingWebServer::webHandleSleepmode(AsyncWebServerRequest *request,
                                          JsonVariant &json) {
  if (!isAuthenticated(request)) {
    return;
  }

  PERF_BEGIN("webserver-api-sleepmode");
  Log.notice(F("WEB : webServer callback for /api/sleepmode." CR));
  JsonObject obj = json.as<JsonObject>();
  sleepModeAlwaysSkip = obj[PARAM_SLEEP_MODE].as<bool>();

  AsyncJsonResponse *response = new AsyncJsonResponse(false);
  obj = response->getRoot().as<JsonObject>();
  obj[PARAM_SLEEP_MODE] = sleepModeAlwaysSkip;
  response->setLength();
  request->send(response);
  PERF_END("webserver-api-sleepmode");
}

void BrewingWebServer::webHandleConfigFormatGravityWrite(
    AsyncWebServerRequest *request, JsonVariant &json) {
  if (!isAuthenticated(request)) {
    return;
  }

  PERF_BEGIN("webserver-api-config-format-gravity-write");
  Log.notice(
      F("WEB : webServer callback for /api/config/format(gravity-post)." CR));

  JsonObject obj = json.as<JsonObject>();
  int success = 0;

  if (!obj[PARAM_FORMAT_POST].isNull()) {
    success +=
        writeFile(TPL_GRAVITY_FNAME_POST, obj[PARAM_FORMAT_POST]) ? 1 : 0;
  }
  if (!obj[PARAM_FORMAT_POST2].isNull()) {
    success +=
        writeFile(TPL_GRAVITY_FNAME_POST2, obj[PARAM_FORMAT_POST2]) ? 1 : 0;
  }
  if (!obj[PARAM_FORMAT_GET].isNull()) {
    success += writeFile(TPL_GRAVITY_FNAME_GET, obj[PARAM_FORMAT_GET]) ? 1 : 0;
  }
  if (!obj[PARAM_FORMAT_INFLUXDB].isNull()) {
    success += writeFile(TPL_GRAVITY_FNAME_INFLUXDB, obj[PARAM_FORMAT_INFLUXDB])
                   ? 1
                   : 0;
  }
  if (!obj[PARAM_FORMAT_MQTT].isNull()) {
    success +=
        writeFile(TPL_GRAVITY_FNAME_MQTT, obj[PARAM_FORMAT_MQTT]) ? 1 : 0;
  }

  AsyncJsonResponse *response = new AsyncJsonResponse(false);
  obj = response->getRoot().as<JsonObject>();
  obj[PARAM_SUCCESS] = success > 0 ? true : false;
  obj[PARAM_MESSAGE] = success > 0 ? "Gravity template stored"
                                   : "Failed to store gravity template";
  response->setLength();
  request->send(response);
  PERF_END("webserver-api-config-format-gravity-write");
}

void BrewingWebServer::webHandleConfigFormatPressureWrite(
    AsyncWebServerRequest *request, JsonVariant &json) {
  if (!isAuthenticated(request)) {
    return;
  }

  PERF_BEGIN("webserver-api-config-format-pressure-write");
  Log.notice(
      F("WEB : webServer callback for /api/config/format(pressure-post)." CR));

  JsonObject obj = json.as<JsonObject>();
  int success = 0;

  if (!obj[PARAM_FORMAT_POST].isNull()) {
    success +=
        writeFile(TPL_PRESSURE_FNAME_POST, obj[PARAM_FORMAT_POST]) ? 1 : 0;
  }
  if (!obj[PARAM_FORMAT_POST2].isNull()) {
    success +=
        writeFile(TPL_PRESSURE_FNAME_POST2, obj[PARAM_FORMAT_POST2]) ? 1 : 0;
  }
  if (!obj[PARAM_FORMAT_GET].isNull()) {
    success += writeFile(TPL_PRESSURE_FNAME_GET, obj[PARAM_FORMAT_GET]) ? 1 : 0;
  }
  if (!obj[PARAM_FORMAT_INFLUXDB].isNull()) {
    success +=
        writeFile(TPL_PRESSURE_FNAME_INFLUXDB, obj[PARAM_FORMAT_INFLUXDB]) ? 1
                                                                           : 0;
  }
  if (!obj[PARAM_FORMAT_MQTT].isNull()) {
    success +=
        writeFile(TPL_PRESSURE_FNAME_MQTT, obj[PARAM_FORMAT_MQTT]) ? 1 : 0;
  }

  AsyncJsonResponse *response = new AsyncJsonResponse(false);
  obj = response->getRoot().as<JsonObject>();
  obj[PARAM_SUCCESS] = success > 0 ? true : false;
  obj[PARAM_MESSAGE] = success > 0 ? "Pressure template stored"
                                   : "Failed to store pressure template";
  response->setLength();
  request->send(response);
  PERF_END("webserver-api-config-format-pressure-write");
}

void BrewingWebServer::webHandleTestPush(AsyncWebServerRequest *request,
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
  AsyncJsonResponse *response = new AsyncJsonResponse(false);
  obj = response->getRoot().as<JsonObject>();
  obj[PARAM_SUCCESS] = true;
  obj[PARAM_MESSAGE] = "Scheduled test for " + _pushTestTarget;
  response->setLength();
  request->send(response);
  PERF_END("webserver-api-test-push");
}

void BrewingWebServer::webHandleTestPushStatus(AsyncWebServerRequest *request) {
  PERF_BEGIN("webserver-api-test-push-status");
  Log.notice(F("WEB : webServer callback for /api/test/push/status." CR));
  AsyncJsonResponse *response = new AsyncJsonResponse(false);
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

bool BrewingWebServer::writeFile(String fname, String data) {
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

void BrewingWebServer::webHandleHardwareScan(AsyncWebServerRequest *request) {
  if (!isAuthenticated(request)) {
    return;
  }

  Log.notice(F("WEB : webServer callback for /api/hardware." CR));
  _hardwareScanTask = true;
  _hardwareScanData = "";
  AsyncJsonResponse *response = new AsyncJsonResponse(false);
  JsonObject obj = response->getRoot().as<JsonObject>();
  obj[PARAM_SUCCESS] = true;
  obj[PARAM_MESSAGE] = "Scheduled hardware scanning";
  response->setLength();
  request->send(response);
}

void BrewingWebServer::webHandleHardwareScanStatus(
    AsyncWebServerRequest *request) {
  if (!isAuthenticated(request)) {
    return;
  }

  Log.notice(F("WEB : webServer callback for /api/hardware/status." CR));

  if (_hardwareScanTask || !_hardwareScanData.length()) {
    AsyncJsonResponse *response = new AsyncJsonResponse(false);
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

String BrewingWebServer::readFile(String fname) {
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

void BrewingWebServer::webHandleConfigFormatGravityRead(
    AsyncWebServerRequest *request) {
  if (!isAuthenticated(request)) {
    return;
  }

  PERF_BEGIN("webserver-api-config-format-gravity-read");
  Log.notice(
      F("WEB : webServer callback for /api/config/format(gravity-read)." CR));

  AsyncJsonResponse *response = new AsyncJsonResponse(false);
  JsonObject obj = response->getRoot().as<JsonObject>();
  String s;

  s = readFile(TPL_GRAVITY_FNAME_POST);
  obj[PARAM_FORMAT_POST] =
      s.length() ? urlencode(s) : urlencode(String(&iGravityHttpPostFormat[0]));
  s = readFile(TPL_GRAVITY_FNAME_POST2);
  obj[PARAM_FORMAT_POST2] =
      s.length() ? urlencode(s) : urlencode(String(&iGravityHttpPostFormat[0]));
  s = readFile(TPL_GRAVITY_FNAME_GET);
  obj[PARAM_FORMAT_GET] =
      s.length() ? urlencode(s) : urlencode(String(&iGravityHttpGetFormat[0]));
  s = readFile(TPL_GRAVITY_FNAME_INFLUXDB);
  obj[PARAM_FORMAT_INFLUXDB] =
      s.length() ? urlencode(s) : urlencode(String(&iGravityInfluxDbFormat[0]));
  s = readFile(TPL_GRAVITY_FNAME_MQTT);
  obj[PARAM_FORMAT_MQTT] =
      s.length() ? urlencode(s) : urlencode(String(&iGravityMqttFormat[0]));

  response->setLength();
  request->send(response);
  PERF_END("webserver-api-config-format-gravity-read");
}

void BrewingWebServer::webHandleConfigFormatPressureRead(
    AsyncWebServerRequest *request) {
  if (!isAuthenticated(request)) {
    return;
  }

  PERF_BEGIN("webserver-api-config-format-pressure-read");
  Log.notice(
      F("WEB : webServer callback for /api/config/format(pressure-read)." CR));

  AsyncJsonResponse *response = new AsyncJsonResponse(false);
  JsonObject obj = response->getRoot().as<JsonObject>();
  String s;

  s = readFile(TPL_PRESSURE_FNAME_POST);
  obj[PARAM_FORMAT_POST] = s.length()
                               ? urlencode(s)
                               : urlencode(String(&iPressureHttpPostFormat[0]));
  s = readFile(TPL_PRESSURE_FNAME_POST2);
  obj[PARAM_FORMAT_POST2] =
      s.length() ? urlencode(s)
                 : urlencode(String(&iPressureHttpPostFormat[0]));
  s = readFile(TPL_PRESSURE_FNAME_GET);
  obj[PARAM_FORMAT_GET] =
      s.length() ? urlencode(s) : urlencode(String(&iPressureHttpGetFormat[0]));
  s = readFile(TPL_PRESSURE_FNAME_INFLUXDB);
  obj[PARAM_FORMAT_INFLUXDB] =
      s.length() ? urlencode(s)
                 : urlencode(String(&iPressureInfluxDbFormat[0]));
  s = readFile(TPL_PRESSURE_FNAME_MQTT);
  obj[PARAM_FORMAT_MQTT] =
      s.length() ? urlencode(s) : urlencode(String(&iPressureMqttFormat[0]));

  response->setLength();
  request->send(response);
  PERF_END("webserver-api-config-format-pressure-read");
}

void BrewingWebServer::webHandleStatus(AsyncWebServerRequest *request) {
  PERF_BEGIN("webserver-api-status");
  Log.notice(F("WEB : webServer callback for /api/status(get)." CR));

  // Fallback since sometimes the loop() does not always run after firmware
  // update...
  if (_rebootTask) {
    Log.notice(F("WEB : Rebooting using fallback..." CR));
    delay(500);
    ESP_RESET();
  }

  AsyncJsonResponse *response = new AsyncJsonResponse(false);
  JsonObject obj = response->getRoot().as<JsonObject>();

  obj[PARAM_ID] = myConfig.getID();
  obj[PARAM_TEMP_FORMAT] = String(myConfig.getTempFormat());
  obj[PARAM_APP_VER] = CFG_APPVER;
  obj[PARAM_APP_BUILD] = CFG_GITREV;
  obj[PARAM_MDNS] = myConfig.getMDNS();

  obj[PARAM_SLEEP_MODE] = sleepModeAlwaysSkip;

#if defined(ESP8266)
  obj[PARAM_PLATFORM] = "esp8266";
#elif defined(ESP32C3)
  obj[PARAM_PLATFORM] = "esp32c3";
#elif defined(ESP32S2)
  obj[PARAM_PLATFORM] = "esp32s2";
#elif defined(ESP32S3)
  obj[PARAM_PLATFORM] = "esp32s3";
#elif defined(ESP32)
  obj[PARAM_PLATFORM] = "esp32";
#else  // esp32 miniz
#error "Unknown target platform";
#endif

#if defined(BOARD)
  obj[PARAM_BOARD] = BOARD;
  // #pragma message("BOARD is defined as: " BOARD)
#endif

  obj[PARAM_BATTERY] =
      serialized(String(myBatteryVoltage.getVoltage(), DECIMALS_BATTERY));
  obj[PARAM_RSSI] = WiFi.RSSI();
  obj[PARAM_SSID] = WiFi.SSID();

#if defined(ESP8266)
  obj[PARAM_TOTAL_HEAP] = 81920;
  obj[PARAM_FREE_HEAP] = ESP.getFreeHeap();
  obj[PARAM_IP] = WiFi.localIP().toString();
#else
  obj[PARAM_TOTAL_HEAP] = ESP.getHeapSize();
  obj[PARAM_FREE_HEAP] = ESP.getFreeHeap();
  obj[PARAM_IP] = WiFi.localIP().toString();
#endif
  obj[PARAM_WIFI_SETUP] = (runMode == RunMode::wifiSetupMode) ? true : false;

  obj[PARAM_RUNTIME_AVERAGE] = serialized(
      String(_averageRunTime ? _averageRunTime / 1000 : 0, DECIMALS_RUNTIME));

  float v = myBatteryVoltage.getVoltage();

  obj[PARAM_SELF][PARAM_SELF_BATTERY_LEVEL] = v < 3.2 || v > 5.1 ? false : true;
  obj[PARAM_SELF][PARAM_SELF_PUSH_TARGET] =
      myConfig.isBleActive() || myConfig.hasTargetHttpPost() ||
              myConfig.hasTargetHttpPost() || myConfig.hasTargetHttpGet() ||
              myConfig.hasTargetMqtt() || myConfig.hasTargetInfluxDb2()
          ? true
          : false;

  doWebStatus(obj);

  response->setLength();
  request->send(response);
  PERF_END("webserver-api-status");
}

void BrewingWebServer::webHandleCalibrateStatus(
    AsyncWebServerRequest *request) {
  if (!isAuthenticated(request)) {
    return;
  }

  PERF_BEGIN("webserver-api-calibrate-status");
  Log.notice(F("WEB : webServer callback for /api/calibrate/status." CR));
  AsyncJsonResponse *response = new AsyncJsonResponse(false);
  JsonObject obj = response->getRoot().as<JsonObject>();
  obj[PARAM_STATUS] = static_cast<bool>(_sensorCalibrationTask);
  obj[PARAM_SUCCESS] = false;
  obj[PARAM_MESSAGE] = "Calibration running";

  if (!_sensorCalibrationTask) {
    doWebCalibrateStatus(obj);
  }

  response->setLength();
  request->send(response);
  PERF_END("webserver-api-calibrate-status");
}

bool BrewingWebServer::setupWebServer(const char *serviceName) {
  Log.notice(F("WEB : Configuring web server." CR));

  BaseWebServer::setupWebServer();
  MDNS.addService(serviceName, "tcp", 80);

  HistoryLog runLog(RUNTIME_FILENAME);
  _averageRunTime = runLog.getAverage()._runTime;

  Log.notice(F("WEB : Setting up handlers for web server." CR));

  AsyncCallbackJsonWebHandler *handler;
  _server->on("/api/format2", HTTP_GET,
              std::bind(&BrewingWebServer::webHandleConfigFormatPressureRead,
                        this, std::placeholders::_1));
  handler = new AsyncCallbackJsonWebHandler(
      "/api/format2",
      std::bind(&BrewingWebServer::webHandleConfigFormatPressureWrite, this,
                std::placeholders::_1, std::placeholders::_2));
  _server->addHandler(handler);
  _server->on("/api/format", HTTP_GET,
              std::bind(&BrewingWebServer::webHandleConfigFormatGravityRead,
                        this, std::placeholders::_1));
  handler = new AsyncCallbackJsonWebHandler(
      "/api/format",
      std::bind(&BrewingWebServer::webHandleConfigFormatGravityWrite, this,
                std::placeholders::_1, std::placeholders::_2));
  _server->addHandler(handler);
  handler = new AsyncCallbackJsonWebHandler(
      "/api/sleepmode",
      std::bind(&BrewingWebServer::webHandleSleepmode, this,
                std::placeholders::_1, std::placeholders::_2));
  _server->addHandler(handler);
  _server->on("/api/config", HTTP_GET,
              std::bind(&BrewingWebServer::webHandleConfigRead, this,
                        std::placeholders::_1));
  handler = new AsyncCallbackJsonWebHandler(
      "/api/config", std::bind(&BrewingWebServer::webHandleConfigWrite, this,
                               std::placeholders::_1, std::placeholders::_2));
  _server->addHandler(handler);
  _server->on("/api/calibrate/status", HTTP_GET,
              std::bind(&BrewingWebServer::webHandleCalibrateStatus, this,
                        std::placeholders::_1));
  _server->on("/api/calibrate", HTTP_GET,
              std::bind(&BrewingWebServer::webHandleCalibrate, this,
                        std::placeholders::_1));
  _server->on("/api/hardware/status", HTTP_GET,
              std::bind(&BrewingWebServer::webHandleHardwareScanStatus, this,
                        std::placeholders::_1));
  _server->on("/api/hardware", HTTP_GET,
              std::bind(&BrewingWebServer::webHandleHardwareScan, this,
                        std::placeholders::_1));
  _server->on("/api/factory", HTTP_GET,
              std::bind(&BrewingWebServer::webHandleFactoryDefaults, this,
                        std::placeholders::_1));
  _server->on("/api/status", HTTP_GET,
              std::bind(&BrewingWebServer::webHandleStatus, this,
                        std::placeholders::_1));
  _server->on("/api/push/status", HTTP_GET,
              std::bind(&BrewingWebServer::webHandleTestPushStatus, this,
                        std::placeholders::_1));
  handler = new AsyncCallbackJsonWebHandler(
      "/api/push", std::bind(&BrewingWebServer::webHandleTestPush, this,
                             std::placeholders::_1, std::placeholders::_2));
  _server->addHandler(handler);

  Log.notice(F("WEB : Web server started." CR));
  return true;
}

void BrewingWebServer::loop() {
#if defined(ESP8266)
  MDNS.update();
#endif
  BaseWebServer::loop();

  if (_sensorCalibrationTask) {
    doTaskSensorCalibration();
    _sensorCalibrationTask = false;
  }

  if (_pushTestTask) {
    TemplatingEngine engine;
    BrewingPush push(&myConfig);

    doTaskPushTestSetup(engine, push);

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
    JsonDocument doc;
    JsonObject obj = doc.to<JsonObject>();
    obj[PARAM_STATUS] = false;
    obj[PARAM_SUCCESS] = true;
    obj[PARAM_MESSAGE] = "";
    Log.notice(F("WEB : Scanning hardware." CR));

    // Scan the i2c bus for devices, initialized in gyro.cpp
    JsonArray i2c = obj[PARAM_I2C].to<JsonArray>();

    for (int i = 1, j = 0; i < 128; i++) {
      // The i2c_scanner uses the return value of
      // the Write.endTransmisstion to see if
      // a device did acknowledge to the address.
      Wire.beginTransmission(i);
      int err = Wire.endTransmission();

      if (err == 0) {
        Log.notice(F("WEB : Found device at 0x%02X." CR), i);
        i2c[j][PARAM_ADRESS] = "0x" + String(i, 16);
        j++;
      }
    }

    JsonObject cpu = obj[PARAM_CHIP].to<JsonObject>();

#if defined(ESP8266)
    cpu[PARAM_FAMILY] = "ESP8266";
#else
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    cpu[PARAM_REVISION] = chip_info.revision;
    cpu[PARAM_CORES] = chip_info.cores;

    JsonArray feature = cpu[PARAM_FEATURES].to<JsonArray>();

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

    doTaskHardwareScanning(obj);

    serializeJson(obj, _hardwareScanData);
    Log.notice(F("WEB : Scan complete %s." CR), _hardwareScanData.c_str());
    _hardwareScanTask = false;
  }
}

// EOF
