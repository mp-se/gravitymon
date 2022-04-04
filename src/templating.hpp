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
#ifndef SRC_TEMPLATING_HPP_
#define SRC_TEMPLATING_HPP_

// Includes
#include <Arduino.h>

#include <algorithm>
#include <helper.hpp>
#include <main.hpp>

// Templating variables
#define TPL_MDNS "${mdns}"
#define TPL_ID "${id}"
#define TPL_TOKEN "${token}"
#define TPL_TOKEN2 "${token2}"
#define TPL_SLEEP_INTERVAL "${sleep-interval}"
#define TPL_TEMP "${temp}"
#define TPL_TEMP_C "${temp-c}"
#define TPL_TEMP_F "${temp-f}"
#define TPL_TEMP_UNITS "${temp-unit}"  // C or F
#define TPL_BATTERY "${battery}"
#define TPL_RSSI "${rssi}"
#define TPL_RUN_TIME "${run-time}"
#define TPL_ANGLE "${angle}"
#define TPL_TILT "${tilt}"  // same as angle
#define TPL_GRAVITY "${gravity}"
#define TPL_GRAVITY_G "${gravity-sg}"
#define TPL_GRAVITY_P "${gravity-plato}"
#define TPL_GRAVITY_CORR "${corr-gravity}"
#define TPL_GRAVITY_CORR_G "${corr-gravity-sg}"
#define TPL_GRAVITY_CORR_P "${corr-gravity-plato}"
#define TPL_GRAVITY_UNIT "${gravity-unit}"  // G or P

#define TPL_FNAME_HTTP1 "/http-1.tpl"
#define TPL_FNAME_HTTP2 "/http-2.tpl"
#define TPL_FNAME_HTTP3 "/http-3.tpl"
// #define TPL_FNAME_BREWFATHER "/brewfather.tpl"
#define TPL_FNAME_INFLUXDB "/influxdb.tpl"
#define TPL_FNAME_MQTT "/mqtt.tpl"

extern const char iSpindleFormat[] PROGMEM;
extern const char iHttpGetFormat[] PROGMEM;
extern const char brewfatherFormat[] PROGMEM;
extern const char influxDbFormat[] PROGMEM;
extern const char mqttFormat[] PROGMEM;

// Classes
class TemplatingEngine {
 private:
  struct KeyVal {
    String key;
    String val;
  };

  KeyVal items[21] = {{TPL_MDNS, ""},           {TPL_ID, ""},
                      {TPL_SLEEP_INTERVAL, ""}, {TPL_TEMP, ""},
                      {TPL_TEMP_C, ""},         {TPL_TEMP_F, ""},
                      {TPL_TEMP_UNITS, ""},     {TPL_BATTERY, ""},
                      {TPL_RSSI, ""},           {TPL_RUN_TIME, ""},
                      {TPL_ANGLE, ""},          {TPL_TILT, ""},
                      {TPL_GRAVITY, ""},        {TPL_GRAVITY_G, ""},
                      {TPL_GRAVITY_P, ""},      {TPL_GRAVITY_CORR, ""},
                      {TPL_GRAVITY_CORR_G, ""}, {TPL_GRAVITY_CORR_P, ""},
                      {TPL_GRAVITY_UNIT, ""},   {TPL_TOKEN, ""},
                      {TPL_TOKEN2, ""}};

  char buffer[20];
  String baseTemplate;

  void setVal(String key, float val, int dec = 2) {
    String s = convertFloatToString(val, &buffer[0], dec);
    s.trim();
    setVal(key, s);
  }
  void setVal(String key, int val) { setVal(key, String(val)); }
  void setVal(String key, char val) { setVal(key, String(val)); }
  void setVal(String key, String val) {
    int max = sizeof(items) / sizeof(KeyVal);
    for (int i = 0; i < max; i++) {
      if (items[i].key.equals(key)) {
        items[i].val = val;
        return;
      }
    }

    Log.warning(F("TPL : Key not found %s." CR), key.c_str());
  }

  void transform(String& s) {
    int max = sizeof(items) / sizeof(KeyVal);
    for (int i = 0; i < max; i++) {
      while (s.indexOf(items[i].key) != -1)
        s.replace(items[i].key, items[i].val);
    }
  }

  void dumpAll() {
    int max = sizeof(items) / sizeof(KeyVal);
    for (int i = 0; i < max; i++) {
      Serial.print("Key=\'");
      Serial.print(items[i].key.c_str());
      Serial.print("\', Val=\'");
      Serial.print(items[i].val.c_str());
      Serial.println("\'");
    }
  }

 public:
  enum Templates {
    TEMPLATE_HTTP1 = 0,
    TEMPLATE_HTTP2 = 1,
    TEMPLATE_HTTP3 = 2,
    TEMPLATE_BREWFATHER = 3,
    TEMPLATE_INFLUX = 4,
    TEMPLATE_MQTT = 5
  };

  void initialize(float angle, float gravitySG, float corrGravitySG,
                  float tempC, float runTime);
  const String& create(TemplatingEngine::Templates idx);
};

#endif  // SRC_TEMPLATING_HPP_

// EOF
