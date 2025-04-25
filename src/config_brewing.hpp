/*
MIT License

Copyright (c) 2021-2025 Magnus

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
#ifndef SRC_CONFIG_BREWING_HPP_
#define SRC_CONFIG_BREWING_HPP_

#include <baseconfig.hpp>
#include <main.hpp>
#include <utils.hpp>

constexpr auto CONFIG_SKIP_SSL_ON_TEST = "skip_ssl_on_test";
constexpr auto CONFIG_CONFIG_VER = "config_version";
constexpr auto CONFIG_TOKEN = "token";
constexpr auto CONFIG_TOKEN2 = "token2";
constexpr auto CONFIG_USE_WIFI_DIRECT = "use_wifi_direct";
constexpr auto CONFIG_SLEEP_INTERVAL = "sleep_interval";
constexpr auto CONFIG_VOLTAGE_FACTOR = "voltage_factor";
constexpr auto CONFIG_VOLTAGE_CONFIG = "voltage_config";
constexpr auto CONFIG_TEMP_ADJ = "temp_adjustment_value";
constexpr auto CONFIG_VOLTAGE_PIN = "voltage_pin";
constexpr auto CONFIG_BLE_FORMAT = "ble_format";
constexpr auto CONFIG_PUSH_INTERVAL_POST = "http_post_int";
constexpr auto CONFIG_PUSH_INTERVAL_POST2 = "http_post2_int";
constexpr auto CONFIG_PUSH_INTERVAL_GET = "http_get_int";
constexpr auto CONFIG_PUSH_INTERVAL_INFLUX = "influxdb2_int";
constexpr auto CONFIG_PUSH_INTERVAL_MQTT = "mqtt_int";
constexpr auto CONFIG_IGNORE_LOW_ANGLES = "ignore_low_angles";
constexpr auto CONFIG_BATTERY_SAVING = "battery_saving";
constexpr auto CONFIG_TEMPSENSOR_RESOLUTION = "tempsensor_resolution";
constexpr auto CONFIG_FLASH_LOGGING = "flash_logging";

class BrewingConfig : public BaseConfig {
 private:
#if defined(ESP8266)
  float _voltageFactor = 1.59;
#elif defined(ESP32C3)
  float _voltageFactor = 2.55;
#elif defined(ESP32S2)
  float _voltageFactor = 2.45;
#elif defined(ESP32S3)
  float _voltageFactor = 3.05;
#elif defined(ESP32)
  float _voltageFactor = 2.45;
#else
#error "Unknown platform"
#endif

  float _voltageConfig = 4.15;
  float _tempSensorAdjC = 0;
  int _sleepInterval = 900;
  int _voltagePin = PIN_VOLT;

  bool _wifiDirect = false;
#if defined(ESP8266)
  bool _skipSslOnTest = true;
#else
  bool _skipSslOnTest = false;
#endif

  // Push target settings
  String _token = "";
  String _token2 = "";

  int _pushIntervalPost = 0;
  int _pushIntervalPost2 = 0;
  int _pushIntervalGet = 0;
  int _pushIntervalInflux = 0;
  int _pushIntervalMqtt = 0;

  int _tempSensorResolution = 9;  // bits

  bool _flashLogging = true;

 public:
  BrewingConfig(String baseMDNS, String fileName);

  bool isFlashLogging() const { return _flashLogging; }
  void setFlashLogging(bool b) {
    _flashLogging = b;
    _saveNeeded = true;
  }

  bool isWifiDirect() const { return _wifiDirect; }
  void setWifiDirect(bool b) {
    _wifiDirect = b;
    _saveNeeded = true;
  }

  const char* getToken() const { return _token.c_str(); }
  void setToken(String s) {
    _token = s;
    _saveNeeded = true;
  }
  const char* getToken2() const { return _token2.c_str(); }
  void setToken2(String s) {
    _token2 = s;
    _saveNeeded = true;
  }

  int getSleepInterval() const { return _sleepInterval; }
  void setSleepInterval(int v) {
    _sleepInterval = v;
    _saveNeeded = true;
  }
  void setSleepInterval(String s) {
    _sleepInterval = s.toInt();
    _saveNeeded = true;
  }

  float getVoltageFactor() const { return _voltageFactor; }
  void setVoltageFactor(float f) {
    _voltageFactor = f;
    _saveNeeded = true;
  }
  void setVoltageFactor(String s) {
    _voltageFactor = s.toFloat();
    _saveNeeded = true;
  }

  int getVoltagePin() const { return _voltagePin; }
  void setVoltagePin(int v) {
    _voltagePin = v;
    _saveNeeded = true;
  }

  float getVoltageConfig() const { return _voltageConfig; }
  void setVoltageConfig(float f) {
    _voltageConfig = f;
    _saveNeeded = true;
  }
  void setVoltageConfig(String s) {
    _voltageConfig = s.toFloat();
    _saveNeeded = true;
  }

  float getTempSensorAdjC() const { return _tempSensorAdjC; }
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

  bool isSkipSslOnTest() const { return _skipSslOnTest; }
  void setSkipSslOnTest(bool b) {
    _skipSslOnTest = b;
    _saveNeeded = true;
  }

  int getTempSensorResolution() const { return _tempSensorResolution; }
  void setTempSensorResolution(int t) {
    if (t >= 9 && t <= 12) _tempSensorResolution = t;
    _saveNeeded = true;
  }

  bool isWifiPushActive() const {
    return (hasTargetHttpPost() || hasTargetHttpPost2() || hasTargetHttpGet() ||
            hasTargetInfluxDb2() || hasTargetMqtt())
               ? true
               : false;
  }

  int getPushIntervalPost() const { return _pushIntervalPost; }
  void setPushIntervalPost(int t) {
    _pushIntervalPost = t;
    _saveNeeded = true;
  }

  int getPushIntervalPost2() const { return _pushIntervalPost2; }
  void setPushIntervalPost2(int t) {
    _pushIntervalPost2 = t;
    _saveNeeded = true;
  }

  int getPushIntervalGet() const { return _pushIntervalGet; }
  void setPushIntervalGet(int t) {
    _pushIntervalGet = t;
    _saveNeeded = true;
  }

  int getPushIntervalInflux() const { return _pushIntervalInflux; }
  void setPushIntervalInflux(int t) {
    _pushIntervalInflux = t;
    _saveNeeded = true;
  }

  int getPushIntervalMqtt() const { return _pushIntervalMqtt; }
  void setPushIntervalMqtt(int t) {
    _pushIntervalMqtt = t;
    _saveNeeded = true;
  }

  bool isPushIntervalActive() const {
    return (_pushIntervalPost + _pushIntervalPost2 + _pushIntervalGet +
            _pushIntervalInflux + _pushIntervalMqtt) == 0
               ? false
               : true;
  }

  void createJson(JsonObject& doc) const;
  void parseJson(JsonObject& doc);
};

#endif  // SRC_CONFIG_BREWING_HPP_

// EOF
