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
#ifndef SRC_PUSHTARGET_HPP_
#define SRC_PUSHTARGET_HPP_

#include <basepush.hpp>
#include <config.hpp>
#include <templating.hpp>

extern const char iPressureHttpPostFormat[] PROGMEM;
extern const char iPressureHttpGetFormat[] PROGMEM;
extern const char iPressureInfluxDbFormat[] PROGMEM;
extern const char iPressureMqttFormat[] PROGMEM;
extern const char iGravityHttpPostFormat[] PROGMEM;
extern const char iGravityHttpGetFormat[] PROGMEM;
extern const char iGravityInfluxDbFormat[] PROGMEM;
extern const char iGravityMqttFormat[] PROGMEM;

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
constexpr auto TPL_APP_VER = "${app-ver}";
constexpr auto TPL_ANGLE = "${angle}";
constexpr auto TPL_TILT = "${tilt}";  // same as angle
constexpr auto TPL_GRAVITY = "${gravity}";
constexpr auto TPL_GRAVITY_G = "${gravity-sg}";
constexpr auto TPL_GRAVITY_P = "${gravity-plato}";
constexpr auto TPL_GRAVITY_CORR = "${corr-gravity}";
constexpr auto TPL_GRAVITY_CORR_G = "${corr-gravity-sg}";
constexpr auto TPL_GRAVITY_CORR_P = "${corr-gravity-plato}";
constexpr auto TPL_GRAVITY_UNIT = "${gravity-unit}";  // G or P
constexpr auto TPL_PRESSURE = "${pressure}";
constexpr auto TPL_PRESSURE1 = "${pressure1}";
constexpr auto TPL_PRESSURE_PSI = "${pressure-psi}";
constexpr auto TPL_PRESSURE1_PSI = "${pressure1-psi}";
constexpr auto TPL_PRESSURE_BAR = "${pressure-bar}";
constexpr auto TPL_PRESSURE1_BAR = "${pressure1-bar}";
constexpr auto TPL_PRESSURE_KPA = "${pressure-kpa}";
constexpr auto TPL_PRESSURE1_KPA = "${pressure1-kpa}";
constexpr auto TPL_PRESSURE_UNIT = "${pressure-unit}";  // PSI, BAR, KPA
constexpr auto TPL_APP_BUILD = "${app-build}";

constexpr auto TPL_GRAVITY_FNAME_POST = "/http-1.tpl";
constexpr auto TPL_GRAVITY_FNAME_POST2 = "/http-2.tpl";
constexpr auto TPL_GRAVITY_FNAME_GET = "/http-3.tpl";
constexpr auto TPL_GRAVITY_FNAME_INFLUXDB = "/influxdb.tpl";
constexpr auto TPL_GRAVITY_FNAME_MQTT = "/mqtt.tpl";

constexpr auto TPL_PRESSURE_FNAME_POST = "/http-p1.tpl";
constexpr auto TPL_PRESSURE_FNAME_POST2 = "/http-p2.tpl";
constexpr auto TPL_PRESSURE_FNAME_GET = "/http-p3.tpl";
constexpr auto TPL_PRESSURE_FNAME_INFLUXDB = "/influxdb-p.tpl";
constexpr auto TPL_PRESSURE_FNAME_MQTT = "/mqtt-p.tpl";

extern const char iGravityHttpPostFormat[] PROGMEM;
extern const char iGravityHttpGetFormat[] PROGMEM;
extern const char iGravityInfluxDbFormat[] PROGMEM;
extern const char iGravityMqttFormat[] PROGMEM;

extern const char iPressureHttpPostFormat[] PROGMEM;
extern const char iPressureHttpGetFormat[] PROGMEM;
extern const char iPressureInfluxDbFormat[] PROGMEM;
extern const char iPressureMqttFormat[] PROGMEM;

class BrewingPush : public BasePush {
 private:
  BrewingConfig* _BrewingConfig;
  String _baseTemplate;

 public:
  explicit BrewingPush(BrewingConfig* BrewingConfig);

  enum Templates {
    GRAVITY_TEMPLATE_HTTP1 = 0,
    GRAVITY_TEMPLATE_HTTP2 = 1,
    GRAVITY_TEMPLATE_HTTP3 = 2,
    GRAVITY_TEMPLATE_INFLUX = 3,
    GRAVITY_TEMPLATE_MQTT = 4,

    PRESSURE_TEMPLATE_HTTP1 = 10,
    PRESSURE_TEMPLATE_HTTP2 = 11,
    PRESSURE_TEMPLATE_HTTP3 = 12,
    PRESSURE_TEMPLATE_INFLUX = 13,
    PRESSURE_TEMPLATE_MQTT = 14,
  };

  enum MeasurementType {
    GRAVITY = 0,
    PRESSURE = 1,
  };

  void sendAll(TemplatingEngine& engine, MeasurementType type);
  const char* getTemplate(Templates t, bool useDefaultTemplate = false);
  void clearTemplate() { _baseTemplate.clear(); }
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
