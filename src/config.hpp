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
#include <resources.hpp>

#define CFG_JSON_BUFSIZE 3192

#define CFG_APPNAME "GravityMon"          // Name of firmware
#define CFG_FILENAME "/gravitymon.json"   // Name of config file
#define CFG_HW_FILENAME "/hardware.json"  // Name of config file for hw

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

class HardwareConfig {
 private:
  int _wifiPortalTimeout = 120;
  float _maxFormulaCreationDeviation = 1.6;
  float _defaultCalibrationTemp = 20.0;
  int _gyroSensorMovingThreashold = 500;
  int _gyroReadCount = 50;
  int _gyroReadDelay = 3150;  // us, empirical, to hold sampling to 200 Hz

 public:
  int getWifiPortalTimeout() { return _wifiPortalTimeout; }
  void setWifiPortalTimeout(int t) { _wifiPortalTimeout = t; }
  float getMaxFormulaCreationDeviation() {
    return _maxFormulaCreationDeviation;
  }
  void setMaxFormulaCreationDeviation(float f) {
    _maxFormulaCreationDeviation = f;
  }
  float getDefaultCalibrationTemp() { return _defaultCalibrationTemp; }
  void SetDefaultCalibrationTemp(float t) { _defaultCalibrationTemp = t; }
  int getGyroSensorMovingThreashold() { return _gyroSensorMovingThreashold; }
  void setGyroSensorMovingThreashold(int t) { _gyroSensorMovingThreashold = t; }
  int getGyroReadCount() { return _gyroReadCount; }
  void setGyroReadCount(int c) { _gyroReadCount = c; }
  int getGyroReadDelay() { return _gyroReadDelay; }
  void setGyroReadDelay(int d) { _gyroReadDelay = d; }

  bool saveFile();
  bool loadFile();
};

class Config {
 private:
  bool _saveNeeded = false;

  // Device configuration
  String _id = "";
  String _mDNS = "";
  String _otaURL = "";
  char _tempFormat = 'C';
  float _voltageFactor = 0;
  float _tempSensorAdjC = 0;
  int _sleepInterval = 900;
  bool _gyroTemp = false;

  // Wifi Config
  String _wifiSSID = "";
  String _wifiPASS = "";

  // Push target settings
  String _brewfatherPushUrl = "";

  String _token = "";

  String _httpUrl = "";
  String _httpHeader[2] = { "Content-Type: application/json", "" };
  String _http2Url = "";
  String _http2Header[2] = { "Content-Type: application/json", "" };

  String _influxDb2Url = "";
  String _influxDb2Org = "";
  String _influxDb2Bucket = "";
  String _influxDb2Token = "";

  String _mqttUrl = "";
  int _mqttPort = 1883;
  String _mqttUser = "";
  String _mqttPass = "";

  // Gravity and temperature calculations
  String _gravityFormula = "";
  bool _gravityTempAdj = false;
  char _gravityFormat = 'G';

  // Gyro calibration and formula calculation data
  RawGyroData _gyroCalibration = {0, 0, 0, 0, 0, 0};
  RawFormulaData _formulaData = {{0, 0, 0, 0, 0}, {1, 1, 1, 1, 1}};

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

  // Token parameter
  const char* getToken() { return _token.c_str(); }
  void setToken(String s) {
    _token = s;
    _saveNeeded = true;
  }

  // Standard HTTP
  const char* getHttpUrl() { return _httpUrl.c_str(); }
  void setHttpUrl(String s) {
    _httpUrl = s;
    _saveNeeded = true;
  }
  const char* getHttpHeader(int idx) { return _httpHeader[idx].c_str(); }
  void setHttpHeader(String s, int idx) {
    _httpHeader[idx] = s;
    _saveNeeded = true;
  }
  bool isHttpActive() { return _httpUrl.length() ? true : false; }

  const char* getHttp2Url() { return _http2Url.c_str(); }
  void setHttp2Url(String s) {
    _http2Url = s;
    _saveNeeded = true;
  }
  const char* getHttp2Header(int idx) { return _http2Header[idx].c_str(); }
  void setHttp2Header(String s, int idx) {
    _http2Header[idx] = s;
    _saveNeeded = true;
  }
  bool isHttpActive2() { return _http2Url.length() ? true : false; }

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
  int getMqttPort() { return _mqttPort; }
  void setMqttPort(String s) {
    _mqttPort = s.toInt();
    _saveNeeded = true;
  }
  void setMqttPort(int i) {
    _mqttPort = i;
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
extern HardwareConfig myHardwareConfig;

#endif  // SRC_CONFIG_HPP_

// EOF
