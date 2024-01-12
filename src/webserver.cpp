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
#include <calc.hpp>
#include <config.hpp>
#include <gyro.hpp>
#include <helper.hpp>
#include <main.hpp>
#include <pushtarget.hpp>
#include <resources.hpp>
#include <templating.hpp>
#include <tempsensor.hpp>
#include <webserver.hpp>
#include <wifi.hpp>

#if defined(ACTIVATE_GCOV)
extern "C" {
#include <gcov_public.h>
}
#endif

WebServerHandler myWebServerHandler;  // My wrapper class fr webserver functions
extern bool sleepModeActive;
extern bool sleepModeAlwaysSkip;

bool WebServerHandler::isAuthenticated(AsyncWebServerRequest *request) {
  if (request->hasHeader("Authorization")) {
    String token("Bearer ");
    token += myConfig.getID();

    if (request->getHeader("Authorization")->value() == token) {
      Log.info(F("WEB : Auth token is valid." CR));
      return true;
    }
  }

  Log.info(
      F("WEB : No valid authorization header found, returning error 401." CR));
  AsyncWebServerResponse *response = request->beginResponse(401);
  request->send(response);
  return false;
}

void WebServerHandler::webHandleConfigRead(AsyncWebServerRequest *request) {
  if (!isAuthenticated(request)) {
    return;
  }

  LOG_PERF_START("webserver-api-config-read");
  Log.notice(F("WEB : webServer callback for /api/config(read)." CR));
  AsyncJsonResponse *response =
      new AsyncJsonResponse(false, JSON_BUFFER_SIZE_LARGE);
  JsonObject obj = response->getRoot().as<JsonObject>();
  myConfig.createJson(obj);
  obj.remove(PARAM_PASS);  // dont show the wifi password
  obj.remove(PARAM_PASS2);
  response->setLength();
  request->send(response);
  LOG_PERF_STOP("webserver-api-config-read");
}

void WebServerHandler::webHandleConfigWrite(AsyncWebServerRequest *request,
                                            JsonVariant &json) {
  if (!isAuthenticated(request)) {
    return;
  }

  LOG_PERF_START("webserver-api-config-write");
  Log.notice(F("WEB : webServer callback for /api/config(write)." CR));
  JsonObject obj = json.as<JsonObject>();
  obj.remove(PARAM_SSID);  // wifi credentials are managed in separate api
  obj.remove(PARAM_SSID2);
  obj.remove(PARAM_PASS);
  obj.remove(PARAM_PASS2);
  myConfig.parseJson(obj);
  obj.clear();
  myConfig.saveFile();

  AsyncJsonResponse *response =
      new AsyncJsonResponse(false, JSON_BUFFER_SIZE_LARGE);
  obj = response->getRoot().as<JsonObject>();
  myConfig.createJson(obj);
  obj.remove(PARAM_PASS);  // dont show the wifi password
  obj.remove(PARAM_PASS2);
  response->setLength();
  request->send(response);
  LOG_PERF_STOP("webserver-api-config-write");
}

void WebServerHandler::webReturnOK(AsyncWebServerRequest *request) {
  _rebootTask = !Update.hasError();
  Log.notice(
      F("WEB : Upload completed closing session, return=%d, success=%s." CR),
      _uploadReturn, _rebootTask ? "Yes" : "No");
  AsyncWebServerResponse *response = request->beginResponse(
      _uploadReturn, "text/plain", _rebootTask ? "SUCCESS" : "ERROR");
  response->addHeader("Connection", "close");
  request->send(response);
}

void WebServerHandler::webHandleUploadFile(AsyncWebServerRequest *request,
                                           String filename, size_t index,
                                           uint8_t *data, size_t len,
                                           bool final) {
  uint32_t maxSketchSpace = MAX_SKETCH_SPACE;
  Log.verbose(F("WEB : BaseWebHandler callback for /api/upload(post)." CR));

  if (!index) {
    _uploadedSize = 0;
#if defined(ESP8266)
    Update.runAsync(true);
#endif
    if (!Update.begin(request->contentLength(), U_FLASH)) {
      _uploadReturn = 500;
      Log.error(F("WEB : Not enough space to store for this firmware." CR));
    } else {
      _uploadReturn = 200;
      Log.notice(
          F("WEB : Start firmware upload, max sketch size %d kb, size %d." CR),
          maxSketchSpace / 1024, request->contentLength());
    }
  }

  _uploadedSize += len;

  if (_uploadedSize > maxSketchSpace) {
    _uploadReturn = 500;
    Log.error(F("WEB : Firmware file is to large." CR));
  } else if (Update.write(data, len) != len) {
    _uploadReturn = 500;
    Log.notice(F("WEB : Writing firmware upload %d (%d)." CR), len,
               maxSketchSpace);
  } else {
    EspSerial.print(".");
  }

  if (final) {
    EspSerial.print("\n");
    Log.notice(F("WEB : Finished firmware upload." CR));
    request->send(200);

    if (Update.end(true)) {
      // Calling reset here will not wait for all the data to be sent, lets wait
      // a second before rebooting in loop.
    } else {
      Log.error(F("WEB : Failed to finish firmware flashing, error %d" CR),
                Update.getError());
      _uploadReturn = 500;
    }
  }
}

void WebServerHandler::webHandleCalibrate(AsyncWebServerRequest *request) {
  if (!isAuthenticated(request)) {
    return;
  }

  LOG_PERF_START("webserver-api-calibrate");
  Log.notice(F("WEB : webServer callback for /api/calibrate." CR));
  _sensorCalibrationTask = true;
  AsyncJsonResponse *response =
      new AsyncJsonResponse(false, JSON_BUFFER_SIZE_SMALL);
  JsonObject obj = response->getRoot().as<JsonObject>();
  obj[PARAM_STATUS] = true;
  obj[PARAM_MESSAGE] = "Scheduled device calibration";
  response->setLength();
  request->send(response);
  LOG_PERF_STOP("webserver-api-calibrate");
}

void WebServerHandler::webHandleCalibrateStatus(
    AsyncWebServerRequest *request) {
  LOG_PERF_START("webserver-api-calibrate-status");
  Log.notice(F("WEB : webServer callback for /api/calibrate/status." CR));
  AsyncJsonResponse *response =
      new AsyncJsonResponse(false, JSON_BUFFER_SIZE_SMALL);
  JsonObject obj = response->getRoot().as<JsonObject>();
  obj[PARAM_STATUS] = static_cast<bool>(_sensorCalibrationTask);
  obj[PARAM_MESSAGE] = "";
  response->setLength();
  request->send(response);
  LOG_PERF_STOP("webserver-api-calibrate-status");
}

void WebServerHandler::webHandleFactoryDefaults(
    AsyncWebServerRequest *request) {
  if (!isAuthenticated(request)) {
    return;
  }

  Log.notice(F("WEB : webServer callback for /api/factory." CR));
  LittleFS.remove(CFG_FILENAME);
  LittleFS.remove(CFG_FILENAME);
  LittleFS.remove(ERR_FILENAME);
  LittleFS.remove(RUNTIME_FILENAME);
  LittleFS.remove(TPL_FNAME_HTTP1);
  LittleFS.remove(TPL_FNAME_HTTP2);
  LittleFS.remove(TPL_FNAME_INFLUXDB);
  LittleFS.remove(TPL_FNAME_MQTT);
  LittleFS.end();
  Log.notice(F("WEB : Deleted files in filesystem, rebooting." CR));

  AsyncJsonResponse *response =
      new AsyncJsonResponse(false, JSON_BUFFER_SIZE_SMALL);
  JsonObject obj = response->getRoot().as<JsonObject>();
  obj[PARAM_STATUS] = true;
  obj[PARAM_MESSAGE] = "Factory reset completed, rebooting";
  response->setLength();
  request->send(response);
  delay(500);
  ESP_RESET();
}

void WebServerHandler::webHandleLogClear(AsyncWebServerRequest *request) {
  if (!isAuthenticated(request)) {
    return;
  }

  LOG_PERF_START("webserver-api-log-clear");
  Log.notice(F("WEB : webServer callback for /api/log/clear." CR));
  LittleFS.remove(ERR_FILENAME);
  LittleFS.remove(ERR_FILENAME2);

  AsyncJsonResponse *response =
      new AsyncJsonResponse(false, JSON_BUFFER_SIZE_SMALL);
  JsonObject obj = response->getRoot().as<JsonObject>();
  obj[PARAM_STATUS] = true;
  obj[PARAM_MESSAGE] = "Logfiles removed";
  response->setLength();
  request->send(response);
  LOG_PERF_STOP("webserver-api-log-clear");

#if defined(ACTIVATE_GCOV)
  __gcov_exit();
#endif
}

void WebServerHandler::webHandleStatus(AsyncWebServerRequest *request) {
  LOG_PERF_START("webserver-api-status");
  Log.notice(F("WEB : webServer callback for /api/status(get)." CR));

  // Fallback since sometimes the loop() does not always run after firmware
  // update...
  if (_rebootTask) {
    Log.notice(F("WEB : Rebooting using fallback..." CR));
    delay(500);
    ESP_RESET();
  }

  AsyncJsonResponse *response =
      new AsyncJsonResponse(false, JSON_BUFFER_SIZE_SMALL);
  JsonObject obj = response->getRoot().as<JsonObject>();

  double angle = 0;  // Indicate we have no valid gyro value

  if (myGyro.hasValue()) angle = myGyro.getAngle();

  double tempC = myTempSensor.getTempC();
  double gravity = calculateGravity(angle, tempC);

  obj[PARAM_ID] = myConfig.getID();
  obj[PARAM_TEMPFORMAT] = String(myConfig.getTempFormat());
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

  if (myConfig.isTempC()) {
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
#else
  obj[PARAM_ISPINDEL_CONFIG] = false;
#endif

  obj[PARAM_TOTAL_HEAP] = ESP.getHeapSize();
  obj[PARAM_FREE_HEAP] = ESP.getFreeHeap();
  obj[PARAM_IP] = WiFi.localIP().toString();

  FloatHistoryLog runLog(RUNTIME_FILENAME);
  obj[PARAM_RUNTIME_AVERAGE] = serialized(String(
      runLog.getAverage() ? runLog.getAverage() / 1000 : 0, DECIMALS_RUNTIME));

  JsonObject self = obj.createNestedObject(PARAM_SELF);
  float v = myBatteryVoltage.getVoltage();

#if defined(ESP32LITE)
  self[PARAM_SELF_BATTERY_LEVEL] = true;  // No hardware support for these
  self[PARAM_SELF_TEMP_CONNECTED] = true;
#else
  self[PARAM_SELF_BATTERY_LEVEL] = v < 3.0 || v > 4.4 ? false : true;
  self[PARAM_SELF_TEMP_CONNECTED] = myTempSensor.isSensorAttached();
#endif
  self[PARAM_SELF_GRAVITY_FORMULA] =
      strlen(myConfig.getGravityFormula()) > 0 ? true : false;
  self[PARAM_SELF_GYRO_CALIBRATION] = myConfig.hasGyroCalibration();
  self[PARAM_SELF_GYRO_CONNECTED] = myGyro.isConnected();
  self[PARAM_SELF_PUSH_TARGET] =
      myConfig.isBleActive() || myConfig.isHttpActive() ||
              myConfig.isHttp2Active() || myConfig.isHttp3Active() ||
              myConfig.isMqttActive() || myConfig.isInfluxDb2Active()
          ? true
          : false;

  response->setLength();
  request->send(response);
  LOG_PERF_STOP("webserver-api-status");
}

void WebServerHandler::webHandleClearWifi(AsyncWebServerRequest *request) {
  if (!isAuthenticated(request)) {
    return;
  }

  Log.notice(F("WEB : webServer callback for /api/wifi/clear." CR));

  AsyncJsonResponse *response =
      new AsyncJsonResponse(false, JSON_BUFFER_SIZE_SMALL);
  JsonObject obj = response->getRoot().as<JsonObject>();
  obj[PARAM_STATUS] = true;
  obj[PARAM_MESSAGE] = "Clearing WIFI credentials and doing reset";
  response->setLength();
  request->send(response);
  myConfig.setWifiPass("", 0);
  myConfig.setWifiSSID("", 0);
  myConfig.setWifiPass("", 1);
  myConfig.setWifiSSID("", 1);
  myConfig.saveFile();
  delay(1000);
  WiFi.disconnect();  // Clear credentials
  ESP_RESET();
}

void WebServerHandler::webHandleRestart(AsyncWebServerRequest *request) {
  if (!isAuthenticated(request)) {
    return;
  }

  Log.notice(F("WEB : webServer callback for /api/restart." CR));
  AsyncJsonResponse *response =
      new AsyncJsonResponse(false, JSON_BUFFER_SIZE_SMALL);
  JsonObject obj = response->getRoot().as<JsonObject>();
  obj[PARAM_STATUS] = true;
  obj[PARAM_MESSAGE] = "Restarting...";
  response->setLength();
  request->send(response);

  delay(1000);
  ESP_RESET();
}

void WebServerHandler::webHandleSleepmode(AsyncWebServerRequest *request,
                                          JsonVariant &json) {
  if (!isAuthenticated(request)) {
    return;
  }

  LOG_PERF_START("webserver-api-config-sleepmode");
  Log.notice(F("WEB : webServer callback for /api/config/sleepmode." CR));
  JsonObject obj = json.as<JsonObject>();
  sleepModeAlwaysSkip = obj[PARAM_SLEEP_MODE].as<bool>();

  AsyncJsonResponse *response =
      new AsyncJsonResponse(false, JSON_BUFFER_SIZE_SMALL);
  obj = response->getRoot().as<JsonObject>();
  obj[PARAM_SLEEP_MODE] = sleepModeAlwaysSkip;
  response->setLength();
  request->send(response);
  LOG_PERF_STOP("webserver-api-config-sleepmode");
}

void WebServerHandler::webHandleConfigWifi(AsyncWebServerRequest *request,
                                           JsonVariant &json) {
  if (!isAuthenticated(request)) {
    return;
  }

  LOG_PERF_START("webserver-api-config-wifi");
  Log.notice(F("WEB : webServer callback for /api/config/wifi." CR));
  JsonObject obj = json.as<JsonObject>();

  if (!obj[PARAM_SSID].isNull()) myConfig.setWifiSSID(obj[PARAM_SSID], 0);
  if (!obj[PARAM_SSID2].isNull()) myConfig.setWifiSSID(obj[PARAM_SSID2], 1);
  if (!obj[PARAM_PASS].isNull()) myConfig.setWifiPass(obj[PARAM_PASS], 0);
  if (!obj[PARAM_PASS2].isNull()) myConfig.setWifiPass(obj[PARAM_PASS2], 1);

  AsyncJsonResponse *response =
      new AsyncJsonResponse(false, JSON_BUFFER_SIZE_SMALL);
  obj = response->getRoot().as<JsonObject>();
  obj[PARAM_STATUS] = true;
  obj[PARAM_MESSAGE] = "Wifi settings updated.";
  response->setLength();
  request->send(response);
  LOG_PERF_STOP("webserver-api-config-wifi");
}

void WebServerHandler::webHandleFormulaCreate(AsyncWebServerRequest *request) {
  if (!isAuthenticated(request)) {
    return;
  }

  LOG_PERF_START("webserver-api-formula-create");
  Log.notice(F("WEB : webServer callback for /api/formula." CR));

  int e, createErr;
  char buf[100];
  RawFormulaData fd = myConfig.getFormulaData();

  e = createFormula(fd, &buf[0], sizeof(buf), 2);

  if (e) {
    // If we fail with order=2 try with 3
    Log.warning(F("WEB : Failed to find formula with order 3." CR), e);
    e = createFormula(fd, &buf[0], sizeof(buf), 3);
  }

  if (e) {
    // If we fail with order=3 try with 4
    Log.warning(F("WEB : Failed to find formula with order 4." CR), e);
    e = createFormula(fd, &buf[0], sizeof(buf), 4);
  }

  if (e) {
    // If we fail with order=4 then we mark it as failed
    Log.error(
        F("WEB : Unable to find formula based on provided values err=%d." CR),
        e);
    createErr = e;
  } else {
    // Save the formula as succesful
    Log.info(F("WEB : Found valid formula: '%s'" CR), &buf[0]);
    myConfig.setGravityFormula(buf);
    myConfig.saveFile();
    createErr = 0;
  }

  AsyncJsonResponse *response =
      new AsyncJsonResponse(false, JSON_BUFFER_SIZE_SMALL);
  JsonObject obj = response->getRoot().as<JsonObject>();

  obj[PARAM_STATUS] = createErr ? false : true;
  obj[PARAM_ANGLE] = serialized(String(myGyro.getAngle(), DECIMALS_TILT));
  obj[PARAM_GRAVITY_FORMAT] = String(myConfig.getGravityFormat());
  obj[PARAM_GRAVITY_FORMULA] = "";

  switch (createErr) {
    case ERR_FORMULA_INTERNAL:
      obj[PARAM_MESSAGE] = "Internal error creating formula.";
      break;
    case ERR_FORMULA_NOTENOUGHVALUES:
      obj[PARAM_MESSAGE] =
          "Not enough values to create formula, need at least 3 angles.";
      break;
    case ERR_FORMULA_UNABLETOFFIND:
      obj[PARAM_MESSAGE] =
          "Unable to find an accurate formula based on input, check error log "
          "and graph below.";
      break;
    default:
      obj[PARAM_GRAVITY_FORMULA] = myConfig.getGravityFormula();
      break;
  }

  response->setLength();
  request->send(response);
  LOG_PERF_STOP("webserver-api-formula-create");
}

void WebServerHandler::webHandleConfigFormatWrite(
    AsyncWebServerRequest *request, JsonVariant &json) {
  if (!isAuthenticated(request)) {
    return;
  }

  LOG_PERF_START("webserver-api-config-format-write");
  Log.notice(F("WEB : webServer callback for /api/config/format(post)." CR));

  JsonObject obj = json.as<JsonObject>();
  int success = 0;

  if (!obj[PARAM_FORMAT_HTTP1].isNull()) {
    success += writeFile(TPL_FNAME_HTTP1, obj[PARAM_FORMAT_HTTP1]) ? 1 : 0;
  }
  if (!obj[PARAM_FORMAT_HTTP2].isNull()) {
    success += writeFile(TPL_FNAME_HTTP2, obj[PARAM_FORMAT_HTTP2]) ? 1 : 0;
  }
  if (!obj[PARAM_FORMAT_HTTP3].isNull()) {
    success += writeFile(TPL_FNAME_HTTP3, obj[PARAM_FORMAT_HTTP3]) ? 1 : 0;
  }
  if (!obj[PARAM_FORMAT_INFLUXDB].isNull()) {
    success +=
        writeFile(TPL_FNAME_INFLUXDB, obj[PARAM_FORMAT_INFLUXDB]) ? 1 : 0;
  }
  if (!obj[PARAM_FORMAT_MQTT].isNull()) {
    success += writeFile(TPL_FNAME_MQTT, obj[PARAM_FORMAT_MQTT]) ? 1 : 0;
  }

  AsyncJsonResponse *response =
      new AsyncJsonResponse(false, JSON_BUFFER_SIZE_SMALL);
  obj = response->getRoot().as<JsonObject>();
  obj[PARAM_STATUS] = success > 0 ? true : false;
  obj[PARAM_MESSAGE] = success > 0 ? "Format template stored"
                                   : "Failed to store format template";
  response->setLength();
  request->send(response);

  LOG_PERF_STOP("webserver-api-config-format-write");
}

void WebServerHandler::webHandleTestPush(AsyncWebServerRequest *request,
                                         JsonVariant &json) {
  if (!isAuthenticated(request)) {
    return;
  }

  LOG_PERF_START("webserver-api-test-push");
  Log.notice(F("WEB : webServer callback for /api/test/push." CR));
  JsonObject obj = json.as<JsonObject>();

  _pushTestData = obj[PARAM_PUSH_FORMAT].as<String>();
  _pushTestTask = true;

  AsyncJsonResponse *response =
      new AsyncJsonResponse(false, JSON_BUFFER_SIZE_SMALL);
  obj = response->getRoot().as<JsonObject>();
  obj[PARAM_STATUS] = true;
  obj[PARAM_MESSAGE] = "Scheduled test for " + _pushTestData;
  response->setLength();
  request->send(response);
  LOG_PERF_STOP("webserver-api-test-push");
}

void WebServerHandler::webHandleTestPushStatus(AsyncWebServerRequest *request) {
  LOG_PERF_START("webserver-api-test-push-status");
  Log.notice(F("WEB : webServer callback for /api/test/push/status." CR));
  AsyncJsonResponse *response =
      new AsyncJsonResponse(false, JSON_BUFFER_SIZE_SMALL);
  JsonObject obj = response->getRoot().as<JsonObject>();
  obj[PARAM_STATUS] = static_cast<bool>(_pushTestTask);
  obj[PARAM_MESSAGE] = "";
  response->setLength();
  request->send(response);
  LOG_PERF_STOP("webserver-api-test-push-status");
}

bool WebServerHandler::writeFile(String fname, String data) {
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

String WebServerHandler::readFile(String fname) {
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

void WebServerHandler::webHandleConfigFormatRead(
    AsyncWebServerRequest *request) {
  if (!isAuthenticated(request)) {
    return;
  }

  LOG_PERF_START("webserver-api-config-format-read");
  Log.notice(F("WEB : webServer callback for /api/config/format(read)." CR));

  AsyncJsonResponse *response =
      new AsyncJsonResponse(false, JSON_BUFFER_SIZE_XL);
  JsonObject obj = response->getRoot().as<JsonObject>();
  String s;

  s = readFile(TPL_FNAME_HTTP1);
  obj[PARAM_FORMAT_HTTP1] =
      s.length() ? urlencode(s) : urlencode(String(&iSpindleFormat[0]));
  s = readFile(TPL_FNAME_HTTP2);
  obj[PARAM_FORMAT_HTTP2] =
      s.length() ? urlencode(s) : urlencode(String(&iSpindleFormat[0]));
  s = readFile(TPL_FNAME_HTTP3);
  obj[PARAM_FORMAT_HTTP3] =
      s.length() ? urlencode(s) : urlencode(String(&iHttpGetFormat[0]));
  s = readFile(TPL_FNAME_INFLUXDB);
  obj[PARAM_FORMAT_INFLUXDB] =
      s.length() ? urlencode(s) : urlencode(String(&influxDbFormat[0]));
  s = readFile(TPL_FNAME_MQTT);
  obj[PARAM_FORMAT_MQTT] =
      s.length() ? urlencode(s) : urlencode(String(&mqttFormat[0]));

  response->setLength();
  request->send(response);
  LOG_PERF_STOP("webserver-api-config-format-read");
}

void WebServerHandler::webHandleMigrate(AsyncWebServerRequest *request) {
  if (!isAuthenticated(request)) {
    return;
  }

  LOG_PERF_START("webserver-api-migrate");
  Log.notice(F("WEB : webServer callback for /api/migrate." CR));

#if defined(ESP8266)
  LittleFS.rename("/config.json", "/ispindel.json");
#endif

  AsyncJsonResponse *response =
      new AsyncJsonResponse(false, JSON_BUFFER_SIZE_SMALL);
  JsonObject obj = response->getRoot().as<JsonObject>();
  obj[PARAM_STATUS] = true;
  obj[PARAM_MESSAGE] = "";
  response->setLength();
  request->send(response);
  LOG_PERF_STOP("webserver-api-migrate");
}

void WebServerHandler::webHandlePageNotFound(AsyncWebServerRequest *request) {
  if (request->method() == HTTP_OPTIONS) {
    Log.notice(F("WEB : Got OPTIONS request for %s." CR), request->url().c_str());
#if defined(ENABLE_REMOTE_UI_DEVELOPMENT)
    AsyncWebServerResponse *resp = request->beginResponse(200);
    resp->addHeader("Access-Control-Allow-Credentials", "true");
    resp->addHeader("Access-Control-Allow-Methods", "GET,HEAD,OPTIONS,POST,PUT");
    resp->addHeader("Access-Control-Allow-Headers", "Access-Control-Allow-Headers, Origin,Accept, X-Requested-With, Content-Type, Access-Control-Request-Method, Access-Control-Request-Headers, Authorization");
    request->send(resp);
    return;
#endif    
  }

  if (request->method() == HTTP_GET)
    Log.error(F("WEB : GET on %s not recognized." CR), request->url().c_str());
  else if (request->method() == HTTP_POST)
    Log.error(F("WEB : POST on %s not recognized." CR), request->url().c_str());
  else if (request->method() == HTTP_PUT)
    Log.error(F("WEB : PUT on %s not recognized." CR), request->url().c_str());
  else if (request->method() == HTTP_DELETE)
    Log.error(F("WEB : DELETE on %s not recognized." CR), request->url().c_str());
  else 
    Log.error(F("WEB : Unknown on %s not recognized." CR), request->url().c_str());

  request->send(404, "application/json", "{\"message\":\"Not found\"}");
}

bool WebServerHandler::setupWebServer() {
  Log.notice(F("WEB : Configuring web server." CR));

  _server = new AsyncWebServer(80);

  MDNS.begin(myConfig.getMDNS());
  MDNS.addService("http", "tcp", 80);
  MDNS.addService("gravitymon", "tcp", 80);

  // Show files in the filessytem at startup
#if defined(ESP8266)
  FSInfo fs;
  LittleFS.info(fs);
  Log.notice(F("WEB : File system Total=%d, Used=%d." CR), fs.totalBytes,
             fs.usedBytes);
  Dir dir = LittleFS.openDir("/");
  while (dir.next()) {
    Log.notice(F("WEB : File=%s, %d bytes" CR), dir.fileName().c_str(),
               dir.fileSize());
    if (!dir.fileSize()) {
      Log.notice(F("WEB : Empty file detected, removing file." CR));
      LittleFS.remove(dir.fileName().c_str());
    }
  }
#else  // ESP32
  File root = LittleFS.open("/");
  File f = root.openNextFile();
  while (f) {
    Log.notice(F("WEB : File=%s, %d bytes" CR), f.name(), f.size());
    if (!f.size()) {
      Log.notice(F("WEB : Empty file detected, removing file." CR));
      LittleFS.remove(f.name());
    }

    f = root.openNextFile();
  }
  f.close();
  root.close();
#endif
  // Static content
  Log.notice(F("WEB : Setting up handlers for web server." CR));
  /* TODO: REPLACE THSE WITH NEW VUEJS URLs
  _server->on("/", std::bind(&WebServerHandler::webReturnIndexHtm, this,
                             std::placeholders::_1));
  _server->on("/index.htm", std::bind(&WebServerHandler::webReturnIndexHtm,
                                      this, std::placeholders::_1));
  _server->on("/config.htm", std::bind(&WebServerHandler::webReturnConfigHtm,
                                       this, std::placeholders::_1));
  _server->on("/calibration.htm",
              std::bind(&WebServerHandler::webReturnCalibrationHtm, this,
                        std::placeholders::_1));
  _server->on("/format.htm", std::bind(&WebServerHandler::webReturnFormatHtm,
                                       this, std::placeholders::_1));
  _server->on("/about.htm", std::bind(&WebServerHandler::webReturnAboutHtm,
                                      this, std::placeholders::_1));
  _server->on("/test.htm", std::bind(&WebServerHandler::webReturnTestHtm, this,
                                     std::placeholders::_1));
  _server->on("/firmware.htm",
              std::bind(&WebServerHandler::webReturnFirmwareHtm, this,
                        std::placeholders::_1));
  _server->on("/backup.htm", std::bind(&WebServerHandler::webReturnBackupHtm,
                                       this, std::placeholders::_1));*/
  _server->serveStatic("/log", LittleFS, ERR_FILENAME);
  _server->serveStatic("/log2", LittleFS, ERR_FILENAME2);
  _server->serveStatic("/runtime", LittleFS, RUNTIME_FILENAME);
  _server->serveStatic("/migrate", LittleFS, "/config.json");

  _server->serveStatic("/debug1", LittleFS, "/gravitymon.json");
  _server->serveStatic("/debug2", LittleFS, "/gravitymon2.json");

  AsyncCallbackJsonWebHandler *handler;

  // Dynamic content
  _server->on("/api/clearlog", HTTP_GET,
              std::bind(&WebServerHandler::webHandleLogClear, this,
                        std::placeholders::_1));
  _server->on("/api/config/format", HTTP_GET,
              std::bind(&WebServerHandler::webHandleConfigFormatRead, this,
                        std::placeholders::_1));
  handler = new AsyncCallbackJsonWebHandler(
      "/api/config/format",
      std::bind(&WebServerHandler::webHandleConfigFormatWrite, this,
                std::placeholders::_1, std::placeholders::_2),
      JSON_BUFFER_SIZE_LARGE);
  _server->addHandler(handler);
  handler = new AsyncCallbackJsonWebHandler(
      "/api/config/wifi",
      std::bind(&WebServerHandler::webHandleConfigWifi, this,
                std::placeholders::_1, std::placeholders::_2),
      JSON_BUFFER_SIZE_SMALL);
  _server->addHandler(handler);

  handler = new AsyncCallbackJsonWebHandler(
      "/api/config/sleepmode",
      std::bind(&WebServerHandler::webHandleSleepmode, this,
                std::placeholders::_1, std::placeholders::_2),
      JSON_BUFFER_SIZE_SMALL);
  _server->addHandler(handler);
  handler = new AsyncCallbackJsonWebHandler(
      "/api/config",
      std::bind(&WebServerHandler::webHandleConfigWrite, this,
                std::placeholders::_1, std::placeholders::_2),
      JSON_BUFFER_SIZE_LARGE);
  _server->addHandler(handler);
  _server->on("/api/config", HTTP_GET,
              std::bind(&WebServerHandler::webHandleConfigRead, this,
                        std::placeholders::_1));
  _server->on("/api/formula", HTTP_GET,
              std::bind(&WebServerHandler::webHandleFormulaCreate, this,
                        std::placeholders::_1));
  _server->on("/api/calibrate/status", HTTP_GET,
              std::bind(&WebServerHandler::webHandleCalibrateStatus, this,
                        std::placeholders::_1));
  _server->on("/api/calibrate", HTTP_GET,
              std::bind(&WebServerHandler::webHandleCalibrate, this,
                        std::placeholders::_1));
  _server->on("/api/factory", HTTP_GET,
              std::bind(&WebServerHandler::webHandleFactoryDefaults, this,
                        std::placeholders::_1));
  _server->on("/api/status", HTTP_GET,
              std::bind(&WebServerHandler::webHandleStatus, this,
                        std::placeholders::_1));
  _server->on("/api/clearwifi", HTTP_GET,
              std::bind(&WebServerHandler::webHandleClearWifi, this,
                        std::placeholders::_1));
  _server->on("/api/restart", HTTP_GET,
              std::bind(&WebServerHandler::webHandleRestart, this,
                        std::placeholders::_1));
  _server->on("/api/migrate", HTTP_POST,
              std::bind(&WebServerHandler::webHandleMigrate, this,
                        std::placeholders::_1));
  _server->on(
      "/api/upload", HTTP_POST,
      std::bind(&WebServerHandler::webReturnOK, this, std::placeholders::_1),
      std::bind(&WebServerHandler::webHandleUploadFile, this,
                std::placeholders::_1, std::placeholders::_2,
                std::placeholders::_3, std::placeholders::_4,
                std::placeholders::_5, std::placeholders::_6));
  _server->on("/api/test/push/status", HTTP_GET,
              std::bind(&WebServerHandler::webHandleTestPushStatus, this,
                        std::placeholders::_1));
  handler = new AsyncCallbackJsonWebHandler(
      "/api/test/push",
      std::bind(&WebServerHandler::webHandleTestPush, this,
                std::placeholders::_1, std::placeholders::_2),
      JSON_BUFFER_SIZE_SMALL);
  _server->addHandler(handler);
  _server->onNotFound(std::bind(&WebServerHandler::webHandlePageNotFound, this,
                                std::placeholders::_1));

#if defined(ENABLE_REMOTE_UI_DEVELOPMENT)
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
#endif
  _server->begin();
  Log.notice(F("WEB : Web server started." CR));
  return true;
}

void WebServerHandler::loop() {
#if defined(ESP8266)
  MDNS.update();
#endif

  if (_rebootTask) {
    Log.notice(F("WEB : Rebooting..." CR));
    delay(500);
    ESP_RESET();
  }

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

    TemplatingEngine engine;
    engine.initialize(angle, gravitySG, corrGravitySG, tempC, 1.0,
                      myBatteryVoltage.getVoltage());

    PushTarget push;
    bool enabled = false;

    if (!_pushTestData.compareTo(PARAM_FORMAT_HTTP1) &&
        myConfig.isHttpActive()) {
      push.sendHttp1(engine, myConfig.isHttpSSL());
      enabled = true;
    } else if (!_pushTestData.compareTo(PARAM_FORMAT_HTTP2) &&
               myConfig.isHttp2Active()) {
      push.sendHttp2(engine, myConfig.isHttp2SSL());
      enabled = true;
    } else if (!_pushTestData.compareTo(PARAM_FORMAT_HTTP3) &&
               myConfig.isHttp3Active()) {
      push.sendHttp3(engine, myConfig.isHttp3SSL());
      enabled = true;
    } else if (!_pushTestData.compareTo(PARAM_FORMAT_INFLUXDB) &&
               myConfig.isInfluxDb2Active()) {
      push.sendInfluxDb2(engine, myConfig.isInfluxSSL());
      enabled = true;
    } else if (!_pushTestData.compareTo(PARAM_FORMAT_MQTT) &&
               myConfig.isMqttActive()) {
      push.sendMqtt(engine, myConfig.isMqttSSL(), false);
      enabled = true;
    }

    engine.freeMemory();
    DynamicJsonDocument doc(JSON_BUFFER_SIZE_SMALL);
    doc[PARAM_STATUS] = false;
    doc[PARAM_PUSH_ENABLED] = enabled;
    doc[PARAM_PUSH_SUCCESS] = push.getLastSuccess();
    doc[PARAM_PUSH_CODE] = push.getLastCode();

    _pushTestData.clear();
    serializeJson(doc, _pushTestData);
    doc.clear();

#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
    serializeJson(_pushTestData, EspSerial);
    EspSerial.print(CR);
#endif

    Log.notice(F("WEB : Scheduled push test result %s" CR),
               _pushTestData.c_str());
    _pushTestTask = false;
  }
}

// EOF
