/*
MIT License

Copyright (c) 2022-2024 Magnus

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
#include <AUnit.h>

#include <templating.hpp>
#include <pushtarget.hpp>
#include <config_gravitymon.hpp>
#include <push_gravitymon.hpp>
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif

extern GravitymonConfig myConfig;

#if defined(GRAVITYMON)
test(template_applyTemplate1) {
  TemplatingEngine e;
  BrewingPush p(&myConfig);
  myConfig.setMDNS("gravitymon");

  String t = p.getTemplate(BrewingPush::GRAVITY_TEMPLATE_HTTP1);
  setupTemplateEngineGravity(&myConfig, e, 45.0, 0, 1.123, 1.223, 21.2, 2.98, 3.88);
  String s = e.create(t.c_str());
  String id = myConfig.getID();
  String rssi = String(WiFi.RSSI());
  String v = "{\"name\": \"gravitymon\", \"ID\": \"" + id +
             "\", \"token\": \"\", \"interval\": 900, \"temperature\": 21.20, "
             "\"temp_units\": \"C\", \"gravity\": 1.1230, \"velocity\": 0.0, \"angle\": 45.000, "
             "\"battery\": 3.88"
             ", \"RSSI\": " + rssi + ", \"corr-gravity\": 1.2230, \"gravity-unit\": "
             "\"G\", \"run-time\": 2.98 }";
  assertEqual(s, v);
}

test(template_applyTemplate2) {
  TemplatingEngine e;
  BrewingPush p(&myConfig);
  myConfig.setMDNS("gravitymon");

  String t = p.getTemplate(BrewingPush::GRAVITY_TEMPLATE_HTTP2);
  setupTemplateEngineGravity(&myConfig, e, 45.0, 2.1, 1.123, 1.223, 21.2, 2.98, 3.88);
  String s = e.create(t.c_str());
  String id = myConfig.getID();
  String rssi = String(WiFi.RSSI());
  String v = "{\"name\": \"gravitymon\", \"ID\": \"" + id +
             "\", \"token\": \"\", \"interval\": 900, \"temperature\": 21.20, "
             "\"temp_units\": \"C\", \"gravity\": 1.1230, \"velocity\": 2.1, \"angle\": 45.000, "
             "\"battery\": 3.88"
             ", \"RSSI\": " + rssi + ", \"corr-gravity\": 1.2230, \"gravity-unit\": "
             "\"G\", \"run-time\": 2.98 }";
  assertEqual(s, v);
}

test(template_applyTemplate3) {
  TemplatingEngine e;
  BrewingPush p(&myConfig);
  myConfig.setMDNS("gravitymon");

  String t = p.getTemplate(BrewingPush::GRAVITY_TEMPLATE_HTTP3);
  setupTemplateEngineGravity(&myConfig, e, 45.0, 0.1, 1.123, 1.223, 21.2, 2.98, 3.88);
  String s = e.create(t.c_str());
  String id = myConfig.getID();
  String rssi = String(WiFi.RSSI());
  String v = "?name=gravitymon&id=" + id +
             "&token=&interval=900&temperature=21.20&temp-units=C&gravity=1."
             "1230&velocity=0.1&angle=45.000&battery=3.88"
             "&rssi=" + rssi + "&corr-gravity=1.2230&gravity-unit=G&run-time=2.98";
  assertEqual(s, v);
}

test(template_applyTemplate4) {
  TemplatingEngine e;
  BrewingPush p(&myConfig);
  myConfig.setMDNS("gravitymon");

  String t = p.getTemplate(BrewingPush::GRAVITY_TEMPLATE_INFLUX);
  setupTemplateEngineGravity(&myConfig, e, 45.0, 3.1, 1.123, 1.223, 21.2, 2.98, 3.88);
  String s = e.create(t.c_str());
  String id = myConfig.getID();
  String rssi = String(WiFi.RSSI());
  String v =
      "measurement,host=gravitymon,device=" + id +
      ",temp-format=C,gravity-format=G "
      "gravity=1.1230,velocity=3.1,corr-gravity=1.2230,angle=45.000,temp=21.20,battery=3.88"
      ",rssi=" + rssi + "\n";
  assertEqual(s, v);
}

test(template_applyTemplate5) {
  TemplatingEngine e;
  BrewingPush p(&myConfig);
  myConfig.setMDNS("gravitymon");

  String t = p.getTemplate(BrewingPush::GRAVITY_TEMPLATE_MQTT);
  setupTemplateEngineGravity(&myConfig, e, 45.0, 3.4, 1.123, 1.223, 21.2, 2.98, 3.88);
  String s = e.create(t.c_str());
  String rssi = String(WiFi.RSSI());
  String v =
      "ispindel/gravitymon/tilt:45.000|ispindel/gravitymon/"
      "temperature:21.20|ispindel/gravitymon/temp_units:C|ispindel/gravitymon/"
      "battery:3.88"
      "|ispindel/gravitymon/gravity:1.1230|ispindel/gravitymon/velocity:3.4|ispindel/gravitymon/"
      "interval:900|ispindel/gravitymon/RSSI:" + rssi + "|";
  assertEqual(s, v);
}

test(template_applyTemplate6) {
  TemplatingEngine e;
  BrewingPush p(&myConfig);
  myConfig.setMDNS("gravitymon");

  const char* tpl = 
    "<prtg><result><channel>Densite</channel><float>1</float><value>${gravity}</value></result>"
    "<result><channel>Batterie</channel><float>1</float><value>${battery}</value></result>"
    "<result><channel>Temperature</channel><float>1</float><value>${temp}</value></result></prtg>";

  setupTemplateEngineGravity(&myConfig, e, 45.0, 0, 1.123, 1.223, 21.2, 2.98, 3.88);
  String s = e.create(tpl);
  String v = "<prtg><result><channel>Densite</channel><float>1</float><value>1.1230</value></result>"
             "<result><channel>Batterie</channel><float>1</float><value>3.88</value></result>"
             "<result><channel>Temperature</channel><float>1</float><value>21.20</value></result></prtg>";
  assertEqual(s, v);
}

test(template_applyTemplate7) {
  TemplatingEngine e;
  BrewingPush p(&myConfig);
  myConfig.setMDNS("gravitymon");

  const char* tpl = 
    "${battery}-${battery-percent}";

  setupTemplateEngineGravity(&myConfig, e, 45.0, 0, 1.123, 1.223, 21.2, 2.98, 4.20);
  String s = e.create(tpl);

  // When run using charger the level should be 100%
  String v = "4.20-100";
  assertEqual(s, v);
}
#endif // GRAVITYMON

// EOF
