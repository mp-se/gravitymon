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
#include <baseconfig.hpp>
#include <espframework.hpp>
#include <log.hpp>

BaseConfig::BaseConfig(String baseMDNS, String fileName, int dynamicJsonSize) {
  _dynamicJsonSize = dynamicJsonSize;

  char buf[30];
#if defined(ESP8266)
  snprintf(buf, sizeof(buf), "%06x", (unsigned int)ESP.getChipId());
#else  // defined (ESP32)
  uint32_t chipId = 0;
  for (int i = 0; i < 17; i = i + 8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  snprintf(&buf[0], sizeof(buf), "%06x", chipId);
#endif
  _id = String(buf);
  snprintf(buf, sizeof(buf), "%s%s", baseMDNS.c_str(), _id.c_str());
  _mDNS = String(buf);
  _fileName = fileName;
  _saveNeeded = false;
#if LOG_LEVEL == 6
  Log.verbose(F("CFG : Creating hostname=%s, id=%s." CR), _mDNS.c_str(),
              _id.c_str());
#endif
}

void BaseConfig::createJsonWifi(JsonObject& doc) {
#if LOG_LEVEL == 6
  Log.verbose(F("CFG : Create json (wifi)." CR));
#endif
  doc[PARAM_MDNS] = getMDNS();
  doc[PARAM_SSID] = getWifiSSID(0);
  doc[PARAM_SSID2] = getWifiSSID(1);
  doc[PARAM_PASS] = getWifiPass(0);
  doc[PARAM_PASS2] = getWifiPass(1);
  doc[PARAM_WIFI_PORTAL_TIMEOUT] = getWifiPortalTimeout();
  doc[PARAM_WIFI_CONNECT_TIMEOUT] = getWifiConnectionTimeout();
}

void BaseConfig::parseJsonWifi(JsonObject& doc) {
#if LOG_LEVEL == 6
  Log.verbose(F("CFG : Parsing json (wifi)." CR));
#endif
  if (!doc[PARAM_MDNS].isNull()) setMDNS(doc[PARAM_MDNS]);
  if (!doc[PARAM_SSID].isNull()) setWifiSSID(doc[PARAM_SSID], 0);
  if (!doc[PARAM_PASS].isNull()) setWifiPass(doc[PARAM_PASS], 0);
  if (!doc[PARAM_SSID2].isNull()) setWifiSSID(doc[PARAM_SSID2], 1);
  if (!doc[PARAM_PASS2].isNull()) setWifiPass(doc[PARAM_PASS2], 1);
  if (!doc[PARAM_WIFI_PORTAL_TIMEOUT].isNull())
    this->setWifiPortalTimeout(doc[PARAM_WIFI_PORTAL_TIMEOUT].as<int>());
  if (!doc[PARAM_WIFI_CONNECT_TIMEOUT].isNull())
    this->setWifiConnectionTimeout(doc[PARAM_WIFI_CONNECT_TIMEOUT].as<int>());

  _saveNeeded = true;
}

void BaseConfig::createJsonOta(JsonObject& doc) {
#if LOG_LEVEL == 6
  Log.verbose(F("CFG : Create json (ota)." CR));
#endif
  doc[PARAM_OTA_URL] = getOtaURL();
}

void BaseConfig::parseJsonOta(JsonObject& doc) {
#if LOG_LEVEL == 6
  Log.verbose(F("CFG : Parsing json (ota)." CR));
#endif
  if (!doc[PARAM_OTA_URL].isNull()) setOtaURL(doc[PARAM_OTA_URL]);

  _saveNeeded = true;
}

void BaseConfig::createJsonPush(JsonObject& doc) {
#if LOG_LEVEL == 6
  Log.verbose(F("CFG : Create json (push)." CR));
#endif
  JsonArray headers;

  doc[PARAM_HTTP_POST_TARGET] = getTargetHttpPost();
  doc[PARAM_HTTP_POST_HEADER1] = getHeader1HttpPost();
  doc[PARAM_HTTP_POST_HEADER2] = getHeader2HttpPost();
  doc[PARAM_HTTP_POST2_TARGET] = getTargetHttpPost2();
  doc[PARAM_HTTP_POST2_HEADER1] = getHeader1HttpPost2();
  doc[PARAM_HTTP_POST2_HEADER2] = getHeader2HttpPost2();
  doc[PARAM_HTTP_GET_TARGET] = getTargetHttpGet();
  doc[PARAM_HTTP_GET_HEADER1] = getHeader1HttpGet();
  doc[PARAM_HTTP_GET_HEADER2] = getHeader2HttpGet();
  doc[PARAM_INFLUXDB2_TARGET] = getTargetInfluxDB2();
  doc[PARAM_INFLUXDB2_ORG] = getOrgInfluxDB2();
  doc[PARAM_INFLUXDB2_BUCKET] = getBucketInfluxDB2();
  doc[PARAM_INFLUXDB2_TOKEN] = getTokenInfluxDB2();
  doc[PARAM_MQTT_TARGET] = getTargetMqtt();
  doc[PARAM_MQTT_PORT] = getPortMqtt();
  doc[PARAM_MQTT_USER] = getUserMqtt();
  doc[PARAM_MQTT_PASS] = getPassMqtt();
  doc[PARAM_PUSH_TIMEOUT] = getPushTimeout();
  doc[PARAM_DARK_MODE] = getDarkMode();
}

void BaseConfig::parseJsonPush(JsonObject& doc) {
#if LOG_LEVEL == 6
  Log.verbose(F("CFG : Parsing json (push)." CR));
#endif
  if (!doc[PARAM_HTTP_POST_TARGET].isNull())
    setTargetHttpPost(doc[PARAM_HTTP_POST_TARGET]);
  if (!doc[PARAM_HTTP_POST_HEADER1].isNull())
    setHeader1HttpPost(doc[PARAM_HTTP_POST_HEADER1]);
  if (!doc[PARAM_HTTP_POST_HEADER2].isNull())
    setHeader2HttpPost(doc[PARAM_HTTP_POST_HEADER2]);

  if (!doc[PARAM_HTTP_POST2_TARGET].isNull())
    setTargetHttpPost2(doc[PARAM_HTTP_POST2_TARGET]);
  if (!doc[PARAM_HTTP_POST2_HEADER1].isNull())
    setHeader1HttpPost2(doc[PARAM_HTTP_POST2_HEADER1]);
  if (!doc[PARAM_HTTP_POST2_HEADER2].isNull())
    setHeader2HttpPost2(doc[PARAM_HTTP_POST2_HEADER2]);

  if (!doc[PARAM_HTTP_GET_TARGET].isNull())
    setTargetHttpGet(doc[PARAM_HTTP_GET_TARGET]);
  if (!doc[PARAM_HTTP_GET_HEADER1].isNull())
    setHeader1HttpGet(doc[PARAM_HTTP_GET_HEADER1]);
  if (!doc[PARAM_HTTP_GET_HEADER2].isNull())
    setHeader2HttpGet(doc[PARAM_HTTP_GET_HEADER2]);

  if (!doc[PARAM_INFLUXDB2_TARGET].isNull())
    setTargetInfluxDB2(doc[PARAM_INFLUXDB2_TARGET]);
  if (!doc[PARAM_INFLUXDB2_ORG].isNull())
    setOrgInfluxDB2(doc[PARAM_INFLUXDB2_ORG]);
  if (!doc[PARAM_INFLUXDB2_BUCKET].isNull())
    setBucketInfluxDB2(doc[PARAM_INFLUXDB2_BUCKET]);
  if (!doc[PARAM_INFLUXDB2_TOKEN].isNull())
    setTokenInfluxDB2(doc[PARAM_INFLUXDB2_TOKEN]);

  if (!doc[PARAM_MQTT_TARGET].isNull()) setTargetMqtt(doc[PARAM_MQTT_TARGET]);
  if (!doc[PARAM_MQTT_PORT].isNull())
    setPortMqtt(doc[PARAM_MQTT_PORT].as<int>());
  if (!doc[PARAM_MQTT_USER].isNull()) setUserMqtt(doc[PARAM_MQTT_USER]);
  if (!doc[PARAM_MQTT_PASS].isNull()) setPassMqtt(doc[PARAM_MQTT_PASS]);

  if (!doc[PARAM_PUSH_TIMEOUT].isNull())
    setPushTimeout(doc[PARAM_PUSH_TIMEOUT].as<int>());

  if (!doc[PARAM_DARK_MODE].isNull())
    setDarkMode(doc[PARAM_DARK_MODE].as<bool>());

  _saveNeeded = true;
}

void BaseConfig::createJsonBase(JsonObject& doc) {
#if LOG_LEVEL == 6
  Log.verbose(F("CFG : Create json (base)." CR));
#endif
  doc[PARAM_ID] = getID();
  doc[PARAM_TEMP_FORMAT] = String(getTempFormat());
}

void BaseConfig::parseJsonBase(JsonObject& doc) {
#if LOG_LEVEL == 6
  Log.verbose(F("CFG : Parsing json (base)." CR));
#endif

  // Dont parse ID since this is always set based on device ID.

  if (!doc[PARAM_TEMP_FORMAT].isNull()) {
    String s = doc[PARAM_TEMP_FORMAT];
    setTempFormat(s.charAt(0));
  }

  _saveNeeded = true;
}

bool BaseConfig::saveFile() {
  if (!_saveNeeded) {
#if LOG_LEVEL == 6
    Log.verbose(F("CFG : Skipping save, not needed." CR));
#endif
    return true;
  }

  File configFile = LittleFS.open(_fileName.c_str(), "w");

  if (!configFile) {
    Log.error(F("CFG : Failed to open file %s for save." CR),
              _fileName.c_str());
    return false;
  }

  DynamicJsonDocument doc(_dynamicJsonSize);
  JsonObject obj = doc.createNestedObject();
  createJson(obj);
#if LOG_LEVEL == 6
  serializeJson(doc, EspSerial);
  EspSerial.print(CR);
#endif
  serializeJson(obj, configFile);
  configFile.flush();
  configFile.close();
  _saveNeeded = false;
  Log.notice(F("CFG : Configuration saved." CR));
  return true;
}

bool BaseConfig::loadFile() {
#if LOG_LEVEL == 6
  Log.verbose(F("CFG : Loading configuration from file." CR));
#endif

  if (!LittleFS.exists(_fileName.c_str())) {
    Log.error(F("CFG : Configuration file %s does not exist." CR),
              _fileName.c_str());
    return false;
  }

  File configFile = LittleFS.open(_fileName.c_str(), "r");

  if (!configFile) {
    Log.error(F("CFG : Failed to open %s." CR), _fileName.c_str());
    return false;
  }

  DynamicJsonDocument doc(_dynamicJsonSize);
  DeserializationError err = deserializeJson(doc, configFile);
#if LOG_LEVEL == 6
  serializeJson(doc, EspSerial);
  EspSerial.print(CR);
#endif
  configFile.close();

  if (err) {
    Log.error(F("CFG : Failed to parse file, Err: %s, %d." CR), err.c_str(),
              doc.capacity());
    return false;
  }

  JsonObject obj = doc.as<JsonObject>();
  parseJson(obj);
  Log.notice(F("CFG : Configuration file loaded." CR));
  return true;
}

void BaseConfig::formatFileSystem() {
#if LOG_LEVEL == 6
  Log.verbose(F("CFG : Formating filesystem." CR));
#endif
  LittleFS.format();
}

void BaseConfig::checkFileSystem() {
#if LOG_LEVEL == 6
  Log.verbose(F("CFG : Checking if filesystem is valid." CR));
#endif
#ifdef ESP8266
  if (LittleFS.begin()) {
#if LOG_LEVEL == 6
    Log.verbose(F("CFG : Filesystem mounted." CR));
#endif
  } else {
    Log.error(F("CFG : Unable to mount file system, formatting..." CR));
    LittleFS.format();
    LittleFS.begin();
  }
#else
  if (LittleFS.begin(true)) {
#if LOG_LEVEL == 6
    Log.verbose(F("CFG : Filesystem mounted." CR));
#endif
  } else {
    Log.error(F("CFG : Unable to mount/format file system..." CR));
  }
#endif
}

bool BaseConfig::saveFileWifiOnly() {
  File configFile = LittleFS.open(_fileName, "w");

  if (!configFile) {
    writeErrorLog("CFG : Failed to save configuration.");
    return false;
  }

  DynamicJsonDocument doc(JSON_BUFFER_SIZE_S);
  JsonObject obj = doc.createNestedObject();

  obj[PARAM_SSID] = getWifiSSID(0);
  obj[PARAM_PASS] = getWifiPass(0);
  obj[PARAM_SSID2] = getWifiSSID(1);
  obj[PARAM_PASS2] = getWifiPass(1);

  serializeJson(obj, configFile);
  configFile.flush();
  configFile.close();

  _saveNeeded = false;
  Log.notice(F("CFG : WIFI configuration saved to %s." CR), _fileName.c_str());
  return true;
}

// EOF
