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
#ifndef SRC_INTERFACE_HPP_
#define SRC_INTERFACE_HPP_

#include <ArduinoJson.h>

class OtaConfig {
 public:
  virtual const char* getOtaURL() = 0;
  virtual void setOtaURL(String s);
  virtual bool isOtaActive();
  virtual bool isOtaSSL();
};

class WebConfig {
 public:
  virtual void createJson(JsonObject& doc);
  virtual void parseJson(JsonObject& doc);

  virtual bool saveFile();

  virtual const char* getID();
  virtual const char* getMDNS();
  virtual int getWifiPortalTimeout();
};

class WifiConfig {
 public:
  virtual int getWifiConnectionTimeout() = 0;
  virtual void setWifiConnectionTimeout(int t);
  virtual int getWifiPortalTimeout();
  virtual void setWifiPortalTimeout(int t);

  virtual const char* getMDNS();
  virtual void setMDNS(String s);
  virtual const char* getWifiSSID(int idx);
  virtual void setWifiSSID(String s, int idx);
  virtual const char* getWifiPass(int idx);
  virtual void setWifiPass(String s, int idx);

  virtual bool saveFile();
};

class PushConfig {
 public:
  virtual const char* getID();
  virtual const char* getMDNS();

  virtual int getPushTimeout();
  virtual void setPushTimeout(int t);

  virtual const char* getTargetHttpPost() = 0;
  virtual void setTargetHttpPost(String target);
  virtual const char* getHeader1HttpPost();
  virtual void setHeader1HttpPost(String header);
  virtual const char* getHeader2HttpPost();
  virtual void setHeader2HttpPost(String header);
  virtual bool hasTargetHttpPost();

  virtual const char* getTargetHttpPost2() = 0;
  virtual void setTargetHttpPost2(String target);
  virtual const char* getHeader1HttpPost2();
  virtual void setHeader1HttpPost2(String header);
  virtual const char* getHeader2HttpPost2();
  virtual void setHeader2HttpPost2(String header);
  virtual bool hasTargetHttpPost2();

  virtual const char* getTargetHttpGet();
  virtual void setTargetHttpGet(String target);
  virtual const char* getHeader1HttpGet();
  virtual void setHeader1HttpGet(String header);
  virtual const char* getHeader2HttpGet();
  virtual void setHeader2HttpGet(String header);
  virtual bool hasTargetHttpGet();

  virtual const char* getTargetInfluxDB2();
  virtual void setTargetInfluxDB2(String target);
  virtual const char* getOrgInfluxDB2();
  virtual void setOrgInfluxDB2(String org);
  virtual const char* getBucketInfluxDB2();
  virtual void setBucketInfluxDB2(String bucket);
  virtual const char* getTokenInfluxDB2();
  virtual void setTokenInfluxDB2(String token);
  virtual bool hasTargetInfluxDb2();

  virtual const char* getTargetMqtt();
  virtual void setTargetMqtt(String target);
  virtual int getPortMqtt();
  virtual void setPortMqtt(int port);
  virtual const char* getUserMqtt();
  virtual void setUserMqtt(String user);
  virtual const char* getPassMqtt();
  virtual void setPassMqtt(String pass);
  virtual bool hasTargetMqtt();
};

#endif  // SRC_INTERFACE_HPP_

// EOF
