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
#ifndef SRC_WEBSERVER_HPP_
#define SRC_WEBSERVER_HPP_

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#define MAX_SKETCH_SPACE 1044464
#else
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
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
INCBIN_EXTERN(BackupHtm);
#else  // ESP32
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
extern const uint8_t backupHtmStart[] asm("_binary_html_backup_min_htm_start");
extern const uint8_t backupHtmEnd[] asm("_binary_html_backup_min_htm_end");
#endif

class WebServerHandler {
 private:
  AsyncWebServer *_server = 0;
  File _uploadFile;
  int _uploadedSize = 0;
  int _lastFormulaCreateError = 0;
  int _uploadReturn = 200;
  volatile bool _rebootTask = false;
  volatile bool _sensorCalibrationTask = false;
  volatile bool _pushTestTask = false;
  String _pushTestData;

  void webHandleConfig(AsyncWebServerRequest *request);
  void webHandleFormulaWrite(AsyncWebServerRequest *request);
  void webHandleFormulaRead(AsyncWebServerRequest *request);
  void webHandleConfigAdvancedRead(AsyncWebServerRequest *request);
  void webHandleConfigAdvancedWrite(AsyncWebServerRequest *request);
  void webHandleConfigHardware(AsyncWebServerRequest *request);
  void webHandleConfigGravity(AsyncWebServerRequest *request);
  void webHandleConfigPush(AsyncWebServerRequest *request);
  void webHandleConfigDevice(AsyncWebServerRequest *request);
  void webHandleConfigFormatRead(AsyncWebServerRequest *request);
  void webHandleConfigFormatWrite(AsyncWebServerRequest *request);
  void webHandleConfigWifi(AsyncWebServerRequest *request);
  void webHandleTestPush(AsyncWebServerRequest *request);
  void webHandleTestPushStatus(AsyncWebServerRequest *request);
  void webHandleStatusSleepmode(AsyncWebServerRequest *request);
  void webHandleClearWIFI(AsyncWebServerRequest *request);
  void webHandleStatus(AsyncWebServerRequest *request);
  void webHandleFactoryDefaults(AsyncWebServerRequest *request);
  void webHandleCalibrate(AsyncWebServerRequest *request);
  void webHandleCalibrateStatus(AsyncWebServerRequest *request);
  void webHandleUploadFile(AsyncWebServerRequest *request, String filename,
                           size_t index, uint8_t *data, size_t len, bool final);
  void webHandleLogClear(AsyncWebServerRequest *request);
  void webHandlePageNotFound(AsyncWebServerRequest *request);
  void webHandleRestart(AsyncWebServerRequest *request);
  void webHandleMigrate(AsyncWebServerRequest *request);
  void webHandleMigrateUndo(AsyncWebServerRequest *request);

  String readFile(String fname);
  bool writeFile(String fname, String data);

  String getRequestArguments(AsyncWebServerRequest *request);
  void webReturnOK(AsyncWebServerRequest *request);

#if defined(ESP8266)
  void webReturnIndexHtm(AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", (const uint8_t *)gIndexHtmData,
                    gIndexHtmSize);
  }
  void webReturnConfigHtm(AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", (const uint8_t *)gConfigHtmData,
                    gConfigHtmSize);
  }
  void webReturnCalibrationHtm(AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", (const uint8_t *)gCalibrationHtmData,
                    gCalibrationHtmSize);
  }
  void webReturnFormatHtm(AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", (const uint8_t *)gFormatHtmData,
                    gFormatHtmSize);
  }
  void webReturnAboutHtm(AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", (const uint8_t *)gAboutHtmData,
                    gAboutHtmSize);
  }
  void webReturnTestHtm(AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", (const uint8_t *)gTestHtmData,
                    gTestHtmSize);
  }
  void webReturnFirmwareHtm(AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", (const uint8_t *)gFirmwareHtmData,
                    gFirmwareHtmSize);
  }
  void webReturnBackupHtm(AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", (const uint8_t *)gBackupHtmData,
                    gBackupHtmSize);
  }
#else  // ESP32
  void webReturnIndexHtm(AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", (const uint8_t *)indexHtmStart,
                    strlen(reinterpret_cast<const char *>(&indexHtmStart[0])));
  }
  void webReturnConfigHtm(AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", (const uint8_t *)configHtmStart,
                    strlen(reinterpret_cast<const char *>(&configHtmStart[0])));
  }
  void webReturnCalibrationHtm(AsyncWebServerRequest *request) {
    request->send_P(
        200, "text/html", (const uint8_t *)calibrationHtmStart,
        strlen(reinterpret_cast<const char *>(&calibrationHtmStart[0])));
  }
  void webReturnFormatHtm(AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", (const uint8_t *)formatHtmStart,
                    strlen(reinterpret_cast<const char *>(&formatHtmStart[0])));
  }
  void webReturnAboutHtm(AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", (const uint8_t *)aboutHtmStart,
                    strlen(reinterpret_cast<const char *>(&aboutHtmStart[0])));
  }
  void webReturnTestHtm(AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", (const uint8_t *)testHtmStart,
                    strlen(reinterpret_cast<const char *>(&testHtmStart[0])));
  }
  void webReturnFirmwareHtm(AsyncWebServerRequest *request) {
    request->send_P(
        200, "text/html", (const uint8_t *)firmwareHtmStart,
        strlen(reinterpret_cast<const char *>(&firmwareHtmStart[0])));
  }
  void webReturnBackupHtm(AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", (const uint8_t *)backupHtmStart,
                    strlen(reinterpret_cast<const char *>(&backupHtmStart[0])));
  }
#endif

 public:
  bool setupWebServer();
  void loop();

  AsyncWebServer *getWebServer() { return _server; }
};

// Global instance created
extern WebServerHandler myWebServerHandler;

#endif  // SRC_WEBSERVER_HPP_

// EOF
