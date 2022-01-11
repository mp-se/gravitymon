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
#include <ArduinoJson.h>
#include <LittleFS.h>

#include <calc.hpp>
#include <config.hpp>
#include <gyro.hpp>
#include <helper.hpp>
#include <tempsensor.hpp>
#include <webserver.hpp>

WebServer myWebServer;  // My wrapper class fr webserver functions
extern bool sleepModeActive;
extern bool sleepModeAlwaysSkip;

//
// Callback from webServer when / has been accessed.
//
void WebServer::webHandleDevice() {
  LOG_PERF_START("webserver-api-device");
#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  Log.verbose(F("WEB : webServer callback for /api/device." CR));
#endif

  DynamicJsonDocument doc(100);
  doc[CFG_PARAM_ID] = myConfig.getID();
  doc[CFG_PARAM_APP_NAME] = CFG_APPNAME;
  doc[CFG_PARAM_APP_VER] = CFG_APPVER;
  doc[CFG_PARAM_MDNS] = myConfig.getMDNS();
#if LOG_LEVEL == 6
  serializeJson(doc, Serial);
  Serial.print(CR);
#endif
  String out;
  serializeJson(doc, out);
  server->send(200, "application/json", out.c_str());
  LOG_PERF_STOP("webserver-api-device");
}

//
// Callback from webServer when / has been accessed.
//
void WebServer::webHandleConfig() {
  LOG_PERF_START("webserver-api-config");
  Log.notice(F("WEB : webServer callback for /api/config." CR));

  DynamicJsonDocument doc(CFG_JSON_BUFSIZE);
  myConfig.createJson(doc);

  doc[CFG_PARAM_PASS] = "";  // dont show the wifi password

  double angle = myGyro.getAngle();
  double temp = myTempSensor.getTempC(myConfig.isGyroTemp());
  double gravity = calculateGravity(angle, temp);

  doc[CFG_PARAM_ANGLE] = reduceFloatPrecision(angle);
  if (myConfig.isGravityTempAdj())
    doc[CFG_PARAM_GRAVITY] = reduceFloatPrecision(
        gravityTemperatureCorrection(gravity, temp, myConfig.getTempFormat()),
        4);
  else
    doc[CFG_PARAM_GRAVITY] = reduceFloatPrecision(gravity, 4);
  doc[CFG_PARAM_BATTERY] = reduceFloatPrecision(myBatteryVoltage.getVoltage());

#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  serializeJson(doc, Serial);
  Serial.print(CR);
#endif

  String out;
  serializeJson(doc, out);
  server->send(200, "application/json", out.c_str());
  LOG_PERF_STOP("webserver-api-config");
}

//
// Callback from webServer when / has been accessed.
//
void WebServer::webHandleUpload() {
  LOG_PERF_START("webserver-api-upload");
  Log.notice(F("WEB : webServer callback for /api/upload." CR));
  DynamicJsonDocument doc(100);

  doc["index"] = myWebServer.checkHtmlFile(WebServer::HTML_INDEX);
  doc["device"] = myWebServer.checkHtmlFile(WebServer::HTML_DEVICE);
  doc["config"] = myWebServer.checkHtmlFile(WebServer::HTML_CONFIG);
  doc["calibration"] = myWebServer.checkHtmlFile(WebServer::HTML_CALIBRATION);
  doc["about"] = myWebServer.checkHtmlFile(WebServer::HTML_ABOUT);

#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  serializeJson(doc, Serial);
  Serial.print(CR);
#endif

  String out;
  serializeJson(doc, out);
  server->send(200, "application/json", out.c_str());
  LOG_PERF_STOP("webserver-api-upload");
}

//
// Callback from webServer when / has been accessed.
//
void WebServer::webHandleUploadFile() {
  LOG_PERF_START("webserver-api-upload-file");
  Log.notice(F("WEB : webServer callback for /api/upload/file." CR));
  HTTPUpload& upload = server->upload();
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
    if (validFilename) uploadFile = LittleFS.open(f, "w");
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    Log.notice(F("WEB : Writing upload." CR));
    if (uploadFile)
      uploadFile.write(
          upload.buf,
          upload.currentSize);  // Write the received bytes to the file
  } else if (upload.status == UPLOAD_FILE_END) {
    Log.notice(F("WEB : Finish upload." CR));
    if (uploadFile) {
      uploadFile.close();
      Log.notice(F("WEB : File uploaded %d bytes." CR), upload.totalSize);
    }
    server->sendHeader("Location", "/");
    server->send(303);
  } else {
    server->send(500, "text/plain", "Couldn't create file.");
  }
  LOG_PERF_STOP("webserver-api-upload-file");
}

//
// Callback from webServer when / has been accessed.
//
void WebServer::webHandleCalibrate() {
  LOG_PERF_START("webserver-api-calibrate");
  String id = server->arg(CFG_PARAM_ID);
  Log.notice(F("WEB : webServer callback for /api/calibrate." CR));

  if (!id.equalsIgnoreCase(myConfig.getID())) {
    Log.error(F("WEB : Wrong ID received %s, expected %s" CR), id.c_str(),
              myConfig.getID());
    server->send(400, "text/plain", "Invalid ID.");
    LOG_PERF_STOP("webserver-api-calibrate");
    return;
  }
  myGyro.calibrateSensor();
  server->send(200, "text/plain", "Device calibrated");
  LOG_PERF_STOP("webserver-api-calibrate");
}

//
// Callback from webServer when / has been accessed.
//
void WebServer::webHandleFactoryReset() {
  String id = server->arg(CFG_PARAM_ID);
  Log.notice(F("WEB : webServer callback for /api/factory." CR));

  if (!id.compareTo(myConfig.getID())) {
    server->send(200, "text/plain", "Doing reset...");
    LittleFS.remove(CFG_FILENAME);
    LittleFS.end();
    delay(500);
    ESP.reset();
  } else {
    server->send(400, "text/plain", "Unknown ID.");
  }
}

//
// Callback from webServer when / has been accessed.
//
void WebServer::webHandleStatus() {
  LOG_PERF_START("webserver-api-status");
  Log.notice(F("WEB : webServer callback for /api/status." CR));

  DynamicJsonDocument doc(256);

  double angle = myGyro.getAngle();
  double temp = myTempSensor.getTempC(myConfig.isGyroTemp());
  double gravity = calculateGravity(angle, temp);

  doc[CFG_PARAM_ID] = myConfig.getID();
  doc[CFG_PARAM_ANGLE] = reduceFloatPrecision(angle);
  if (myConfig.isGravityTempAdj())
    doc[CFG_PARAM_GRAVITY] = reduceFloatPrecision(
        gravityTemperatureCorrection(gravity, temp, myConfig.getTempFormat()),
        4);
  else
    doc[CFG_PARAM_GRAVITY] = reduceFloatPrecision(gravity, 4);
  doc[CFG_PARAM_TEMP_C] = reduceFloatPrecision(temp, 1);
  doc[CFG_PARAM_TEMP_F] =
      reduceFloatPrecision(myTempSensor.getTempF(myConfig.isGyroTemp()), 1);
  doc[CFG_PARAM_BATTERY] = reduceFloatPrecision(myBatteryVoltage.getVoltage());
  doc[CFG_PARAM_TEMPFORMAT] = String(myConfig.getTempFormat());
  doc[CFG_PARAM_SLEEP_MODE] = sleepModeAlwaysSkip;
  doc[CFG_PARAM_RSSI] = WiFi.RSSI();

#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  serializeJson(doc, Serial);
  Serial.print(CR);
#endif

  String out;
  serializeJson(doc, out);
  server->send(200, "application/json", out.c_str());
  LOG_PERF_STOP("webserver-api-status");
}

//
// Callback from webServer when / has been accessed.
//
void WebServer::webHandleClearWIFI() {
  String id = server->arg(CFG_PARAM_ID);
  Log.notice(F("WEB : webServer callback for /api/clearwifi." CR));

  if (!id.compareTo(myConfig.getID())) {
    server->send(200, "text/plain",
                 "Clearing WIFI credentials and doing reset...");
    delay(1000);
    WiFi.disconnect();  // Clear credentials
    ESP.reset();
  } else {
    server->send(400, "text/plain", "Unknown ID.");
  }
}

//
// Used to force the device to never sleep.
//
void WebServer::webHandleStatusSleepmode() {
  LOG_PERF_START("webserver-api-sleepmode");
  String id = server->arg(CFG_PARAM_ID);
  Log.notice(F("WEB : webServer callback for /api/status/sleepmode." CR));

  if (!id.equalsIgnoreCase(myConfig.getID())) {
    Log.error(F("WEB : Wrong ID received %s, expected %s" CR), id.c_str(),
              myConfig.getID());
    server->send(400, "text/plain", "Invalid ID.");
    LOG_PERF_STOP("webserver-api-sleepmode");
    return;
  }

#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  Log.verbose(F("WEB : sleep-mode=%s." CR),
              server->arg(CFG_PARAM_SLEEP_MODE).c_str());
#endif

  if (server->arg(CFG_PARAM_SLEEP_MODE).equalsIgnoreCase("true"))
    sleepModeAlwaysSkip = true;
  else
    sleepModeAlwaysSkip = false;
  server->send(200, "text/plain", "Sleep mode updated");
  LOG_PERF_STOP("webserver-api-sleepmode");
}

//
// Update device settings.
//
void WebServer::webHandleConfigDevice() {
  LOG_PERF_START("webserver-api-config-device");
  String id = server->arg(CFG_PARAM_ID);
  Log.notice(F("WEB : webServer callback for /api/config/device." CR));

  if (!id.equalsIgnoreCase(myConfig.getID())) {
    Log.error(F("WEB : Wrong ID received %s, expected %s" CR), id.c_str(),
              myConfig.getID());
    server->send(400, "text/plain", "Invalid ID.");
    LOG_PERF_STOP("webserver-api-config-device");
    return;
  }

#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  Log.verbose(F("WEB : mdns=%s, temp-format=%s." CR),
              server->arg(CFG_PARAM_MDNS).c_str(),
              server->arg(CFG_PARAM_TEMPFORMAT).c_str());
#endif

  myConfig.setMDNS(server->arg(CFG_PARAM_MDNS).c_str());
  myConfig.setTempFormat(server->arg(CFG_PARAM_TEMPFORMAT).charAt(0));
  myConfig.setSleepInterval(server->arg(CFG_PARAM_SLEEP_INTERVAL).c_str());
  myConfig.saveFile();
  server->sendHeader("Location", "/config.htm#collapseOne", true);
  server->send(302, "text/plain", "Device config updated");
  LOG_PERF_STOP("webserver-api-config-device");
}

//
// Update push settings.
//
void WebServer::webHandleConfigPush() {
  LOG_PERF_START("webserver-api-config-push");
  String id = server->arg(CFG_PARAM_ID);
  Log.notice(F("WEB : webServer callback for /api/config/push." CR));

  if (!id.equalsIgnoreCase(myConfig.getID())) {
    Log.error(F("WEB : Wrong ID received %s, expected %s" CR), id.c_str(),
              myConfig.getID());
    server->send(400, "text/plain", "Invalid ID.");
    LOG_PERF_STOP("webserver-api-config-push");
    return;
  }
#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  Log.verbose(F("WEB : http=%s,%s, bf=%s influx2=%s, %s, %s, %s." CR),
              server->arg(CFG_PARAM_PUSH_HTTP).c_str(),
              server->arg(CFG_PARAM_PUSH_HTTP2).c_str(),
              server->arg(CFG_PARAM_PUSH_BREWFATHER).c_str(),
              server->arg(CFG_PARAM_PUSH_INFLUXDB2).c_str(),
              server->arg(CFG_PARAM_PUSH_INFLUXDB2_ORG).c_str(),
              server->arg(CFG_PARAM_PUSH_INFLUXDB2_BUCKET).c_str(),
              server->arg(CFG_PARAM_PUSH_INFLUXDB2_AUTH).c_str());
#endif

  myConfig.setHttpPushUrl(server->arg(CFG_PARAM_PUSH_HTTP).c_str());
  myConfig.setHttpPushUrl2(server->arg(CFG_PARAM_PUSH_HTTP2).c_str());
  myConfig.setBrewfatherPushUrl(server->arg(CFG_PARAM_PUSH_BREWFATHER).c_str());
  myConfig.setInfluxDb2PushUrl(server->arg(CFG_PARAM_PUSH_INFLUXDB2).c_str());
  myConfig.setInfluxDb2PushOrg(
      server->arg(CFG_PARAM_PUSH_INFLUXDB2_ORG).c_str());
  myConfig.setInfluxDb2PushBucket(
      server->arg(CFG_PARAM_PUSH_INFLUXDB2_BUCKET).c_str());
  myConfig.setInfluxDb2PushToken(
      server->arg(CFG_PARAM_PUSH_INFLUXDB2_AUTH).c_str());
  myConfig.saveFile();
  server->sendHeader("Location", "/config.htm#collapseTwo", true);
  server->send(302, "text/plain", "Push config updated");
  LOG_PERF_STOP("webserver-api-config-push");
}

//
// Update gravity settings.
//
void WebServer::webHandleConfigGravity() {
  LOG_PERF_START("webserver-api-config-gravity");
  String id = server->arg(CFG_PARAM_ID);
  Log.notice(F("WEB : webServer callback for /api/config/gravity." CR));

  if (!id.equalsIgnoreCase(myConfig.getID())) {
    Log.error(F("WEB : Wrong ID received %s, expected %s" CR), id.c_str(),
              myConfig.getID());
    server->send(400, "text/plain", "Invalid ID.");
    LOG_PERF_STOP("webserver-api-config-gravity");
    return;
  }

#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  Log.verbose(F("WEB : formula=%s, temp-corr=%s." CR),
              server->arg(CFG_PARAM_GRAVITY_FORMULA).c_str(),
              server->arg(CFG_PARAM_GRAVITY_TEMP_ADJ).c_str());
#endif

  myConfig.setGravityFormula(server->arg(CFG_PARAM_GRAVITY_FORMULA).c_str());
  myConfig.setGravityTempAdj(
      server->arg(CFG_PARAM_GRAVITY_TEMP_ADJ).equalsIgnoreCase("on") ? true
                                                                     : false);
  myConfig.saveFile();
  server->sendHeader("Location", "/config.htm#collapseThree", true);
  server->send(302, "text/plain", "Gravity config updated");
  LOG_PERF_STOP("webserver-api-config-gravity");
}

//
// Update hardware settings.
//
void WebServer::webHandleConfigHardware() {
  LOG_PERF_START("webserver-api-config-hardware");
  String id = server->arg(CFG_PARAM_ID);
  Log.notice(F("WEB : webServer callback for /api/config/hardware." CR));

  if (!id.equalsIgnoreCase(myConfig.getID())) {
    Log.error(F("WEB : Wrong ID received %s, expected %s" CR), id.c_str(),
              myConfig.getID());
    server->send(400, "text/plain", "Invalid ID.");
    LOG_PERF_STOP("webserver-api-config-hardware");
    return;
  }

#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  Log.verbose(F("WEB : vf=%s, tempadj=%s, ota=%s gyrotemp=%s." CR),
              server->arg(CFG_PARAM_VOLTAGEFACTOR).c_str(),
              server->arg(CFG_PARAM_TEMP_ADJ).c_str(),
              server->arg(CFG_PARAM_OTA).c_str(),
              server->arg(CFG_PARAM_GYRO_TEMP).c_str());
#endif

  myConfig.setVoltageFactor(server->arg(CFG_PARAM_VOLTAGEFACTOR).toFloat());
  myConfig.setTempSensorAdj(server->arg(CFG_PARAM_TEMP_ADJ).toFloat());
  myConfig.setOtaURL(server->arg(CFG_PARAM_OTA).c_str());
  myConfig.setGyroTemp(
      server->arg(CFG_PARAM_GYRO_TEMP).equalsIgnoreCase("on") ? true : false);
  myConfig.saveFile();
  server->sendHeader("Location", "/config.htm#collapseFour", true);
  server->send(302, "text/plain", "Hardware config updated");
  LOG_PERF_STOP("webserver-api-config-hardware");
}

//
// Callback from webServer when / has been accessed.
//
void WebServer::webHandleFormulaRead() {
  LOG_PERF_START("webserver-api-formula-read");
  Log.notice(F("WEB : webServer callback for /api/formula/get." CR));

  DynamicJsonDocument doc(250);
  const RawFormulaData& fd = myConfig.getFormulaData();

#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  Log.verbose(F("WEB : %F %F %F %F %F." CR), fd.a[0], fd.a[1], fd.a[2], fd.a[3],
              fd.a[4]);
  Log.verbose(F("WEB : %F %F %F %F %F." CR), fd.g[0], fd.g[1], fd.g[2], fd.g[3],
              fd.g[4]);
#endif

  doc[CFG_PARAM_ID] = myConfig.getID();
  doc[CFG_PARAM_ANGLE] = reduceFloatPrecision(myGyro.getAngle());

  switch (lastFormulaCreateError) {
    case ERR_FORMULA_INTERNAL:
      doc[CFG_PARAM_GRAVITY_FORMULA] = "Internal error creating formula.";
      break;
    case ERR_FORMULA_NOTENOUGHVALUES:
      doc[CFG_PARAM_GRAVITY_FORMULA] = "Not enough values to create formula.";
      break;
    case ERR_FORMULA_UNABLETOFFIND:
      doc[CFG_PARAM_GRAVITY_FORMULA] =
          "Unable to find an accurate formula based on input.";
      break;
    default:
      doc[CFG_PARAM_GRAVITY_FORMULA] = myConfig.getGravityFormula();
      break;
  }

  doc["a1"] = reduceFloatPrecision(fd.a[0], 2);
  doc["a2"] = reduceFloatPrecision(fd.a[1], 2);
  doc["a3"] = reduceFloatPrecision(fd.a[2], 2);
  doc["a4"] = reduceFloatPrecision(fd.a[3], 2);
  doc["a5"] = reduceFloatPrecision(fd.a[4], 2);

  doc["g1"] = reduceFloatPrecision(fd.g[0], 4);
  doc["g2"] = reduceFloatPrecision(fd.g[1], 4);
  doc["g3"] = reduceFloatPrecision(fd.g[2], 4);
  doc["g4"] = reduceFloatPrecision(fd.g[3], 4);
  doc["g5"] = reduceFloatPrecision(fd.g[4], 4);

#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  serializeJson(doc, Serial);
  Serial.print(CR);
#endif

  String out;
  serializeJson(doc, out);
  server->send(200, "application/json", out.c_str());
  LOG_PERF_STOP("webserver-api-formula-read");
}

//
// Update hardware settings.
//
void WebServer::webHandleFormulaWrite() {
  LOG_PERF_START("webserver-api-formula-write");
  String id = server->arg(CFG_PARAM_ID);
  Log.notice(F("WEB : webServer callback for /api/formula/post." CR));

  if (!id.equalsIgnoreCase(myConfig.getID())) {
    Log.error(F("WEB : Wrong ID received %s, expected %s" CR), id.c_str(),
              myConfig.getID());
    server->send(400, "text/plain", "Invalid ID.");
    LOG_PERF_STOP("webserver-api-formula-write");
    return;
  }

#if LOG_LEVEL == 6 && !defined(WEB_DISABLE_LOGGING)
  Log.verbose(F("WEB : angles=%F,%F,%F,%F,%F." CR), server->arg("a1").toFloat(),
              server->arg("a2").toFloat(), server->arg("a3").toFloat(),
              server->arg("a4").toFloat(), server->arg("a5").toFloat());
  Log.verbose(F("WEB : gravity=%F,%F,%F,%F,%F." CR),
              server->arg("g1").toFloat(), server->arg("g2").toFloat(),
              server->arg("g3").toFloat(), server->arg("g4").toFloat(),
              server->arg("g5").toFloat());
#endif

  RawFormulaData fd;
  fd.a[0] = server->arg("a1").toDouble();
  fd.a[1] = server->arg("a2").toDouble();
  fd.a[2] = server->arg("a3").toDouble();
  fd.a[3] = server->arg("a4").toDouble();
  fd.a[4] = server->arg("a5").toDouble();
  fd.g[0] = server->arg("g1").toDouble();
  fd.g[1] = server->arg("g2").toDouble();
  fd.g[2] = server->arg("g3").toDouble();
  fd.g[3] = server->arg("g4").toDouble();
  fd.g[4] = server->arg("g5").toDouble();
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
    lastFormulaCreateError = e;
  } else {
    // Save the formula as succesful
    Log.info(F("WEB : Found valid formula: '%s'" CR), &buf[0]);
    myConfig.setGravityFormula(buf);
    lastFormulaCreateError = 0;
  }

  myConfig.saveFile();
  server->sendHeader("Location", "/calibration.htm", true);
  server->send(302, "text/plain", "Formula updated");
  LOG_PERF_STOP("webserver-api-formula-write");
}

//
// Helper function to check if files exist on file system.
//
const char* WebServer::getHtmlFileName(HtmlFile item) {
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
    case HTML_ABOUT:
      return "about.min.htm";
  }

  return "";
}

//
// Helper function to check if files exist on file system.
//
bool WebServer::checkHtmlFile(HtmlFile item) {
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
void WebServer::webHandlePageNotFound() {
  Log.error(F("WEB : URL not found %s received." CR), server->uri().c_str());
  server->send(404, "text/plain", F("URL not found"));
}

//
// Setup the Web Server callbacks and start it
//
bool WebServer::setupWebServer() {
  Log.notice(F("WEB : Configuring web server." CR));

  server = new ESP8266WebServer();

  MDNS.begin(myConfig.getMDNS());
  MDNS.addService("http", "tcp", 80);

  // Static content
#if defined(EMBED_HTML)
  server->on("/", std::bind(&WebServer::webReturnIndexHtm, this));
  server->on("/index.htm", std::bind(&WebServer::webReturnIndexHtm, this));
  server->on("/device.htm", std::bind(&WebServer::webReturnDeviceHtm, this));
  server->on("/config.htm", std::bind(&WebServer::webReturnConfigHtm, this));
  server->on("/calibration.htm",
             std::bind(&WebServer::webReturnCalibrationHtm, this));
  server->on("/about.htm", std::bind(&WebServer::webReturnAboutHtm, this));
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
      checkHtmlFile(HTML_ABOUT)) {
    Log.notice(F("WEB : All html files exist, starting in normal mode." CR));

    server->serveStatic("/", LittleFS, "/index.min.htm");
    server->serveStatic("/index.htm", LittleFS, "/index.min.htm");
    server->serveStatic("/device.htm", LittleFS, "/device.min.htm");
    server->serveStatic("/config.htm", LittleFS, "/config.min.htm");
    server->serveStatic("/about.htm", LittleFS, "/about.min.htm");
    server->serveStatic("/calibration.htm", LittleFS, "/calibration.min.htm");

    // Also add the static upload view in case we we have issues that needs to
    // be fixed.
    server->on("/upload.htm", std::bind(&WebServer::webReturnUploadHtm, this));
  } else {
    Log.error(F("WEB : Missing html files, starting with upload UI." CR));
    server->on("/", std::bind(&WebServer::webReturnUploadHtm, this));
  }
#endif

  // Dynamic content
  server->on("/api/config", HTTP_GET,
             std::bind(&WebServer::webHandleConfig, this));  // Get config.json
  server->on("/api/device", HTTP_GET,
             std::bind(&WebServer::webHandleDevice, this));  // Get device.json
  server->on("/api/formula", HTTP_GET,
             std::bind(&WebServer::webHandleFormulaRead,
                       this));  // Get formula.json (calibration page)
  server->on("/api/formula", HTTP_POST,
             std::bind(&WebServer::webHandleFormulaWrite,
                       this));  // Get formula.json (calibration page)
  server->on("/api/calibrate", HTTP_POST,
             std::bind(&WebServer::webHandleCalibrate,
                       this));  // Run calibration routine (param id)
  server->on(
      "/api/factory", HTTP_GET,
      std::bind(&WebServer::webHandleFactoryReset, this));  // Reset the device
  server->on(
      "/api/status", HTTP_GET,
      std::bind(&WebServer::webHandleStatus, this));  // Get the status.json
  server->on(
      "/api/clearwifi", HTTP_GET,
      std::bind(&WebServer::webHandleClearWIFI, this));  // Clear wifi settings
  server->on("/api/upload", HTTP_GET,
             std::bind(&WebServer::webHandleUpload, this));  // Get upload.json

  server->on(
      "/api/upload", HTTP_POST, std::bind(&WebServer::webReturnOK, this),
      std::bind(&WebServer::webHandleUploadFile, this));  // File upload data
  server->on("/api/status/sleepmode", HTTP_POST,
             std::bind(&WebServer::webHandleStatusSleepmode,
                       this));  // Change sleep mode
  server->on("/api/config/device", HTTP_POST,
             std::bind(&WebServer::webHandleConfigDevice,
                       this));  // Change device settings
  server->on("/api/config/push", HTTP_POST,
             std::bind(&WebServer::webHandleConfigPush,
                       this));  // Change push settings
  server->on("/api/config/gravity", HTTP_POST,
             std::bind(&WebServer::webHandleConfigGravity,
                       this));  // Change gravity settings
  server->on("/api/config/hardware", HTTP_POST,
             std::bind(&WebServer::webHandleConfigHardware,
                       this));  // Change hardware settings

  server->onNotFound(std::bind(&WebServer::webHandlePageNotFound, this));
  server->begin();
  Log.notice(F("WEB : Web server started." CR));
  return true;
}

//
// called from main loop
//
void WebServer::loop() {
  // Dont put serial debug output in this call
  server->handleClient();
  MDNS.update();
}

// EOF
