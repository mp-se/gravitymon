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
#include <config.hpp>
#include <main.hpp>
#include <wifi.hpp>

Config myConfig;

Config::Config() {
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

void Config::createJson(DynamicJsonDocument& doc) {
  doc[PARAM_MDNS] = getMDNS();
  doc[PARAM_ID] = getID();
  doc[PARAM_OTA] = getOtaURL();
  doc[PARAM_SSID] = getWifiSSID(0);
  doc[PARAM_PASS] = getWifiPass(0);
  doc[PARAM_SSID2] = getWifiSSID(1);
  doc[PARAM_PASS2] = getWifiPass(1);
  doc[PARAM_BLE] = getBleColor();
  doc[PARAM_BLE_FORMAT] = getBleFormat();
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
  doc[PARAM_VOLTAGE_FACTOR] =
      serialized(String(getVoltageFactor(), DECIMALS_BATTERY));
  doc[PARAM_VOLTAGE_CONFIG] =
      serialized(String(getVoltageConfig(), DECIMALS_BATTERY));
  doc[PARAM_GRAVITY_FORMULA] = getGravityFormula();
  doc[PARAM_GRAVITY_FORMAT] = String(getGravityFormat());
  doc[PARAM_TEMP_ADJ] = serialized(String(getTempSensorAdjC(), DECIMALS_TEMP));
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

  JsonArray fdArray = doc.createNestedArray(PARAM_FORMULA_DATA);
  for (int i = 0; i < FORMULA_DATA_SIZE; i++) {
    JsonObject fd = fdArray.createNestedObject();
    fd["a"] = serialized(String(_formulaData.a[i], DECIMALS_TILT));
    fd["g"] = serialized(String(_formulaData.g[i], DECIMALS_SG));
  }

  doc[PARAM_GYRO_READ_COUNT] = this->getGyroReadCount();
  // doc[PARAM_GYRO_READ_DELAY] = this->getGyroReadDelay();
  doc[PARAM_GYRO_MOVING_THREASHOLD] = this->getGyroSensorMovingThreashold();
  doc[PARAM_FORMULA_DEVIATION] = this->getMaxFormulaCreationDeviation();
  doc[PARAM_WIFI_PORTAL_TIMEOUT] = this->getWifiPortalTimeout();
  doc[PARAM_WIFI_CONNECT_TIMEOUT] = this->getWifiConnectTimeout();
  doc[PARAM_FORMULA_CALIBRATION_TEMP] = this->getDefaultCalibrationTemp();
  doc[PARAM_PUSH_INTERVAL_HTTP1] = this->getPushIntervalHttp1();
  doc[PARAM_PUSH_INTERVAL_HTTP2] = this->getPushIntervalHttp2();
  doc[PARAM_PUSH_INTERVAL_HTTP3] = this->getPushIntervalHttp3();
  doc[PARAM_PUSH_INTERVAL_INFLUX] = this->getPushIntervalInflux();
  doc[PARAM_PUSH_INTERVAL_MQTT] = this->getPushIntervalMqtt();
  doc[PARAM_TEMPSENSOR_RESOLUTION] = this->getTempSensorResolution();
  doc[PARAM_IGNORE_LOW_ANGLES] = this->isIgnoreLowAnges();
  doc[PARAM_BATTERY_SAVING] = this->isBatterySaving();
}

void Config::parseJson(DynamicJsonDocument& doc) {
  /* for iterating over the array, needed when we need to migrate from the old
  format. for (JsonPair kv : doc.as<JsonObject>()) {
    Serial.println(kv.key().c_str());
  }*/

  if (!doc[PARAM_OTA].isNull()) setOtaURL(doc[PARAM_OTA]);
  if (!doc[PARAM_MDNS].isNull()) setMDNS(doc[PARAM_MDNS]);
  if (!doc[PARAM_SSID].isNull()) setWifiSSID(doc[PARAM_SSID], 0);
  if (!doc[PARAM_PASS].isNull()) setWifiPass(doc[PARAM_PASS], 0);
  if (!doc[PARAM_SSID2].isNull()) setWifiSSID(doc[PARAM_SSID2], 1);
  if (!doc[PARAM_PASS2].isNull()) setWifiPass(doc[PARAM_PASS2], 1);
  if (!doc[PARAM_BLE].isNull()) setBleColor(doc[PARAM_BLE]);
  if (!doc[PARAM_BLE_FORMAT].isNull())
    setBleFormat(doc[PARAM_BLE_FORMAT].as<int>());

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

  if (!doc[PARAM_FORMULA_DATA].isNull()) {
    JsonArray array = doc[PARAM_FORMULA_DATA].as<JsonArray>();
    int i = 0;

    for (JsonVariant v : array) {
      _formulaData.a[i] = v["a"].as<double>();
      _formulaData.g[i] = v["g"].as<double>();
      i++;
    }

    if (i != FORMULA_DATA_SIZE) {
      Log.warning(F("Size of formula array is not as expected (%d)" CR), i);
    }
  }

  if (!doc[PARAM_GYRO_READ_COUNT].isNull())
    this->setGyroReadCount(doc[PARAM_GYRO_READ_COUNT].as<int>());
  // if (!doc[PARAM_GYRO_READ_DELAY].isNull())
  //  this->setGyroReadDelay(doc[PARAM_GYRO_READ_DELAY].as<int>());
  if (!doc[PARAM_GYRO_MOVING_THREASHOLD].isNull())
    this->setGyroSensorMovingThreashold(
        doc[PARAM_GYRO_MOVING_THREASHOLD].as<int>());
  if (!doc[PARAM_FORMULA_DEVIATION].isNull())
    this->setMaxFormulaCreationDeviation(
        doc[PARAM_FORMULA_DEVIATION].as<float>());
  if (!doc[PARAM_FORMULA_CALIBRATION_TEMP].isNull())
    this->SetDefaultCalibrationTemp(
        doc[PARAM_FORMULA_CALIBRATION_TEMP].as<float>());
  if (!doc[PARAM_WIFI_PORTAL_TIMEOUT].isNull())
    this->setWifiPortalTimeout(doc[PARAM_WIFI_PORTAL_TIMEOUT].as<int>());
  if (!doc[PARAM_WIFI_CONNECT_TIMEOUT].isNull())
    this->setWifiConnectTimeout(doc[PARAM_WIFI_CONNECT_TIMEOUT].as<int>());
  if (!doc[PARAM_PUSH_TIMEOUT].isNull())
    this->setPushTimeout(doc[PARAM_PUSH_TIMEOUT].as<int>());
  if (!doc[PARAM_PUSH_INTERVAL_HTTP1].isNull())
    this->setPushIntervalHttp1(doc[PARAM_PUSH_INTERVAL_HTTP1].as<int>());
  if (!doc[PARAM_PUSH_INTERVAL_HTTP2].isNull())
    this->setPushIntervalHttp2(doc[PARAM_PUSH_INTERVAL_HTTP2].as<int>());
  if (!doc[PARAM_PUSH_INTERVAL_HTTP3].isNull())
    this->setPushIntervalHttp3(doc[PARAM_PUSH_INTERVAL_HTTP3].as<int>());
  if (!doc[PARAM_PUSH_INTERVAL_INFLUX].isNull())
    this->setPushIntervalInflux(doc[PARAM_PUSH_INTERVAL_INFLUX].as<int>());
  if (!doc[PARAM_PUSH_INTERVAL_MQTT].isNull())
    this->setPushIntervalMqtt(doc[PARAM_PUSH_INTERVAL_MQTT].as<int>());
  if (!doc[PARAM_TEMPSENSOR_RESOLUTION].isNull())
    this->setTempSensorResolution(doc[PARAM_TEMPSENSOR_RESOLUTION].as<int>());
  if (!doc[PARAM_IGNORE_LOW_ANGLES].isNull())
    setIgnoreLowAnges(doc[PARAM_IGNORE_LOW_ANGLES].as<bool>());
  if (!doc[PARAM_BATTERY_SAVING].isNull())
    setBatterySaving(doc[PARAM_BATTERY_SAVING].as<bool>());
}

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
    writeErrorLog("CFG : Failed to save configuration.");
    return false;
  }

  DynamicJsonDocument doc(JSON_BUFFER_SIZE_LARGE);
  createJson(doc);

#if LOG_LEVEL == 6 && !defined(DISABLE_LOGGING)
  serializeJson(doc, EspSerial);
  EspSerial.print(CR);
#endif

  serializeJson(doc, configFile);
  configFile.flush();
  configFile.close();

  _saveNeeded = false;
  Log.notice(F("CFG : Configuration saved to " CFG_FILENAME "." CR));
  return true;
}

bool Config::loadFile() {
#if LOG_LEVEL == 6 && !defined(DISABLE_LOGGING)
  Log.verbose(F("CFG : Loading configuration from file." CR));
#endif

  if (!LittleFS.exists(CFG_FILENAME)) {
    writeErrorLog("CFG : Configuration file does not exist.");
    return false;
  }

  File configFile = LittleFS.open(CFG_FILENAME, "r");

  if (!configFile) {
    writeErrorLog("CFG : Failed to load configuration.");
    return false;
  }

  Log.notice(F("CFG : Size of configuration file=%d bytes." CR),
             configFile.size());

  DynamicJsonDocument doc(JSON_BUFFER_SIZE_LARGE);
  DeserializationError err = deserializeJson(doc, configFile);
#if LOG_LEVEL == 6
  serializeJson(doc, EspSerial);
  EspSerial.print(CR);
#endif
  configFile.close();

  if (err) {
    writeErrorLog("CFG : Failed to parse configuration (json)");
    return false;
  }

#if LOG_LEVEL == 6
  Log.verbose(F("CFG : Parsed configuration file." CR));
#endif

  parseJson(doc);

  _saveNeeded = false;  // Reset save flag
  Log.notice(F("CFG : Configuration file " CFG_FILENAME " loaded." CR));
  return true;
}

void Config::formatFileSystem() {
  Log.notice(F("CFG : Formating filesystem." CR));
  LittleFS.format();
}

void Config::checkFileSystem() {
#if LOG_LEVEL == 6 && !defined(DISABLE_LOGGING)
  Log.verbose(F("CFG : Checking if filesystem is valid." CR));
#endif

#if defined(ESP8266)
  if (LittleFS.begin()) {
    Log.notice(F("CFG : Filesystem mounted." CR));
  } else {
    Log.error(F("CFG : Unable to mount file system, formatting..." CR));
    LittleFS.format();
  }
#else
  if (LittleFS.begin(true)) {
    Log.notice(F("CFG : Filesystem mounted." CR));
  } else {
    Log.error(F("CFG : Unable to mount file system..." CR));
  }
#endif
}

// EOF
