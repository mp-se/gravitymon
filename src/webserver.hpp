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

#if defined(ESP8266)
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#define MAX_SKETCH_SPACE 1044464
#else
#include <ESPmDNS.h>
#include <Update.h>
#include <WebServer.h>
#include <WiFi.h>
#define MAX_SKETCH_SPACE 1835008
#endif

#if defined(ESP8266)
#include <incbin.h>
INCBIN_EXTERN(IndexHtm);
INCBIN_EXTERN(ConfigHtm);
INCBIN_EXTERN(CalibrationHtm);
INCBIN_EXTERN(FormatHtm);
INCBIN_EXTERN(TestHtm);
INCBIN_EXTERN(AboutHtm);
INCBIN_EXTERN(FirmwareHtm);
#else // ESP32
extern const uint8_t indexHtmStart[] asm("_binary_html_index_min_htm_start");
extern const uint8_t indexHtmEnd[] asm("_binary_html_index_min_htm_end");
extern const uint8_t configHtmStart[] asm("_binary_html_config_min_htm_start");
extern const uint8_t configHtmEnd[] asm("_binary_html_config_min_htm_end");
extern const uint8_t calibrationHtmStart[] asm(
    "_binary_html_calibration_min_htm_start");
extern const uint8_t calibrationHtmEnd[] asm(
    "_binary_html_calibration_min_htm_end");
extern const uint8_t formatHtmStart[] asm("_binary_html_format_min_htm_start");
extern const uint8_t formatHtmEnd[] asm("_binary_html_format_min_htm_end");
extern const uint8_t testHtmStart[] asm("_binary_html_test_min_htm_start");
extern const uint8_t testHtmEnd[] asm("_binary_html_test_min_htm_end");
extern const uint8_t aboutHtmStart[] asm("_binary_html_about_min_htm_start");
extern const uint8_t aboutHtmEnd[] asm("_binary_html_about_min_htm_end");
extern const uint8_t firmwareHtmStart[] asm(
    "_binary_html_firmware_min_htm_start");
extern const uint8_t firmwareHtmEnd[] asm("_binary_html_firmware_min_htm_end");
#endif

class WebServerHandler {
 private:
  ESP8266WebServer* _server = 0;
  File _uploadFile;
  int _lastFormulaCreateError = 0;
  int _uploadReturn = 200;

  void webHandleConfig();
  void webHandleFormulaWrite();
  void webHandleFormulaRead();
  void webHandleConfigAdvancedRead();
  void webHandleConfigAdvancedWrite();
  void webHandleConfigHardware();
  void webHandleConfigGravity();
  void webHandleConfigPush();
  void webHandleConfigDevice();
  void webHandleConfigFormatRead();
  void webHandleConfigFormatWrite();
  void webHandleTestPush();
  void webHandleStatusSleepmode();
  void webHandleClearWIFI();
  void webHandleStatus();
  void webHandleFactoryDefaults();
  void webHandleCalibrate();
  void webHandleUploadFile();
  void webHandleLogClear();
  void webHandlePageNotFound();

  String readFile(String fname);
  bool writeFile(String fname, String data);

  String getRequestArguments();

  // Inline functions.
  void webReturnOK() { _server->send(_uploadReturn); }
#if defined(ESP8266)
  void webReturnIndexHtm() {
    _server->send_P(200, "text/html", (const char*)gIndexHtmData,
                    gIndexHtmSize);
  }
  void webReturnConfigHtm() {
    _server->send_P(200, "text/html", (const char*)gConfigHtmData,
                    gConfigHtmSize);
  }
  void webReturnCalibrationHtm() {
    _server->send_P(200, "text/html", (const char*)gCalibrationHtmData,
                    gCalibrationHtmSize);
  }
  void webReturnFormatHtm() {
    _server->send_P(200, "text/html", (const char*)gFormatHtmData,
                    gFormatHtmSize);
  }
  void webReturnAboutHtm() {
    _server->send_P(200, "text/html", (const char*)gAboutHtmData,
                    gAboutHtmSize);
  }
  void webReturnTestHtm() {
    _server->send_P(200, "text/html", (const char*)gTestHtmData, gTestHtmSize);
  }
  void webReturnFirmwareHtm() {
    _server->send_P(200, "text/html", (const char*)gFirmwareHtmData,
                    gFirmwareHtmSize);
  }
#else  // ESP32
  void webReturnIndexHtm() {
    _server->send_P(200, "text/html", (const char*)indexHtmStart,
                    strlen(reinterpret_cast<const char*>(&indexHtmStart[0])));
  }
  void webReturnConfigHtm() {
    _server->send_P(200, "text/html", (const char*)configHtmStart,
                    strlen(reinterpret_cast<const char*>(&configHtmStart[0])));
  }
  void webReturnCalibrationHtm() {
    _server->send_P(
        200, "text/html", (const char*)calibrationHtmStart,
        strlen(reinterpret_cast<const char*>(&calibrationHtmStart[0])));
  }
  void webReturnFormatHtm() {
    _server->send_P(200, "text/html", (const char*)formatHtmStart,
                    strlen(reinterpret_cast<const char*>(&formatHtmStart[0])));
  }
  void webReturnAboutHtm() {
    _server->send_P(200, "text/html", (const char*)aboutHtmStart,
                    strlen(reinterpret_cast<const char*>(&aboutHtmStart[0])));
  }
  void webReturnTestHtm() {
    _server->send_P(200, "text/html", (const char*)testHtmStart,
                    strlen(reinterpret_cast<const char*>(&testHtmStart[0])));
  }
  void webReturnFirmwareHtm() {
    _server->send_P(
        200, "text/html", (const char*)firmwareHtmStart,
        strlen(reinterpret_cast<const char*>(&firmwareHtmStart[0])));
  }
#endif

 public:
  bool setupWebServer();
  void loop();
};

// Global instance created
extern WebServerHandler myWebServerHandler;

#endif  // SRC_WEBSERVER_HPP_

// EOF
