/*
MIT License

Copyright (c) 2021-2023 Magnus

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

void WebServerHandler::webHandleConfig(AsyncWebServerRequest *request) {
  LOG_PERF_START("webserver-api-config");
  Log.notice(F("WEB : webServer callback for /api/config(get)." CR));

  DynamicJsonDocument doc(2000);
  myConfig.createJson(doc);

  doc[PARAM_PASS] = "";  // dont show the wifi password
  doc[PARAM_PASS2] = "";

  doc[PARAM_APP_VER] = String(CFG_APPVER);
  doc[PARAM_APP_BUILD] = String(CFG_GITREV);

  double angle = 0;

  if (myGyro.hasValue()) angle = myGyro.getAngle();

  double tempC = myTempSensor.getTempC();
  double gravity = calculateGravity(angle, tempC);

  doc[PARAM_ANGLE] = serialized(String(angle, DECIMALS_TILT));
  doc[PARAM_GRAVITY_FORMAT] = String(myConfig.getGravityFormat());

  // Format the adjustment so we get rid of rounding errors
  if (myConfig.isTempF())
    // We want the delta value (32F = 0C).
    doc[PARAM_TEMP_ADJ] = serialized(
        String(convertCtoF(myConfig.getTempSensorAdjC()) - 32, DECIMALS_TEMP));
  else
    doc[PARAM_TEMP_ADJ] =
        serialized(String(myConfig.getTempSensorAdjC(), DECIMALS_TEMP));

  if (myConfig.isGravityTempAdj()) {
    gravity = gravityTemperatureCorrectionC(
        gravity, tempC, myAdvancedConfig.getDefaultCalibrationTemp());
  }

  if (myConfig.isGravityPlato()) {
    doc[PARAM_GRAVITY] =
        serialized(String(convertToPlato(gravity), DECIMALS_PLATO));
  } else {
    doc[PARAM_GRAVITY] = serialized(String(gravity, DECIMALS_SG));
  }

  doc[PARAM_BATTERY] =
      serialized(String(myBatteryVoltage.getVoltage(), DECIMALS_BATTERY));

  FloatHistoryLog runLog(RUNTIME_FILENAME);
  doc[PARAM_RUNTIME_AVERAGE] = serialized(String(
      runLog.getAverage() ? runLog.getAverage() / 1000 : 0, DECIMALS_RUNTIME));

#if defined(ESP8266)
  doc[PARAM_PLATFORM] = "esp8266";
#elif defined(ESP32C3)
  doc[PARAM_PLATFORM] = "esp32c3";
#elif defined(ESP32S2)
  doc[PARAM_PLATFORM] = "esp32s2";
#elif defined(ESP32S3)
  doc[PARAM_PLATFORM] = "esp32s3";
#elif defined(ESP32LITE)
  doc[PARAM_PLATFORM] = "esp32lite";
#else  // esp32 mini
  doc[PARAM_PLATFORM] = "esp32";
#endif

#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  serializeJson(doc, EspSerial);
  EspSerial.print(CR);
#endif

  String out;
  out.reserve(2000);
  serializeJson(doc, out);
  doc.clear();
  request->send(200, "application/json", out.c_str());
  LOG_PERF_STOP("webserver-api-config");
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
    if (!Update.begin(request->contentLength(), U_FLASH, PIN_LED)) {
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
  LOG_PERF_START("webserver-api-calibrate");
  String id = request->arg(PARAM_ID);
  Log.notice(F("WEB : webServer callback for /api/calibrate." CR));

  if (!id.equalsIgnoreCase(myConfig.getID())) {
    Log.error(F("WEB : Wrong ID received %s, expected %s" CR), id.c_str(),
              myConfig.getID());
    request->send(400, "text/plain", "Invalid ID.");
    LOG_PERF_STOP("webserver-api-calibrate");
    return;
  }

  _sensorCalibrationTask = true;
  request->send(200, "text/plain", "Device calibration scheduled");
  LOG_PERF_STOP("webserver-api-calibrate");
}

void WebServerHandler::webHandleCalibrateStatus(
    AsyncWebServerRequest *request) {
  LOG_PERF_START("webserver-api-calibrate-status");
  Log.notice(F("WEB : webServer callback for /api/calibrate." CR));

  DynamicJsonDocument doc(100);

  doc[PARAM_STATUS] = _sensorCalibrationTask;

#if LOG_LEVEL == 6
  serializeJson(doc, Serial);
  EspSerial.print(CR);
#endif

  String out;
  out.reserve(100);
  serializeJson(doc, out);
  doc.clear();

  request->send(200, "application/json", out.c_str());
  LOG_PERF_STOP("webserver-api-calibrate-status");
}

void WebServerHandler::webHandleFactoryDefaults(
    AsyncWebServerRequest *request) {
  String id = request->arg(PARAM_ID);
  Log.notice(F("WEB : webServer callback for /api/factory." CR));

  if (!id.compareTo(myConfig.getID())) {
    request->send(200, "text/plain",
                  "Removing configuration and restarting...");
    LittleFS.remove(CFG_FILENAME);
    LittleFS.remove(CFG_HW_FILENAME);
    LittleFS.remove(ERR_FILENAME);
    LittleFS.remove(RUNTIME_FILENAME);
    LittleFS.remove(TPL_FNAME_HTTP1);
    LittleFS.remove(TPL_FNAME_HTTP2);
    LittleFS.remove(TPL_FNAME_INFLUXDB);
    LittleFS.remove(TPL_FNAME_MQTT);
    LittleFS.end();
    Log.notice(F("WEB : Deleted files in filesystem, rebooting." CR));
    request->send(200, "text/plain", "Resetting device.");
    delay(500);
    ESP_RESET();
  } else {
    request->send(400, "text/plain", "Unknown ID.");
  }
}

void WebServerHandler::webHandleLogClear(AsyncWebServerRequest *request) {
  String id = request->arg(PARAM_ID);
  Log.notice(F("WEB : webServer callback for /api/clearlog." CR));

  if (!id.compareTo(myConfig.getID())) {
    request->send(200, "text/plain", "Removing logfiles...");
    LittleFS.remove(ERR_FILENAME);
    LittleFS.remove(ERR_FILENAME2);
    request->send(200, "text/plain", "Logfiles cleared.");
  } else {
    request->send(400, "text/plain", "Unknown ID.");
  }
#if defined(ACTIVATE_GCOV)
  __gcov_exit();
#endif
}

void WebServerHandler::webHandleStatus(AsyncWebServerRequest *request) {
  LOG_PERF_START("webserver-api-status");
  Log.notice(F("WEB : webServer callback for /api/status(get)." CR));

  // This is a fallback since sometimes the loop() doesent run after firmware
  // update...
  if (_rebootTask) {
    Log.notice(F("WEB : Rebooting using fallback..." CR));
    delay(500);
    ESP_RESET();
  }

  DynamicJsonDocument doc(500);

  double angle = 0;  // Indicate we have no valid gyro value

  if (myGyro.hasValue()) angle = myGyro.getAngle();

  double tempC = myTempSensor.getTempC();
  double gravity = calculateGravity(angle, tempC);

  doc[PARAM_ID] = myConfig.getID();

  if (myGyro.isConnected()) {
    doc[PARAM_ANGLE] = serialized(String(angle, DECIMALS_TILT));
  } else {
    doc[PARAM_ANGLE] = -1;  // Indicate that there is no connection to gyro
  }

  if (myConfig.isGravityTempAdj()) {
    gravity = gravityTemperatureCorrectionC(
        gravity, tempC, myAdvancedConfig.getDefaultCalibrationTemp());
  }
  if (myConfig.isGravityPlato()) {
    doc[PARAM_GRAVITY] =
        serialized(String(convertToPlato(gravity), DECIMALS_PLATO));
  } else {
    doc[PARAM_GRAVITY] = serialized(String(gravity, DECIMALS_SG));
  }
  doc[PARAM_TEMP_C] = serialized(String(tempC, DECIMALS_TEMP));
  doc[PARAM_TEMP_F] = serialized(String(convertCtoF(tempC), DECIMALS_TEMP));
  doc[PARAM_BATTERY] =
      serialized(String(myBatteryVoltage.getVoltage(), DECIMALS_BATTERY));
  doc[PARAM_TEMPFORMAT] = String(myConfig.getTempFormat());
  doc[PARAM_GRAVITY_FORMAT] = String(myConfig.getGravityFormat());
  doc[PARAM_SLEEP_MODE] = sleepModeAlwaysSkip;
  doc[PARAM_RSSI] = WiFi.RSSI();
  doc[PARAM_SLEEP_INTERVAL] = myConfig.getSleepInterval();
  doc[PARAM_TOKEN] = myConfig.getToken();
  doc[PARAM_TOKEN2] = myConfig.getToken2();

  doc[PARAM_APP_VER] = CFG_APPVER;
  doc[PARAM_APP_BUILD] = CFG_GITREV;
  doc[PARAM_MDNS] = myConfig.getMDNS();
  doc[PARAM_SSID] = WiFi.SSID();

#if defined(ESP8266)
  doc[PARAM_ISPINDEL_CONFIG] = LittleFS.exists("/config.json");
#else
  doc[PARAM_ISPINDEL_CONFIG] = false;
#endif

  FloatHistoryLog runLog(RUNTIME_FILENAME);
  doc[PARAM_RUNTIME_AVERAGE] = serialized(String(
      runLog.getAverage() ? runLog.getAverage() / 1000 : 0, DECIMALS_RUNTIME));

#if defined(ESP8266)
  doc[PARAM_PLATFORM] = "esp8266";
#elif defined(ESP32C3)
  doc[PARAM_PLATFORM] = "esp32c3";
#elif defined(ESP32S2)
  doc[PARAM_PLATFORM] = "esp32s2";
#elif defined(ESP32S3)
  doc[PARAM_PLATFORM] = "esp32s3";
#elif defined(ESP32LITE)
  doc[PARAM_PLATFORM] = "esp32lite";
#else  // esp32 mini
  doc[PARAM_PLATFORM] = "esp32";
#endif

  JsonObject self = doc.createNestedObject(PARAM_SELF);
  float v = myBatteryVoltage.getVoltage();
#if defined(ESP32LITE)
  self[PARAM_SELF_BATTERY_LEVEL] = true;
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

#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  serializeJson(doc, EspSerial);
  EspSerial.print(CR);
#endif

  String out;
  out.reserve(500);
  serializeJson(doc, out);
  doc.clear();
  request->send(200, "application/json", out.c_str());
  LOG_PERF_STOP("webserver-api-status");
}

void WebServerHandler::webHandleClearWIFI(AsyncWebServerRequest *request) {
  String id = request->arg(PARAM_ID);
  Log.notice(F("WEB : webServer callback for /api/clearwifi." CR));

  if (!id.compareTo(myConfig.getID())) {
    request->send(200, "text/plain",
                  "Clearing WIFI credentials and doing reset...");
    myConfig.setWifiPass("", 0);
    myConfig.setWifiSSID("", 0);
    myConfig.setWifiPass("", 1);
    myConfig.setWifiSSID("", 1);
    myConfig.saveFile();
    delay(1000);
    WiFi.disconnect();  // Clear credentials
    ESP_RESET();
  } else {
    request->send(400, "text/plain", "Unknown ID.");
  }
}

void WebServerHandler::webHandleRestart(AsyncWebServerRequest *request) {
  Log.notice(F("WEB : webServer callback for /api/restart." CR));
  request->send(200, "text/plain", "Restarting...");
  delay(1000);
  ESP_RESET();
}

void WebServerHandler::webHandleStatusSleepmode(
    AsyncWebServerRequest *request) {
  LOG_PERF_START("webserver-api-sleepmode");
  String id = request->arg(PARAM_ID);
  Log.notice(F("WEB : webServer callback for /api/status/sleepmode(post)." CR));

  if (!id.equalsIgnoreCase(myConfig.getID())) {
    Log.error(F("WEB : Wrong ID received %s, expected %s" CR), id.c_str(),
              myConfig.getID());
    request->send(400, "text/plain", "Invalid ID.");
    LOG_PERF_STOP("webserver-api-sleepmode");
    return;
  }

#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  Log.verbose(F("WEB : %s." CR), getRequestArguments().c_str());
#endif

  if (request->arg(PARAM_SLEEP_MODE).equalsIgnoreCase("true") ||
      request->arg(PARAM_SLEEP_MODE).equalsIgnoreCase("on"))
    sleepModeAlwaysSkip = true;
  else
    sleepModeAlwaysSkip = false;
  request->send(200, "text/plain", "Sleep mode updated");
  LOG_PERF_STOP("webserver-api-sleepmode");
}

void WebServerHandler::webHandleConfigDevice(AsyncWebServerRequest *request) {
  LOG_PERF_START("webserver-api-config-device");
  String id = request->arg(PARAM_ID);
  Log.notice(F("WEB : webServer callback for /api/config/device(post)." CR));

  if (!id.equalsIgnoreCase(myConfig.getID())) {
    Log.error(F("WEB : Wrong ID received %s, expected %s" CR), id.c_str(),
              myConfig.getID());
    request->send(400, "text/plain", "Invalid ID.");
    LOG_PERF_STOP("webserver-api-config-device");
    return;
  }

#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  Log.verbose(F("WEB : %s." CR), getRequestArguments().c_str());
#endif

  if (request->hasArg(PARAM_MDNS))
    myConfig.setMDNS(request->arg(PARAM_MDNS).c_str());
  if (request->hasArg(PARAM_TEMPFORMAT))
    myConfig.setTempFormat(request->arg(PARAM_TEMPFORMAT).charAt(0));
  if (request->hasArg(PARAM_SLEEP_INTERVAL))
    myConfig.setSleepInterval(request->arg(PARAM_SLEEP_INTERVAL).c_str());
  myConfig.saveFile();

  AsyncWebServerResponse *response =
      request->beginResponse(302, "text/plain", "Device config updated");
  response->addHeader("Location", "/config.htm#collapseDevice");
  request->send(response);
  LOG_PERF_STOP("webserver-api-config-device");
}

void WebServerHandler::webHandleConfigPush(AsyncWebServerRequest *request) {
  LOG_PERF_START("webserver-api-config-push");
  String id = request->arg(PARAM_ID);
  Log.notice(F("WEB : webServer callback for /api/config/push(post)." CR));

  if (!id.equalsIgnoreCase(myConfig.getID())) {
    Log.error(F("WEB : Wrong ID received %s, expected %s" CR), id.c_str(),
              myConfig.getID());
    request->send(400, "text/plain", "Invalid ID.");
    LOG_PERF_STOP("webserver-api-config-push");
    return;
  }
#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  Log.verbose(F("WEB : %s." CR), getRequestArguments().c_str());
#endif

  if (request->hasArg(PARAM_TOKEN))
    myConfig.setToken(request->arg(PARAM_TOKEN).c_str());
  if (request->hasArg(PARAM_TOKEN2))
    myConfig.setToken2(request->arg(PARAM_TOKEN2).c_str());
  if (request->hasArg(PARAM_PUSH_HTTP))
    myConfig.setHttpUrl(request->arg(PARAM_PUSH_HTTP).c_str());
  if (request->hasArg(PARAM_PUSH_HTTP_H1))
    myConfig.setHttpHeader(request->arg(PARAM_PUSH_HTTP_H1).c_str(), 0);
  if (request->hasArg(PARAM_PUSH_HTTP_H2))
    myConfig.setHttpHeader(request->arg(PARAM_PUSH_HTTP_H2).c_str(), 1);
  if (request->hasArg(PARAM_PUSH_HTTP2))
    myConfig.setHttp2Url(request->arg(PARAM_PUSH_HTTP2).c_str());
  if (request->hasArg(PARAM_PUSH_HTTP2_H1))
    myConfig.setHttp2Header(request->arg(PARAM_PUSH_HTTP2_H1).c_str(), 0);
  if (request->hasArg(PARAM_PUSH_HTTP2_H2))
    myConfig.setHttp2Header(request->arg(PARAM_PUSH_HTTP2_H2).c_str(), 1);
  if (request->hasArg(PARAM_PUSH_HTTP3))
    myConfig.setHttp3Url(request->arg(PARAM_PUSH_HTTP3).c_str());
  if (request->hasArg(PARAM_PUSH_INFLUXDB2))
    myConfig.setInfluxDb2PushUrl(request->arg(PARAM_PUSH_INFLUXDB2).c_str());
  if (request->hasArg(PARAM_PUSH_INFLUXDB2_ORG))
    myConfig.setInfluxDb2PushOrg(
        request->arg(PARAM_PUSH_INFLUXDB2_ORG).c_str());
  if (request->hasArg(PARAM_PUSH_INFLUXDB2_BUCKET))
    myConfig.setInfluxDb2PushBucket(
        request->arg(PARAM_PUSH_INFLUXDB2_BUCKET).c_str());
  if (request->hasArg(PARAM_PUSH_INFLUXDB2_AUTH))
    myConfig.setInfluxDb2PushToken(
        request->arg(PARAM_PUSH_INFLUXDB2_AUTH).c_str());
  if (request->hasArg(PARAM_PUSH_MQTT))
    myConfig.setMqttUrl(request->arg(PARAM_PUSH_MQTT).c_str());
  if (request->hasArg(PARAM_PUSH_MQTT_PORT))
    myConfig.setMqttPort(request->arg(PARAM_PUSH_MQTT_PORT).c_str());
  if (request->hasArg(PARAM_PUSH_MQTT_USER))
    myConfig.setMqttUser(request->arg(PARAM_PUSH_MQTT_USER).c_str());
  if (request->hasArg(PARAM_PUSH_MQTT_PASS))
    myConfig.setMqttPass(request->arg(PARAM_PUSH_MQTT_PASS).c_str());
  myConfig.saveFile();
  String section("/config.htm#");
  section += request->arg("section");

  AsyncWebServerResponse *response =
      request->beginResponse(302, "text/plain", "Push config updated");
  response->addHeader("Location", section.c_str());
  request->send(response);
  LOG_PERF_STOP("webserver-api-config-push");
}

String WebServerHandler::getRequestArguments(AsyncWebServerRequest *request) {
  String debug;

  for (int i = 0; i < request->args(); i++) {
    if (!request->argName(i).equals(
            "plain")) {  // this contains all the arguments, we dont need that.
      if (debug.length()) debug += ", ";

      debug += request->argName(i);
      debug += "=";
      debug += request->arg(i);
    }
  }
  return debug;
}

void WebServerHandler::webHandleConfigGravity(AsyncWebServerRequest *request) {
  LOG_PERF_START("webserver-api-config-gravity");
  String id = request->arg(PARAM_ID);
  Log.notice(F("WEB : webServer callback for /api/config/gravity(post)." CR));

  if (!id.equalsIgnoreCase(myConfig.getID())) {
    Log.error(F("WEB : Wrong ID received %s, expected %s" CR), id.c_str(),
              myConfig.getID());
    request->send(400, "text/plain", "Invalid ID.");
    LOG_PERF_STOP("webserver-api-config-gravity");
    return;
  }

#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  Log.verbose(F("WEB : %s." CR), getRequestArguments().c_str());
#endif

  if (request->hasArg(PARAM_GRAVITY_FORMAT))
    myConfig.setGravityFormat(request->arg(PARAM_GRAVITY_FORMAT).charAt(0));
  if (request->hasArg(PARAM_GRAVITY_FORMULA))
    myConfig.setGravityFormula(request->arg(PARAM_GRAVITY_FORMULA).c_str());
  if (request->hasArg(PARAM_GRAVITY_TEMP_ADJ))
    myConfig.setGravityTempAdj(
        request->arg(PARAM_GRAVITY_TEMP_ADJ).equalsIgnoreCase("on") ? true
                                                                    : false);
  else
    myConfig.setGravityTempAdj(false);
  myConfig.saveFile();

  AsyncWebServerResponse *response =
      request->beginResponse(302, "text/plain", "Gravity config updated");
  response->addHeader("Location", "/config.htm#collapseGravity");
  request->send(response);
  LOG_PERF_STOP("webserver-api-config-gravity");
}

void WebServerHandler::webHandleConfigHardware(AsyncWebServerRequest *request) {
  LOG_PERF_START("webserver-api-config-hardware");
  String id = request->arg(PARAM_ID);
  Log.notice(F("WEB : webServer callback for /api/config/hardware(post)." CR));

  if (!id.equalsIgnoreCase(myConfig.getID())) {
    Log.error(F("WEB : Wrong ID received %s, expected %s" CR), id.c_str(),
              myConfig.getID());
    request->send(400, "text/plain", "Invalid ID.");
    LOG_PERF_STOP("webserver-api-config-hardware");
    return;
  }

#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  Log.verbose(F("WEB : %s." CR), getRequestArguments().c_str());
#endif

  if (request->hasArg(PARAM_VOLTAGE_FACTOR))
    myConfig.setVoltageFactor(request->arg(PARAM_VOLTAGE_FACTOR).toFloat());
  if (request->hasArg(PARAM_VOLTAGE_CONFIG))
    myConfig.setVoltageConfig(request->arg(PARAM_VOLTAGE_CONFIG).toFloat());
  if (request->hasArg(PARAM_TEMP_ADJ)) {
    if (myConfig.isTempC()) {
      myConfig.setTempSensorAdjC(request->arg(PARAM_TEMP_ADJ));
    } else {
      // Data is delta so we add 32 in order to convert to C.
      myConfig.setTempSensorAdjF(request->arg(PARAM_TEMP_ADJ), 32);
    }
  }
  if (request->hasArg(PARAM_BLE))
    myConfig.setBleColor(request->arg(PARAM_BLE).c_str());
  if (request->hasArg(PARAM_BLE_FORMAT))
    myConfig.setBleFormat(request->arg(PARAM_BLE_FORMAT).toInt());
  if (request->hasArg(PARAM_OTA))
    myConfig.setOtaURL(request->arg(PARAM_OTA).c_str());
  if (request->hasArg(PARAM_GYRO_TEMP))
    myConfig.setGyroTemp(
        request->arg(PARAM_GYRO_TEMP).equalsIgnoreCase("on") ? true : false);
  else
    myConfig.setGyroTemp(false);
  if (request->hasArg(PARAM_STORAGE_SLEEP))
    myConfig.setStorageSleep(
        request->arg(PARAM_STORAGE_SLEEP).equalsIgnoreCase("on") ? true
                                                                 : false);
  else
    myConfig.setStorageSleep(false);

  myConfig.saveFile();

  AsyncWebServerResponse *response =
      request->beginResponse(302, "text/plain", "Hardware config updated");
  response->addHeader("Location", "/config.htm#collapseHardware");
  request->send(response);
  LOG_PERF_STOP("webserver-api-config-hardware");
}

void WebServerHandler::webHandleConfigWifi(AsyncWebServerRequest *request) {
  LOG_PERF_START("webserver-api-config-wifi");
  String id = request->arg(PARAM_ID);
  Log.notice(F("WEB : webServer callback for /api/config/wifi(post)." CR));

  if (!id.equalsIgnoreCase(myConfig.getID())) {
    Log.error(F("WEB : Wrong ID received %s, expected %s" CR), id.c_str(),
              myConfig.getID());
    request->send(400, "text/plain", "Invalid ID.");
    LOG_PERF_STOP("webserver-api-config-wifi");
    return;
  }

#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  Log.verbose(F("WEB : %s." CR), getRequestArguments().c_str());
#endif

  if (request->hasArg(PARAM_SSID))
    myConfig.setWifiSSID(request->arg(PARAM_SSID), 0);
  if (request->hasArg(PARAM_SSID2))
    myConfig.setWifiSSID(request->arg(PARAM_SSID2), 1);
  if (request->hasArg(PARAM_PASS))
    myConfig.setWifiPass(request->arg(PARAM_PASS), 0);
  if (request->hasArg(PARAM_PASS2))
    myConfig.setWifiPass(request->arg(PARAM_PASS2), 1);

  Serial.println(myConfig.getWifiSSID(0));
  Serial.println(myConfig.getWifiSSID(1));
  Serial.println(myConfig.getWifiPass(0));
  Serial.println(myConfig.getWifiPass(1));
  myConfig.saveFile();

  AsyncWebServerResponse *response =
      request->beginResponse(302, "text/plain", "Device config updated");
  response->addHeader("Location", "/config.htm#collapseDevice");
  request->send(response);
  LOG_PERF_STOP("webserver-api-config-wifi");
}

void WebServerHandler::webHandleConfigAdvancedWrite(
    AsyncWebServerRequest *request) {
  LOG_PERF_START("webserver-api-config-advanced");
  String id = request->arg(PARAM_ID);
  Log.notice(F("WEB : webServer callback for /api/config/advaced(post)." CR));

  if (!id.equalsIgnoreCase(myConfig.getID())) {
    Log.error(F("WEB : Wrong ID received %s, expected %s" CR), id.c_str(),
              myConfig.getID());
    request->send(400, "text/plain", "Invalid ID.");
    LOG_PERF_STOP("webserver-api-config-advanced");
    return;
  }

#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  Log.verbose(F("WEB : %s." CR), getRequestArguments().c_str());
#endif

  if (request->hasArg(PARAM_HW_GYRO_READ_COUNT))
    myAdvancedConfig.setGyroReadCount(
        request->arg(PARAM_HW_GYRO_READ_COUNT).toInt());
  // if (request->hasArg(PARAM_HW_GYRO_READ_DELAY))
  //  myAdvancedConfig.setGyroReadDelay(
  //      request->arg(PARAM_HW_GYRO_READ_DELAY).toInt());
  if (request->hasArg(PARAM_HW_GYRO_MOVING_THREASHOLD))
    myAdvancedConfig.setGyroSensorMovingThreashold(
        request->arg(PARAM_HW_GYRO_MOVING_THREASHOLD).toInt());
  if (request->hasArg(PARAM_HW_FORMULA_DEVIATION))
    myAdvancedConfig.setMaxFormulaCreationDeviation(
        request->arg(PARAM_HW_FORMULA_DEVIATION).toFloat());
  if (request->hasArg(PARAM_HW_FORMULA_CALIBRATION_TEMP)) {
    float t = request->arg(PARAM_HW_FORMULA_CALIBRATION_TEMP).toFloat();
    if (myConfig.isTempF()) t = convertFtoC(t);
    myAdvancedConfig.SetDefaultCalibrationTemp(t);
  }
  if (request->hasArg(PARAM_HW_WIFI_PORTAL_TIMEOUT))
    myAdvancedConfig.setWifiPortalTimeout(
        request->arg(PARAM_HW_WIFI_PORTAL_TIMEOUT).toInt());
  if (request->hasArg(PARAM_HW_WIFI_CONNECT_TIMEOUT))
    myAdvancedConfig.setWifiConnectTimeout(
        request->arg(PARAM_HW_WIFI_CONNECT_TIMEOUT).toInt());
  if (request->hasArg(PARAM_HW_PUSH_TIMEOUT))
    myAdvancedConfig.setPushTimeout(
        request->arg(PARAM_HW_PUSH_TIMEOUT).toInt());
  if (request->hasArg(PARAM_HW_PUSH_INTERVAL_HTTP1))
    myAdvancedConfig.setPushIntervalHttp1(
        request->arg(PARAM_HW_PUSH_INTERVAL_HTTP1).toInt());
  if (request->hasArg(PARAM_HW_PUSH_INTERVAL_HTTP2))
    myAdvancedConfig.setPushIntervalHttp2(
        request->arg(PARAM_HW_PUSH_INTERVAL_HTTP2).toInt());
  if (request->hasArg(PARAM_HW_PUSH_INTERVAL_HTTP3))
    myAdvancedConfig.setPushIntervalHttp3(
        request->arg(PARAM_HW_PUSH_INTERVAL_HTTP3).toInt());
  if (request->hasArg(PARAM_HW_PUSH_INTERVAL_INFLUX))
    myAdvancedConfig.setPushIntervalInflux(
        request->arg(PARAM_HW_PUSH_INTERVAL_INFLUX).toInt());
  if (request->hasArg(PARAM_HW_PUSH_INTERVAL_MQTT))
    myAdvancedConfig.setPushIntervalMqtt(
        request->arg(PARAM_HW_PUSH_INTERVAL_MQTT).toInt());
  if (request->hasArg(PARAM_HW_TEMPSENSOR_RESOLUTION))
    myAdvancedConfig.setTempSensorResolution(
        request->arg(PARAM_HW_TEMPSENSOR_RESOLUTION).toInt());
  if (request->hasArg(PARAM_HW_IGNORE_LOW_ANGLES))
    myAdvancedConfig.setIgnoreLowAnges(
        request->arg(PARAM_HW_IGNORE_LOW_ANGLES).equalsIgnoreCase("on")
            ? true
            : false);
  else
    myAdvancedConfig.setIgnoreLowAnges(false);
  if (request->hasArg(PARAM_HW_BATTERY_SAVING))
    myAdvancedConfig.setBatterySaving(
        request->arg(PARAM_HW_BATTERY_SAVING).equalsIgnoreCase("on") ? true
                                                                     : false);
  else
    myAdvancedConfig.setBatterySaving(false);
  myAdvancedConfig.saveFile();

  AsyncWebServerResponse *response =
      request->beginResponse(302, "text/plain", "Advanced config updated");
  response->addHeader("Location", "/config.htm#collapseAdvanced");
  request->send(response);
  LOG_PERF_STOP("webserver-api-config-advanced");
}

void WebServerHandler::webHandleConfigAdvancedRead(
    AsyncWebServerRequest *request) {
  LOG_PERF_START("webserver-api-config-advanced");
  Log.notice(F("WEB : webServer callback for /api/config/advanced(get)." CR));

  DynamicJsonDocument doc(500);

  doc[PARAM_HW_GYRO_READ_COUNT] = myAdvancedConfig.getGyroReadCount();
  // doc[PARAM_HW_GYRO_READ_DELAY] = myAdvancedConfig.getGyroReadDelay();
  doc[PARAM_HW_GYRO_MOVING_THREASHOLD] =
      myAdvancedConfig.getGyroSensorMovingThreashold();
  doc[PARAM_HW_FORMULA_DEVIATION] = serialized(
      String(myAdvancedConfig.getMaxFormulaCreationDeviation(), DECIMALS_SG));
  doc[PARAM_HW_WIFI_PORTAL_TIMEOUT] = myAdvancedConfig.getWifiPortalTimeout();
  doc[PARAM_HW_WIFI_CONNECT_TIMEOUT] = myAdvancedConfig.getWifiConnectTimeout();
  doc[PARAM_HW_PUSH_TIMEOUT] = myAdvancedConfig.getPushTimeout();
  float t = myAdvancedConfig.getDefaultCalibrationTemp();

  if (myConfig.isTempC()) {
    doc[PARAM_HW_FORMULA_CALIBRATION_TEMP] =
        serialized(String(t, DECIMALS_TEMP));
  } else {
    doc[PARAM_HW_FORMULA_CALIBRATION_TEMP] =
        serialized(String(convertCtoF(t), DECIMALS_TEMP));
  }

  doc[PARAM_HW_PUSH_INTERVAL_HTTP1] = myAdvancedConfig.getPushIntervalHttp1();
  doc[PARAM_HW_PUSH_INTERVAL_HTTP2] = myAdvancedConfig.getPushIntervalHttp2();
  doc[PARAM_HW_PUSH_INTERVAL_HTTP3] = myAdvancedConfig.getPushIntervalHttp3();
  doc[PARAM_HW_PUSH_INTERVAL_INFLUX] = myAdvancedConfig.getPushIntervalInflux();
  doc[PARAM_HW_PUSH_INTERVAL_MQTT] = myAdvancedConfig.getPushIntervalMqtt();
  doc[PARAM_HW_TEMPSENSOR_RESOLUTION] =
      myAdvancedConfig.getTempSensorResolution();
  doc[PARAM_HW_IGNORE_LOW_ANGLES] = myAdvancedConfig.isIgnoreLowAnges();
  doc[PARAM_HW_BATTERY_SAVING] = myAdvancedConfig.isBatterySaving();

#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  serializeJson(doc, EspSerial);
  EspSerial.print(CR);
#endif

  String out;
  out.reserve(500);
  serializeJson(doc, out);
  doc.clear();
  request->send(200, "application/json", out.c_str());
  LOG_PERF_STOP("webserver-api-config-advanced");
}

void WebServerHandler::webHandleFormulaRead(AsyncWebServerRequest *request) {
  LOG_PERF_START("webserver-api-formula-read");
  Log.notice(F("WEB : webServer callback for /api/formula(get)." CR));

  DynamicJsonDocument doc(1000);
  const RawFormulaData &fd = myConfig.getFormulaData();

#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  Log.verbose(F("WEB : %s." CR), getRequestArguments().c_str());
#endif

  doc[PARAM_ID] = myConfig.getID();
  doc[PARAM_ANGLE] = serialized(String(myGyro.getAngle(), DECIMALS_TILT));
  doc[PARAM_GRAVITY_FORMAT] = String(myConfig.getGravityFormat());
  doc[PARAM_GRAVITY_FORMULA] = "";
  doc[PARAM_ERROR] = "";

  switch (_lastFormulaCreateError) {
    case ERR_FORMULA_INTERNAL:
      doc[PARAM_ERROR] = "Internal error creating formula.";
      break;
    case ERR_FORMULA_NOTENOUGHVALUES:
      doc[PARAM_ERROR] =
          "Not enough values to create formula, need at least 3 angles.";
      break;
    case ERR_FORMULA_UNABLETOFFIND:
      doc[PARAM_ERROR] =
          "Unable to find an accurate formula based on input, check error log "
          "and graph below.";
      break;
    default:
      doc[PARAM_GRAVITY_FORMULA] = myConfig.getGravityFormula();
      break;
  }

  doc["a1"] = serialized(String(fd.a[0], DECIMALS_TILT));
  doc["a2"] = serialized(String(fd.a[1], DECIMALS_TILT));
  doc["a3"] = serialized(String(fd.a[2], DECIMALS_TILT));
  doc["a4"] = serialized(String(fd.a[3], DECIMALS_TILT));
  doc["a5"] = serialized(String(fd.a[4], DECIMALS_TILT));
  doc["a6"] = serialized(String(fd.a[5], DECIMALS_TILT));
  doc["a7"] = serialized(String(fd.a[6], DECIMALS_TILT));
  doc["a8"] = serialized(String(fd.a[7], DECIMALS_TILT));
  doc["a9"] = serialized(String(fd.a[8], DECIMALS_TILT));
  doc["a10"] = serialized(String(fd.a[9], DECIMALS_TILT));

  if (myConfig.isGravityPlato()) {
    doc["g1"] = serialized(String(convertToPlato(fd.g[0]), DECIMALS_PLATO));
    doc["g2"] = serialized(String(convertToPlato(fd.g[1]), DECIMALS_PLATO));
    doc["g3"] = serialized(String(convertToPlato(fd.g[2]), DECIMALS_PLATO));
    doc["g4"] = serialized(String(convertToPlato(fd.g[3]), DECIMALS_PLATO));
    doc["g5"] = serialized(String(convertToPlato(fd.g[4]), DECIMALS_PLATO));
    doc["g6"] = serialized(String(convertToPlato(fd.g[5]), DECIMALS_PLATO));
    doc["g7"] = serialized(String(convertToPlato(fd.g[6]), DECIMALS_PLATO));
    doc["g8"] = serialized(String(convertToPlato(fd.g[7]), DECIMALS_PLATO));
    doc["g9"] = serialized(String(convertToPlato(fd.g[8]), DECIMALS_PLATO));
    doc["g10"] = serialized(String(convertToPlato(fd.g[9]), DECIMALS_PLATO));
  } else {
    doc["g1"] = serialized(String(fd.g[0], DECIMALS_SG));
    doc["g2"] = serialized(String(fd.g[1], DECIMALS_SG));
    doc["g3"] = serialized(String(fd.g[2], DECIMALS_SG));
    doc["g4"] = serialized(String(fd.g[3], DECIMALS_SG));
    doc["g5"] = serialized(String(fd.g[4], DECIMALS_SG));
    doc["g6"] = serialized(String(fd.g[5], DECIMALS_SG));
    doc["g7"] = serialized(String(fd.g[6], DECIMALS_SG));
    doc["g8"] = serialized(String(fd.g[7], DECIMALS_SG));
    doc["g9"] = serialized(String(fd.g[8], DECIMALS_SG));
    doc["g10"] = serialized(String(fd.g[9], DECIMALS_SG));
  }

#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  serializeJson(doc, EspSerial);
  EspSerial.print(CR);
#endif

  String out;
  out.reserve(100);
  serializeJson(doc, out);
  doc.clear();
  request->send(200, "application/json", out.c_str());
  LOG_PERF_STOP("webserver-api-formula-read");
}

void WebServerHandler::webHandleConfigFormatWrite(
    AsyncWebServerRequest *request) {
  LOG_PERF_START("webserver-api-config-format-write");
  String id = request->arg(PARAM_ID);
  Log.notice(F("WEB : webServer callback for /api/config/format(post)." CR));

  if (!id.equalsIgnoreCase(myConfig.getID())) {
    Log.error(F("WEB : Wrong ID received %s, expected %s" CR), id.c_str(),
              myConfig.getID());
    request->send(400, "text/plain", "Invalid ID.");
    LOG_PERF_STOP("webserver-api-config-format-write");
    return;
  }

#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  Log.verbose(F("WEB : %s." CR), getRequestArguments().c_str());
#endif
  bool success = false;

  // Only one option is posted so we done need to check them all.
  if (request->hasArg(PARAM_FORMAT_HTTP1)) {
    success = writeFile(TPL_FNAME_HTTP1, request->arg(PARAM_FORMAT_HTTP1));
  } else if (request->hasArg(PARAM_FORMAT_HTTP2)) {
    success = writeFile(TPL_FNAME_HTTP2, request->arg(PARAM_FORMAT_HTTP2));
  } else if (request->hasArg(PARAM_FORMAT_HTTP3)) {
    success = writeFile(TPL_FNAME_HTTP3, request->arg(PARAM_FORMAT_HTTP3));
  } else if (request->hasArg(PARAM_FORMAT_INFLUXDB)) {
    success =
        writeFile(TPL_FNAME_INFLUXDB, request->arg(PARAM_FORMAT_INFLUXDB));
  } else if (request->hasArg(PARAM_FORMAT_MQTT)) {
    success = writeFile(TPL_FNAME_MQTT, request->arg(PARAM_FORMAT_MQTT));
  }

  if (success) {
    AsyncWebServerResponse *response =
        request->beginResponse(302, "text/plain", "Format updated");
    response->addHeader("Location", "/format.htm");
    request->send(response);
  } else {
    writeErrorLog("WEB : Unable to store format file");
    request->send(400, "text/plain", "Unable to store format in file.");
  }

  LOG_PERF_STOP("webserver-api-config-format-write");
}

void WebServerHandler::webHandleTestPush(AsyncWebServerRequest *request) {
  LOG_PERF_START("webserver-api-test-push");
  String id = request->arg(PARAM_ID);
  Log.notice(F("WEB : webServer callback for /api/test/push." CR));

  if (!id.equalsIgnoreCase(myConfig.getID())) {
    Log.error(F("WEB : Wrong ID received %s, expected %s" CR), id.c_str(),
              myConfig.getID());
    request->send(400, "text/plain", "Invalid ID.");
    LOG_PERF_STOP("webserver-api-test-push");
    return;
  }

  _pushTestData = request->arg(PARAM_PUSH_FORMAT);
  _pushTestTask = true;
  Log.notice(F("WEB : Scheduling push test for %s" CR), _pushTestData.c_str());

  request->send(200, "application/json", "{}");
  LOG_PERF_STOP("webserver-api-test-push");
}

void WebServerHandler::webHandleTestPushStatus(AsyncWebServerRequest *request) {
  LOG_PERF_START("webserver-api-test-push-status");
  Log.notice(F("WEB : webServer callback for /api/test/push/status." CR));

#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  Log.verbose(F("WEB : %s." CR), getRequestArguments().c_str());
#endif

  if (!_pushTestTask) {
    request->send(200, "application/json", _pushTestData.c_str());
    _pushTestData.clear();
  } else {
    DynamicJsonDocument doc(100);

    doc[PARAM_STATUS] = _pushTestTask;

#if LOG_LEVEL == 6
    serializeJson(doc, Serial);
    EspSerial.print(CR);
#endif

    String out;
    out.reserve(100);
    serializeJson(doc, out);
    doc.clear();

    request->send(200, "application/json", out.c_str());
  }

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
  LOG_PERF_START("webserver-api-config-format-read");
  Log.notice(F("WEB : webServer callback for /api/config/formula(get)." CR));

  String out;
  out.reserve(7000);
  out += "{\"id\":\"" + String(myConfig.getID()) + "\",";

  String s = readFile(TPL_FNAME_HTTP1);
  out += "\"" + String(PARAM_FORMAT_HTTP1) + "\":\"";
  if (s.length())
    out += urlencode(s);
  else
    out += urlencode(String(&iSpindleFormat[0]));

  s = readFile(TPL_FNAME_HTTP2);
  out += "\",\"" + String(PARAM_FORMAT_HTTP2) + "\":\"";
  if (s.length())
    out += urlencode(s);
  else
    out += urlencode(String(&iSpindleFormat[0]));

  s = readFile(TPL_FNAME_HTTP3);
  out += "\",\"" + String(PARAM_FORMAT_HTTP3) + "\":\"";
  if (s.length())
    out += urlencode(s);
  else
    out += urlencode(String(&iHttpGetFormat[0]));

  s = readFile(TPL_FNAME_INFLUXDB);
  out += "\",\"" + String(PARAM_FORMAT_INFLUXDB) + "\":\"";
  if (s.length())
    out += urlencode(s);
  else
    out += urlencode(String(&influxDbFormat[0]));

  s = readFile(TPL_FNAME_MQTT);
  out += "\",\"" + String(PARAM_FORMAT_MQTT) + "\":\"";
  if (s.length())
    out += urlencode(s);
  else
    out += urlencode(String(&mqttFormat[0]));

  out += "\"}";

#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  EspSerial.print(out.c_str());
  EspSerial.print(CR);
#endif

  request->send(200, "application/json", out.c_str());
  LOG_PERF_STOP("webserver-api-config-format-read");
}

void WebServerHandler::webHandleFormulaWrite(AsyncWebServerRequest *request) {
  LOG_PERF_START("webserver-api-formula-write");
  String id = request->arg(PARAM_ID);
  Log.notice(F("WEB : webServer callback for /api/formula(post)." CR));

  if (!id.equalsIgnoreCase(myConfig.getID())) {
    Log.error(F("WEB : Wrong ID received %s, expected %s" CR), id.c_str(),
              myConfig.getID());
    request->send(400, "text/plain", "Invalid ID.");
    LOG_PERF_STOP("webserver-api-formula-write");
    return;
  }

#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  Log.verbose(F("WEB : %s." CR), getRequestArguments().c_str());
#endif

  RawFormulaData fd;
  fd.a[0] = request->arg("a1").toDouble();
  fd.a[1] = request->arg("a2").toDouble();
  fd.a[2] = request->arg("a3").toDouble();
  fd.a[3] = request->arg("a4").toDouble();
  fd.a[4] = request->arg("a5").toDouble();
  fd.a[5] = request->arg("a6").toDouble();
  fd.a[6] = request->arg("a7").toDouble();
  fd.a[7] = request->arg("a8").toDouble();
  fd.a[8] = request->arg("a9").toDouble();
  fd.a[9] = request->arg("a10").toDouble();

  if (myConfig.isGravityPlato()) {
    fd.g[0] = convertToSG(request->arg("g1").toDouble());
    fd.g[1] = convertToSG(request->arg("g2").toDouble());
    fd.g[2] = convertToSG(request->arg("g3").toDouble());
    fd.g[3] = convertToSG(request->arg("g4").toDouble());
    fd.g[4] = convertToSG(request->arg("g5").toDouble());
    fd.g[5] = convertToSG(request->arg("g6").toDouble());
    fd.g[6] = convertToSG(request->arg("g7").toDouble());
    fd.g[7] = convertToSG(request->arg("g8").toDouble());
    fd.g[8] = convertToSG(request->arg("g9").toDouble());
    fd.g[9] = convertToSG(request->arg("g10").toDouble());
  } else {
    fd.g[0] = request->arg("g1").toDouble();
    fd.g[1] = request->arg("g2").toDouble();
    fd.g[2] = request->arg("g3").toDouble();
    fd.g[3] = request->arg("g4").toDouble();
    fd.g[4] = request->arg("g5").toDouble();
    fd.g[5] = request->arg("g6").toDouble();
    fd.g[6] = request->arg("g7").toDouble();
    fd.g[7] = request->arg("g8").toDouble();
    fd.g[8] = request->arg("g9").toDouble();
    fd.g[9] = request->arg("g10").toDouble();
  }

  fd.g[0] = 1;  // force first point to SG gravity of water
  myConfig.setFormulaData(fd);

  int e;
  char buf[100];

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
    _lastFormulaCreateError = e;
  } else {
    // Save the formula as succesful
    Log.info(F("WEB : Found valid formula: '%s'" CR), &buf[0]);
    myConfig.setGravityFormula(buf);
    _lastFormulaCreateError = 0;
  }

  myConfig.saveFile();

  AsyncWebServerResponse *response =
      request->beginResponse(302, "text/plain", "Formula updated");
  response->addHeader("Location", "/calibration.htm");
  request->send(response);
  LOG_PERF_STOP("webserver-api-formula-write");
}

void WebServerHandler::webHandleMigrate(AsyncWebServerRequest *request) {
  LOG_PERF_START("webserver-api-migrate");
  Log.notice(F("WEB : webServer callback for /api/migrate." CR));

#if defined(ESP8266)
  DynamicJsonDocument doc(500);
  DeserializationError err = deserializeJson(doc, request->arg("plain"));

  if (err) {
    writeErrorLog("CFG : Failed to parse migration data (json)");
    request->send(400, "text/plain", F("Unable to parse data"));
    LOG_PERF_STOP("webserver-api-migrate");
    return;
  }

  myConfig.setGravityFormula(doc[PARAM_GRAVITY_FORMULA]);

  RawGyroData gyro = {0, 0, 0, 0, 0, 0};
  gyro.ax = doc[PARAM_GYRO_CALIBRATION]["ax"];
  gyro.ay = doc[PARAM_GYRO_CALIBRATION]["ay"];
  gyro.az = doc[PARAM_GYRO_CALIBRATION]["az"];
  gyro.gx = doc[PARAM_GYRO_CALIBRATION]["gx"];
  gyro.gy = doc[PARAM_GYRO_CALIBRATION]["gy"];
  gyro.gz = doc[PARAM_GYRO_CALIBRATION]["gz"];

  myConfig.setGyroCalibration(gyro);
  myConfig.saveFile();

  LittleFS.rename("/config.json", "/ispindel.json");
  request->send(200, "text/plain", F("Data migrated"));
#else
  request->send(404, "text/plain", F("Not implemented"));
#endif

  LOG_PERF_STOP("webserver-api-migrate");
}

void WebServerHandler::webHandlePageNotFound(AsyncWebServerRequest *request) {
  Log.error(F("WEB : URL not found %s received." CR), request->url().c_str());
  request->send(404, "text/plain", F("URL not found"));
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
                                       this, std::placeholders::_1));
  _server->serveStatic("/log", LittleFS, ERR_FILENAME);
  _server->serveStatic("/log2", LittleFS, ERR_FILENAME2);
  _server->serveStatic("/runtime", LittleFS, RUNTIME_FILENAME);
  _server->serveStatic("/migrate", LittleFS, "/config.json");

  // Dynamic content
  _server->on("/api/clearlog", HTTP_GET,
              std::bind(&WebServerHandler::webHandleLogClear, this,
                        std::placeholders::_1));
  _server->on("/api/config/device", HTTP_POST,
              std::bind(&WebServerHandler::webHandleConfigDevice, this,
                        std::placeholders::_1));  // Change device settings
  _server->on("/api/config/push", HTTP_POST,
              std::bind(&WebServerHandler::webHandleConfigPush, this,
                        std::placeholders::_1));  // Change push settings
  _server->on("/api/config/gravity", HTTP_POST,
              std::bind(&WebServerHandler::webHandleConfigGravity, this,
                        std::placeholders::_1));  // Change gravity settings
  _server->on("/api/config/hardware", HTTP_POST,
              std::bind(&WebServerHandler::webHandleConfigHardware, this,
                        std::placeholders::_1));  // Change hardware settings
  _server->on("/api/config/format", HTTP_GET,
              std::bind(&WebServerHandler::webHandleConfigFormatRead, this,
                        std::placeholders::_1));  // Change template formats
  _server->on("/api/config/format", HTTP_POST,
              std::bind(&WebServerHandler::webHandleConfigFormatWrite, this,
                        std::placeholders::_1));  // Change template formats
  _server->on("/api/config/advanced", HTTP_GET,
              std::bind(&WebServerHandler::webHandleConfigAdvancedRead, this,
                        std::placeholders::_1));  // Read advanced settings
  _server->on("/api/config/advanced", HTTP_POST,
              std::bind(&WebServerHandler::webHandleConfigAdvancedWrite, this,
                        std::placeholders::_1));  // Change advanced params
  _server->on("/api/config/wifi", HTTP_POST,
              std::bind(&WebServerHandler::webHandleConfigWifi, this,
                        std::placeholders::_1));  // Change wiif settings
  _server->on("/api/config", HTTP_GET,
              std::bind(&WebServerHandler::webHandleConfig, this,
                        std::placeholders::_1));
  _server->on("/api/formula", HTTP_GET,
              std::bind(&WebServerHandler::webHandleFormulaRead, this,
                        std::placeholders::_1));
  _server->on("/api/formula", HTTP_POST,
              std::bind(&WebServerHandler::webHandleFormulaWrite, this,
                        std::placeholders::_1));
  _server->on("/api/calibrate/status", HTTP_GET,
              std::bind(&WebServerHandler::webHandleCalibrateStatus, this,
                        std::placeholders::_1));
  _server->on("/api/calibrate", HTTP_POST,
              std::bind(&WebServerHandler::webHandleCalibrate, this,
                        std::placeholders::_1));
  _server->on("/api/factory", HTTP_GET,
              std::bind(&WebServerHandler::webHandleFactoryDefaults, this,
                        std::placeholders::_1));
  _server->on("/api/status/sleepmode", HTTP_POST,
              std::bind(&WebServerHandler::webHandleStatusSleepmode, this,
                        std::placeholders::_1));  // Change sleep mode
  _server->on("/api/status", HTTP_GET,
              std::bind(&WebServerHandler::webHandleStatus, this,
                        std::placeholders::_1));
  _server->on("/api/clearwifi", HTTP_GET,
              std::bind(&WebServerHandler::webHandleClearWIFI, this,
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
      std::bind(
          &WebServerHandler::webHandleUploadFile, this, std::placeholders::_1,
          std::placeholders::_2, std::placeholders::_3, std::placeholders::_4,
          std::placeholders::_5, std::placeholders::_6));  // File upload data
  _server->on("/api/test/push/status", HTTP_GET,
              std::bind(&WebServerHandler::webHandleTestPushStatus, this,
                        std::placeholders::_1));  //
  _server->on("/api/test/push", HTTP_GET,
              std::bind(&WebServerHandler::webHandleTestPush, this,
                        std::placeholders::_1));  //

  _server->onNotFound(std::bind(&WebServerHandler::webHandlePageNotFound, this,
                                std::placeholders::_1));
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
        gravitySG, tempC, myAdvancedConfig.getDefaultCalibrationTemp());

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
    DynamicJsonDocument doc(100);
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
