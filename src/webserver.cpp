/*
MIT License

Copyright (c) 2021-22 Magnus

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

WebServerHandler myWebServerHandler;  // My wrapper class fr webserver functions
extern bool sleepModeActive;
extern bool sleepModeAlwaysSkip;

void WebServerHandler::webHandleConfig() {
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

  double tempC = myTempSensor.getTempC(myConfig.isGyroTemp());
  double gravity = calculateGravity(angle, tempC);

  doc[PARAM_ANGLE] = reduceFloatPrecision(angle, DECIMALS_TILT);
  doc[PARAM_GRAVITY_FORMAT] = String(myConfig.getGravityFormat());

  // Format the adjustment so we get rid of rounding errors
  if (myConfig.isTempF())
    // We want the delta value (32F = 0C).
    doc[PARAM_TEMP_ADJ] = reduceFloatPrecision(
        convertCtoF(myConfig.getTempSensorAdjC()) - 32, DECIMALS_TEMP);
  else
    doc[PARAM_TEMP_ADJ] =
        reduceFloatPrecision(myConfig.getTempSensorAdjC(), DECIMALS_TEMP);

  if (myConfig.isGravityTempAdj()) {
    gravity = gravityTemperatureCorrectionC(
        gravity, tempC, myAdvancedConfig.getDefaultCalibrationTemp());
  }

  if (myConfig.isGravityPlato()) {
    doc[PARAM_GRAVITY] =
        reduceFloatPrecision(convertToPlato(gravity), DECIMALS_PLATO);
  } else {
    doc[PARAM_GRAVITY] = reduceFloatPrecision(gravity, DECIMALS_SG);
  }

  doc[PARAM_BATTERY] =
      reduceFloatPrecision(myBatteryVoltage.getVoltage(), DECIMALS_BATTERY);

  FloatHistoryLog runLog(RUNTIME_FILENAME);
  doc[PARAM_RUNTIME_AVERAGE] = reduceFloatPrecision(
      runLog.getAverage() ? runLog.getAverage() / 1000 : 0, DECIMALS_RUNTIME);

#if defined(ESP8266)
  doc[PARAM_PLATFORM] = "esp8266";
#elif defined(ESP32C3)
  doc[PARAM_PLATFORM] = "esp32c3";
#elif defined(ESP32S2)
  doc[PARAM_PLATFORM] = "esp32s2";
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
  _server->send(200, "application/json", out.c_str());
  LOG_PERF_STOP("webserver-api-config");
}

void WebServerHandler::webHandleUploadFile() {
  LOG_PERF_START("webserver-api-upload-file");
  Log.verbose(F("WEB : webServer callback for /api/upload(post)." CR));
  HTTPUpload& upload = _server->upload();
  String f = upload.filename;

#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  Log.verbose(F("WEB : webServer callback for /api/upload, receiving file %s, "
                "%d(%d)." CR),
              f.c_str(), upload.currentSize, upload.totalSize);
#endif

  // Handle firmware update, hardcode since function return wrong value.
  // (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
  uint32_t maxSketchSpace = MAX_SKETCH_SPACE;

  if (upload.status == UPLOAD_FILE_START) {
    _uploadReturn = 200;
    Log.notice(F("WEB : Start firmware upload, max sketch size %d kb." CR),
               maxSketchSpace / 1024);

    if (!Update.begin(maxSketchSpace, U_FLASH, PIN_LED)) {
      writeErrorLog("WEB : Not enough space to store for this firmware.");
      _uploadReturn = 500;
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    Log.notice(F("WEB : Writing firmware upload %d (%d)." CR), upload.totalSize,
               maxSketchSpace);

    if (upload.totalSize > maxSketchSpace) {
      Log.error(F("WEB : Firmware file is to large." CR));
      _uploadReturn = 500;
    } else if (Update.write(upload.buf, upload.currentSize) !=
               upload.currentSize) {
      Log.warning(F("WEB : Firmware write was unsuccessful." CR));
      _uploadReturn = 500;
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    Log.notice(F("WEB : Finish firmware upload." CR));
    if (Update.end(true)) {
      _server->send(200);
      delay(500);
      ESP_RESET();
    } else {
      writeErrorLog("WEB : Failed to finish firmware flashing error=%d",
                    Update.getError());
      _uploadReturn = 500;
    }
  } else {
    Update.end();
    Log.notice(F("WEB : Firmware flashing aborted." CR));
    _uploadReturn = 500;
  }

  delay(0);
  LOG_PERF_STOP("webserver-api-upload-file");
}

void WebServerHandler::webHandleCalibrate() {
  LOG_PERF_START("webserver-api-calibrate");
  String id = _server->arg(PARAM_ID);
  Log.notice(F("WEB : webServer callback for /api/calibrate." CR));

  if (!id.equalsIgnoreCase(myConfig.getID())) {
    Log.error(F("WEB : Wrong ID received %s, expected %s" CR), id.c_str(),
              myConfig.getID());
    _server->send(400, "text/plain", "Invalid ID.");
    LOG_PERF_STOP("webserver-api-calibrate");
    return;
  }

  if (myGyro.isConnected()) {
    myGyro.calibrateSensor();
    _server->send(200, "text/plain", "Device calibrated");
  } else {
    Log.error(F("WEB : No gyro connected, skipping calibrate" CR));
    _server->send(400, "text/plain", "No gyro connected.");
  } 

  LOG_PERF_STOP("webserver-api-calibrate");
}

void WebServerHandler::webHandleFactoryDefaults() {
  String id = _server->arg(PARAM_ID);
  Log.notice(F("WEB : webServer callback for /api/factory." CR));

  if (!id.compareTo(myConfig.getID())) {
    _server->send(200, "text/plain",
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
    delay(500);
    ESP_RESET();
  } else {
    _server->send(400, "text/plain", "Unknown ID.");
  }
}

void WebServerHandler::webHandleLogClear() {
  String id = _server->arg(PARAM_ID);
  Log.notice(F("WEB : webServer callback for /api/clearlog." CR));

  if (!id.compareTo(myConfig.getID())) {
    _server->send(200, "text/plain", "Removing logfiles...");
    LittleFS.remove(ERR_FILENAME);
    LittleFS.remove(ERR_FILENAME2);
    _server->send(200, "text/plain", "Logfiles cleared.");
  } else {
    _server->send(400, "text/plain", "Unknown ID.");
  }
}

void WebServerHandler::webHandleStatus() {
  LOG_PERF_START("webserver-api-status");
  Log.notice(F("WEB : webServer callback for /api/status(get)." CR));

  DynamicJsonDocument doc(500);

  double angle = 0;  // Indicate we have no valid gyro value

  if (myGyro.hasValue()) angle = myGyro.getAngle();

  double tempC = myTempSensor.getTempC(myConfig.isGyroTemp());
  double gravity = calculateGravity(angle, tempC);

  doc[PARAM_ID] = myConfig.getID();
  doc[PARAM_ANGLE] = myGyro.isConnected()
                         ? reduceFloatPrecision(angle, DECIMALS_TILT)
                         : -1;  // Indicate that we have no connection to gyro

  if (myConfig.isGravityTempAdj()) {
    gravity = gravityTemperatureCorrectionC(
        gravity, tempC, myAdvancedConfig.getDefaultCalibrationTemp());
  }
  if (myConfig.isGravityPlato()) {
    doc[PARAM_GRAVITY] =
        reduceFloatPrecision(convertToPlato(gravity), DECIMALS_PLATO);
  } else {
    doc[PARAM_GRAVITY] = reduceFloatPrecision(gravity, DECIMALS_SG);
  }
  doc[PARAM_TEMP_C] = reduceFloatPrecision(tempC, DECIMALS_TEMP);
  doc[PARAM_TEMP_F] = reduceFloatPrecision(convertCtoF(tempC), DECIMALS_TEMP);
  doc[PARAM_BATTERY] =
      reduceFloatPrecision(myBatteryVoltage.getVoltage(), DECIMALS_BATTERY);
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

  FloatHistoryLog runLog(RUNTIME_FILENAME);
  doc[PARAM_RUNTIME_AVERAGE] = reduceFloatPrecision(
      runLog.getAverage() ? runLog.getAverage() / 1000 : 0, DECIMALS_RUNTIME);

#if defined(ESP8266)
  doc[PARAM_PLATFORM] = "esp8266";
#elif defined(ESP32C3)
  doc[PARAM_PLATFORM] = "esp32c3";
#elif defined(ESP32S2)
  doc[PARAM_PLATFORM] = "esp32s2";
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
      myConfig.isBLEActive() || myConfig.isHttpActive() ||
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
  _server->send(200, "application/json", out.c_str());
  LOG_PERF_STOP("webserver-api-status");
}

void WebServerHandler::webHandleClearWIFI() {
  String id = _server->arg(PARAM_ID);
  Log.notice(F("WEB : webServer callback for /api/clearwifi." CR));

  if (!id.compareTo(myConfig.getID())) {
    _server->send(200, "text/plain",
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
    _server->send(400, "text/plain", "Unknown ID.");
  }
}

void WebServerHandler::webHandleStatusSleepmode() {
  LOG_PERF_START("webserver-api-sleepmode");
  String id = _server->arg(PARAM_ID);
  Log.notice(F("WEB : webServer callback for /api/status/sleepmode(post)." CR));

  if (!id.equalsIgnoreCase(myConfig.getID())) {
    Log.error(F("WEB : Wrong ID received %s, expected %s" CR), id.c_str(),
              myConfig.getID());
    _server->send(400, "text/plain", "Invalid ID.");
    LOG_PERF_STOP("webserver-api-sleepmode");
    return;
  }

#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  Log.verbose(F("WEB : %s." CR), getRequestArguments().c_str());
#endif

  if (_server->arg(PARAM_SLEEP_MODE).equalsIgnoreCase("true"))
    sleepModeAlwaysSkip = true;
  else
    sleepModeAlwaysSkip = false;
  _server->send(200, "text/plain", "Sleep mode updated");
  LOG_PERF_STOP("webserver-api-sleepmode");
}

void WebServerHandler::webHandleConfigDevice() {
  LOG_PERF_START("webserver-api-config-device");
  String id = _server->arg(PARAM_ID);
  Log.notice(F("WEB : webServer callback for /api/config/device(post)." CR));

  if (!id.equalsIgnoreCase(myConfig.getID())) {
    Log.error(F("WEB : Wrong ID received %s, expected %s" CR), id.c_str(),
              myConfig.getID());
    _server->send(400, "text/plain", "Invalid ID.");
    LOG_PERF_STOP("webserver-api-config-device");
    return;
  }

#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  Log.verbose(F("WEB : %s." CR), getRequestArguments().c_str());
#endif

  if (_server->hasArg(PARAM_MDNS))
    myConfig.setMDNS(_server->arg(PARAM_MDNS).c_str());
  if (_server->hasArg(PARAM_TEMPFORMAT))
    myConfig.setTempFormat(_server->arg(PARAM_TEMPFORMAT).charAt(0));
  if (_server->hasArg(PARAM_SLEEP_INTERVAL))
    myConfig.setSleepInterval(_server->arg(PARAM_SLEEP_INTERVAL).c_str());
  myConfig.saveFile();
  _server->sendHeader("Location", "/config.htm#collapseDevice", true);
  _server->send(302, "text/plain", "Device config updated");
  LOG_PERF_STOP("webserver-api-config-device");
}

void WebServerHandler::webHandleConfigPush() {
  LOG_PERF_START("webserver-api-config-push");
  String id = _server->arg(PARAM_ID);
  Log.notice(F("WEB : webServer callback for /api/config/push(post)." CR));

  if (!id.equalsIgnoreCase(myConfig.getID())) {
    Log.error(F("WEB : Wrong ID received %s, expected %s" CR), id.c_str(),
              myConfig.getID());
    _server->send(400, "text/plain", "Invalid ID.");
    LOG_PERF_STOP("webserver-api-config-push");
    return;
  }
#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  Log.verbose(F("WEB : %s." CR), getRequestArguments().c_str());
#endif

  if (_server->hasArg(PARAM_TOKEN))
    myConfig.setToken(_server->arg(PARAM_TOKEN).c_str());
  if (_server->hasArg(PARAM_TOKEN2))
    myConfig.setToken2(_server->arg(PARAM_TOKEN2).c_str());
  if (_server->hasArg(PARAM_PUSH_HTTP))
    myConfig.setHttpUrl(_server->arg(PARAM_PUSH_HTTP).c_str());
  if (_server->hasArg(PARAM_PUSH_HTTP_H1))
    myConfig.setHttpHeader(_server->arg(PARAM_PUSH_HTTP_H1).c_str(), 0);
  if (_server->hasArg(PARAM_PUSH_HTTP_H2))
    myConfig.setHttpHeader(_server->arg(PARAM_PUSH_HTTP_H2).c_str(), 1);
  if (_server->hasArg(PARAM_PUSH_HTTP2))
    myConfig.setHttp2Url(_server->arg(PARAM_PUSH_HTTP2).c_str());
  if (_server->hasArg(PARAM_PUSH_HTTP2_H1))
    myConfig.setHttp2Header(_server->arg(PARAM_PUSH_HTTP2_H1).c_str(), 0);
  if (_server->hasArg(PARAM_PUSH_HTTP2_H2))
    myConfig.setHttp2Header(_server->arg(PARAM_PUSH_HTTP2_H2).c_str(), 1);
  if (_server->hasArg(PARAM_PUSH_HTTP3))
    myConfig.setHttp3Url(_server->arg(PARAM_PUSH_HTTP3).c_str());
  if (_server->hasArg(PARAM_PUSH_INFLUXDB2))
    myConfig.setInfluxDb2PushUrl(_server->arg(PARAM_PUSH_INFLUXDB2).c_str());
  if (_server->hasArg(PARAM_PUSH_INFLUXDB2_ORG))
    myConfig.setInfluxDb2PushOrg(
        _server->arg(PARAM_PUSH_INFLUXDB2_ORG).c_str());
  if (_server->hasArg(PARAM_PUSH_INFLUXDB2_BUCKET))
    myConfig.setInfluxDb2PushBucket(
        _server->arg(PARAM_PUSH_INFLUXDB2_BUCKET).c_str());
  if (_server->hasArg(PARAM_PUSH_INFLUXDB2_AUTH))
    myConfig.setInfluxDb2PushToken(
        _server->arg(PARAM_PUSH_INFLUXDB2_AUTH).c_str());
  if (_server->hasArg(PARAM_PUSH_MQTT))
    myConfig.setMqttUrl(_server->arg(PARAM_PUSH_MQTT).c_str());
  if (_server->hasArg(PARAM_PUSH_MQTT_PORT))
    myConfig.setMqttPort(_server->arg(PARAM_PUSH_MQTT_PORT).c_str());
  if (_server->hasArg(PARAM_PUSH_MQTT_USER))
    myConfig.setMqttUser(_server->arg(PARAM_PUSH_MQTT_USER).c_str());
  if (_server->hasArg(PARAM_PUSH_MQTT_PASS))
    myConfig.setMqttPass(_server->arg(PARAM_PUSH_MQTT_PASS).c_str());
  myConfig.saveFile();
  String section("/config.htm#");
  section += _server->arg("section");
  _server->sendHeader("Location", section.c_str(), true);
  _server->send(302, "text/plain", "Push config updated");
  LOG_PERF_STOP("webserver-api-config-push");
}

String WebServerHandler::getRequestArguments() {
  String debug;

  for (int i = 0; i < _server->args(); i++) {
    if (!_server->argName(i).equals(
            "plain")) {  // this contains all the arguments, we dont need that.
      if (debug.length()) debug += ", ";

      debug += _server->argName(i);
      debug += "=";
      debug += _server->arg(i);
    }
  }
  return debug;
}

void WebServerHandler::webHandleConfigGravity() {
  LOG_PERF_START("webserver-api-config-gravity");
  String id = _server->arg(PARAM_ID);
  Log.notice(F("WEB : webServer callback for /api/config/gravity(post)." CR));

  if (!id.equalsIgnoreCase(myConfig.getID())) {
    Log.error(F("WEB : Wrong ID received %s, expected %s" CR), id.c_str(),
              myConfig.getID());
    _server->send(400, "text/plain", "Invalid ID.");
    LOG_PERF_STOP("webserver-api-config-gravity");
    return;
  }

#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  Log.verbose(F("WEB : %s." CR), getRequestArguments().c_str());
#endif

  if (_server->hasArg(PARAM_GRAVITY_FORMAT))
    myConfig.setGravityFormat(_server->arg(PARAM_GRAVITY_FORMAT).charAt(0));
  if (_server->hasArg(PARAM_GRAVITY_FORMULA))
    myConfig.setGravityFormula(_server->arg(PARAM_GRAVITY_FORMULA).c_str());
  if (_server->hasArg(PARAM_GRAVITY_TEMP_ADJ))
    myConfig.setGravityTempAdj(
        _server->arg(PARAM_GRAVITY_TEMP_ADJ).equalsIgnoreCase("on") ? true
                                                                    : false);
  else
    myConfig.setGravityTempAdj(false);

  myConfig.saveFile();
  _server->sendHeader("Location", "/config.htm#collapseGravity", true);
  _server->send(302, "text/plain", "Gravity config updated");
  LOG_PERF_STOP("webserver-api-config-gravity");
}

void WebServerHandler::webHandleConfigHardware() {
  LOG_PERF_START("webserver-api-config-hardware");
  String id = _server->arg(PARAM_ID);
  Log.notice(F("WEB : webServer callback for /api/config/hardware(post)." CR));

  if (!id.equalsIgnoreCase(myConfig.getID())) {
    Log.error(F("WEB : Wrong ID received %s, expected %s" CR), id.c_str(),
              myConfig.getID());
    _server->send(400, "text/plain", "Invalid ID.");
    LOG_PERF_STOP("webserver-api-config-hardware");
    return;
  }

#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  Log.verbose(F("WEB : %s." CR), getRequestArguments().c_str());
#endif

  if (_server->hasArg(PARAM_VOLTAGE_FACTOR))
    myConfig.setVoltageFactor(_server->arg(PARAM_VOLTAGE_FACTOR).toFloat());
  if (_server->hasArg(PARAM_VOLTAGE_CONFIG))
    myConfig.setVoltageConfig(_server->arg(PARAM_VOLTAGE_CONFIG).toFloat());
  if (_server->hasArg(PARAM_TEMP_ADJ)) {
    if (myConfig.isTempC()) {
      myConfig.setTempSensorAdjC(_server->arg(PARAM_TEMP_ADJ));
    } else {
      // Data is delta so we add 32 in order to conver to C.
      myConfig.setTempSensorAdjF(_server->arg(PARAM_TEMP_ADJ), 32);
    }
  }
  if (_server->hasArg(PARAM_BLE))
    myConfig.setColorBLE(_server->arg(PARAM_BLE).c_str());
  if (_server->hasArg(PARAM_OTA))
    myConfig.setOtaURL(_server->arg(PARAM_OTA).c_str());
  if (_server->hasArg(PARAM_GYRO_TEMP))
    myConfig.setGyroTemp(
        _server->arg(PARAM_GYRO_TEMP).equalsIgnoreCase("on") ? true : false);
  else
    myConfig.setGyroTemp(false);
  if (_server->hasArg(PARAM_STORAGE_SLEEP))
    myConfig.setStorageSleep(
        _server->arg(PARAM_STORAGE_SLEEP).equalsIgnoreCase("on") ? true
                                                                 : false);
  else
    myConfig.setStorageSleep(false);

  myConfig.saveFile();
  _server->sendHeader("Location", "/config.htm#collapseHardware", true);
  _server->send(302, "text/plain", "Hardware config updated");
  LOG_PERF_STOP("webserver-api-config-hardware");
}

void WebServerHandler::webHandleConfigAdvancedWrite() {
  LOG_PERF_START("webserver-api-config-advanced");
  String id = _server->arg(PARAM_ID);
  Log.notice(F("WEB : webServer callback for /api/config/advaced(post)." CR));

  if (!id.equalsIgnoreCase(myConfig.getID())) {
    Log.error(F("WEB : Wrong ID received %s, expected %s" CR), id.c_str(),
              myConfig.getID());
    _server->send(400, "text/plain", "Invalid ID.");
    LOG_PERF_STOP("webserver-api-config-advanced");
    return;
  }

#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  Log.verbose(F("WEB : %s." CR), getRequestArguments().c_str());
#endif

  if (_server->hasArg(PARAM_HW_GYRO_READ_COUNT))
    myAdvancedConfig.setGyroReadCount(
        _server->arg(PARAM_HW_GYRO_READ_COUNT).toInt());
  // if (_server->hasArg(PARAM_HW_GYRO_READ_DELAY))
  //  myAdvancedConfig.setGyroReadDelay(
  //      _server->arg(PARAM_HW_GYRO_READ_DELAY).toInt());
  if (_server->hasArg(PARAM_HW_GYRO_MOVING_THREASHOLD))
    myAdvancedConfig.setGyroSensorMovingThreashold(
        _server->arg(PARAM_HW_GYRO_MOVING_THREASHOLD).toInt());
  if (_server->hasArg(PARAM_HW_FORMULA_DEVIATION))
    myAdvancedConfig.setMaxFormulaCreationDeviation(
        _server->arg(PARAM_HW_FORMULA_DEVIATION).toFloat());
  if (_server->hasArg(PARAM_HW_FORMULA_CALIBRATION_TEMP)) {
    float t = _server->arg(PARAM_HW_FORMULA_CALIBRATION_TEMP).toFloat();
    if (myConfig.isTempF()) t = convertFtoC(t);
    myAdvancedConfig.SetDefaultCalibrationTemp(t);
  }
  if (_server->hasArg(PARAM_HW_WIFI_PORTAL_TIMEOUT))
    myAdvancedConfig.setWifiPortalTimeout(
        _server->arg(PARAM_HW_WIFI_PORTAL_TIMEOUT).toInt());
  if (_server->hasArg(PARAM_HW_WIFI_CONNECT_TIMEOUT))
    myAdvancedConfig.setWifiConnectTimeout(
        _server->arg(PARAM_HW_WIFI_CONNECT_TIMEOUT).toInt());
  if (_server->hasArg(PARAM_HW_PUSH_TIMEOUT))
    myAdvancedConfig.setPushTimeout(
        _server->arg(PARAM_HW_PUSH_TIMEOUT).toInt());
  if (_server->hasArg(PARAM_HW_PUSH_INTERVAL_HTTP1))
    myAdvancedConfig.setPushIntervalHttp1(
        _server->arg(PARAM_HW_PUSH_INTERVAL_HTTP1).toInt());
  if (_server->hasArg(PARAM_HW_PUSH_INTERVAL_HTTP2))
    myAdvancedConfig.setPushIntervalHttp2(
        _server->arg(PARAM_HW_PUSH_INTERVAL_HTTP2).toInt());
  if (_server->hasArg(PARAM_HW_PUSH_INTERVAL_HTTP3))
    myAdvancedConfig.setPushIntervalHttp3(
        _server->arg(PARAM_HW_PUSH_INTERVAL_HTTP3).toInt());
  if (_server->hasArg(PARAM_HW_PUSH_INTERVAL_INFLUX))
    myAdvancedConfig.setPushIntervalInflux(
        _server->arg(PARAM_HW_PUSH_INTERVAL_INFLUX).toInt());
  if (_server->hasArg(PARAM_HW_PUSH_INTERVAL_MQTT))
    myAdvancedConfig.setPushIntervalMqtt(
        _server->arg(PARAM_HW_PUSH_INTERVAL_MQTT).toInt());
  if (_server->hasArg(PARAM_HW_TEMPSENSOR_RESOLUTION))
    myAdvancedConfig.setTempSensorResolution(
        _server->arg(PARAM_HW_TEMPSENSOR_RESOLUTION).toInt());
  if (_server->hasArg(PARAM_HW_IGNORE_LOW_ANGLES))
    myAdvancedConfig.setIgnoreLowAnges(
        _server->arg(PARAM_HW_IGNORE_LOW_ANGLES).equalsIgnoreCase("on")
            ? true
            : false);
  else
    myAdvancedConfig.setIgnoreLowAnges(false);
  if (_server->hasArg(PARAM_HW_BATTERY_SAVING))
    myAdvancedConfig.setBatterySaving(
        _server->arg(PARAM_HW_BATTERY_SAVING).equalsIgnoreCase("on") ? true
                                                                     : false);
  else
    myAdvancedConfig.setBatterySaving(false);

  myAdvancedConfig.saveFile();
  _server->sendHeader("Location", "/config.htm#collapseAdvanced", true);
  _server->send(302, "text/plain", "Advanced config updated");
  LOG_PERF_STOP("webserver-api-config-advanced");
}

void WebServerHandler::webHandleConfigAdvancedRead() {
  LOG_PERF_START("webserver-api-config-advanced");
  Log.notice(F("WEB : webServer callback for /api/config/advanced(get)." CR));

  DynamicJsonDocument doc(500);

  doc[PARAM_HW_GYRO_READ_COUNT] = myAdvancedConfig.getGyroReadCount();
  // doc[PARAM_HW_GYRO_READ_DELAY] = myAdvancedConfig.getGyroReadDelay();
  doc[PARAM_HW_GYRO_MOVING_THREASHOLD] =
      myAdvancedConfig.getGyroSensorMovingThreashold();
  doc[PARAM_HW_FORMULA_DEVIATION] =
      myAdvancedConfig.getMaxFormulaCreationDeviation();
  doc[PARAM_HW_WIFI_PORTAL_TIMEOUT] = myAdvancedConfig.getWifiPortalTimeout();
  doc[PARAM_HW_WIFI_CONNECT_TIMEOUT] = myAdvancedConfig.getWifiConnectTimeout();
  doc[PARAM_HW_PUSH_TIMEOUT] = myAdvancedConfig.getPushTimeout();
  float t = myAdvancedConfig.getDefaultCalibrationTemp();
  doc[PARAM_HW_FORMULA_CALIBRATION_TEMP] =
      myConfig.isTempC() ? t
                         : reduceFloatPrecision(convertCtoF(t), DECIMALS_TEMP);
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
  _server->send(200, "application/json", out.c_str());
  LOG_PERF_STOP("webserver-api-config-advanced");
}

void WebServerHandler::webHandleFormulaRead() {
  LOG_PERF_START("webserver-api-formula-read");
  Log.notice(F("WEB : webServer callback for /api/formula(get)." CR));

  DynamicJsonDocument doc(500);
  const RawFormulaData& fd = myConfig.getFormulaData();

#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  Log.verbose(F("WEB : %s." CR), getRequestArguments().c_str());
#endif

  doc[PARAM_ID] = myConfig.getID();
  doc[PARAM_ANGLE] = reduceFloatPrecision(myGyro.getAngle(), DECIMALS_TILT);
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

  doc["a1"] = reduceFloatPrecision(fd.a[0], DECIMALS_TILT);
  doc["a2"] = reduceFloatPrecision(fd.a[1], DECIMALS_TILT);
  doc["a3"] = reduceFloatPrecision(fd.a[2], DECIMALS_TILT);
  doc["a4"] = reduceFloatPrecision(fd.a[3], DECIMALS_TILT);
  doc["a5"] = reduceFloatPrecision(fd.a[4], DECIMALS_TILT);
  doc["a6"] = reduceFloatPrecision(fd.a[5], DECIMALS_TILT);
  doc["a7"] = reduceFloatPrecision(fd.a[6], DECIMALS_TILT);
  doc["a8"] = reduceFloatPrecision(fd.a[7], DECIMALS_TILT);
  doc["a9"] = reduceFloatPrecision(fd.a[8], DECIMALS_TILT);
  doc["a10"] = reduceFloatPrecision(fd.a[9], DECIMALS_TILT);

  if (myConfig.isGravityPlato()) {
    doc["g1"] = reduceFloatPrecision(convertToPlato(fd.g[0]), DECIMALS_PLATO);
    doc["g2"] = reduceFloatPrecision(convertToPlato(fd.g[1]), DECIMALS_PLATO);
    doc["g3"] = reduceFloatPrecision(convertToPlato(fd.g[2]), DECIMALS_PLATO);
    doc["g4"] = reduceFloatPrecision(convertToPlato(fd.g[3]), DECIMALS_PLATO);
    doc["g5"] = reduceFloatPrecision(convertToPlato(fd.g[4]), DECIMALS_PLATO);
    doc["g6"] = reduceFloatPrecision(convertToPlato(fd.g[5]), DECIMALS_PLATO);
    doc["g7"] = reduceFloatPrecision(convertToPlato(fd.g[6]), DECIMALS_PLATO);
    doc["g8"] = reduceFloatPrecision(convertToPlato(fd.g[7]), DECIMALS_PLATO);
    doc["g9"] = reduceFloatPrecision(convertToPlato(fd.g[8]), DECIMALS_PLATO);
    doc["g10"] = reduceFloatPrecision(convertToPlato(fd.g[9]), DECIMALS_PLATO);
  } else {
    doc["g1"] = reduceFloatPrecision(fd.g[0], DECIMALS_SG);
    doc["g2"] = reduceFloatPrecision(fd.g[1], DECIMALS_SG);
    doc["g3"] = reduceFloatPrecision(fd.g[2], DECIMALS_SG);
    doc["g4"] = reduceFloatPrecision(fd.g[3], DECIMALS_SG);
    doc["g5"] = reduceFloatPrecision(fd.g[4], DECIMALS_SG);
    doc["g6"] = reduceFloatPrecision(fd.g[5], DECIMALS_SG);
    doc["g7"] = reduceFloatPrecision(fd.g[6], DECIMALS_SG);
    doc["g8"] = reduceFloatPrecision(fd.g[7], DECIMALS_SG);
    doc["g9"] = reduceFloatPrecision(fd.g[8], DECIMALS_SG);
    doc["g10"] = reduceFloatPrecision(fd.g[9], DECIMALS_SG);
  }

#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  serializeJson(doc, EspSerial);
  EspSerial.print(CR);
#endif

  String out;
  out.reserve(500);
  serializeJson(doc, out);
  doc.clear();
  _server->send(200, "application/json", out.c_str());
  LOG_PERF_STOP("webserver-api-formula-read");
}

void WebServerHandler::webHandleConfigFormatWrite() {
  LOG_PERF_START("webserver-api-config-format-write");
  String id = _server->arg(PARAM_ID);
  Log.notice(F("WEB : webServer callback for /api/config/format(post)." CR));

  if (!id.equalsIgnoreCase(myConfig.getID())) {
    Log.error(F("WEB : Wrong ID received %s, expected %s" CR), id.c_str(),
              myConfig.getID());
    _server->send(400, "text/plain", "Invalid ID.");
    LOG_PERF_STOP("webserver-api-config-format-write");
    return;
  }

#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  Log.verbose(F("WEB : %s." CR), getRequestArguments().c_str());
#endif
  bool success = false;

  // Only one option is posted so we done need to check them all.
  if (_server->hasArg(PARAM_FORMAT_HTTP1)) {
    success = writeFile(TPL_FNAME_HTTP1, _server->arg(PARAM_FORMAT_HTTP1));
  } else if (_server->hasArg(PARAM_FORMAT_HTTP2)) {
    success = writeFile(TPL_FNAME_HTTP2, _server->arg(PARAM_FORMAT_HTTP2));
  } else if (_server->hasArg(PARAM_FORMAT_HTTP3)) {
    success = writeFile(TPL_FNAME_HTTP3, _server->arg(PARAM_FORMAT_HTTP3));
  } else if (_server->hasArg(PARAM_FORMAT_INFLUXDB)) {
    success =
        writeFile(TPL_FNAME_INFLUXDB, _server->arg(PARAM_FORMAT_INFLUXDB));
  } else if (_server->hasArg(PARAM_FORMAT_MQTT)) {
    success = writeFile(TPL_FNAME_MQTT, _server->arg(PARAM_FORMAT_MQTT));
  }

  if (success) {
    _server->sendHeader("Location", "/format.htm", true);
    _server->send(302, "text/plain", "Format updated");
  } else {
    writeErrorLog("WEB : Unable to store format file");
    _server->send(400, "text/plain", "Unable to store format in file.");
  }

  LOG_PERF_STOP("webserver-api-config-format-write");
}

void WebServerHandler::webHandleTestPush() {
  LOG_PERF_START("webserver-api-test-push");
  String id = _server->arg(PARAM_ID);
  Log.notice(F("WEB : webServer callback for /api/test/push." CR));

  if (!id.equalsIgnoreCase(myConfig.getID())) {
    Log.error(F("WEB : Wrong ID received %s, expected %s" CR), id.c_str(),
              myConfig.getID());
    _server->send(400, "text/plain", "Invalid ID.");
    LOG_PERF_STOP("webserver-api-test-push");
    return;
  }

#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  Log.verbose(F("WEB : %s." CR), getRequestArguments().c_str());
#endif

  float angle = myGyro.getAngle();
  float tempC = myTempSensor.getTempC(myConfig.isGyroTemp());
  float gravitySG = calculateGravity(angle, tempC);
  float corrGravitySG = gravityTemperatureCorrectionC(
      gravitySG, tempC, myAdvancedConfig.getDefaultCalibrationTemp());

  TemplatingEngine engine;
  engine.initialize(angle, gravitySG, corrGravitySG, tempC, 1.0,
                    myBatteryVoltage.getVoltage());

  const String& type = _server->arg(PARAM_PUSH_FORMAT);
  PushTarget push;
  bool enabled = false;

  if (!type.compareTo(PARAM_FORMAT_HTTP1) && myConfig.isHttpActive()) {
    push.sendHttp1(engine, myConfig.isHttpSSL());
    enabled = true;
  } else if (!type.compareTo(PARAM_FORMAT_HTTP2) && myConfig.isHttp2Active()) {
    push.sendHttp2(engine, myConfig.isHttp2SSL());
    enabled = true;
  } else if (!type.compareTo(PARAM_FORMAT_HTTP3) && myConfig.isHttp3Active()) {
    push.sendHttp3(engine, myConfig.isHttp3SSL());
    enabled = true;
  } else if (!type.compareTo(PARAM_FORMAT_INFLUXDB) &&
             myConfig.isInfluxDb2Active()) {
    push.sendInfluxDb2(engine, myConfig.isInfluxSSL());
    enabled = true;
  } else if (!type.compareTo(PARAM_FORMAT_MQTT) && myConfig.isMqttActive()) {
    push.sendMqtt(engine, myConfig.isMqttSSL(), false);
    enabled = true;
  }

  engine.freeMemory();
  DynamicJsonDocument doc(100);
  doc[PARAM_PUSH_ENABLED] = enabled;
  doc[PARAM_PUSH_SUCCESS] = push.getLastSuccess();
  doc[PARAM_PUSH_CODE] = push.getLastCode();

  String out;
  out.reserve(100);
  serializeJson(doc, out);
  doc.clear();

#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  serializeJson(doc, EspSerial);
  EspSerial.print(CR);
#endif

  _server->send(200, "application/json", out.c_str());
  LOG_PERF_STOP("webserver-api-test-push");
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
      file.write((unsigned char*)data.c_str(), data.length());
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

void WebServerHandler::webHandleConfigFormatRead() {
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

  _server->send(200, "application/json", out.c_str());
  LOG_PERF_STOP("webserver-api-config-format-read");
}

void WebServerHandler::webHandleFormulaWrite() {
  LOG_PERF_START("webserver-api-formula-write");
  String id = _server->arg(PARAM_ID);
  Log.notice(F("WEB : webServer callback for /api/formula(post)." CR));

  if (!id.equalsIgnoreCase(myConfig.getID())) {
    Log.error(F("WEB : Wrong ID received %s, expected %s" CR), id.c_str(),
              myConfig.getID());
    _server->send(400, "text/plain", "Invalid ID.");
    LOG_PERF_STOP("webserver-api-formula-write");
    return;
  }

#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  Log.verbose(F("WEB : %s." CR), getRequestArguments().c_str());
#endif

  RawFormulaData fd;
  fd.a[0] = _server->arg("a1").toDouble();
  fd.a[1] = _server->arg("a2").toDouble();
  fd.a[2] = _server->arg("a3").toDouble();
  fd.a[3] = _server->arg("a4").toDouble();
  fd.a[4] = _server->arg("a5").toDouble();
  fd.a[5] = _server->arg("a6").toDouble();
  fd.a[6] = _server->arg("a7").toDouble();
  fd.a[7] = _server->arg("a8").toDouble();
  fd.a[8] = _server->arg("a9").toDouble();
  fd.a[9] = _server->arg("a10").toDouble();

  if (myConfig.isGravityPlato()) {
    fd.g[0] = convertToSG(_server->arg("g1").toDouble());
    fd.g[1] = convertToSG(_server->arg("g2").toDouble());
    fd.g[2] = convertToSG(_server->arg("g3").toDouble());
    fd.g[3] = convertToSG(_server->arg("g4").toDouble());
    fd.g[4] = convertToSG(_server->arg("g5").toDouble());
    fd.g[5] = convertToSG(_server->arg("g6").toDouble());
    fd.g[6] = convertToSG(_server->arg("g7").toDouble());
    fd.g[7] = convertToSG(_server->arg("g8").toDouble());
    fd.g[8] = convertToSG(_server->arg("g9").toDouble());
    fd.g[9] = convertToSG(_server->arg("g10").toDouble());
  } else {
    fd.g[0] = _server->arg("g1").toDouble();
    fd.g[1] = _server->arg("g2").toDouble();
    fd.g[2] = _server->arg("g3").toDouble();
    fd.g[3] = _server->arg("g4").toDouble();
    fd.g[4] = _server->arg("g5").toDouble();
    fd.g[5] = _server->arg("g6").toDouble();
    fd.g[6] = _server->arg("g7").toDouble();
    fd.g[7] = _server->arg("g8").toDouble();
    fd.g[8] = _server->arg("g9").toDouble();
    fd.g[9] = _server->arg("g10").toDouble();
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
  _server->sendHeader("Location", "/calibration.htm", true);
  _server->send(302, "text/plain", "Formula updated");
  LOG_PERF_STOP("webserver-api-formula-write");
}

void WebServerHandler::webHandlePageNotFound() {
  Log.error(F("WEB : URL not found %s received." CR), _server->uri().c_str());
  _server->send(404, "text/plain", F("URL not found"));
}

bool WebServerHandler::setupWebServer() {
  Log.notice(F("WEB : Configuring web server." CR));
  _server = new ESP8266WebServer();
  MDNS.begin(myConfig.getMDNS());
  MDNS.addService("http", "tcp", 80);

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
  _server->on("/", std::bind(&WebServerHandler::webReturnIndexHtm, this));
  _server->on("/index.htm",
              std::bind(&WebServerHandler::webReturnIndexHtm, this));
  _server->on("/config.htm",
              std::bind(&WebServerHandler::webReturnConfigHtm, this));
  _server->on("/calibration.htm",
              std::bind(&WebServerHandler::webReturnCalibrationHtm, this));
  _server->on("/format.htm",
              std::bind(&WebServerHandler::webReturnFormatHtm, this));
  _server->on("/about.htm",
              std::bind(&WebServerHandler::webReturnAboutHtm, this));
  _server->on("/test.htm",
              std::bind(&WebServerHandler::webReturnTestHtm, this));
  _server->on("/firmware.htm",
              std::bind(&WebServerHandler::webReturnFirmwareHtm, this));
  _server->on("/backup.htm",
              std::bind(&WebServerHandler::webReturnBackupHtm, this));
  _server->serveStatic("/log", LittleFS, ERR_FILENAME);
  _server->serveStatic("/log2", LittleFS, ERR_FILENAME2);
  _server->serveStatic("/runtime", LittleFS, RUNTIME_FILENAME);

  // Dynamic content
  _server->on("/api/clearlog", HTTP_GET,
              std::bind(&WebServerHandler::webHandleLogClear, this));
  _server->on("/api/config", HTTP_GET,
              std::bind(&WebServerHandler::webHandleConfig, this));
  _server->on("/api/formula", HTTP_GET,
              std::bind(&WebServerHandler::webHandleFormulaRead, this));
  _server->on("/api/formula", HTTP_POST,
              std::bind(&WebServerHandler::webHandleFormulaWrite, this));
  _server->on("/api/calibrate", HTTP_POST,
              std::bind(&WebServerHandler::webHandleCalibrate, this));
  _server->on("/api/factory", HTTP_GET,
              std::bind(&WebServerHandler::webHandleFactoryDefaults, this));
  _server->on("/api/status", HTTP_GET,
              std::bind(&WebServerHandler::webHandleStatus, this));
  _server->on("/api/clearwifi", HTTP_GET,
              std::bind(&WebServerHandler::webHandleClearWIFI, this));

  _server->on("/api/upload", HTTP_POST,
              std::bind(&WebServerHandler::webReturnOK, this),
              std::bind(&WebServerHandler::webHandleUploadFile,
                        this));  // File upload data
  _server->on("/api/status/sleepmode", HTTP_POST,
              std::bind(&WebServerHandler::webHandleStatusSleepmode,
                        this));  // Change sleep mode
  _server->on("/api/config/device", HTTP_POST,
              std::bind(&WebServerHandler::webHandleConfigDevice,
                        this));  // Change device settings
  _server->on("/api/config/push", HTTP_POST,
              std::bind(&WebServerHandler::webHandleConfigPush,
                        this));  // Change push settings
  _server->on("/api/config/gravity", HTTP_POST,
              std::bind(&WebServerHandler::webHandleConfigGravity,
                        this));  // Change gravity settings
  _server->on("/api/config/hardware", HTTP_POST,
              std::bind(&WebServerHandler::webHandleConfigHardware,
                        this));  // Change hardware settings
  _server->on("/api/config/format", HTTP_GET,
              std::bind(&WebServerHandler::webHandleConfigFormatRead,
                        this));  // Change template formats
  _server->on("/api/config/format", HTTP_POST,
              std::bind(&WebServerHandler::webHandleConfigFormatWrite,
                        this));  // Change template formats
  _server->on("/api/config/advanced", HTTP_GET,
              std::bind(&WebServerHandler::webHandleConfigAdvancedRead,
                        this));  // Read advanced settings
  _server->on("/api/config/advanced", HTTP_POST,
              std::bind(&WebServerHandler::webHandleConfigAdvancedWrite,
                        this));  // Change advanced params
  _server->on("/api/test/push", HTTP_GET,
              std::bind(&WebServerHandler::webHandleTestPush,
                        this));  //

  _server->onNotFound(
      std::bind(&WebServerHandler::webHandlePageNotFound, this));
  _server->begin();
  Log.notice(F("WEB : Web server started." CR));
  return true;
}

void WebServerHandler::loop() {
#if defined(ESP8266)
  MDNS.update();
#endif
  _server->handleClient();
}

// EOF
