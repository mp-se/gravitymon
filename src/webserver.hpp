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
#ifndef SRC_WEBSERVER_HPP_
#define SRC_WEBSERVER_HPP_

#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <incbin.h>

#if defined(EMBED_HTML)
INCBIN_EXTERN(IndexHtm);
INCBIN_EXTERN(DeviceHtm);
INCBIN_EXTERN(ConfigHtm);
INCBIN_EXTERN(CalibrationHtm);
INCBIN_EXTERN(AboutHtm);
#else
INCBIN_EXTERN(UploadHtm);
#endif

class WebServer {
 private:
  ESP8266WebServer* _server = 0;
  File _uploadFile;
  int _lastFormulaCreateError = 0;

  void webHandleConfig();
  void webHandleFormulaWrite();
  void webHandleFormulaRead();
  void webHandleConfigHardware();
  void webHandleConfigGravity();
  void webHandleConfigPush();
  void webHandleConfigDevice();
  void webHandleStatusSleepmode();
  void webHandleClearWIFI();
  void webHandleStatus();
  void webHandleFactoryReset();
  void webHandleCalibrate();
  void webHandleUploadFile();
  void webHandleUpload();
  void webHandleDevice();
  void webHandlePageNotFound();

  String getRequestArguments();

  // Inline functions.
  void webReturnOK() { _server->send(200); }
#if defined(EMBED_HTML)
  void webReturnIndexHtm() {
    _server->send_P(200, "text/html", (const char*)gIndexHtmData,
                    gIndexHtmSize);
  }
  void webReturnDeviceHtm() {
    _server->send_P(200, "text/html", (const char*)gDeviceHtmData,
                    gDeviceHtmSize);
  }
  void webReturnConfigHtm() {
    _server->send_P(200, "text/html", (const char*)gConfigHtmData,
                    gConfigHtmSize);
  }
  void webReturnCalibrationHtm() {
    _server->send_P(200, "text/html", (const char*)gCalibrationHtmData,
                    gCalibrationHtmSize);
  }
  void webReturnAboutHtm() {
    _server->send_P(200, "text/html", (const char*)gAboutHtmData,
                    gAboutHtmSize);
  }
#else
  void webReturnUploadHtm() {
    _server->send_P(200, "text/html", (const char*)gUploadHtmData,
                    gUploadHtmSize);
  }
#endif

 public:
  enum HtmlFile {
    HTML_INDEX = 0,
    HTML_DEVICE = 1,
    HTML_CONFIG = 2,
    HTML_ABOUT = 3,
    HTML_CALIBRATION = 4
  };

  bool setupWebServer();
  void loop();
  bool checkHtmlFile(HtmlFile item);
  const char* getHtmlFileName(HtmlFile item);
};

// Global instance created
extern WebServer myWebServer;

#endif  // SRC_WEBSERVER_HPP_

// EOF
