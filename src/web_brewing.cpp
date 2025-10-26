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
#ifndef ESPFWK_DISABLE_WEBSERVER

#include <Wire.h>

#include <config_brewing.hpp>
#include <helper.hpp>
#include <main.hpp>
#include <perf.hpp>
#include <pushtarget.hpp>
#include <web_brewing.hpp>

#if !defined(ESP8266)
#include <esp_task_wdt.h>
#endif

#if !defined(ESP8266) && ESP_ARDUINO_VERSION_MAJOR == 2 // For Arduino Core 2.x
#include <esp_int_wdt.h>
#include <esp_system.h>
#endif

#if !defined(ESP8266) && ESP_ARDUINO_VERSION_MAJOR >= 3 // For Arduino Core 3.x
#include <esp_chip_info.h>
#include <esp_heap_caps.h>
#endif

extern bool sleepModeActive;
extern bool sleepModeAlwaysSkip;

BrewingWebServer::BrewingWebServer(BrewingConfig *config)
    : BaseWebServer(config) {
  _brewingConfig = config;
}

void BrewingWebServer::webHandleConfigRead(AsyncWebServerRequest *request) {
  if (!isAuthenticated(request)) {
    return;
  }

  PERF_BEGIN("webserver-api-config-read");
  Log.notice(F("WEB : webServer callback for /api/config(read)." CR));
  AsyncJsonResponse *response = new AsyncJsonResponse(false);
  JsonObject obj = response->getRoot().as<JsonObject>();
  _webConfig->createJson(obj);
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
  _webConfig->parseJson(obj);
  obj.clear();
  _webConfig->saveFile();
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
  _brewingConfig->saveFileWifiOnly();
  LittleFS.remove(ERR_FILENAME);

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

void BrewingWebServer::webHandleConfigFormatWrite(
    AsyncWebServerRequest *request, JsonVariant &json) {
  if (!isAuthenticated(request)) {
    return;
  }

  PERF_BEGIN("webserver-api-config-format-write");
  Log.notice(F("WEB : webServer callback for /api/config/format(write)." CR));

  JsonObject obj = json.as<JsonObject>();
  int success = 0;

  if (!obj[PARAM_FORMAT_POST_GRAVITY].isNull()) {
    success += writeFile(TPL_GRAVITY_FNAME_POST, obj[PARAM_FORMAT_POST_GRAVITY])
                   ? 1
                   : 0;
  }
  if (!obj[PARAM_FORMAT_POST2_GRAVITY].isNull()) {
    success +=
        writeFile(TPL_GRAVITY_FNAME_POST2, obj[PARAM_FORMAT_POST2_GRAVITY]) ? 1
                                                                            : 0;
  }
  if (!obj[PARAM_FORMAT_GET_GRAVITY].isNull()) {
    success +=
        writeFile(TPL_GRAVITY_FNAME_GET, obj[PARAM_FORMAT_GET_GRAVITY]) ? 1 : 0;
  }
  if (!obj[PARAM_FORMAT_INFLUXDB_GRAVITY].isNull()) {
    success += writeFile(TPL_GRAVITY_FNAME_INFLUXDB,
                         obj[PARAM_FORMAT_INFLUXDB_GRAVITY])
                   ? 1
                   : 0;
  }
  if (!obj[PARAM_FORMAT_MQTT_GRAVITY].isNull()) {
    success += writeFile(TPL_GRAVITY_FNAME_MQTT, obj[PARAM_FORMAT_MQTT_GRAVITY])
                   ? 1
                   : 0;
  }

  if (!obj[PARAM_FORMAT_POST_PRESSURE].isNull()) {
    success +=
        writeFile(TPL_PRESSURE_FNAME_POST, obj[PARAM_FORMAT_POST_PRESSURE]) ? 1
                                                                            : 0;
  }
  if (!obj[PARAM_FORMAT_POST2_PRESSURE].isNull()) {
    success +=
        writeFile(TPL_PRESSURE_FNAME_POST2, obj[PARAM_FORMAT_POST2_PRESSURE])
            ? 1
            : 0;
  }
  if (!obj[PARAM_FORMAT_GET_PRESSURE].isNull()) {
    success += writeFile(TPL_PRESSURE_FNAME_GET, obj[PARAM_FORMAT_GET_PRESSURE])
                   ? 1
                   : 0;
  }
  if (!obj[PARAM_FORMAT_INFLUXDB_PRESSURE].isNull()) {
    success += writeFile(TPL_PRESSURE_FNAME_INFLUXDB,
                         obj[PARAM_FORMAT_INFLUXDB_PRESSURE])
                   ? 1
                   : 0;
  }
  if (!obj[PARAM_FORMAT_MQTT_PRESSURE].isNull()) {
    success +=
        writeFile(TPL_PRESSURE_FNAME_MQTT, obj[PARAM_FORMAT_MQTT_PRESSURE]) ? 1
                                                                            : 0;
  }

  AsyncJsonResponse *response = new AsyncJsonResponse(false);
  obj = response->getRoot().as<JsonObject>();
  obj[PARAM_SUCCESS] = success > 0 ? true : false;
  obj[PARAM_MESSAGE] = success > 0 ? "Pressure template stored"
                                   : "Failed to store pressure template";
  response->setLength();
  request->send(response);
  PERF_END("webserver-api-config-format-write");
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
  else if (!_pushTestTask && _pushTestLastCode == 0)
    s = "No push test has been started";
  else if (!_pushTestTask && _pushTestLastCode < 0)
    s = "Push test has failed";
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
    size_t fileSize = file.size();
    
    char* buf = (char*)malloc(fileSize + 1);
    if (buf == nullptr) {
      Log.error(F("WEB : Failed to allocate %d bytes for file %s." CR), fileSize, fname.c_str());
      file.close();
      return "";
    }
    
    memset(buf, 0, fileSize + 1);
    size_t bytesRead = file.readBytes(buf, fileSize);
    file.close();
    
    if (bytesRead != fileSize) {
      Log.warning(F("WEB : Only read %d of %d bytes from %s." CR), bytesRead, fileSize, fname.c_str());
    }
    
    Log.notice(F("WEB : Read %d bytes from %s." CR), bytesRead, fname.c_str());
    String result(buf);
    free(buf);
    return result;
  }
  return "";
}

void BrewingWebServer::webHandleConfigFormatRead(
    AsyncWebServerRequest *request) {
  if (!isAuthenticated(request)) {
    return;
  }

  PERF_BEGIN("webserver-api-config-format-read");
  Log.notice(F("WEB : webServer callback for /api/config/format(read)." CR));

  AsyncJsonResponse *response = new AsyncJsonResponse(false);
  JsonObject obj = response->getRoot().as<JsonObject>();
  String s;

#if defined(GRAVITYMON) || defined(GATEWAY)
  s = readFile(TPL_GRAVITY_FNAME_POST);
  obj[PARAM_FORMAT_POST_GRAVITY] =
      s.length() ? urlencode(s) : urlencode(String(&iGravityHttpPostFormat[0]));
  s = readFile(TPL_GRAVITY_FNAME_POST2);
  obj[PARAM_FORMAT_POST2_GRAVITY] =
      s.length() ? urlencode(s) : urlencode(String(&iGravityHttpPostFormat[0]));
  s = readFile(TPL_GRAVITY_FNAME_GET);
  obj[PARAM_FORMAT_GET_GRAVITY] =
      s.length() ? urlencode(s) : urlencode(String(&iGravityHttpGetFormat[0]));
  s = readFile(TPL_GRAVITY_FNAME_INFLUXDB);
  obj[PARAM_FORMAT_INFLUXDB_GRAVITY] =
      s.length() ? urlencode(s) : urlencode(String(&iGravityInfluxDbFormat[0]));
  s = readFile(TPL_GRAVITY_FNAME_MQTT);
  obj[PARAM_FORMAT_MQTT_GRAVITY] =
      s.length() ? urlencode(s) : urlencode(String(&iGravityMqttFormat[0]));
#endif

#if defined(PRESSUREMON) || defined(GATEWAY)
  s = readFile(TPL_PRESSURE_FNAME_POST);
  obj[PARAM_FORMAT_POST_PRESSURE] =
      s.length() ? urlencode(s)
                 : urlencode(String(&iPressureHttpPostFormat[0]));
  s = readFile(TPL_PRESSURE_FNAME_POST2);
  obj[PARAM_FORMAT_POST2_PRESSURE] =
      s.length() ? urlencode(s)
                 : urlencode(String(&iPressureHttpPostFormat[0]));
  s = readFile(TPL_PRESSURE_FNAME_GET);
  obj[PARAM_FORMAT_GET_PRESSURE] =
      s.length() ? urlencode(s) : urlencode(String(&iPressureHttpGetFormat[0]));
  s = readFile(TPL_PRESSURE_FNAME_INFLUXDB);
  obj[PARAM_FORMAT_INFLUXDB_PRESSURE] =
      s.length() ? urlencode(s)
                 : urlencode(String(&iPressureInfluxDbFormat[0]));
  s = readFile(TPL_PRESSURE_FNAME_MQTT);
  obj[PARAM_FORMAT_MQTT_PRESSURE] =
      s.length() ? urlencode(s) : urlencode(String(&iPressureMqttFormat[0]));
#endif

  response->setLength();
  request->send(response);
  PERF_END("webserver-api-config-format-read");
}

void BrewingWebServer::webHandleFeature(AsyncWebServerRequest *request) {
  PERF_BEGIN("webserver-api-feature");
  Log.notice(F("WEB : webServer callback for /api/feature(get)." CR));

  AsyncJsonResponse *response = new AsyncJsonResponse(false);
  JsonObject obj = response->getRoot().as<JsonObject>();

  obj[PARAM_PLATFORM] = platform;
#if defined(BOARD)
  obj[PARAM_BOARD] = BOARD;
#else
  obj[PARAM_BOARD] = "UNDEFINED";
#endif
  obj[PARAM_APP_VER] = CFG_APPVER;
  obj[PARAM_APP_BUILD] = CFG_GITREV;

  doWebFeature(obj);

  response->setLength();
  request->send(response);
  PERF_END("webserver-api-feature");
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

  obj[PARAM_ID] = _webConfig->getID();
  obj[PARAM_TEMP_UNIT] = String(_brewingConfig->getTempUnit());
  obj[PARAM_MDNS] = _webConfig->getMDNS();

  obj[PARAM_SLEEP_MODE] = sleepModeAlwaysSkip;
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

  float v = myBatteryVoltage.getVoltage();

  obj[PARAM_SELF][PARAM_SELF_BATTERY_LEVEL] = v < 3.2 || v > 5.1 ? false : true;

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

  Log.notice(F("WEB : Setting up handlers for web server." CR));

  AsyncCallbackJsonWebHandler *handler;
  _server->on("/api/format", HTTP_GET,
              std::bind(&BrewingWebServer::webHandleConfigFormatRead, this,
                        std::placeholders::_1));
  handler = new AsyncCallbackJsonWebHandler(
      "/api/format",
      std::bind(&BrewingWebServer::webHandleConfigFormatWrite, this,
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
  _server->on("/api/feature", HTTP_GET,
              std::bind(&BrewingWebServer::webHandleFeature, this,
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
    BrewingPush push(_brewingConfig);

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

    for (int i = 1, j = 0; i < 128 && j < 32; i++) {  // Limit to max 32 devices
      // Abort if we get a timeout error (5)
      // The i2c_scanner uses the return value of
      // the Write.endTransmisstion to see if
      // a device did acknowledge to the address.
      Wire.beginTransmission(i);
      int err = Wire.endTransmission();

      // Log.notice(F("WEB : Scanning 0x%x, response %d." CR), i, err);
      char addr_str[8];
      snprintf(addr_str, sizeof(addr_str), "0x%x", i);

      if (err == 0) {
        Log.notice(F("WEB : Found device at %s." CR), addr_str);
        i2c[j][PARAM_ADRESS] = addr_str;
        i2c[j][PARAM_BUS] = "Wire";
        j++;
      } else if(err == 5) {
        Log.notice(F("WEB : Timeout error at %s." CR), addr_str);
        i2c[j][PARAM_ADRESS] = addr_str;
        i2c[j][PARAM_BUS] = "Timeout error, aborting scan";
        j++;
        break;
      } else {
        // Ignore the other errors, we are just scanning for devices
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
      feature.add("Embedded flash");
    if (chip_info.features & CHIP_FEATURE_WIFI_BGN) feature.add("2.4Ghz WIFI");
    if (chip_info.features & CHIP_FEATURE_BLE) feature.add("Bluetooth LE");
    if (chip_info.features & CHIP_FEATURE_BT) feature.add("Bluetooth Classic");
    if (chip_info.features & CHIP_FEATURE_IEEE802154)
      feature.add("IEEE 802.15.4/LR-WPAN");
    if (chip_info.features & CHIP_FEATURE_EMB_PSRAM)
      feature.add("Embedded PSRAM");
#if ESP_ARDUINO_VERSION_MAJOR >= 3
    if(esp_heap_caps_get_free_size(MALLOC_CAP_RTCRAM) > 0) {
#else
    if(heap_caps_get_free_size(MALLOC_CAP_RTCRAM) > 0) {
#endif
      feature.add("Embedded RTC RAM");
    }

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

#endif // ESPFWK_DISABLE_WEBSERVER

// EOF
