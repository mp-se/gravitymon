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

#include <baseconfig.hpp>
#include <main.hpp>
#include <utils.hpp>

enum BleFormat {
  BLE_DISABLED = 0,
  BLE_TILT = 1,
  BLE_TILT_PRO = 2,
  BLE_GRAVITYMON_EDDYSTONE = 4,
  BLE_GRAVITYMON_IBEACON = 5
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

// Used for holding result data
struct GyroResultData {
  bool isValid;
  float angle;
  float temp;
};

// Used for holding formulaData (used for calculating formula on device)
#define FORMULA_DATA_SIZE 20

struct RawFormulaData {
  double a[FORMULA_DATA_SIZE];
  double g[FORMULA_DATA_SIZE];
};

class GravmonConfig : public BaseConfig {
 private:
  int _configVersion = 2;

  // Device configuration
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
  bool _gyroDisabled = false;
  int _voltagePin = PIN_VOLT;

  bool _wifiDirect = false;

  // Push target settings
  String _token = "";
  String _token2 = "";

  // Gravity and temperature calculations
  String _gravityFormula = "";
  bool _gravityTempAdj = false;
  char _gravityFormat = 'G';

  // BLE (ESP32 only)
  String _bleTiltColor;
  BleFormat _bleFormat = BleFormat::BLE_DISABLED;

  // Gyro calibration and formula calculation data
  RawGyroData _gyroCalibration = {0, 0, 0, 0, 0, 0};
  RawFormulaData _formulaData = {
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};

  float _maxFormulaCreationDeviation = 0.01;  // SG
  float _defaultCalibrationTemp = 20.0;    // C
  int _gyroSensorMovingThreashold = 500;
  int _tempSensorResolution = 9;  // bits
  int _gyroReadCount = 50;
  int _gyroReadDelay = 3150;  // us, empirical, to hold sampling to 200 Hz
  int _pushIntervalPost = 0;
  int _pushIntervalPost2 = 0;
  int _pushIntervalGet = 0;
  int _pushIntervalInflux = 0;
  int _pushIntervalMqtt = 0;
  bool _ignoreLowAnges = false;
#if defined(ESP32LITE)
  bool _batterySaving = false;
#else
  bool _batterySaving = true;
#endif
  bool _darkMode = false;

  void formatFileSystem();

 public:
  GravmonConfig(String baseMDNS, String fileName);
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

  const bool isWifiDirect() { return _wifiDirect; }
  void setWifiDirect(bool b) {
    _wifiDirect = b;
    _saveNeeded = true;
  }

  const bool isStorageSleep() { return _storageSleep; }
  void setStorageSleep(bool b) {
    _storageSleep = b;
    _saveNeeded = true;
  }

  const bool isGyroDisabled() { return _gyroDisabled; }
  void setGyroDisabled(bool b) {
    _gyroDisabled = b;
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

  const int getSleepInterval() const { return _sleepInterval; }
  void setSleepInterval(int v) {
    _sleepInterval = v;
    _saveNeeded = true;
  }
  void setSleepInterval(String s) {
    _sleepInterval = s.toInt();
    _saveNeeded = true;
  }

  int getVoltagePin() {
#if defined(ESP32LITE)  // Can only be configured for the floaty hardware
    return _voltagePin;
#else
    return PIN_VOLT;
#endif
  }
  void setVoltagePin(int v) {
    _voltagePin = v;
    _saveNeeded = true;
  }

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
    return (hasTargetHttpPost() || hasTargetHttpPost2() || hasTargetHttpGet() ||
            hasTargetInfluxDb2() || hasTargetMqtt())
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

  float getMaxFormulaCreationDeviation() {
    return _maxFormulaCreationDeviation;
  }
  void setMaxFormulaCreationDeviation(float f) {
    _maxFormulaCreationDeviation = f;
    _saveNeeded = true;
  }

  int getTempSensorResolution() { return _tempSensorResolution; }
  void setTempSensorResolution(int t) {
    if (t >= 9 && t <= 12) _tempSensorResolution = t;
    _saveNeeded = true;
  }

  float getDefaultCalibrationTemp() { return _defaultCalibrationTemp; }
  void SetDefaultCalibrationTemp(float t) {
    _defaultCalibrationTemp = t;
    _saveNeeded = true;
  }

  int getGyroSensorMovingThreashold() { return _gyroSensorMovingThreashold; }
  void setGyroSensorMovingThreashold(int t) {
    _gyroSensorMovingThreashold = t;
    _saveNeeded = true;
  }

  int getGyroReadCount() { return _gyroReadCount; }
  void setGyroReadCount(int c) {
    _gyroReadCount = c;
    _saveNeeded = true;
  }

  int getGyroReadDelay() { return _gyroReadDelay; }
  void setGyroReadDelay(int d) {
    _gyroReadDelay = d;
    _saveNeeded = true;
  }

  int getPushIntervalPost() { return _pushIntervalPost; }
  void setPushIntervalPost(int t) {
    _pushIntervalPost = t;
    _saveNeeded = true;
  }

  int getPushIntervalPost2() { return _pushIntervalPost2; }
  void setPushIntervalPost2(int t) {
    _pushIntervalPost2 = t;
    _saveNeeded = true;
  }

  int getPushIntervalGet() { return _pushIntervalGet; }
  void setPushIntervalGet(int t) {
    _pushIntervalGet = t;
    _saveNeeded = true;
  }

  int getPushIntervalInflux() { return _pushIntervalInflux; }
  void setPushIntervalInflux(int t) {
    _pushIntervalInflux = t;
    _saveNeeded = true;
  }

  int getPushIntervalMqtt() { return _pushIntervalMqtt; }
  void setPushIntervalMqtt(int t) {
    _pushIntervalMqtt = t;
    _saveNeeded = true;
  }

  bool isPushIntervalActive() {
    return (_pushIntervalPost + _pushIntervalPost2 + _pushIntervalGet +
            _pushIntervalInflux + _pushIntervalMqtt) == 0
               ? false
               : true;
  }

  bool isSkipSslOnTest() { return _skipSslOnTest; }
  void setSkipSslOnTest(bool b) {
    _skipSslOnTest = b;
    _saveNeeded = true;
  }

  const bool isIgnoreLowAnges() { return _ignoreLowAnges; }
  void setIgnoreLowAnges(bool b) {
    _ignoreLowAnges = b;
    _saveNeeded = true;
  }

  const bool isBatterySaving() { return _batterySaving; }
  void setBatterySaving(bool b) {
    _batterySaving = b;
    _saveNeeded = true;
  }

  // IO functions
  void createJson(JsonObject& doc);
  void parseJson(JsonObject& doc);
  void migrateSettings();
  void migrateHwSettings();
};

extern GravmonConfig myConfig;

#endif  // SRC_CONFIG_HPP_

// EOF
