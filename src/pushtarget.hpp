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

#include <basepush.hpp>
#include <templating.hpp>

constexpr auto TPL_MDNS = "${mdns}";
constexpr auto TPL_ID = "${id}";
constexpr auto TPL_TOKEN = "${token}";
constexpr auto TPL_TOKEN2 = "${token2}";
constexpr auto TPL_SLEEP_INTERVAL = "${sleep-interval}";
constexpr auto TPL_TEMP = "${temp}";
constexpr auto TPL_TEMP_C = "${temp-c}";
constexpr auto TPL_TEMP_F = "${temp-f}";
constexpr auto TPL_TEMP_UNITS = "${temp-unit}";  // C or F
constexpr auto TPL_BATTERY = "${battery}";
constexpr auto TPL_BATTERY_PERCENT = "${battery-percent}";
constexpr auto TPL_RSSI = "${rssi}";
constexpr auto TPL_RUN_TIME = "${run-time}";
constexpr auto TPL_ANGLE = "${angle}";
constexpr auto TPL_TILT = "${tilt}";  // same as angle
constexpr auto TPL_GRAVITY = "${gravity}";
constexpr auto TPL_GRAVITY_G = "${gravity-sg}";
constexpr auto TPL_GRAVITY_P = "${gravity-plato}";
constexpr auto TPL_GRAVITY_CORR = "${corr-gravity}";
constexpr auto TPL_GRAVITY_CORR_G = "${corr-gravity-sg}";
constexpr auto TPL_GRAVITY_CORR_P = "${corr-gravity-plato}";
constexpr auto TPL_GRAVITY_UNIT = "${gravity-unit}";  // G or P
constexpr auto TPL_APP_VER = "${app-ver}";
constexpr auto TPL_APP_BUILD = "${app-build}";

constexpr auto TPL_FNAME_POST = "/http-1.tpl";
constexpr auto TPL_FNAME_POST2 = "/http-2.tpl";
constexpr auto TPL_FNAME_GET = "/http-3.tpl";
constexpr auto TPL_FNAME_INFLUXDB = "/influxdb.tpl";
constexpr auto TPL_FNAME_MQTT = "/mqtt.tpl";

extern const char iSpindleFormat[] PROGMEM;
extern const char iHttpGetFormat[] PROGMEM;
extern const char influxDbFormat[] PROGMEM;
extern const char mqttFormat[] PROGMEM;

class GravmonPush : public BasePush {
 private:
  GravmonConfig* _gravmonConfig;
  String _baseTemplate;

 public:
  explicit GravmonPush(GravmonConfig* gravmonConfig);

  enum Templates {
    TEMPLATE_HTTP1 = 0,
    TEMPLATE_HTTP2 = 1,
    TEMPLATE_HTTP3 = 2,
    TEMPLATE_INFLUX = 3,
    TEMPLATE_MQTT = 4,
    TEMPLATE_BLE = 5
  };

  void sendAll(float angle, float gravitySG, float corrGravitySG, float tempC,
               float runTime);

  const char* getTemplate(Templates t, bool useDefaultTemplate = false);
  void clearTemplate() { _baseTemplate.clear(); }
  void setupTemplateEngine(TemplatingEngine& engine, float angle,
                           float gravitySG, float corrGravitySG, float tempC,
                           float runTime, float voltage);
  int getLastCode() { return _lastResponseCode; }
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
