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
#include <config.hpp>
#include <main.hpp>
#include <wifi.hpp>

Config myConfig;
AdvancedConfig myAdvancedConfig;

//
// Create the config class with default settings.
//
Config::Config() {
  // Assiging default values
  char buf[30];
#if defined(ESP8266)
  snprintf(&buf[0], sizeof(buf), "%06x", (unsigned int)ESP.getChipId());
#else  // defined (ESP32)
  uint32_t chipId = 0;
  for (int i = 0; i < 17; i = i + 8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  snprintf(&buf[0], sizeof(buf), "%06x", chipId);
#endif
  _id = String(&buf[0]);
  snprintf(&buf[0], sizeof(buf), "" WIFI_MDNS "%s", getID());
  _mDNS = String(&buf[0]);

#if LOG_LEVEL == 6 && !defined(DISABLE_LOGGING)
  Log.verbose(F("CFG : Created config for %s (%s)." CR), _id.c_str(),
              _mDNS.c_str());
#endif
}

//
// Populate the json document with all configuration parameters (used in both
// web and saving to file)
//
void Config::createJson(DynamicJsonDocument& doc) {
  doc[PARAM_MDNS] = getMDNS();
  // doc[PARAM_CONFIG_VER] = getConfigVersion();
  doc[PARAM_ID] = getID();
  doc[PARAM_OTA] = getOtaURL();
  doc[PARAM_SSID] = getWifiSSID(0);
  doc[PARAM_PASS] = getWifiPass(0);
  doc[PARAM_SSID2] = getWifiSSID(1);
  doc[PARAM_PASS2] = getWifiPass(1);
  doc[PARAM_BLE] = getColorBLE();
  doc[PARAM_TEMPFORMAT] = String(getTempFormat());
  doc[PARAM_TOKEN] = getToken();
  doc[PARAM_TOKEN2] = getToken2();
  doc[PARAM_PUSH_HTTP] = getHttpUrl();
  doc[PARAM_PUSH_HTTP_H1] = getHttpHeader(0);
  doc[PARAM_PUSH_HTTP_H2] = getHttpHeader(1);
  doc[PARAM_PUSH_HTTP2] = getHttp2Url();
  doc[PARAM_PUSH_HTTP2_H1] = getHttp2Header(0);
  doc[PARAM_PUSH_HTTP2_H2] = getHttp2Header(1);
  doc[PARAM_PUSH_HTTP3] = getHttp3Url();
  doc[PARAM_PUSH_INFLUXDB2] = getInfluxDb2PushUrl();
  doc[PARAM_PUSH_INFLUXDB2_ORG] = getInfluxDb2PushOrg();
  doc[PARAM_PUSH_INFLUXDB2_BUCKET] = getInfluxDb2PushBucket();
  doc[PARAM_PUSH_INFLUXDB2_AUTH] = getInfluxDb2PushToken();
  doc[PARAM_PUSH_MQTT] = getMqttUrl();
  doc[PARAM_PUSH_MQTT_PORT] = getMqttPort();
  doc[PARAM_PUSH_MQTT_USER] = getMqttUser();
  doc[PARAM_PUSH_MQTT_PASS] = getMqttPass();
  doc[PARAM_SLEEP_INTERVAL] = getSleepInterval();
  doc[PARAM_VOLTAGE_FACTOR] = getVoltageFactor();
  doc[PARAM_VOLTAGE_CONFIG] = getVoltageConfig();
  doc[PARAM_GRAVITY_FORMULA] = getGravityFormula();
  doc[PARAM_GRAVITY_FORMAT] = String(getGravityFormat());
  doc[PARAM_TEMP_ADJ] = getTempSensorAdjC();
  doc[PARAM_GRAVITY_TEMP_ADJ] = isGravityTempAdj();
  doc[PARAM_GYRO_TEMP] = isGyroTemp();
  doc[PARAM_STORAGE_SLEEP] = isStorageSleep();

  JsonObject cal = doc.createNestedObject(PARAM_GYRO_CALIBRATION);
  cal["ax"] = _gyroCalibration.ax;
  cal["ay"] = _gyroCalibration.ay;
  cal["az"] = _gyroCalibration.az;
  cal["gx"] = _gyroCalibration.gx;
  cal["gy"] = _gyroCalibration.gy;
  cal["gz"] = _gyroCalibration.gz;

  JsonObject cal2 = doc.createNestedObject(PARAM_FORMULA_DATA);
  cal2["a1"] = reduceFloatPrecision(_formulaData.a[0], 2);
  cal2["a2"] = reduceFloatPrecision(_formulaData.a[1], 2);
  cal2["a3"] = reduceFloatPrecision(_formulaData.a[2], 2);
  cal2["a4"] = reduceFloatPrecision(_formulaData.a[3], 2);
  cal2["a5"] = reduceFloatPrecision(_formulaData.a[4], 2);
  cal2["a6"] = reduceFloatPrecision(_formulaData.a[5], 2);
  cal2["a7"] = reduceFloatPrecision(_formulaData.a[6], 2);
  cal2["a8"] = reduceFloatPrecision(_formulaData.a[7], 2);
  cal2["a9"] = reduceFloatPrecision(_formulaData.a[8], 2);
  cal2["a10"] = reduceFloatPrecision(_formulaData.a[9], 2);

  cal2["g1"] = reduceFloatPrecision(_formulaData.g[0], 4);
  cal2["g2"] = reduceFloatPrecision(_formulaData.g[1], 4);
  cal2["g3"] = reduceFloatPrecision(_formulaData.g[2], 4);
  cal2["g4"] = reduceFloatPrecision(_formulaData.g[3], 4);
  cal2["g5"] = reduceFloatPrecision(_formulaData.g[4], 4);
  cal2["g6"] = reduceFloatPrecision(_formulaData.g[5], 4);
  cal2["g7"] = reduceFloatPrecision(_formulaData.g[6], 4);
  cal2["g8"] = reduceFloatPrecision(_formulaData.g[7], 4);
  cal2["g9"] = reduceFloatPrecision(_formulaData.g[8], 4);
  cal2["g10"] = reduceFloatPrecision(_formulaData.g[9], 4);
}

//
// Save json document to file
//
bool Config::saveFile() {
  if (!_saveNeeded) {
#if LOG_LEVEL == 6 && !defined(DISABLE_LOGGING)
    Log.verbose(F("CFG : Skipping save, not needed." CR));
#endif
    return true;
  }

#if LOG_LEVEL == 6 && !defined(DISABLE_LOGGING)
  Log.verbose(F("CFG : Saving configuration to file." CR));
#endif

  File configFile = LittleFS.open(CFG_FILENAME, "w");

  if (!configFile) {
    ErrorFileLog errLog;
    errLog.addEntry(F("CFG : Failed to save configuration."));
    return false;
  }

  DynamicJsonDocument doc(3000);
  createJson(doc);

#if LOG_LEVEL == 6 && !defined(DISABLE_LOGGING)
  serializeJson(doc, Serial);
  Serial.print(CR);
#endif

  serializeJson(doc, configFile);
  configFile.flush();
  configFile.close();

  _saveNeeded = false;
  Log.notice(F("CFG : Configuration saved to " CFG_FILENAME "." CR));
  return true;
}

//
// Load config file from disk
//
bool Config::loadFile() {
#if LOG_LEVEL == 6 && !defined(DISABLE_LOGGING)
  Log.verbose(F("CFG : Loading configuration from file." CR));
#endif

  if (!LittleFS.exists(CFG_FILENAME)) {
    ErrorFileLog errLog;
    errLog.addEntry(F("CFG : Configuration file does not exist."));
    return false;
  }

  File configFile = LittleFS.open(CFG_FILENAME, "r");

  if (!configFile) {
    ErrorFileLog errLog;
    errLog.addEntry(F("CFG : Failed to load configuration."));
    return false;
  }

  Log.notice(F("CFG : Size of configuration file=%d bytes." CR),
             configFile.size());

  DynamicJsonDocument doc(3000);
  DeserializationError err = deserializeJson(doc, configFile);
#if LOG_LEVEL == 6
  serializeJson(doc, Serial);
  Serial.print(CR);
#endif
  configFile.close();

  if (err) {
    ErrorFileLog errLog;
    errLog.addEntry(F("CFG : Failed to parse configuration (json)"));
    return false;
  }

#if LOG_LEVEL == 6
  Log.verbose(F("CFG : Parsed configuration file." CR));
#endif
  if (!doc[PARAM_OTA].isNull()) setOtaURL(doc[PARAM_OTA]);
  if (!doc[PARAM_MDNS].isNull()) setMDNS(doc[PARAM_MDNS]);
  if (!doc[PARAM_SSID].isNull()) setWifiSSID(doc[PARAM_SSID], 0);
  if (!doc[PARAM_PASS].isNull()) setWifiPass(doc[PARAM_PASS], 0);
  if (!doc[PARAM_SSID2].isNull()) setWifiSSID(doc[PARAM_SSID2], 1);
  if (!doc[PARAM_PASS2].isNull()) setWifiPass(doc[PARAM_PASS2], 1);
  if (!doc[PARAM_BLE].isNull()) setColorBLE(doc[PARAM_BLE]);

  if (!doc[PARAM_TEMPFORMAT].isNull()) {
    String s = doc[PARAM_TEMPFORMAT];
    setTempFormat(s.charAt(0));
  }

  if (!doc[PARAM_TOKEN].isNull()) setToken(doc[PARAM_TOKEN]);
  if (!doc[PARAM_TOKEN2].isNull()) setToken2(doc[PARAM_TOKEN2]);
  if (!doc[PARAM_PUSH_HTTP].isNull()) setHttpUrl(doc[PARAM_PUSH_HTTP]);
  if (!doc[PARAM_PUSH_HTTP_H1].isNull())
    setHttpHeader(doc[PARAM_PUSH_HTTP_H1], 0);
  if (!doc[PARAM_PUSH_HTTP_H2].isNull())
    setHttpHeader(doc[PARAM_PUSH_HTTP_H2], 1);
  if (!doc[PARAM_PUSH_HTTP2].isNull()) setHttp2Url(doc[PARAM_PUSH_HTTP2]);
  if (!doc[PARAM_PUSH_HTTP2_H1].isNull())
    setHttp2Header(doc[PARAM_PUSH_HTTP2_H1], 0);
  if (!doc[PARAM_PUSH_HTTP2_H2].isNull())
    setHttp2Header(doc[PARAM_PUSH_HTTP2_H2], 1);
  if (!doc[PARAM_PUSH_HTTP3].isNull()) setHttp3Url(doc[PARAM_PUSH_HTTP3]);

  if (!doc[PARAM_PUSH_INFLUXDB2].isNull())
    setInfluxDb2PushUrl(doc[PARAM_PUSH_INFLUXDB2]);
  if (!doc[PARAM_PUSH_INFLUXDB2_ORG].isNull())
    setInfluxDb2PushOrg(doc[PARAM_PUSH_INFLUXDB2_ORG]);
  if (!doc[PARAM_PUSH_INFLUXDB2_BUCKET].isNull())
    setInfluxDb2PushBucket(doc[PARAM_PUSH_INFLUXDB2_BUCKET]);
  if (!doc[PARAM_PUSH_INFLUXDB2_AUTH].isNull())
    setInfluxDb2PushToken(doc[PARAM_PUSH_INFLUXDB2_AUTH]);

  if (!doc[PARAM_PUSH_MQTT].isNull()) setMqttUrl(doc[PARAM_PUSH_MQTT]);
  if (!doc[PARAM_PUSH_MQTT_PORT].isNull())
    setMqttPort(doc[PARAM_PUSH_MQTT_PORT].as<int>());
  if (!doc[PARAM_PUSH_MQTT_USER].isNull())
    setMqttUser(doc[PARAM_PUSH_MQTT_USER]);
  if (!doc[PARAM_PUSH_MQTT_PASS].isNull())
    setMqttPass(doc[PARAM_PUSH_MQTT_PASS]);

  if (!doc[PARAM_SLEEP_INTERVAL].isNull())
    setSleepInterval(doc[PARAM_SLEEP_INTERVAL].as<int>());
  if (!doc[PARAM_VOLTAGE_FACTOR].isNull())
    setVoltageFactor(doc[PARAM_VOLTAGE_FACTOR].as<float>());
  if (!doc[PARAM_VOLTAGE_CONFIG].isNull())
    setVoltageConfig(doc[PARAM_VOLTAGE_CONFIG].as<float>());
  if (!doc[PARAM_GRAVITY_FORMULA].isNull())
    setGravityFormula(doc[PARAM_GRAVITY_FORMULA]);
  if (!doc[PARAM_GRAVITY_TEMP_ADJ].isNull())
    setGravityTempAdj(doc[PARAM_GRAVITY_TEMP_ADJ].as<bool>());
  if (!doc[PARAM_GYRO_TEMP].isNull())
    setGyroTemp(doc[PARAM_GYRO_TEMP].as<bool>());
  if (!doc[PARAM_STORAGE_SLEEP].isNull())
    setStorageSleep(doc[PARAM_STORAGE_SLEEP].as<bool>());
  if (!doc[PARAM_GRAVITY_FORMAT].isNull()) {
    String s = doc[PARAM_GRAVITY_FORMAT];
    setGravityFormat(s.charAt(0));
  }
  if (!doc[PARAM_TEMP_ADJ].isNull())
    setTempSensorAdjC(doc[PARAM_TEMP_ADJ].as<float>());

  if (!doc[PARAM_GYRO_CALIBRATION]["ax"].isNull())
    _gyroCalibration.ax = doc[PARAM_GYRO_CALIBRATION]["ax"];
  if (!doc[PARAM_GYRO_CALIBRATION]["ay"].isNull())
    _gyroCalibration.ay = doc[PARAM_GYRO_CALIBRATION]["ay"];
  if (!doc[PARAM_GYRO_CALIBRATION]["az"].isNull())
    _gyroCalibration.az = doc[PARAM_GYRO_CALIBRATION]["az"];
  if (!doc[PARAM_GYRO_CALIBRATION]["gx"].isNull())
    _gyroCalibration.gx = doc[PARAM_GYRO_CALIBRATION]["gx"];
  if (!doc[PARAM_GYRO_CALIBRATION]["gy"].isNull())
    _gyroCalibration.gy = doc[PARAM_GYRO_CALIBRATION]["gy"];
  if (!doc[PARAM_GYRO_CALIBRATION]["gz"].isNull())
    _gyroCalibration.gz = doc[PARAM_GYRO_CALIBRATION]["gz"];

  if (!doc[PARAM_FORMULA_DATA]["a1"].isNull())
    _formulaData.a[0] = doc[PARAM_FORMULA_DATA]["a1"].as<double>();
  if (!doc[PARAM_FORMULA_DATA]["a2"].isNull())
    _formulaData.a[1] = doc[PARAM_FORMULA_DATA]["a2"].as<double>();
  if (!doc[PARAM_FORMULA_DATA]["a3"].isNull())
    _formulaData.a[2] = doc[PARAM_FORMULA_DATA]["a3"].as<double>();
  if (!doc[PARAM_FORMULA_DATA]["a4"].isNull())
    _formulaData.a[3] = doc[PARAM_FORMULA_DATA]["a4"].as<double>();
  if (!doc[PARAM_FORMULA_DATA]["a5"].isNull())
    _formulaData.a[4] = doc[PARAM_FORMULA_DATA]["a5"].as<double>();
  if (!doc[PARAM_FORMULA_DATA]["a6"].isNull())
    _formulaData.a[5] = doc[PARAM_FORMULA_DATA]["a6"].as<double>();
  if (!doc[PARAM_FORMULA_DATA]["a7"].isNull())
    _formulaData.a[6] = doc[PARAM_FORMULA_DATA]["a7"].as<double>();
  if (!doc[PARAM_FORMULA_DATA]["a8"].isNull())
    _formulaData.a[7] = doc[PARAM_FORMULA_DATA]["a8"].as<double>();
  if (!doc[PARAM_FORMULA_DATA]["a9"].isNull())
    _formulaData.a[8] = doc[PARAM_FORMULA_DATA]["a9"].as<double>();
  if (!doc[PARAM_FORMULA_DATA]["a10"].isNull())
    _formulaData.a[9] = doc[PARAM_FORMULA_DATA]["a10"].as<double>();

  if (!doc[PARAM_FORMULA_DATA]["g1"].isNull())
    _formulaData.g[0] = doc[PARAM_FORMULA_DATA]["g1"].as<double>();
  if (!doc[PARAM_FORMULA_DATA]["g2"].isNull())
    _formulaData.g[1] = doc[PARAM_FORMULA_DATA]["g2"].as<double>();
  if (!doc[PARAM_FORMULA_DATA]["g3"].isNull())
    _formulaData.g[2] = doc[PARAM_FORMULA_DATA]["g3"].as<double>();
  if (!doc[PARAM_FORMULA_DATA]["g4"].isNull())
    _formulaData.g[3] = doc[PARAM_FORMULA_DATA]["g4"].as<double>();
  if (!doc[PARAM_FORMULA_DATA]["g5"].isNull())
    _formulaData.g[4] = doc[PARAM_FORMULA_DATA]["g5"].as<double>();
  if (!doc[PARAM_FORMULA_DATA]["g6"].isNull())
    _formulaData.g[5] = doc[PARAM_FORMULA_DATA]["g6"].as<double>();
  if (!doc[PARAM_FORMULA_DATA]["g7"].isNull())
    _formulaData.g[6] = doc[PARAM_FORMULA_DATA]["g7"].as<double>();
  if (!doc[PARAM_FORMULA_DATA]["g8"].isNull())
    _formulaData.g[7] = doc[PARAM_FORMULA_DATA]["g8"].as<double>();
  if (!doc[PARAM_FORMULA_DATA]["g9"].isNull())
    _formulaData.g[8] = doc[PARAM_FORMULA_DATA]["g9"].as<double>();
  if (!doc[PARAM_FORMULA_DATA]["g10"].isNull())
    _formulaData.g[9] = doc[PARAM_FORMULA_DATA]["g10"].as<double>();

  /*if( doc[PARAM_CONFIG_VER].isNull() ) {
    // If this parameter is missing we need to reset the gyrocalibaration due to
  bug #29 _gyroCalibration.ax = _gyroCalibration.ay = _gyroCalibration.az = 0;
    _gyroCalibration.gx = _gyroCalibration.gy = _gyroCalibration.gz = 0;
    Log.warning(F("CFG : Old configuration format, clearing gyro calibration."
  CR));
  }*/

  _saveNeeded = false;  // Reset save flag
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
#if LOG_LEVEL == 6 && !defined(DISABLE_LOGGING)
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
// Save json document to file
//
bool AdvancedConfig::saveFile() {
#if LOG_LEVEL == 6 && !defined(DISABLE_LOGGING)
  Log.verbose(F("CFG : Saving hardware configuration to file." CR));
#endif

  File configFile = LittleFS.open(CFG_HW_FILENAME, "w");

  if (!configFile) {
    ErrorFileLog errLog;
    errLog.addEntry(F("CFG : Failed to write hardware configuration "));
    return false;
  }

  DynamicJsonDocument doc(512);

  doc[PARAM_HW_GYRO_READ_COUNT] = this->getGyroReadCount();
  // doc[PARAM_HW_GYRO_READ_DELAY] = this->getGyroReadDelay();
  doc[PARAM_HW_GYRO_MOVING_THREASHOLD] = this->getGyroSensorMovingThreashold();
  doc[PARAM_HW_FORMULA_DEVIATION] = this->getMaxFormulaCreationDeviation();
  doc[PARAM_HW_WIFI_PORTAL_TIMEOUT] = this->getWifiPortalTimeout();
  doc[PARAM_HW_WIFI_CONNECT_TIMEOUT] = this->getWifiConnectTimeout();
  doc[PARAM_HW_FORMULA_CALIBRATION_TEMP] = this->getDefaultCalibrationTemp();
  doc[PARAM_HW_PUSH_INTERVAL_HTTP1] = this->getPushIntervalHttp1();
  doc[PARAM_HW_PUSH_INTERVAL_HTTP2] = this->getPushIntervalHttp2();
  doc[PARAM_HW_PUSH_INTERVAL_HTTP3] = this->getPushIntervalHttp3();
  doc[PARAM_HW_PUSH_INTERVAL_INFLUX] = this->getPushIntervalInflux();
  doc[PARAM_HW_PUSH_INTERVAL_MQTT] = this->getPushIntervalMqtt();
  doc[PARAM_HW_TEMPSENSOR_RESOLUTION] = this->getTempSensorResolution();
  doc[PARAM_HW_IGNORE_LOW_ANGLES] = this->isIgnoreLowAnges();

#if LOG_LEVEL == 6 && !defined(DISABLE_LOGGING)
  serializeJson(doc, Serial);
  Serial.print(CR);
#endif

  serializeJson(doc, configFile);
  configFile.flush();
  configFile.close();

  Log.notice(F("CFG : Configuration saved to " CFG_HW_FILENAME "." CR));
  return true;
}

//
// Load config file from disk
//
bool AdvancedConfig::loadFile() {
#if LOG_LEVEL == 6 && !defined(DISABLE_LOGGING)
  Log.verbose(F("CFG : Loading hardware configuration from file." CR));
#endif

  if (!LittleFS.exists(CFG_HW_FILENAME)) {
    Log.warning(
        F("CFG : Configuration file does not exist " CFG_HW_FILENAME "." CR));
    return false;
  }

  File configFile = LittleFS.open(CFG_HW_FILENAME, "r");

  if (!configFile) {
    ErrorFileLog errLog;
    errLog.addEntry(F("CFG : Failed to read hardware configuration "));
    return false;
  }

  Log.notice(F("CFG : Size of configuration file=%d bytes." CR),
             configFile.size());

  DynamicJsonDocument doc(512);
  DeserializationError err = deserializeJson(doc, configFile);
#if LOG_LEVEL == 6
  serializeJson(doc, Serial);
  Serial.print(CR);
#endif
  configFile.close();

  if (err) {
    ErrorFileLog errLog;
    errLog.addEntry(F("CFG : Failed to parse hardware configuration (json)"));
    return false;
  }

#if LOG_LEVEL == 6
  Log.verbose(F("CFG : Parsed hardware configuration file." CR));
#endif

  if (!doc[PARAM_HW_GYRO_READ_COUNT].isNull())
    this->setGyroReadCount(doc[PARAM_HW_GYRO_READ_COUNT].as<int>());
  // if (!doc[PARAM_HW_GYRO_READ_DELAY].isNull())
  //  this->setGyroReadDelay(doc[PARAM_HW_GYRO_READ_DELAY].as<int>());
  if (!doc[PARAM_HW_GYRO_MOVING_THREASHOLD].isNull())
    this->setGyroSensorMovingThreashold(
        doc[PARAM_HW_GYRO_MOVING_THREASHOLD].as<int>());
  if (!doc[PARAM_HW_FORMULA_DEVIATION].isNull())
    this->setMaxFormulaCreationDeviation(
        doc[PARAM_HW_FORMULA_DEVIATION].as<float>());
  if (!doc[PARAM_HW_FORMULA_CALIBRATION_TEMP].isNull())
    this->SetDefaultCalibrationTemp(
        doc[PARAM_HW_FORMULA_CALIBRATION_TEMP].as<float>());
  if (!doc[PARAM_HW_WIFI_PORTAL_TIMEOUT].isNull())
    this->setWifiPortalTimeout(doc[PARAM_HW_WIFI_PORTAL_TIMEOUT].as<int>());
  if (!doc[PARAM_HW_WIFI_CONNECT_TIMEOUT].isNull())
    this->setWifiConnectTimeout(doc[PARAM_HW_WIFI_CONNECT_TIMEOUT].as<int>());
  if (!doc[PARAM_HW_PUSH_TIMEOUT].isNull())
    this->setPushTimeout(doc[PARAM_HW_PUSH_TIMEOUT].as<int>());
  if (!doc[PARAM_HW_PUSH_INTERVAL_HTTP1].isNull())
    this->setPushIntervalHttp1(doc[PARAM_HW_PUSH_INTERVAL_HTTP1].as<int>());
  if (!doc[PARAM_HW_PUSH_INTERVAL_HTTP2].isNull())
    this->setPushIntervalHttp2(doc[PARAM_HW_PUSH_INTERVAL_HTTP2].as<int>());
  if (!doc[PARAM_HW_PUSH_INTERVAL_HTTP3].isNull())
    this->setPushIntervalHttp3(doc[PARAM_HW_PUSH_INTERVAL_HTTP3].as<int>());
  if (!doc[PARAM_HW_PUSH_INTERVAL_INFLUX].isNull())
    this->setPushIntervalInflux(doc[PARAM_HW_PUSH_INTERVAL_INFLUX].as<int>());
  if (!doc[PARAM_HW_PUSH_INTERVAL_MQTT].isNull())
    this->setPushIntervalMqtt(doc[PARAM_HW_PUSH_INTERVAL_MQTT].as<int>());
  if (!doc[PARAM_HW_TEMPSENSOR_RESOLUTION].isNull())
    this->setTempSensorResolution(
        doc[PARAM_HW_TEMPSENSOR_RESOLUTION].as<int>());
  if (!doc[PARAM_HW_IGNORE_LOW_ANGLES].isNull())
    setIgnoreLowAnges(doc[PARAM_HW_IGNORE_LOW_ANGLES].as<bool>());

  Log.notice(F("CFG : Configuration file " CFG_HW_FILENAME " loaded." CR));
  return true;
}

// EOF
