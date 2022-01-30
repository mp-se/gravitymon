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
#include <resources.hpp>
#include <templating.hpp>
#include <tempsensor.hpp>
#include <webserver.hpp>
#include <wifi.hpp>

WebServerHandler myWebServerHandler;  // My wrapper class fr webserver functions
extern bool sleepModeActive;
extern bool sleepModeAlwaysSkip;

//
// Callback from webServer when / has been accessed.
//
void WebServerHandler::webHandleDevice() {
  LOG_PERF_START("webserver-api-device");
#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  Log.verbose(F("WEB : webServer callback for /api/device(get)." CR));
#endif

  DynamicJsonDocument doc(100);
  doc[PARAM_ID] = myConfig.getID();
  doc[PARAM_APP_NAME] = CFG_APPNAME;
  doc[PARAM_APP_VER] = CFG_APPVER;
  doc[PARAM_MDNS] = myConfig.getMDNS();
#if LOG_LEVEL == 6
  serializeJson(doc, Serial);
  Serial.print(CR);
#endif
  String out;
  serializeJson(doc, out);
  _server->send(200, "application/json", out.c_str());
  LOG_PERF_STOP("webserver-api-device");
}

//
// Callback from webServer when / has been accessed.
//
void WebServerHandler::webHandleConfig() {
  LOG_PERF_START("webserver-api-config");
  Log.notice(F("WEB : webServer callback for /api/config(get)." CR));

  DynamicJsonDocument doc(CFG_JSON_BUFSIZE);
  myConfig.createJson(doc);

  doc[PARAM_PASS] = "";  // dont show the wifi password

  double angle = myGyro.getAngle();
  double tempC = myTempSensor.getTempC(myConfig.isGyroTemp());
  double gravity = calculateGravity(angle, tempC);

  doc[PARAM_ANGLE] = reduceFloatPrecision(angle);
  doc[PARAM_GRAVITY_FORMAT] = String(myConfig.getGravityFormat());

  if (myConfig.isGravityTempAdj()) {
    gravity =
        gravityTemperatureCorrectionC(gravity, tempC, myConfig.getTempFormat());
  }

  if (myConfig.isGravityPlato()) {
    doc[PARAM_GRAVITY] = reduceFloatPrecision(convertToPlato(gravity), 1);
  } else {
    doc[PARAM_GRAVITY] = reduceFloatPrecision(gravity, 4);
  }

  doc[PARAM_BATTERY] = reduceFloatPrecision(myBatteryVoltage.getVoltage());

#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  serializeJson(doc, Serial);
  Serial.print(CR);
#endif

  String out;
  serializeJson(doc, out);
  _server->send(200, "application/json", out.c_str());
  LOG_PERF_STOP("webserver-api-config");
}

//
// Callback from webServer when / has been accessed.
//
void WebServerHandler::webHandleUpload() {
  LOG_PERF_START("webserver-api-upload");
  Log.notice(F("WEB : webServer callback for /api/upload." CR));
  DynamicJsonDocument doc(100);

  doc["index"] = checkHtmlFile(WebServerHandler::HTML_INDEX);
  doc["device"] = checkHtmlFile(WebServerHandler::HTML_DEVICE);
  doc["config"] = checkHtmlFile(WebServerHandler::HTML_CONFIG);
  doc["calibration"] = checkHtmlFile(WebServerHandler::HTML_CALIBRATION);
  doc["format"] = checkHtmlFile(WebServerHandler::HTML_FORMAT);
  doc["about"] = checkHtmlFile(WebServerHandler::HTML_ABOUT);

#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  serializeJson(doc, Serial);
  Serial.print(CR);
#endif

  String out;
  serializeJson(doc, out);
  _server->send(200, "application/json", out.c_str());
  LOG_PERF_STOP("webserver-api-upload");
}

//
// Callback from webServer when / has been accessed.
//
void WebServerHandler::webHandleUploadFile() {
  LOG_PERF_START("webserver-api-upload-file");
  Log.notice(F("WEB : webServer callback for /api/upload/file." CR));
  HTTPUpload& upload = _server->upload();
  String f = upload.filename;
  bool validFilename = false;

  if (f.equalsIgnoreCase("index.min.htm") ||
      f.equalsIgnoreCase("device.min.htm") ||
      f.equalsIgnoreCase("calibration.min.htm") ||
      f.equalsIgnoreCase("config.min.htm") ||
      f.equalsIgnoreCase("about.min.htm")) {
    validFilename = true;
  }

#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  Log.verbose(F("WEB : webServer callback for /api/upload, receiving file %s, "
                "valid=%s." CR),
              f.c_str(), validFilename ? "yes" : "no");
#endif

  if (upload.status == UPLOAD_FILE_START) {
    Log.notice(F("WEB : Start upload." CR));
    if (validFilename) _uploadFile = LittleFS.open(f, "w");
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    Log.notice(F("WEB : Writing upload." CR));
    if (_uploadFile)
      _uploadFile.write(
          upload.buf,
          upload.currentSize);  // Write the received bytes to the file
  } else if (upload.status == UPLOAD_FILE_END) {
    Log.notice(F("WEB : Finish upload." CR));
    if (_uploadFile) {
      _uploadFile.close();
      Log.notice(F("WEB : File uploaded %d bytes." CR), upload.totalSize);
    }
    _server->sendHeader("Location", "/");
    _server->send(303);
  } else {
    _server->send(500, "text/plain", "Couldn't create file.");
  }
  LOG_PERF_STOP("webserver-api-upload-file");
}

//
// Callback from webServer when / has been accessed.
//
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
  myGyro.calibrateSensor();
  _server->send(200, "text/plain", "Device calibrated");
  LOG_PERF_STOP("webserver-api-calibrate");
}

//
// Callback from webServer when / has been accessed.
//
void WebServerHandler::webHandleFactoryDefaults() {
  String id = _server->arg(PARAM_ID);
  Log.notice(F("WEB : webServer callback for /api/factory." CR));

  if (!id.compareTo(myConfig.getID())) {
    _server->send(200, "text/plain", "Removing configuration and restarting...");
    LittleFS.remove(CFG_FILENAME);
    LittleFS.remove(CFG_HW_FILENAME);
    LittleFS.remove(ERR_FILENAME);
    LittleFS.remove(RUNTIME_FILENAME);
    LittleFS.remove(TPL_FNAME_HTTP1);
    LittleFS.remove(TPL_FNAME_HTTP2);
    LittleFS.remove(TPL_FNAME_INFLUXDB);
    LittleFS.remove(TPL_FNAME_MQTT);
    LittleFS.end();
    delay(500);
    ESP_RESET();
  } else {
    _server->send(400, "text/plain", "Unknown ID.");
  }
}

//
// Callback from webServer when / has been accessed.
//
void WebServerHandler::webHandleStatus() {
  LOG_PERF_START("webserver-api-status");
  Log.notice(F("WEB : webServer callback for /api/status(get)." CR));

  DynamicJsonDocument doc(256);

  double angle = myGyro.getAngle();
  double tempC = myTempSensor.getTempC(myConfig.isGyroTemp());
  double gravity = calculateGravity(angle, tempC);

  doc[PARAM_ID] = myConfig.getID();
  doc[PARAM_ANGLE] = reduceFloatPrecision(angle);
  if (myConfig.isGravityTempAdj()) {
    gravity = gravityTemperatureCorrectionC(gravity, tempC);  //
  }
  if (myConfig.isGravityPlato()) {
    doc[PARAM_GRAVITY] = reduceFloatPrecision(convertToPlato(gravity), 1);
  } else {
    doc[PARAM_GRAVITY] = reduceFloatPrecision(gravity, 4);
  }
  doc[PARAM_TEMP_C] = reduceFloatPrecision(tempC, 1);
  doc[PARAM_TEMP_F] = reduceFloatPrecision(convertCtoF(tempC), 1);
  doc[PARAM_BATTERY] = reduceFloatPrecision(myBatteryVoltage.getVoltage());
  doc[PARAM_TEMPFORMAT] = String(myConfig.getTempFormat());
  doc[PARAM_GRAVITY_FORMAT] = String(myConfig.getGravityFormat());
  doc[PARAM_SLEEP_MODE] = sleepModeAlwaysSkip;
  doc[PARAM_RSSI] = WiFi.RSSI();

#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  serializeJson(doc, Serial);
  Serial.print(CR);
#endif

  String out;
  serializeJson(doc, out);
  _server->send(200, "application/json", out.c_str());
  LOG_PERF_STOP("webserver-api-status");
}

//
// Callback from webServer when / has been accessed.
//
void WebServerHandler::webHandleClearWIFI() {
  String id = _server->arg(PARAM_ID);
  Log.notice(F("WEB : webServer callback for /api/clearwifi." CR));

  if (!id.compareTo(myConfig.getID())) {
    _server->send(200, "text/plain",
                  "Clearing WIFI credentials and doing reset...");
    delay(1000);
    WiFi.disconnect();  // Clear credentials
    ESP_RESET();
  } else {
    _server->send(400, "text/plain", "Unknown ID.");
  }
}

//
// Used to force the device to never sleep.
//
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

//
// Update device settings.
//
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
  _server->sendHeader("Location", "/config.htm#collapseOne", true);
  _server->send(302, "text/plain", "Device config updated");
  LOG_PERF_STOP("webserver-api-config-device");
}

//
// Update push settings.
//
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
  if (_server->hasArg(PARAM_PUSH_BREWFATHER))
    myConfig.setBrewfatherPushUrl(_server->arg(PARAM_PUSH_BREWFATHER).c_str());
  if (_server->hasArg(PARAM_PUSH_INFLUXDB2))
    myConfig.setInfluxDb2PushUrl(_server->arg(PARAM_PUSH_INFLUXDB2).c_str());
  if (_server->hasArg(PARAM_PUSH_INFLUXDB2_ORG))
    myConfig.setInfluxDb2PushOrg(_server->arg(PARAM_PUSH_INFLUXDB2_ORG).c_str());
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
  _server->sendHeader("Location", "/config.htm#collapseTwo", true);
  _server->send(302, "text/plain", "Push config updated");
  LOG_PERF_STOP("webserver-api-config-push");
}

//
// Get string with all received arguments. Used for debugging only.
//
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

//
// Update gravity settings.
//
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
  myConfig.saveFile();
  _server->sendHeader("Location", "/config.htm#collapseThree", true);
  _server->send(302, "text/plain", "Gravity config updated");
  LOG_PERF_STOP("webserver-api-config-gravity");
}

//
// Update hardware settings.
//
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

  if (_server->hasArg(PARAM_VOLTAGEFACTOR))
    myConfig.setVoltageFactor(_server->arg(PARAM_VOLTAGEFACTOR).toFloat());
  if (_server->hasArg(PARAM_TEMP_ADJ)) {
    if (myConfig.isTempC()) {
      myConfig.setTempSensorAdjC(_server->arg(PARAM_TEMP_ADJ));
    } else {
      myConfig.setTempSensorAdjF(_server->arg(PARAM_TEMP_ADJ));
    }
  }
  if (_server->hasArg(PARAM_OTA))
    myConfig.setOtaURL(_server->arg(PARAM_OTA).c_str());
  if (_server->hasArg(PARAM_GYRO_TEMP))
    myConfig.setGyroTemp(
      _server->arg(PARAM_GYRO_TEMP).equalsIgnoreCase("on") ? true : false);
  myConfig.saveFile();
  _server->sendHeader("Location", "/config.htm#collapseFour", true);
  _server->send(302, "text/plain", "Hardware config updated");
  LOG_PERF_STOP("webserver-api-config-hardware");
}

//
// Update device parameters.
//
void WebServerHandler::webHandleDeviceParam() {
  LOG_PERF_START("webserver-api-device-param");
  String id = _server->arg(PARAM_ID);
  Log.notice(F("WEB : webServer callback for /api/device/param(post)." CR));

  if (!id.equalsIgnoreCase(myConfig.getID())) {
    Log.error(F("WEB : Wrong ID received %s, expected %s" CR), id.c_str(),
              myConfig.getID());
    _server->send(400, "text/plain", "Invalid ID.");
    LOG_PERF_STOP("webserver-api-device-param");
    return;
  }

#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  Log.verbose(F("WEB : %s." CR), getRequestArguments().c_str());
#endif

  for (int i = 0; i < _server->args(); i++) {
    String s = _server->arg(i);

    if (_server->argName(i).equalsIgnoreCase(PARAM_HW_GYRO_READ_COUNT))
      myHardwareConfig.setGyroReadCount(s.toInt());
    else if (_server->argName(i).equalsIgnoreCase(PARAM_HW_GYRO_READ_DELAY))
      myHardwareConfig.setGyroReadDelay(s.toInt());
    else if (_server->argName(i).equalsIgnoreCase(
                 PARAM_HW_GYRO_MOVING_THREASHOLD))
      myHardwareConfig.setGyroSensorMovingThreashold(s.toInt());
    else if (_server->argName(i).equalsIgnoreCase(PARAM_HW_FORMULA_DEVIATION))
      myHardwareConfig.setMaxFormulaCreationDeviation(s.toFloat());
    else if (_server->argName(i).equalsIgnoreCase(
                 PARAM_HW_FORMULA_CALIBRATION_TEMP))
      myHardwareConfig.SetDefaultCalibrationTemp(s.toFloat());
    else if (_server->argName(i).equalsIgnoreCase(PARAM_HW_WIFI_PORTALTIMEOUT))
      myHardwareConfig.setWifiPortalTimeout(s.toInt());
  }

  myHardwareConfig.saveFile();

  // Return the current configuration.
  DynamicJsonDocument doc(512);

  doc[PARAM_HW_GYRO_READ_COUNT] = myHardwareConfig.getGyroReadCount();
  doc[PARAM_HW_GYRO_READ_DELAY] = myHardwareConfig.getGyroReadDelay();
  doc[PARAM_HW_GYRO_MOVING_THREASHOLD] =
      myHardwareConfig.getGyroSensorMovingThreashold();
  doc[PARAM_HW_FORMULA_DEVIATION] =
      myHardwareConfig.getMaxFormulaCreationDeviation();
  doc[PARAM_HW_WIFI_PORTALTIMEOUT] = myHardwareConfig.getWifiPortalTimeout();
  doc[PARAM_HW_FORMULA_CALIBRATION_TEMP] =
      myHardwareConfig.getDefaultCalibrationTemp();

#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  serializeJson(doc, Serial);
  Serial.print(CR);
#endif

  String out;
  serializeJson(doc, out);
  _server->send(200, "application/json", out.c_str());
  LOG_PERF_STOP("webserver-api-device-param");
}

//
// Callback from webServer when / has been accessed.
//
void WebServerHandler::webHandleFormulaRead() {
  LOG_PERF_START("webserver-api-formula-read");
  Log.notice(F("WEB : webServer callback for /api/formula(get)." CR));

  DynamicJsonDocument doc(250);
  const RawFormulaData& fd = myConfig.getFormulaData();

#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  Log.verbose(F("WEB : %s." CR), getRequestArguments().c_str());
#endif

  doc[PARAM_ID] = myConfig.getID();
  doc[PARAM_ANGLE] = reduceFloatPrecision(myGyro.getAngle());
  doc[PARAM_GRAVITY_FORMAT] = String(myConfig.getGravityFormat());
  doc[PARAM_GRAVITY_FORMULA] = "";
  doc[PARAM_ERROR] = "";

  switch (_lastFormulaCreateError) {
    case ERR_FORMULA_INTERNAL:
      doc[PARAM_ERROR] = "Internal error creating formula.";
      break;
    case ERR_FORMULA_NOTENOUGHVALUES:
      doc[PARAM_ERROR] = "Not enough values to create formula.";
      break;
    case ERR_FORMULA_UNABLETOFFIND:
      doc[PARAM_ERROR] = "Unable to find an accurate formula based on input.";
      break;
    default:
      doc[PARAM_GRAVITY_FORMULA] = myConfig.getGravityFormula();
      break;
  }

  doc["a1"] = reduceFloatPrecision(fd.a[0], 2);
  doc["a2"] = reduceFloatPrecision(fd.a[1], 2);
  doc["a3"] = reduceFloatPrecision(fd.a[2], 2);
  doc["a4"] = reduceFloatPrecision(fd.a[3], 2);
  doc["a5"] = reduceFloatPrecision(fd.a[4], 2);

  if (myConfig.isGravityPlato()) {
    doc["g1"] = reduceFloatPrecision(convertToPlato(fd.g[0]), 1);
    doc["g2"] = reduceFloatPrecision(convertToPlato(fd.g[1]), 1);
    doc["g3"] = reduceFloatPrecision(convertToPlato(fd.g[2]), 1);
    doc["g4"] = reduceFloatPrecision(convertToPlato(fd.g[3]), 1);
    doc["g5"] = reduceFloatPrecision(convertToPlato(fd.g[4]), 1);
  } else {
    doc["g1"] = reduceFloatPrecision(fd.g[0], 4);
    doc["g2"] = reduceFloatPrecision(fd.g[1], 4);
    doc["g3"] = reduceFloatPrecision(fd.g[2], 4);
    doc["g4"] = reduceFloatPrecision(fd.g[3], 4);
    doc["g5"] = reduceFloatPrecision(fd.g[4], 4);
  }

#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  serializeJson(doc, Serial);
  Serial.print(CR);
#endif

  String out;
  serializeJson(doc, out);
  _server->send(200, "application/json", out.c_str());
  LOG_PERF_STOP("webserver-api-formula-read");
}

//
// Update format template
//
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
  } else if (_server->hasArg(PARAM_FORMAT_INFLUXDB)) {
    success =
        writeFile(TPL_FNAME_INFLUXDB, _server->arg(PARAM_FORMAT_INFLUXDB));
  } else if (_server->hasArg(PARAM_FORMAT_MQTT)) {
    success = writeFile(TPL_FNAME_MQTT, _server->arg(PARAM_FORMAT_MQTT));
  }
  /*else if (_server->hasArg(PARAM_FORMAT_BREWFATHER)) {
    success = writeFile(TPL_FNAME_BREWFATHER,
  _server->arg(PARAM_FORMAT_BREWFATHER));
  }*/

  if (success) {
    _server->sendHeader("Location", "/format.htm", true);
    _server->send(302, "text/plain", "Format updated");
  } else {
    myLastErrors.addEntry(F("WEB : Unable to store format file"));
    _server->send(400, "text/plain", "Unable to store format in file.");
  }

  LOG_PERF_STOP("webserver-api-config-format-write");
}

//
// Write file to disk, if there is no data then delete the current file (if it
// exists) = reset to default.
//
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

//
// Read file from disk
//
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

//
// Get format templates
//
void WebServerHandler::webHandleConfigFormatRead() {
  LOG_PERF_START("webserver-api-config-format-read");
  Log.notice(F("WEB : webServer callback for /api/config/formula(get)." CR));

  DynamicJsonDocument doc(2048);

  doc[PARAM_ID] = myConfig.getID();

  String s = readFile(TPL_FNAME_HTTP1);
  if (s.length())
    doc[PARAM_FORMAT_HTTP1] = urlencode(s);
  else
    doc[PARAM_FORMAT_HTTP1] = urlencode(String(&iSpindleFormat[0]));

  s = readFile(TPL_FNAME_HTTP2);
  if (s.length())
    doc[PARAM_FORMAT_HTTP2] = urlencode(s);
  else
    doc[PARAM_FORMAT_HTTP2] = urlencode(String(&iSpindleFormat[0]));

  /*s = readFile(TPL_FNAME_BREWFATHER);
  if (s.length())
    doc[PARAM_FORMAT_BREWFATHER] = urlencode(s);
  else
    doc[PARAM_FORMAT_BREWFATHER] = urlencode(&brewfatherFormat[0]);*/

  s = readFile(TPL_FNAME_INFLUXDB);
  if (s.length())
    doc[PARAM_FORMAT_INFLUXDB] = urlencode(s);
  else
    doc[PARAM_FORMAT_INFLUXDB] = urlencode(String(&influxDbFormat[0]));

  s = readFile(TPL_FNAME_MQTT);
  if (s.length())
    doc[PARAM_FORMAT_MQTT] = urlencode(s);
  else
    doc[PARAM_FORMAT_MQTT] = urlencode(String(&mqttFormat[0]));

#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  serializeJson(doc, Serial);
  Serial.print(CR);
#endif

  String out;
  serializeJson(doc, out);
  _server->send(200, "application/json", out.c_str());
  LOG_PERF_STOP("webserver-api-config-format-read");
}

//
// Update hardware settings.
//
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

  if (myConfig.isGravityPlato()) {
    fd.g[0] = convertToSG(_server->arg("g1").toDouble());
    fd.g[1] = convertToSG(_server->arg("g2").toDouble());
    fd.g[2] = convertToSG(_server->arg("g3").toDouble());
    fd.g[3] = convertToSG(_server->arg("g4").toDouble());
    fd.g[4] = convertToSG(_server->arg("g5").toDouble());
  } else {
    fd.g[0] = _server->arg("g1").toDouble();
    fd.g[1] = _server->arg("g2").toDouble();
    fd.g[2] = _server->arg("g3").toDouble();
    fd.g[3] = _server->arg("g4").toDouble();
    fd.g[4] = _server->arg("g5").toDouble();
  }

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

//
// Helper function to check if files exist on file system.
//
const char* WebServerHandler::getHtmlFileName(HtmlFile item) {
  Log.notice(F("WEB : Looking up filename for %d." CR), item);

  switch (item) {
    case HTML_INDEX:
      return "index.min.htm";
    case HTML_DEVICE:
      return "device.min.htm";
    case HTML_CONFIG:
      return "config.min.htm";
    case HTML_CALIBRATION:
      return "calibration.min.htm";
    case HTML_FORMAT:
      return "format.min.htm";
    case HTML_ABOUT:
      return "about.min.htm";
  }

  return "";
}

//
// Helper function to check if files exist on file system.
//
bool WebServerHandler::checkHtmlFile(HtmlFile item) {
  const char* fn = getHtmlFileName(item);

#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  Log.verbose(F("WEB : Checking for file %s." CR), fn);
#endif

  // TODO: We might need to add more checks here like zero file size etc. But
  // for now we only check if the file exist.

  return LittleFS.exists(fn);
}

//
// Handler for page not found
//
void WebServerHandler::webHandlePageNotFound() {
  Log.error(F("WEB : URL not found %s received." CR), _server->uri().c_str());
  _server->send(404, "text/plain", F("URL not found"));
}

//
// Setup the Web Server callbacks and start it
//
bool WebServerHandler::setupWebServer() {
  Log.notice(F("WEB : Configuring web server." CR));

  _server = new ESP8266WebServer();

  MDNS.begin(myConfig.getMDNS());
  MDNS.addService("http", "tcp", 80);

  // Static content
#if defined(EMBED_HTML)
  _server->on("/", std::bind(&WebServerHandler::webReturnIndexHtm, this));
  _server->on("/index.htm",
              std::bind(&WebServerHandler::webReturnIndexHtm, this));
  _server->on("/device.htm",
              std::bind(&WebServerHandler::webReturnDeviceHtm, this));
  _server->on("/config.htm",
              std::bind(&WebServerHandler::webReturnConfigHtm, this));
  _server->on("/calibration.htm",
              std::bind(&WebServerHandler::webReturnCalibrationHtm, this));
  _server->on("/format.htm",
              std::bind(&WebServerHandler::webReturnFormatHtm, this));
  _server->on("/about.htm",
              std::bind(&WebServerHandler::webReturnAboutHtm, this));
#else
  // Show files in the filessytem at startup

  FSInfo fs;
  LittleFS.info(fs);
  Log.notice(F("WEB : File system Total=%d, Used=%d." CR), fs.totalBytes,
             fs.usedBytes);
  Dir dir = LittleFS.openDir("/");
  while (dir.next()) {
    Log.notice(F("WEB : File=%s, %d bytes" CR), dir.fileName().c_str(),
               dir.fileSize());
  }

  // Check if the html files exist, if so serve them, else show the static
  // upload page.
  if (checkHtmlFile(HTML_INDEX) && checkHtmlFile(HTML_DEVICE) &&
      checkHtmlFile(HTML_CONFIG) && checkHtmlFile(HTML_CALIBRATION) &&
      checkHtmlFile(HTML_FORMAT) && checkHtmlFile(HTML_ABOUT)) {
    Log.notice(F("WEB : All html files exist, starting in normal mode." CR));

    _server->serveStatic("/", LittleFS, "/index.min.htm");
    _server->serveStatic("/index.htm", LittleFS, "/index.min.htm");
    _server->serveStatic("/device.htm", LittleFS, "/device.min.htm");
    _server->serveStatic("/config.htm", LittleFS, "/config.min.htm");
    _server->serveStatic("/about.htm", LittleFS, "/about.min.htm");
    _server->serveStatic("/calibration.htm", LittleFS, "/calibration.min.htm");
    _server->serveStatic("/format.htm", LittleFS, "/format.min.htm");

    // Also add the static upload view in case we we have issues that needs to
    // be fixed.
    _server->on("/upload.htm",
                std::bind(&WebServerHandler::webReturnUploadHtm, this));
  } else {
    Log.error(F("WEB : Missing html files, starting with upload UI." CR));
    _server->on("/", std::bind(&WebServerHandler::webReturnUploadHtm, this));
  }
#endif
  _server->serveStatic("/log", LittleFS, ERR_FILENAME);
  _server->serveStatic("/runtime", LittleFS, RUNTIME_FILENAME);

  // Dynamic content
  _server->on(
      "/api/config", HTTP_GET,
      std::bind(&WebServerHandler::webHandleConfig, this));  // Get config.json
  _server->on(
      "/api/device", HTTP_GET,
      std::bind(&WebServerHandler::webHandleDevice, this));  // Get device.json
  _server->on("/api/formula", HTTP_GET,
              std::bind(&WebServerHandler::webHandleFormulaRead,
                        this));  // Get formula.json (calibration page)
  _server->on("/api/formula", HTTP_POST,
              std::bind(&WebServerHandler::webHandleFormulaWrite,
                        this));  // Get formula.json (calibration page)
  _server->on("/api/calibrate", HTTP_POST,
              std::bind(&WebServerHandler::webHandleCalibrate,
                        this));  // Run calibration routine (param id)
  _server->on("/api/factory", HTTP_GET,
              std::bind(&WebServerHandler::webHandleFactoryDefaults,
                        this));  // Reset the device
  _server->on("/api/status", HTTP_GET,
              std::bind(&WebServerHandler::webHandleStatus,
                        this));  // Get the status.json
  _server->on("/api/clearwifi", HTTP_GET,
              std::bind(&WebServerHandler::webHandleClearWIFI,
                        this));  // Clear wifi settings
  _server->on(
      "/api/upload", HTTP_GET,
      std::bind(&WebServerHandler::webHandleUpload, this));  // Get upload.json

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
  _server->on("/api/device/param", HTTP_GET,
              std::bind(&WebServerHandler::webHandleDeviceParam,
                        this));  // Change device params

  _server->onNotFound(
      std::bind(&WebServerHandler::webHandlePageNotFound, this));
  _server->begin();
  Log.notice(F("WEB : Web server started." CR));
  return true;
}

//
// called from main loop
//
void WebServerHandler::loop() {
#if defined(ESP8266)
  MDNS.update();
#endif
  _server->handleClient();
}

// EOF
