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
#include <LittleFS.h>

#include <config.hpp>
#include <helper.hpp>

Config myConfig;

//
// Create the config class with default settings.
//
Config::Config() {
  // Assiging default values
  char buf[30];
  snprintf(&buf[0], sizeof(buf), "%6x", (unsigned int)ESP.getChipId());
  id = String(&buf[0]);
  snprintf(&buf[0], sizeof(buf), "" WIFI_MDNS "%s", getID());
  mDNS = String(&buf[0]);

#if LOG_LEVEL == 6 && !defined(CFG_DISABLE_LOGGING)
  Log.verbose(F("CFG : Created config for %s (%s)." CR), id.c_str(),
              mDNS.c_str());
#endif

  setTempFormat('C');
  setGravityFormat('G');
  setSleepInterval(900);   // 15 minutes
  setVoltageFactor(1.59);  // Conversion factor for battery
  setTempSensorAdj(0.0);
  setGravityTempAdj(false);
  gyroCalibration = {0, 0, 0, 0, 0, 0};
  formulaData = {{0, 0, 0, 0, 0}, {1, 1, 1, 1, 1}};
  gyroTemp = false;
  saveNeeded = false;
}

//
// Populate the json document with all configuration parameters (used in both
// web and saving to file)
//
void Config::createJson(DynamicJsonDocument& doc) {
  doc[CFG_PARAM_MDNS] = getMDNS();
  doc[CFG_PARAM_ID] = getID();
  doc[CFG_PARAM_OTA] = getOtaURL();
  doc[CFG_PARAM_SSID] = getWifiSSID();
  doc[CFG_PARAM_PASS] = getWifiPass();
  doc[CFG_PARAM_TEMPFORMAT] = String(getTempFormat());
  doc[CFG_PARAM_PUSH_BREWFATHER] = getBrewfatherPushUrl();
  doc[CFG_PARAM_PUSH_HTTP] = getHttpPushUrl();
  doc[CFG_PARAM_PUSH_HTTP2] = getHttpPushUrl2();
  doc[CFG_PARAM_PUSH_INFLUXDB2] = getInfluxDb2PushUrl();
  doc[CFG_PARAM_PUSH_INFLUXDB2_ORG] = getInfluxDb2PushOrg();
  doc[CFG_PARAM_PUSH_INFLUXDB2_BUCKET] = getInfluxDb2PushBucket();
  doc[CFG_PARAM_PUSH_INFLUXDB2_AUTH] = getInfluxDb2PushToken();
  doc[CFG_PARAM_SLEEP_INTERVAL] = getSleepInterval();
  doc[CFG_PARAM_VOLTAGEFACTOR] = getVoltageFactor();
  doc[CFG_PARAM_GRAVITY_FORMULA] = getGravityFormula();
  doc[CFG_PARAM_GRAVITY_FORMAT] = String(getGravityFormat());
  doc[CFG_PARAM_TEMP_ADJ] = getTempSensorAdj();
  doc[CFG_PARAM_GRAVITY_TEMP_ADJ] = isGravityTempAdj();
  doc[CFG_PARAM_GYRO_TEMP] = isGyroTemp();

  JsonObject cal = doc.createNestedObject(CFG_PARAM_GYRO_CALIBRATION);
  cal["ax"] = gyroCalibration.ax;
  cal["ay"] = gyroCalibration.ay;
  cal["az"] = gyroCalibration.az;
  cal["gx"] = gyroCalibration.gx;
  cal["gy"] = gyroCalibration.gy;
  cal["gz"] = gyroCalibration.gz;

  JsonObject cal2 = doc.createNestedObject(CFG_PARAM_FORMULA_DATA);
  cal2["a1"] = reduceFloatPrecision(formulaData.a[0], 2);
  cal2["a2"] = reduceFloatPrecision(formulaData.a[1], 2);
  cal2["a3"] = reduceFloatPrecision(formulaData.a[2], 2);
  cal2["a4"] = reduceFloatPrecision(formulaData.a[3], 2);
  cal2["a5"] = reduceFloatPrecision(formulaData.a[4], 2);

  cal2["g1"] = reduceFloatPrecision(formulaData.g[0], 4);
  cal2["g2"] = reduceFloatPrecision(formulaData.g[1], 4);
  cal2["g3"] = reduceFloatPrecision(formulaData.g[2], 4);
  cal2["g4"] = reduceFloatPrecision(formulaData.g[3], 4);
  cal2["g5"] = reduceFloatPrecision(formulaData.g[4], 4);
}

//
// Save json document to file
//
bool Config::saveFile() {
  if (!saveNeeded) {
#if LOG_LEVEL == 6 && !defined(CFG_DISABLE_LOGGING)
    Log.verbose(F("CFG : Skipping save, not needed." CR));
#endif
    return true;
  }

#if LOG_LEVEL == 6 && !defined(CFG_DISABLE_LOGGING)
  Log.verbose(F("CFG : Saving configuration to file." CR));
#endif

  File configFile = LittleFS.open(CFG_FILENAME, "w");

  if (!configFile) {
    Log.error(F("CFG : Failed to open file " CFG_FILENAME " for save." CR));
    return false;
  }

  DynamicJsonDocument doc(CFG_JSON_BUFSIZE);
  createJson(doc);

#if LOG_LEVEL == 6 && !defined(CFG_DISABLE_LOGGING)
  serializeJson(doc, Serial);
  Serial.print(CR);
#endif

  serializeJson(doc, configFile);
  configFile.flush();
  configFile.close();

  saveNeeded = false;
  myConfig.debug();
  Log.notice(F("CFG : Configuration saved to " CFG_FILENAME "." CR));
  return true;
}

//
// Load config file from disk
//
bool Config::loadFile() {
#if LOG_LEVEL == 6 && !defined(CFG_DISABLE_LOGGING)
  Log.verbose(F("CFG : Loading configuration from file." CR));
#endif

  if (!LittleFS.exists(CFG_FILENAME)) {
    Log.error(
        F("CFG : Configuration file does not exist " CFG_FILENAME "." CR));
    return false;
  }

  File configFile = LittleFS.open(CFG_FILENAME, "r");

  if (!configFile) {
    Log.error(F("CFG : Failed to open " CFG_FILENAME "." CR));
    return false;
  }

  Log.notice(F("CFG : Size of configuration file=%d bytes." CR),
             configFile.size());

  DynamicJsonDocument doc(CFG_JSON_BUFSIZE);
  DeserializationError err = deserializeJson(doc, configFile);
#if LOG_LEVEL == 6
  serializeJson(doc, Serial);
  Serial.print(CR);
#endif
  configFile.close();

  if (err) {
    Log.error(F("CFG : Failed to parse " CFG_FILENAME " file, Err: %s, %d." CR),
              err.c_str(), doc.capacity());
    return false;
  }

#if LOG_LEVEL == 6
  Log.verbose(F("CFG : Parsed configuration file." CR));
#endif
  if (!doc[CFG_PARAM_OTA].isNull()) setOtaURL(doc[CFG_PARAM_OTA]);
  if (!doc[CFG_PARAM_MDNS].isNull()) setMDNS(doc[CFG_PARAM_MDNS]);
  if (!doc[CFG_PARAM_SSID].isNull()) setWifiSSID(doc[CFG_PARAM_SSID]);
  if (!doc[CFG_PARAM_PASS].isNull()) setWifiPass(doc[CFG_PARAM_PASS]);
  if (!doc[CFG_PARAM_TEMPFORMAT].isNull()) {
    String s = doc[CFG_PARAM_TEMPFORMAT];
    setTempFormat(s.charAt(0));
  }
  if (!doc[CFG_PARAM_PUSH_BREWFATHER].isNull())
    setBrewfatherPushUrl(doc[CFG_PARAM_PUSH_BREWFATHER]);
  if (!doc[CFG_PARAM_PUSH_HTTP].isNull())
    setHttpPushUrl(doc[CFG_PARAM_PUSH_HTTP]);
  if (!doc[CFG_PARAM_PUSH_HTTP2].isNull())
    setHttpPushUrl2(doc[CFG_PARAM_PUSH_HTTP2]);
  if (!doc[CFG_PARAM_PUSH_INFLUXDB2].isNull())
    setInfluxDb2PushUrl(doc[CFG_PARAM_PUSH_INFLUXDB2]);
  if (!doc[CFG_PARAM_PUSH_INFLUXDB2_ORG].isNull())
    setInfluxDb2PushOrg(doc[CFG_PARAM_PUSH_INFLUXDB2_ORG]);
  if (!doc[CFG_PARAM_PUSH_INFLUXDB2_BUCKET].isNull())
    setInfluxDb2PushBucket(doc[CFG_PARAM_PUSH_INFLUXDB2_BUCKET]);
  if (!doc[CFG_PARAM_PUSH_INFLUXDB2_AUTH].isNull())
    setInfluxDb2PushToken(doc[CFG_PARAM_PUSH_INFLUXDB2_AUTH]);
  if (!doc[CFG_PARAM_SLEEP_INTERVAL].isNull())
    setSleepInterval(doc[CFG_PARAM_SLEEP_INTERVAL].as<int>());
  if (!doc[CFG_PARAM_VOLTAGEFACTOR].isNull())
    setVoltageFactor(doc[CFG_PARAM_VOLTAGEFACTOR].as<float>());
  if (!doc[CFG_PARAM_GRAVITY_FORMULA].isNull())
    setGravityFormula(doc[CFG_PARAM_GRAVITY_FORMULA]);
  if (!doc[CFG_PARAM_GRAVITY_TEMP_ADJ].isNull())
    setGravityTempAdj(doc[CFG_PARAM_GRAVITY_TEMP_ADJ].as<bool>());
  if (!doc[CFG_PARAM_GYRO_TEMP].isNull())
    setGyroTemp(doc[CFG_PARAM_GYRO_TEMP].as<bool>());
  if (!doc[CFG_PARAM_GRAVITY_FORMAT].isNull()) {
    String s = doc[CFG_PARAM_GRAVITY_FORMAT];
    setGravityFormat(s.charAt(0));
  }
  if (!doc[CFG_PARAM_TEMP_ADJ].isNull())
    setTempSensorAdj(doc[CFG_PARAM_TEMP_ADJ].as<float>());

  if (!doc[CFG_PARAM_GYRO_CALIBRATION]["ax"].isNull())
    gyroCalibration.ax = doc[CFG_PARAM_GYRO_CALIBRATION]["ax"];
  if (!doc[CFG_PARAM_GYRO_CALIBRATION]["ay"].isNull())
    gyroCalibration.ay = doc[CFG_PARAM_GYRO_CALIBRATION]["ay"];
  if (!doc[CFG_PARAM_GYRO_CALIBRATION]["az"].isNull())
    gyroCalibration.az = doc[CFG_PARAM_GYRO_CALIBRATION]["az"];
  if (!doc[CFG_PARAM_GYRO_CALIBRATION]["gx"].isNull())
    gyroCalibration.gx = doc[CFG_PARAM_GYRO_CALIBRATION]["gx"];
  if (!doc[CFG_PARAM_GYRO_CALIBRATION]["gy"].isNull())
    gyroCalibration.gy = doc[CFG_PARAM_GYRO_CALIBRATION]["gy"];
  if (!doc[CFG_PARAM_GYRO_CALIBRATION]["gz"].isNull())
    gyroCalibration.gz = doc[CFG_PARAM_GYRO_CALIBRATION]["gz"];

  if (!doc[CFG_PARAM_FORMULA_DATA]["a1"].isNull())
    formulaData.a[0] = doc[CFG_PARAM_FORMULA_DATA]["a1"].as<double>();
  if (!doc[CFG_PARAM_FORMULA_DATA]["a2"].isNull())
    formulaData.a[1] = doc[CFG_PARAM_FORMULA_DATA]["a2"].as<double>();
  if (!doc[CFG_PARAM_FORMULA_DATA]["a3"].isNull())
    formulaData.a[2] = doc[CFG_PARAM_FORMULA_DATA]["a3"].as<double>();
  if (!doc[CFG_PARAM_FORMULA_DATA]["a4"].isNull())
    formulaData.a[3] = doc[CFG_PARAM_FORMULA_DATA]["a4"].as<double>();
  if (!doc[CFG_PARAM_FORMULA_DATA]["a5"].isNull())
    formulaData.a[4] = doc[CFG_PARAM_FORMULA_DATA]["a5"].as<double>();

  if (!doc[CFG_PARAM_FORMULA_DATA]["g1"].isNull())
    formulaData.g[0] = doc[CFG_PARAM_FORMULA_DATA]["g1"].as<double>();
  if (!doc[CFG_PARAM_FORMULA_DATA]["g2"].isNull())
    formulaData.g[1] = doc[CFG_PARAM_FORMULA_DATA]["g2"].as<double>();
  if (!doc[CFG_PARAM_FORMULA_DATA]["g3"].isNull())
    formulaData.g[2] = doc[CFG_PARAM_FORMULA_DATA]["g3"].as<double>();
  if (!doc[CFG_PARAM_FORMULA_DATA]["g4"].isNull())
    formulaData.g[3] = doc[CFG_PARAM_FORMULA_DATA]["g4"].as<double>();
  if (!doc[CFG_PARAM_FORMULA_DATA]["g5"].isNull())
    formulaData.g[4] = doc[CFG_PARAM_FORMULA_DATA]["g5"].as<double>();

  myConfig.debug();
  saveNeeded = false;  // Reset save flag
  Log.notice(F("CFG : Configuration file " CFG_FILENAME " loaded." CR));
  return true;
}

//
// Check if file system can be mounted, if not we format it.
//
void Config::formatFileSystem() {
  Log.notice(F("CFG : Formating filesystem." CR));
  LittleFS.format();
}

//
// Check if file system can be mounted, if not we format it.
//
void Config::checkFileSystem() {
#if LOG_LEVEL == 6 && !defined(CFG_DISABLE_LOGGING)
  Log.verbose(F("CFG : Checking if filesystem is valid." CR));
#endif

  if (LittleFS.begin()) {
    Log.notice(F("CFG : Filesystem mounted." CR));
  } else {
    Log.error(F("CFG : Unable to mount file system, formatting..." CR));
    LittleFS.format();
  }
}

//
// Dump the configuration to the serial port
//
void Config::debug() {
#if LOG_LEVEL == 6 && !defined(CFG_DISABLE_LOGGING)
  Log.verbose(F("CFG : Dumping configration " CFG_FILENAME "." CR));
  Log.verbose(F("CFG : ID; '%s'." CR), getID());
  Log.verbose(F("CFG : WIFI; '%s', '%s'." CR), getWifiSSID(), getWifiPass());
  Log.verbose(F("CFG : mDNS; '%s'." CR), getMDNS());
  Log.verbose(F("CFG : Sleep interval; %d." CR), getSleepInterval());
  Log.verbose(F("CFG : OTA; '%s'." CR), getOtaURL());
  Log.verbose(F("CFG : Temp Format; %c." CR), getTempFormat());
  Log.verbose(F("CFG : Temp Adj; %F." CR), getTempSensorAdj());
  Log.verbose(F("CFG : VoltageFactor; %F." CR), getVoltageFactor());
  Log.verbose(F("CFG : Gravity formula; '%s'." CR), getGravityFormula());
  Log.verbose(F("CFG : Gravity format; '%c'." CR), getGravityFormat());
  Log.verbose(F("CFG : Gravity temp adj; %s." CR),
              isGravityTempAdj() ? "true" : "false");
  Log.verbose(F("CFG : Gyro temp; %s." CR), isGyroTemp() ? "true" : "false");
  Log.verbose(F("CFG : Push brewfather; '%s'." CR), getBrewfatherPushUrl());
  Log.verbose(F("CFG : Push http; '%s'." CR), getHttpPushUrl());
  Log.verbose(F("CFG : Push http2; '%s'." CR), getHttpPushUrl2());
  Log.verbose(F("CFG : InfluxDb2; '%s', '%s', '%s', '%s'." CR),
              getInfluxDb2PushUrl(), getInfluxDb2PushOrg(),
              getInfluxDb2PushBucket(), getInfluxDb2PushToken());
#endif
}

// EOF
