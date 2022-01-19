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
#ifndef SRC_CONFIG_HPP_
#define SRC_CONFIG_HPP_

#include <helper.hpp>

#define CFG_JSON_BUFSIZE 3192

#define CFG_APPNAME "GravityMon "        // Name of firmware
#define CFG_FILENAME "/gravitymon.json"  // Name of config file

#define WIFI_DEFAULT_SSID "GravityMon"  // Name of created SSID
#define WIFI_DEFAULT_PWD "password"     // Password for created SSID
#define WIFI_MDNS "gravitymon"          // Prefix for MDNS name
#define WIFI_PORTAL_TIMEOUT \
  120  // Number of seconds until the config portal is closed

#define CFG_PARAM_ID "id"
#define CFG_PARAM_MDNS "mdns"  // Device name
#define CFG_PARAM_OTA "ota-url"
#define CFG_PARAM_SSID "wifi-ssid"
#define CFG_PARAM_PASS "wifi-pass"

#define CFG_PARAM_PUSH_BREWFATHER "brewfather-push"
#define CFG_PARAM_PUSH_HTTP "http-push"
#define CFG_PARAM_PUSH_HTTP2 "http-push2"
#define CFG_PARAM_PUSH_INFLUXDB2 "influxdb2-push"
#define CFG_PARAM_PUSH_INFLUXDB2_ORG "influxdb2-org"
#define CFG_PARAM_PUSH_INFLUXDB2_BUCKET "influxdb2-bucket"
#define CFG_PARAM_PUSH_INFLUXDB2_AUTH "influxdb2-auth"
#define CFG_PARAM_PUSH_MQTT "mqtt-push"
#define CFG_PARAM_PUSH_MQTT_USER "mqtt-user"
#define CFG_PARAM_PUSH_MQTT_PASS "mqtt-pass"
#define CFG_PARAM_PUSH_MQTT_TOPIC "mqtt-topic"
#define CFG_PARAM_SLEEP_INTERVAL "sleep-interval"  // Sleep interval
#define CFG_PARAM_TEMPFORMAT "temp-format"         // C or F
#define CFG_PARAM_VOLTAGEFACTOR \
  "voltage-factor"  // Factor to calculate the battery voltage
#define CFG_PARAM_GRAVITY_FORMULA \
  "gravity-formula"  // Formula for calculating gravity
#define CFG_PARAM_GRAVITY_FORMAT "gravity-format"  // Gravity format G or P
#define CFG_PARAM_GRAVITY_TEMP_ADJ \
  "gravity-temp-adjustment"  // True/False. Adjust gravity for temperature
#define CFG_PARAM_TEMP_ADJ \
  "temp-adjustment-value"  // Correction value for temp sensor
#define CFG_PARAM_GYRO_CALIBRATION "gyro-calibration-data"  // READ ONLY
#define CFG_PARAM_GYRO_TEMP \
  "gyro-temp"  // True/False. Use temp sensor in gyro (only in gravity mode)

#define CFG_PARAM_FORMULA_DATA \
  "formula-calculation-data"  // Raw data for the formula calculation

#define CFG_PARAM_APP_NAME "app-name"
#define CFG_PARAM_APP_VER "app-ver"
#define CFG_PARAM_ANGLE "angle"
#define CFG_PARAM_GRAVITY "gravity"
#define CFG_PARAM_TEMP_C "temp-c"
#define CFG_PARAM_TEMP_F "temp-f"
#define CFG_PARAM_BATTERY "battery"
#define CFG_PARAM_SLEEP_MODE "sleep-mode"
#define CFG_PARAM_RSSI "rssi"
#define CFG_PARAM_ERROR "error"

// Used for holding sensordata or sensoroffsets
struct RawGyroData {
  int16_t ax;  // Raw Acceleration
  int16_t ay;
  int16_t az;

  int16_t gx;  // Raw Position
  int16_t gy;
  int16_t gz;

  int16_t temp;  // Only for information (temperature of chip)
};

// Used for holding formulaData (used for calculating formula on device)
struct RawFormulaData {
  double a[5];
  double g[5];
};

class Config {
 private:
  bool _saveNeeded;

  // Device configuration
  String _id;
  String _mDNS;
  String _otaURL;
  char _tempFormat;
  float _voltageFactor;
  float _tempSensorAdjC;
  int _sleepInterval;
  bool _gyroTemp;

  // Wifi Config
  String _wifiSSID;
  String _wifiPASS;

  // Push target settings
  String _brewfatherPushUrl;

  String _httpPushUrl;
  String _httpPushUrl2;

  String _influxDb2Url;
  String _influxDb2Org;
  String _influxDb2Bucket;
  String _influxDb2Token;

  String _mqttUrl;
  String _mqttTopic;
  String _mqttUser;
  String _mqttPass;

  // Gravity and temperature calculations
  String _gravityFormula;
  bool _gravityTempAdj;
  char _gravityFormat;

  // Gyro calibration and formula calculation data
  RawGyroData _gyroCalibration;
  RawFormulaData _formulaData;

  void debug();
  void formatFileSystem();

 public:
  Config();
  const char* getID() { return _id.c_str(); }

  const char* getMDNS() { return _mDNS.c_str(); }
  void setMDNS(String s) {
    _mDNS = s;
    _saveNeeded = true;
  }

  const bool isGyroTemp() { return _gyroTemp; }
  void setGyroTemp(bool b) {
    _gyroTemp = b;
    _saveNeeded = true;
  }

  const char* getOtaURL() { return _otaURL.c_str(); }
  void setOtaURL(String s) {
    _otaURL = s;
    _saveNeeded = true;
  }
  bool isOtaActive() { return _otaURL.length() ? true : false; }

  const char* getWifiSSID() { return _wifiSSID.c_str(); }
  void setWifiSSID(String s) {
    _wifiSSID = s;
    _saveNeeded = true;
  }
  const char* getWifiPass() { return _wifiPASS.c_str(); }
  void setWifiPass(String s) {
    _wifiPASS = s;
    _saveNeeded = true;
  }

  // Brewfather
  const char* getBrewfatherPushUrl() { return _brewfatherPushUrl.c_str(); }
  void setBrewfatherPushUrl(String s) {
    _brewfatherPushUrl = s;
    _saveNeeded = true;
  }
  bool isBrewfatherActive() {
    return _brewfatherPushUrl.length() ? true : false;
  }

  // Standard HTTP
  const char* getHttpPushUrl() { return _httpPushUrl.c_str(); }
  void setHttpPushUrl(String s) {
    _httpPushUrl = s;
    _saveNeeded = true;
  }
  bool isHttpActive() { return _httpPushUrl.length() ? true : false; }
  const char* getHttpPushUrl2() { return _httpPushUrl2.c_str(); }
  void setHttpPushUrl2(String s) {
    _httpPushUrl2 = s;
    _saveNeeded = true;
  }
  bool isHttpActive2() { return _httpPushUrl2.length() ? true : false; }

  // InfluxDB2
  const char* getInfluxDb2PushUrl() { return _influxDb2Url.c_str(); }
  void setInfluxDb2PushUrl(String s) {
    _influxDb2Url = s;
    _saveNeeded = true;
  }
  bool isInfluxDb2Active() { return _influxDb2Url.length() ? true : false; }
  const char* getInfluxDb2PushOrg() { return _influxDb2Org.c_str(); }
  void setInfluxDb2PushOrg(String s) {
    _influxDb2Org = s;
    _saveNeeded = true;
  }
  const char* getInfluxDb2PushBucket() { return _influxDb2Bucket.c_str(); }
  void setInfluxDb2PushBucket(String s) {
    _influxDb2Bucket = s;
    _saveNeeded = true;
  }
  const char* getInfluxDb2PushToken() { return _influxDb2Token.c_str(); }
  void setInfluxDb2PushToken(String s) {
    _influxDb2Token = s;
    _saveNeeded = true;
  }

  // MQTT
  bool isMqttActive() { return _mqttUrl.length() ? true : false; }
  const char* getMqttUrl() { return _mqttUrl.c_str(); }
  void setMqttUrl(String s) {
    _mqttUrl = s;
    _saveNeeded = true;
  }
  const char* getMqttTopic() { return _mqttTopic.c_str(); }
  void setMqttTopic(String s) {
    _mqttTopic = s;
    _saveNeeded = true;
  }
  const char* getMqttUser() { return _mqttUser.c_str(); }
  void setMqttUser(String s) {
    _mqttUser = s;
    _saveNeeded = true;
  }
  const char* getMqttPass() { return _mqttPass.c_str(); }
  void setMqttPass(String s) {
    _mqttPass = s;
    _saveNeeded = true;
  }

  int getSleepInterval() { return _sleepInterval; }
  void setSleepInterval(int v) {
    _sleepInterval = v;
    _saveNeeded = true;
  }
  void setSleepInterval(String s) {
    _sleepInterval = s.toInt();
    _saveNeeded = true;
  }

  char getTempFormat() { return _tempFormat; }
  void setTempFormat(char c) {
    if (c == 'C' || c == 'F') {
      _tempFormat = c;
      _saveNeeded = true;
    }
  }
  bool isTempC() { return _tempFormat == 'C'; }
  bool isTempF() { return _tempFormat == 'F'; }

  float getVoltageFactor() { return _voltageFactor; }
  void setVoltageFactor(float f) {
    _voltageFactor = f;
    _saveNeeded = true;
  }
  void setVoltageFactor(String s) {
    _voltageFactor = s.toFloat();
    _saveNeeded = true;
  }

  float getTempSensorAdjC() { return _tempSensorAdjC; }
  void setTempSensorAdjC(float f) {
    _tempSensorAdjC = f;
    _saveNeeded = true;
  }
  void setTempSensorAdjC(String s) {
    _tempSensorAdjC = s.toFloat();
    _saveNeeded = true;
  }
  void setTempSensorAdjF(String s) {
    _tempSensorAdjC = convertFtoC(s.toFloat());
    _saveNeeded = true;
  }

  const char* getGravityFormula() { return _gravityFormula.c_str(); }
  void setGravityFormula(String s) {
    _gravityFormula = s;
    _saveNeeded = true;
  }

  bool isGravityTempAdj() { return _gravityTempAdj; }
  void setGravityTempAdj(bool b) {
    _gravityTempAdj = b;
    _saveNeeded = true;
  }

  char getGravityFormat() { return _gravityFormat; }
  void setGravityFormat(char c) {
    if (c == 'G' || c == 'P') {
      _gravityFormat = c;
      _saveNeeded = true;
    }
  }
  bool isGravitySG() { return _gravityFormat == 'G'; }
  bool isGravityPlato() { return _gravityFormat == 'P'; }

  const RawGyroData& getGyroCalibration() { return _gyroCalibration; }
  void setGyroCalibration(const RawGyroData& r) {
    _gyroCalibration = r;
    _saveNeeded = true;
  }

  const RawFormulaData& getFormulaData() { return _formulaData; }
  void setFormulaData(const RawFormulaData& r) {
    _formulaData = r;
    _saveNeeded = true;
  }

  // IO functions
  void createJson(DynamicJsonDocument& doc);
  bool saveFile();
  bool loadFile();
  void checkFileSystem();
  bool isSaveNeeded() { return _saveNeeded; }
  void setSaveNeeded() { _saveNeeded = true; }
};

extern Config myConfig;

#endif  // SRC_CONFIG_HPP_

// EOF
