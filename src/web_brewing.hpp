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
#ifndef SRC_WEB_BREWING_HPP_
#define SRC_WEB_BREWING_HPP_

#include <basewebserver.hpp>
#include <battery.hpp>
#include <pushtarget.hpp>
#include <templating.hpp>

constexpr auto PARAM_HARDWARE = "hardware";
constexpr auto PARAM_SLEEP_MODE = "sleep_mode";
constexpr auto PARAM_BATTERY = "battery";
constexpr auto PARAM_APP_VER = "app_ver";
constexpr auto PARAM_APP_BUILD = "app_build";
constexpr auto PARAM_PLATFORM = "platform";
constexpr auto PARAM_BOARD = "board";
constexpr auto PARAM_SELF = "self_check";
constexpr auto PARAM_SELF_BATTERY_LEVEL = "battery_level";
constexpr auto PARAM_SELF_PUSH_TARGET = "push_targets";
constexpr auto PARAM_TOTAL_HEAP = "total_heap";
constexpr auto PARAM_FREE_HEAP = "free_heap";
constexpr auto PARAM_IP = "ip";
constexpr auto PARAM_WIFI_SETUP = "wifi_setup";
constexpr auto PARAM_I2C = "i2c";
constexpr auto PARAM_ADRESS = "adress";
constexpr auto PARAM_BUS = "bus";
constexpr auto PARAM_RESOLUTION = "resolution";
constexpr auto PARAM_FAMILY = "family";
constexpr auto PARAM_CHIP = "chip";
constexpr auto PARAM_REVISION = "revision";
constexpr auto PARAM_CORES = "cores";
constexpr auto PARAM_FEATURES = "features";

constexpr auto PARAM_FORMAT_POST_GRAVITY = "http_post_format_gravity";
constexpr auto PARAM_FORMAT_POST2_GRAVITY = "http_post2_format_gravity";
constexpr auto PARAM_FORMAT_GET_GRAVITY = "http_get_format_gravity";
constexpr auto PARAM_FORMAT_INFLUXDB_GRAVITY = "influxdb2_format_gravity";
constexpr auto PARAM_FORMAT_MQTT_GRAVITY = "mqtt_format_gravity";
constexpr auto PARAM_FORMAT_POST_PRESSURE = "http_post_format_pressure";
constexpr auto PARAM_FORMAT_POST2_PRESSURE = "http_post2_format_pressure";
constexpr auto PARAM_FORMAT_GET_PRESSURE = "http_get_format_pressure";
constexpr auto PARAM_FORMAT_INFLUXDB_PRESSURE = "influxdb2_format_pressure";
constexpr auto PARAM_FORMAT_MQTT_PRESSURE = "mqtt_format_pressure";

constexpr auto PARAM_PUSH_FORMAT = "push_format";
constexpr auto PARAM_PUSH_RETURN_CODE = "push_return_code";
constexpr auto PARAM_PUSH_ENABLED = "push_enabled";

class BrewingWebServer : public BaseWebServer {
 protected:
  BrewingConfig *_brewingConfig = nullptr;

  volatile bool _sensorCalibrationTask = false;
  volatile bool _pushTestTask = false;
  volatile bool _hardwareScanTask = false;

  String _hardwareScanData;

  String _pushTestTarget;
  int _pushTestLastCode;
  bool _pushTestLastSuccess, _pushTestEnabled;

  void webHandleStatus(AsyncWebServerRequest *request);
  void webHandleFeature(AsyncWebServerRequest *request);
  void webHandleConfigRead(AsyncWebServerRequest *request);
  void webHandleConfigWrite(AsyncWebServerRequest *request, JsonVariant &json);
  void webHandleConfigFormatRead(AsyncWebServerRequest *request);
  void webHandleConfigFormatWrite(AsyncWebServerRequest *request,
                                  JsonVariant &json);
  void webHandleSleepmode(AsyncWebServerRequest *request, JsonVariant &json);
  void webHandleTestPush(AsyncWebServerRequest *request, JsonVariant &json);
  void webHandleTestPushStatus(AsyncWebServerRequest *request);
  void webHandleCalibrate(AsyncWebServerRequest *request);
  void webHandleCalibrateStatus(AsyncWebServerRequest *request);
  void webHandleFactoryDefaults(AsyncWebServerRequest *request);
  void webHandleHardwareScan(AsyncWebServerRequest *request);
  void webHandleHardwareScanStatus(AsyncWebServerRequest *request);

  String readFile(String fname);
  bool writeFile(String fname, String data);

  virtual void doTaskSensorCalibration() = 0;
  virtual void doTaskPushTestSetup(TemplatingEngine &engine,
                                   BrewingPush &push) = 0;
  virtual void doTaskHardwareScanning(JsonObject &obj) = 0;

  virtual void doWebStatus(JsonObject &obj) = 0;
  virtual void doWebFeature(JsonObject &obj) = 0;
  virtual void doWebConfigWrite() = 0;
  virtual void doWebCalibrateStatus(JsonObject &obj) = 0;

 public:
  explicit BrewingWebServer(BrewingConfig *brewingConfig);

  virtual bool setupWebServer(const char *serviceName);
  virtual void loop();
};

#endif  // SRC_WEB_BREWING_HPP_

// EOF
