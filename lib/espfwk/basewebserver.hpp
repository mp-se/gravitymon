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
#ifndef SRC_BASEWEBSERVER_HPP_
#define SRC_BASEWEBSERVER_HPP_

#if defined(ESP32)
#include <freertos/FreeRTOS.h>
#endif

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#else
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#endif

#include <ArduinoJson.h>
#include <AsyncJson.h>

#include <espframework.hpp>
#include <interface.hpp>

#if defined(ESP8266)
#include <incbin.h>
INCBIN_EXTERN(IndexHtml);
INCBIN_EXTERN(AppJs);
INCBIN_EXTERN(AppCss);
#else
extern const uint8_t indexHtmlStart[] asm("_binary_html_index_html_start");
extern const uint8_t indexHtmlEnd[] asm("_binary_html_index_html_end");
extern const uint8_t appJsStart[] asm("_binary_html_app_js_gz_start");
extern const uint8_t appJsEnd[] asm("_binary_html_app_js_gz_end");
extern const uint8_t appCssStart[] asm("_binary_html_app_css_gz_start");
extern const uint8_t appCssEnd[] asm("_binary_html_app_css_gz_end");
#endif

class BaseWebServer {
 protected:
  AsyncWebServer *_server = 0;
  File _uploadFile;
  WebConfig *_webConfig;
  int _uploadReturn = 200;
  int _dynamicJsonSize = 2000;
  bool _wifiSetup = false;
  uint32_t _uploadedSize = 0;
  uint32_t _rebootTimer = 0;
  uint32_t _wifiPortalTimer = 0;
  volatile bool _wifiScanTask = false;
  String _wifiScanData;
  volatile bool _rebootTask = false;

  void resetWifiPortalTimer() { _wifiPortalTimer = millis(); }
  bool isAuthenticated(AsyncWebServerRequest *request);

  void webReturnOK(AsyncWebServerRequest *request) {
    request->send(_uploadReturn);
  }
#if defined(ESP8266)
  void webReturnIndexHtml(AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", (const uint8_t *)gIndexHtmlData,
                    gIndexHtmlSize);
  }
  void webReturnAppJs(AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", (const uint8_t *)gAppJsData, gAppJsSize);
  }
  void webReturnAppCss(AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", (const uint8_t *)gAppCssData,
                    gAppCssSize);
  }
#else
  void webReturnIndexHtml(AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", (const uint8_t *)indexHtmlStart,
                    reinterpret_cast<uint32_t>(&indexHtmlEnd[0]) -
                        reinterpret_cast<uint32_t>(&indexHtmlStart[0]));
  }
  void webReturnAppJs(AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse_P(
        200, "application/javascript", (const uint8_t *)appJsStart,
        reinterpret_cast<uint32_t>(&appJsEnd[0]) -
            reinterpret_cast<uint32_t>(&appJsStart[0]));
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  }
  void webReturnAppCss(AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse_P(
        200, "text/css", (const uint8_t *)appCssStart,
        reinterpret_cast<uint32_t>(&appCssEnd[0]) -
            reinterpret_cast<uint32_t>(&appCssStart[0]));
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  }
#endif

  void webHandleAuth(AsyncWebServerRequest *request);
  void webHandleWifiScan(AsyncWebServerRequest *request);
  void webHandleWifiScanStatus(AsyncWebServerRequest *request);
  void webHandleWifiClear(AsyncWebServerRequest *request);
  void webHandlePageNotFound(AsyncWebServerRequest *request);
  void webHandleUploadFirmware(AsyncWebServerRequest *request, String filename,
                           size_t index, uint8_t *data, size_t len, bool final);
  void webHandleUploadFile(AsyncWebServerRequest *request, String filename,
                           size_t index, uint8_t *data, size_t len, bool final);
  void webHandleRestart(AsyncWebServerRequest *request);
  void webHandleFileSystem(AsyncWebServerRequest *request, JsonVariant &json);

  virtual void setupWebHandlers();

 public:
  explicit BaseWebServer(WebConfig *config,
                         int dynamicJsonSize = JSON_BUFFER_SIZE_L);

  virtual bool setupWebServer();
  virtual AsyncWebServer *getWebServer() { return _server; }
  virtual void setWifiSetup(bool wifiSetup) { _wifiSetup = wifiSetup; }
  virtual void loop();
};

#endif  // SRC_BASEWEBSERVER_HPP_

// EOF
