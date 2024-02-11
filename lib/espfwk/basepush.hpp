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
#ifndef SRC_BASEPUSH_HPP_
#define SRC_BASEPUSH_HPP_

#if defined(ESP8266)
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#else
#include <HTTPClient.h>
#endif
#include <interface.hpp>

class BasePush {
 protected:
  WiFiClient _wifi;
  WiFiClientSecure _wifiSecure;
  HTTPClient _http;
  HTTPClient _httpSecure;
  int _lastResponseCode = 0;
  bool _lastSuccess = false;
  PushConfig* _config;

  void probeMFLN(String serverPath);
  void addHttpHeader(HTTPClient& http, String header);
  bool isSecure(String target) { return target.startsWith("https://"); }

 public:
  explicit BasePush(PushConfig* config) { _config = config; }

  int getLastResponseCode() { return _lastResponseCode; }
  bool wasLastSuccessful() { return _lastSuccess; }

  String sendHttpPost(String& payload, const char* target, const char* header1,
                      const char* header2);
  String sendHttpPost2(String& payload, const char* target, const char* header1,
                       const char* header2);
  String sendHttpGet(String& payload, const char* target, const char* header1,
                     const char* header2);
  void sendInfluxDb2(String& payload, const char* target, const char* org,
                     const char* bucket, const char* token);
  void sendMqtt(String& payload, const char* target, int port, const char* user,
                const char* pass);

  bool sendHttpPost(String& payload);
  bool sendHttpPost2(String& payload);
  bool sendHttpGet(String& payload);
  bool sendInfluxDb2(String& payload);
  bool sendMqtt(String& payload);
};

#endif  // SRC_BASEPUSH_HPP_

// EOF
