/*
MIT License

Copyright (c) 2021-2023 Magnus

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
#ifndef SRC_PUSHTARGET_HPP_
#define SRC_PUSHTARGET_HPP_

#include <templating.hpp>

#if defined(ESP8266)
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#else  // defined (ESP32)
#include <HTTPClient.h>
#endif

class PushTarget {
 private:
  WiFiClient _wifi;
  WiFiClientSecure _wifiSecure;
  HTTPClient _http;
  HTTPClient _httpSecure;
  int _lastCode;
  bool _lastSuccess;

  void sendHttpPost(TemplatingEngine& engine, bool isSecure, int index);
  void sendHttpGet(TemplatingEngine& engine, bool isSecure);
  void addHttpHeader(HTTPClient& http, String header);
  void probeMaxFragement(String& serverPath);

 public:
  void sendAll(float angle, float gravitySG, float corrGravitySG, float tempC,
               float runTime);

  void sendHttp1(TemplatingEngine& engine, bool isSecure) {
    sendHttpPost(engine, isSecure, 0);
  }
  void sendHttp2(TemplatingEngine& engine, bool isSecure) {
    sendHttpPost(engine, isSecure, 1);
  }
  void sendHttp3(TemplatingEngine& engine, bool isSecure) {
    sendHttpGet(engine, isSecure);
  }
  void sendInfluxDb2(TemplatingEngine& engine, bool isSecure);
  void sendMqtt(TemplatingEngine& engine, bool isSecure,
                bool skipHomeAssistantRegistration = true);
  int getLastCode() { return _lastCode; }
  bool getLastSuccess() { return _lastSuccess; }
};

class PushIntervalTracker {
 private:
  int _counters[5] = {0, 0, 0, 0, 0};
  void update(const int index, const int defaultValue);

 public:
  bool useHttp1() { return _counters[0] == 0 ? true : false; }
  bool useHttp2() { return _counters[1] == 0 ? true : false; }
  bool useHttp3() { return _counters[2] == 0 ? true : false; }
  bool useInflux() { return _counters[3] == 0 ? true : false; }
  bool useMqtt() { return _counters[4] == 0 ? true : false; }
  void load();
  void save();
};

#endif  // SRC_PUSHTARGET_HPP_

// EOF
