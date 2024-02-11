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
#ifndef SRC_BASECONFIG_HPP_
#define SRC_BASECONFIG_HPP_

#include <espframework.hpp>
#include <interface.hpp>

class BaseConfig : public WifiConfig,
                   public OtaConfig,
                   public WebConfig,
                   public PushConfig {
 private:
  // WifiConfig
  String _mDNS;
  String _wifiSSID[2] = {"", ""};
  String _wifiPASS[2] = {"", ""};
  int _wifiConnectionTimeout = 30;
  int _wifiPortalTimeout = 120;

  // OtaConfig
  String _otaURL;

  // PushConfig
  String _targetHttpPost;
  String _header1HttpPost = "Content-Type: application/json";
  String _header2HttpPost;
  String _targetHttpPost2;
  String _header1HttpPost2 = "Content-Type: application/json";
  String _header2HttpPost2;
  String _targetHttpGet;
  String _header1HttpGet;
  String _header2HttpGet;
  String _targetInfluxDb2;
  String _orgInfluxDb2;
  String _bucketInfluxDb2;
  String _tokenInfluxDb2;
  String _targetMqtt;
  int _portMqtt = 1883;
  String _userMqtt;
  String _passMqtt;
  int _pushTimeout = 10;

  // BaseConfig
  String _id;
  char _tempFormat = 'C';
  String _fileName;
  int _dynamicJsonSize;
  bool _darkMode = false;

  void formatFileSystem();

  // Internal
 protected:
  bool _saveNeeded;

  void createJsonBase(JsonObject& doc);
  void createJsonOta(JsonObject& doc);
  void createJsonWifi(JsonObject& doc);
  void createJsonPush(JsonObject& doc);

  void parseJsonBase(JsonObject& doc);
  void parseJsonOta(JsonObject& doc);
  void parseJsonWifi(JsonObject& doc);
  void parseJsonPush(JsonObject& doc);

 public:
  BaseConfig(String baseMDNS, String fileName, int dynamicJsonSize);

  // WifiConfig
  const char* getMDNS() { return _mDNS.c_str(); }
  void setMDNS(String s) {
    _mDNS = s;
    _saveNeeded = true;
  }
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
  int getWifiConnectionTimeout() { return _wifiConnectionTimeout; }
  void setWifiConnectionTimeout(int t) {
    _wifiConnectionTimeout = t;
    _saveNeeded = true;
  }
  int getWifiPortalTimeout() { return _wifiPortalTimeout; }
  void setWifiPortalTimeout(int t) {
    _wifiPortalTimeout = t;
    _saveNeeded = true;
  }

  int getPushTimeout() { return _pushTimeout; }
  void setPushTimeout(int t) { _pushTimeout = t; }

  // OtaConfig
  const char* getOtaURL() { return _otaURL.c_str(); }
  void setOtaURL(String s) {
    _otaURL = s;
    _saveNeeded = true;
  }
  bool isOtaActive() { return _otaURL.length() > 0 ? true : false; }
  bool isOtaSSL() { return _otaURL.startsWith("https://"); }

  // PushConfig
  bool hasTargetHttpPost() { return _targetHttpPost.length() ? true : false; }
  bool hasTargetHttpPost2() { return _targetHttpPost2.length() ? true : false; }
  bool hasTargetHttpGet() { return _targetHttpGet.length() ? true : false; }
  bool hasTargetInfluxDb2() { return _targetInfluxDb2.length() ? true : false; }
  bool hasTargetMqtt() { return _targetMqtt.length() ? true : false; }

  bool isHttpPostSSL() { return _targetHttpPost.startsWith("https://"); }
  bool isHttpPost2SSL() { return _targetHttpPost2.startsWith("https://"); }
  bool isHttpGetSSL() { return _targetHttpGet.startsWith("https://"); }
  bool isHttpInfluxDb2SSL() { return _targetInfluxDb2.startsWith("https://"); }
  bool isMqttSSL() { return _portMqtt > 8000 ? true : false; }

  const char* getTargetHttpPost() { return _targetHttpPost.c_str(); }
  void setTargetHttpPost(String target) {
    _targetHttpPost = target;
    _saveNeeded = true;
  }
  const char* getHeader1HttpPost() { return _header1HttpPost.c_str(); }
  void setHeader1HttpPost(String header) {
    _header1HttpPost = header;
    _saveNeeded = true;
  }
  const char* getHeader2HttpPost() { return _header2HttpPost.c_str(); }
  void setHeader2HttpPost(String header) {
    _header2HttpPost = header;
    _saveNeeded = true;
  }

  const char* getTargetHttpPost2() { return _targetHttpPost2.c_str(); }
  void setTargetHttpPost2(String target) {
    _targetHttpPost2 = target;
    _saveNeeded = true;
  }
  const char* getHeader1HttpPost2() { return _header1HttpPost2.c_str(); }
  void setHeader1HttpPost2(String header) {
    _header1HttpPost2 = header;
    _saveNeeded = true;
  }
  const char* getHeader2HttpPost2() { return _header2HttpPost2.c_str(); }
  void setHeader2HttpPost2(String header) {
    _header2HttpPost2 = header;
    _saveNeeded = true;
  }

  const char* getTargetHttpGet() { return _targetHttpGet.c_str(); }
  void setTargetHttpGet(String target) {
    _targetHttpGet = target;
    _saveNeeded = true;
  }
  const char* getHeader1HttpGet() { return _header1HttpGet.c_str(); }
  void setHeader1HttpGet(String header) {
    _header1HttpGet = header;
    _saveNeeded = true;
  }
  const char* getHeader2HttpGet() { return _header2HttpGet.c_str(); }
  void setHeader2HttpGet(String header) {
    _header2HttpGet = header;
    _saveNeeded = true;
  }

  const char* getTargetInfluxDB2() { return _targetInfluxDb2.c_str(); }
  void setTargetInfluxDB2(String target) {
    _targetInfluxDb2 = target;
    _saveNeeded = true;
  }
  const char* getOrgInfluxDB2() { return _orgInfluxDb2.c_str(); }
  void setOrgInfluxDB2(String org) {
    _orgInfluxDb2 = org;
    _saveNeeded = true;
  }
  const char* getBucketInfluxDB2() { return _bucketInfluxDb2.c_str(); }
  void setBucketInfluxDB2(String bucket) {
    _bucketInfluxDb2 = bucket;
    _saveNeeded = true;
  }
  const char* getTokenInfluxDB2() { return _tokenInfluxDb2.c_str(); }
  void setTokenInfluxDB2(String token) {
    _tokenInfluxDb2 = token;
    _saveNeeded = true;
  }

  const char* getTargetMqtt() { return _targetMqtt.c_str(); }
  void setTargetMqtt(String target) {
    _targetMqtt = target;
    _saveNeeded = true;
  }
  int getPortMqtt() { return _portMqtt; }
  void setPortMqtt(int port) {
    _portMqtt = port;
    _saveNeeded = true;
  }
  const char* getUserMqtt() { return _userMqtt.c_str(); }
  void setUserMqtt(String user) {
    _userMqtt = user;
    _saveNeeded = true;
  }
  const char* getPassMqtt() { return _passMqtt.c_str(); }
  void setPassMqtt(String pass) {
    _passMqtt = pass;
    _saveNeeded = true;
  }

  // Base
  const char* getID() { return _id.c_str(); }

  char getTempFormat() { return _tempFormat; }
  void setTempFormat(char c) {
    if (c == 'C' || c == 'F') {
      _tempFormat = c;
      _saveNeeded = true;
    }
  }
  bool isTempFormatC() { return _tempFormat == 'C' ? true : false; }
  bool isTempFormatF() { return _tempFormat == 'F' ? true : false; }

  bool getDarkMode() { return _darkMode; }
  void setDarkMode(bool b) {
    _darkMode = b;
    _saveNeeded = true;
  }

  // Functions
  virtual void createJson(JsonObject& doc) {}
  virtual void parseJson(JsonObject& doc) {}

  bool saveFile();
  bool loadFile();
  bool saveFileWifiOnly();
  void checkFileSystem();
  bool isSaveNeeded() { return _saveNeeded; }
  void setSaveNeeded() { _saveNeeded = true; }
};

#endif  // SRC_BASECONFIG_HPP_

// EOF
