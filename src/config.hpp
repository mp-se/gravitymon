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
#ifndef SRC_CONFIG_HPP_
#define SRC_CONFIG_HPP_

#include <helper.hpp>
#include <resources.hpp>

#define CFG_APPNAME "GravityMon"
#define CFG_FILENAME "/gravitymon2.json"

#define CFG_FILENAME_OLD \
  "/gravitymon.json"  // TODO: Keep these for migration routine.
#define CFG_HW_FILENAME_OLD \
  "/hardware.json"  // TODO: Keep these for migration routine.

enum BleFormat {
  BLE_DISABLED = 0,
  BLE_TILT = 1,
  BLE_TILT_PRO = 2,
  BLE_GRAVITYMON_SERVICE = 3,
  BLE_GRAVITYMON_EDDYSTONE = 4
};

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
#define FORMULA_DATA_SIZE 10

struct RawFormulaData {
  double a[FORMULA_DATA_SIZE];
  double g[FORMULA_DATA_SIZE];
};

class Config {
 private:
  bool _saveNeeded = false;
  int _configVersion = 2;

  // Device configuration
  String _id = "";
  String _mDNS = "";
  String _otaURL = "";
  char _tempFormat = 'C';
#if defined(ESP8266)
  float _voltageFactor = 1.59;
#elif defined(ESP32C3)
  float _voltageFactor = 1.3;
#elif defined(ESP32S2)
  float _voltageFactor = 0.59;
#elif defined(ESP32S3)
  float _voltageFactor = 1.54;
#elif defined(ESP32LITE)
  float _voltageFactor = 1.59;
#else  // ESP32
  float _voltageFactor = 1.3;
#endif
  float _voltageConfig = 4.15;
  float _tempSensorAdjC = 0;
  int _sleepInterval = 900;
#if defined(FLOATY)
  bool _gyroTemp = true;
#else
  bool _gyroTemp = false;
#endif
  bool _storageSleep = false;
#if defined(ESP8266)
  bool _skipSslOnTest = true;
#else
  bool _skipSslOnTest = false;
#endif

  // Wifi Config
  String _wifiSSID[2] = {"", ""};
  String _wifiPASS[2] = {"", ""};

  // Push target settings
  String _token = "";
  String _token2 = "";

  String _httpUrl = "";
  String _httpHeader[2] = {"Content-Type: application/json", ""};
  String _http2Url = "";
  String _http2Header[2] = {"Content-Type: application/json", ""};
  String _http3Url = "";

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

  // BLE (ESP32 only)
  String _bleTiltColor;
  BleFormat _bleFormat = BleFormat::BLE_DISABLED;

  // Gyro calibration and formula calculation data
  RawGyroData _gyroCalibration = {0, 0, 0, 0, 0, 0};
  RawFormulaData _formulaData = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                 {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};

  int _wifiPortalTimeout = 120;            // seconds
  int _wifiConnectTimeout = 20;            // seconds
  float _maxFormulaCreationDeviation = 3;  // SG
  float _defaultCalibrationTemp = 20.0;    // C
  int _gyroSensorMovingThreashold = 500;
  int _tempSensorResolution = 9;  // bits
  int _gyroReadCount = 50;
  int _gyroReadDelay = 3150;  // us, empirical, to hold sampling to 200 Hz
  int _pushTimeout = 10;      // seconds
  int _pushIntervalHttp1 = 0;
  int _pushIntervalHttp2 = 0;
  int _pushIntervalHttp3 = 0;
  int _pushIntervalInflux = 0;
  int _pushIntervalMqtt = 0;
  bool _ignoreLowAnges = false;
#if defined(ESP32LITE)
  bool _batterySaving = false;
#else
  bool _batterySaving = true;
#endif

  void formatFileSystem();

 public:
  Config();
  const char* getID() { return _id.c_str(); }

  const char* getMDNS() { return _mDNS.c_str(); }
  void setMDNS(String s) {
    _mDNS = s;
    _saveNeeded = true;
  }

  int getConfigVersion() { return _configVersion; }

  const bool isGyroTemp() { return _gyroTemp; }
  void setGyroTemp(bool b) {
#if defined(FLOATY)
    // Floaty hardware dont have a temp sensor, uses gyro temperature
#else
    _gyroTemp = b;
    _saveNeeded = true;
#endif
  }

  const bool isStorageSleep() { return _storageSleep; }
  void setStorageSleep(bool b) {
    _storageSleep = b;
    _saveNeeded = true;
  }

  const char* getOtaURL() { return _otaURL.c_str(); }
  void setOtaURL(String s) {
    _otaURL = s;
    _saveNeeded = true;
  }
  bool isOtaActive() { return _otaURL.length() ? true : false; }
  bool isOtaSSL() { return _otaURL.startsWith("https://"); }

  const char* getWifiSSID(int idx) { return _wifiSSID[idx].c_str(); }
  void setWifiSSID(String s, int idx) {
    _wifiSSID[idx] = s;
    _saveNeeded = true;
  }
  const char* getWifiPass(int idx) { return _wifiPASS[idx].c_str(); }
  void setWifiPass(String s, int idx) {
    _wifiPASS[idx] = s;
    _saveNeeded = true;
  }
  bool dualWifiConfigured() {
    return _wifiSSID[0].length() > 0 && _wifiSSID[1].length() > 0 ? true
                                                                  : false;
  }
  void swapPrimaryWifi() {
    String s = _wifiSSID[0];
    _wifiSSID[0] = _wifiSSID[1];
    _wifiSSID[1] = s;

    String p = _wifiPASS[0];
    _wifiPASS[0] = _wifiPASS[1];
    _wifiPASS[1] = p;

    _saveNeeded = true;
  }

  // Token parameter
  const char* getToken() { return _token.c_str(); }
  void setToken(String s) {
    _token = s;
    _saveNeeded = true;
  }
  const char* getToken2() { return _token2.c_str(); }
  void setToken2(String s) {
    _token2 = s;
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
  bool isHttpSSL() { return _httpUrl.startsWith("https://"); }

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
  bool isHttp2Active() { return _http2Url.length() ? true : false; }
  bool isHttp2SSL() { return _http2Url.startsWith("https://"); }

  const char* getHttp3Url() { return _http3Url.c_str(); }
  void setHttp3Url(String s) {
    _http3Url = s;
    _saveNeeded = true;
  }
  bool isHttp3Active() { return _http3Url.length() ? true : false; }
  bool isHttp3SSL() { return _http3Url.startsWith("https://"); }

  // InfluxDB2
  const char* getInfluxDb2PushUrl() { return _influxDb2Url.c_str(); }
  void setInfluxDb2PushUrl(String s) {
    _influxDb2Url = s;
    _saveNeeded = true;
  }
  bool isInfluxDb2Active() { return _influxDb2Url.length() ? true : false; }
  bool isInfluxSSL() { return _influxDb2Url.startsWith("https://"); }
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
  const char* getMqttUrl() { return _mqttUrl.c_str(); }
  void setMqttUrl(String s) {
    _mqttUrl = s;
    _saveNeeded = true;
  }
  bool isMqttActive() { return _mqttUrl.length() ? true : false; }
  bool isMqttSSL() { return _mqttPort > 8000 ? true : false; }

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

  float getVoltageConfig() { return _voltageConfig; }
  void setVoltageConfig(float f) {
    _voltageConfig = f;
    _saveNeeded = true;
  }
  void setVoltageConfig(String s) {
    _voltageConfig = s.toFloat();
    _saveNeeded = true;
  }

  float getTempSensorAdjC() { return _tempSensorAdjC; }
  void setTempSensorAdjC(float f) {
    _tempSensorAdjC = f;
    _saveNeeded = true;
  }
  void setTempSensorAdjC(String s, float adjustC = 0) {
    _tempSensorAdjC = s.toFloat() + adjustC;
    _saveNeeded = true;
  }
  void setTempSensorAdjF(String s, float adjustF = 0) {
    _tempSensorAdjC = convertFtoC(s.toFloat() + adjustF);
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

  const char* getBleTiltColor() { return _bleTiltColor.c_str(); }
  void setBleTiltColor(String c) {
    _bleTiltColor = c;
    _saveNeeded = true;
  }
  bool isBleActive() { return (_bleFormat != BleFormat::BLE_DISABLED); }
  bool isWifiPushActive() {
    return (isHttpActive() || isHttp2Active() || isHttp3Active() ||
            isInfluxDb2Active() || isMqttActive())
               ? true
               : false;
  }

  const BleFormat getBleFormat() { return _bleFormat; }
  void setBleFormat(int b) {
    _bleFormat = (BleFormat)b;
    _saveNeeded = true;
  }
  void setBleFormat(BleFormat b) {
    _bleFormat = b;
    _saveNeeded = true;
  }

  const RawGyroData& getGyroCalibration() { return _gyroCalibration; }
  void setGyroCalibration(const RawGyroData& r) {
    _gyroCalibration = r;
    _saveNeeded = true;
  }

  bool hasGyroCalibration() {
    return (!_gyroCalibration.ax && !_gyroCalibration.ay &&
            !_gyroCalibration.az && !_gyroCalibration.gx &&
            !_gyroCalibration.gy && !_gyroCalibration.gz)
               ? false
               : true;
  }

  const RawFormulaData& getFormulaData() { return _formulaData; }
  void setFormulaData(const RawFormulaData& r) {
    _formulaData = r;
    _saveNeeded = true;
  }

  int getWifiPortalTimeout() { return _wifiPortalTimeout; }
  void setWifiPortalTimeout(int t) { _wifiPortalTimeout = t; }

  int getWifiConnectTimeout() { return _wifiConnectTimeout; }
  void setWifiConnectTimeout(int t) { _wifiConnectTimeout = t; }

  float getMaxFormulaCreationDeviation() {
    return _maxFormulaCreationDeviation;
  }
  void setMaxFormulaCreationDeviation(float f) {
    _maxFormulaCreationDeviation = f;
  }

  int getTempSensorResolution() { return _tempSensorResolution; }
  void setTempSensorResolution(int t) {
    if (t >= 9 && t <= 12) _tempSensorResolution = t;
  }

  float getDefaultCalibrationTemp() { return _defaultCalibrationTemp; }
  void SetDefaultCalibrationTemp(float t) { _defaultCalibrationTemp = t; }

  int getGyroSensorMovingThreashold() { return _gyroSensorMovingThreashold; }
  void setGyroSensorMovingThreashold(int t) { _gyroSensorMovingThreashold = t; }

  int getGyroReadCount() { return _gyroReadCount; }
  void setGyroReadCount(int c) { _gyroReadCount = c; }

  int getGyroReadDelay() { return _gyroReadDelay; }
  void setGyroReadDelay(int d) { _gyroReadDelay = d; }

  int getPushTimeout() { return _pushTimeout; }
  void setPushTimeout(int t) { _pushTimeout = t; }

  int getPushIntervalHttp1() { return _pushIntervalHttp1; }
  void setPushIntervalHttp1(int t) { _pushIntervalHttp1 = t; }

  int getPushIntervalHttp2() { return _pushIntervalHttp2; }
  void setPushIntervalHttp2(int t) { _pushIntervalHttp2 = t; }

  int getPushIntervalHttp3() { return _pushIntervalHttp3; }
  void setPushIntervalHttp3(int t) { _pushIntervalHttp3 = t; }

  int getPushIntervalInflux() { return _pushIntervalInflux; }
  void setPushIntervalInflux(int t) { _pushIntervalInflux = t; }

  int getPushIntervalMqtt() { return _pushIntervalMqtt; }
  void setPushIntervalMqtt(int t) { _pushIntervalMqtt = t; }

  bool isPushIntervalActive() {
    return (_pushIntervalHttp1 + _pushIntervalHttp2 + _pushIntervalHttp3 +
            _pushIntervalInflux + _pushIntervalMqtt) == 0
               ? false
               : true;
  }

  bool isSkipSslOnTest() { return _skipSslOnTest; }
  void setSkipSslOnTest(bool b) { _skipSslOnTest = b; }

  const bool isIgnoreLowAnges() { return _ignoreLowAnges; }
  void setIgnoreLowAnges(bool b) { _ignoreLowAnges = b; }

  const bool isBatterySaving() { return _batterySaving; }
  void setBatterySaving(bool b) { _batterySaving = b; }

  // IO functions
  void createJson(JsonObject& doc);
  void parseJson(JsonObject& doc);
  void migrateJson(JsonObject& doc);
  bool saveFile();
  bool loadFile();
  void checkFileSystem();
  bool isSaveNeeded() { return _saveNeeded; }
  void setSaveNeeded() { _saveNeeded = true; }
};

extern Config myConfig;

#endif  // SRC_CONFIG_HPP_

// EOF
