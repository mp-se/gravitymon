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
#if defined(ESP8266)
#include <ESP8266mDNS.h>
#else  // defined (ESP32)
#endif
#include <MQTT.h>

#include <battery.hpp>
#include <config.hpp>
#include <cstdio>
#include <helper.hpp>
#include <main.hpp>
#include <perf.hpp>
#include <pushtarget.hpp>
#include <templating.hpp>
constexpr auto PUSHINT_FILENAME = "/push.dat";

#if defined(GRAVITYMON)
// Use iSpindle format for compatibility, HTTP POST
const char iHttpPostFormat[] PROGMEM =
    "{"
    "\"name\": \"${mdns}\", "
    "\"ID\": \"${id}\", "
    "\"token\": \"${token}\", "
    "\"interval\": ${sleep-interval}, "
    "\"temperature\": ${temp}, "
    "\"temp_units\": \"${temp-unit}\", "
    "\"gravity\": ${gravity}, "
    "\"angle\": ${angle}, "
    "\"battery\": ${battery}, "
    "\"RSSI\": ${rssi}, "
    "\"corr-gravity\": ${corr-gravity}, "
    "\"gravity-unit\": \"${gravity-unit}\", "
    "\"run-time\": ${run-time} "
    "}";

// const char bleFormat[] PROGMEM =
//     "{"
//     "\"name\":\"${mdns}\","
//     "\"ID\":\"${id}\","
//     "\"token\":\"${token}\","
//     "\"interval\":${sleep-interval},"
//     "\"temperature\":${temp},"
//     "\"temp_units\":\"${temp-unit}\","
//     "\"gravity\":${gravity},"
//     "\"angle\":${angle},"
//     "\"battery\":${battery},"
//     "\"RSSI\":0,"
//     "\"corr-gravity\":${corr-gravity},"
//     "\"gravity-unit\":\"${gravity-unit}\","
//     "\"run-time\":${run-time}"
//     "}";

// Format for an HTTP GET
const char iHttpGetFormat[] PROGMEM =
    "?name=${mdns}"
    "&id=${id}"
    "&token=${token2}"
    "&interval=${sleep-interval}"
    "&temperature=${temp}"
    "&temp-units=${temp-unit}"
    "&gravity=${gravity}"
    "&angle=${angle}"
    "&battery=${battery}"
    "&rssi=${rssi}"
    "&corr-gravity=${corr-gravity}"
    "&gravity-unit=${gravity-unit}"
    "&run-time=${run-time}";

const char influxDbFormat[] PROGMEM =
    "measurement,host=${mdns},device=${id},temp-format=${temp-unit},gravity-"
    "format=${gravity-unit} "
    "gravity=${gravity},corr-gravity=${corr-gravity},angle=${angle},temp=${"
    "temp},battery=${battery},"
    "rssi=${rssi}\n";

const char mqttFormat[] PROGMEM =
    "ispindel/${mdns}/tilt:${angle}|"
    "ispindel/${mdns}/temperature:${temp}|"
    "ispindel/${mdns}/temp_units:${temp-unit}|"
    "ispindel/${mdns}/battery:${battery}|"
    "ispindel/${mdns}/gravity:${gravity}|"
    "ispindel/${mdns}/interval:${sleep-interval}|"
    "ispindel/${mdns}/RSSI:${rssi}|";
#endif  // GRAVITYMON

#if defined(PRESSUREMON)
const char iHttpPostFormat[] PROGMEM = "";
const char iHttpGetFormat[] PROGMEM = "";
const char influxDbFormat[] PROGMEM = "";
const char mqttFormat[] PROGMEM = "";
#endif

void PushIntervalTracker::update(const int index, const int defaultValue) {
  if (_counters[index] <= 0)
    _counters[index] = defaultValue;
  else
    _counters[index]--;
}

void PushIntervalTracker::load() {
  File intFile = LittleFS.open(PUSHINT_FILENAME, "r");

  if (intFile) {
    String line = intFile.readStringUntil('\n');
    Log.notice(F("PUSH: Read interval tracker %s." CR), line.c_str());

    char temp[80];
    char *s, *p = &temp[0];
    int i = 0;

    snprintf(&temp[0], sizeof(temp), "%s", line.c_str());
    while ((s = strtok_r(p, ":", &p)) != NULL) {
      _counters[i++] = atoi(s);
    }

    intFile.close();
  }

#if LOG_LEVEL == 6
  Log.verbose(F("PUSH: Parsed trackers: %d:%d:%d:%d:%d." CR), _counters[0],
              _counters[1], _counters[2], _counters[3], _counters[4]);
#endif
}

void PushIntervalTracker::save() {
  update(0, myConfig.getPushIntervalPost());
  update(1, myConfig.getPushIntervalPost2());
  update(2, myConfig.getPushIntervalGet());
  update(3, myConfig.getPushIntervalInflux());
  update(4, myConfig.getPushIntervalMqtt());

  // If this feature is disabled we skip saving the file
  if (!myConfig.isPushIntervalActive()) {
#if LOG_LEVEL == 6
    Log.notice(F("PUSH: Variabled push interval disabled." CR));
#endif
    LittleFS.remove(PUSHINT_FILENAME);
  } else {
    Log.notice(
        F("PUSH: Variabled push interval enabled, updating counters." CR));
    File intFile = LittleFS.open(PUSHINT_FILENAME, "w");

    if (intFile) {
      // Format=http1:http2:http3:influx:mqtt
      intFile.printf("%d:%d:%d:%d:%d\n", _counters[0], _counters[1],
                     _counters[2], _counters[3], _counters[4]);
      intFile.close();
    }
  }
}

GravmonPush::GravmonPush(GravmonConfig* gravmonConfig)
    : BasePush(gravmonConfig) {
  _gravmonConfig = gravmonConfig;
}

void GravmonPush::sendAll(float angle, float gravitySG, float corrGravitySG,
                          float tempC, float runTime) {
  printHeap("PUSH");
  _http->setReuse(true);

  TemplatingEngine engine;
#if defined(GRAVITYMON)
  setupTemplateEngine(engine, angle, gravitySG, corrGravitySG, tempC, runTime,
                      myBatteryVoltage.getVoltage());
#endif  // GRAVITYMON

  PushIntervalTracker intDelay;
  intDelay.load();

  if (myConfig.hasTargetHttpPost() && intDelay.useHttp1()) {
    PERF_BEGIN("push-http");
    String tpl = getTemplate(GravmonPush::TEMPLATE_HTTP1);
    String doc = engine.create(tpl.c_str());

    if (myConfig.isHttpPostSSL() && myConfig.isSkipSslOnTest() &&
        runMode != RunMode::measurementMode)
      Log.notice(F("PUSH: SSL enabled, skip run when not in gravity mode." CR));
    else
      sendHttpPost(doc);
    PERF_END("push-http");
  }

  if (myConfig.hasTargetHttpPost2() && intDelay.useHttp2()) {
    PERF_BEGIN("push-http2");
    String tpl = getTemplate(GravmonPush::TEMPLATE_HTTP2);
    String doc = engine.create(tpl.c_str());

    if (myConfig.isHttpPost2SSL() && myConfig.isSkipSslOnTest() &&
        runMode != RunMode::measurementMode)
      Log.notice(F("PUSH: SSL enabled, skip run when not in gravity mode." CR));
    else
      sendHttpPost2(doc);
    PERF_END("push-http2");
  }

  if (myConfig.hasTargetHttpGet() && intDelay.useHttp3()) {
    PERF_BEGIN("push-http3");
    String tpl = getTemplate(GravmonPush::TEMPLATE_HTTP3);
    String doc = engine.create(tpl.c_str());

    if (myConfig.isHttpGetSSL() && myConfig.isSkipSslOnTest() &&
        runMode != RunMode::measurementMode)
      Log.notice(F("PUSH: SSL enabled, skip run when not in gravity mode." CR));
    else
      sendHttpGet(doc);
    PERF_END("push-http3");
  }

  if (myConfig.hasTargetInfluxDb2() && intDelay.useInflux()) {
    PERF_BEGIN("push-influxdb2");
    String tpl = getTemplate(GravmonPush::TEMPLATE_INFLUX);
    String doc = engine.create(tpl.c_str());

    if (myConfig.isHttpInfluxDb2SSL() && myConfig.isSkipSslOnTest() &&
        runMode != RunMode::measurementMode)
      Log.notice(F("PUSH: SSL enabled, skip run when not in gravity mode." CR));
    else
      sendInfluxDb2(doc);
    PERF_END("push-influxdb2");
  }

  if (myConfig.hasTargetMqtt() && intDelay.useMqtt()) {
    PERF_BEGIN("push-mqtt");
    String tpl = getTemplate(GravmonPush::TEMPLATE_MQTT);
    String doc = engine.create(tpl.c_str());

    if (myConfig.isMqttSSL() && myConfig.isSkipSslOnTest() &&
        runMode != RunMode::measurementMode)
      Log.notice(F("PUSH: SSL enabled, skip run when not in gravity mode." CR));
    else
      sendMqtt(doc);
    PERF_END("push-mqtt");
  }

  intDelay.save();
}

const char* GravmonPush::getTemplate(Templates t, bool useDefaultTemplate) {
  String fname;
  _baseTemplate.reserve(600);

  // Load templates from memory
  switch (t) {
    case TEMPLATE_HTTP1:
      _baseTemplate = String(iHttpPostFormat);
      fname = TPL_FNAME_POST;
      break;
    case TEMPLATE_HTTP2:
      _baseTemplate = String(iHttpPostFormat);
      fname = TPL_FNAME_POST2;
      break;
    case TEMPLATE_HTTP3:
      _baseTemplate = String(iHttpGetFormat);
      fname = TPL_FNAME_GET;
      break;
    case TEMPLATE_INFLUX:
      _baseTemplate = String(influxDbFormat);
      fname = TPL_FNAME_INFLUXDB;
      break;
    case TEMPLATE_MQTT:
      _baseTemplate = String(mqttFormat);
      fname = TPL_FNAME_MQTT;
      break;
      // case TEMPLATE_BLE:
      //   _baseTemplate = String(bleFormat);
      //   fname =
      //       "/dummy";  // this file should not exist, use standard template
      //       only
      //   break;
  }

  if (!useDefaultTemplate) {
    File file = LittleFS.open(fname, "r");
    if (file) {
      char buf[file.size() + 1];
      memset(&buf[0], 0, file.size() + 1);
      file.readBytes(&buf[0], file.size());
      _baseTemplate = String(&buf[0]);
      file.close();
      Log.notice(F("PUSH: Template loaded from disk %s." CR), fname.c_str());
    }
  }

#if LOG_LEVEL == 6
  Log.verbose(F("TPL : Base '%s'." CR), _baseTemplate.c_str());
#endif

  return _baseTemplate.c_str();
}

#if defined(GRAVITYMON)
void GravmonPush::setupTemplateEngine(TemplatingEngine& engine, float angle,
                                      float gravitySG, float corrGravitySG,
                                      float tempC, float runTime,
                                      float voltage) {
  // Names
  engine.setVal(TPL_MDNS, myConfig.getMDNS());
  engine.setVal(TPL_ID, myConfig.getID());
  engine.setVal(TPL_TOKEN, myConfig.getToken());
  engine.setVal(TPL_TOKEN2, myConfig.getToken2());

  // Temperature
  if (myConfig.isTempFormatC()) {
    engine.setVal(TPL_TEMP, tempC, DECIMALS_TEMP);
  } else {
    engine.setVal(TPL_TEMP, convertCtoF(tempC), DECIMALS_TEMP);
  }

  engine.setVal(TPL_TEMP_C, tempC, DECIMALS_TEMP);
  engine.setVal(TPL_TEMP_F, convertCtoF(tempC), DECIMALS_TEMP);
  engine.setVal(TPL_TEMP_UNITS, myConfig.getTempFormat());

  // Battery & Timer
  engine.setVal(TPL_BATTERY, voltage, DECIMALS_BATTERY);
  engine.setVal(TPL_SLEEP_INTERVAL, myConfig.getSleepInterval());

  int charge = 0;

  if (voltage > 4.15)
    charge = 100;
  else if (voltage > 4.05)
    charge = 90;
  else if (voltage > 3.97)
    charge = 80;
  else if (voltage > 3.91)
    charge = 70;
  else if (voltage > 3.86)
    charge = 60;
  else if (voltage > 3.81)
    charge = 50;
  else if (voltage > 3.78)
    charge = 40;
  else if (voltage > 3.76)
    charge = 30;
  else if (voltage > 3.73)
    charge = 20;
  else if (voltage > 3.67)
    charge = 10;
  else if (voltage > 3.44)
    charge = 5;

  engine.setVal(TPL_BATTERY_PERCENT, charge);

  // Performance metrics
  engine.setVal(TPL_RUN_TIME, runTime, DECIMALS_RUNTIME);
  engine.setVal(TPL_RSSI, WiFi.RSSI());

  // Angle/Tilt
  engine.setVal(TPL_TILT, angle, DECIMALS_TILT);
  engine.setVal(TPL_ANGLE, angle, DECIMALS_TILT);

  // Gravity options
  if (myConfig.isGravitySG()) {
    engine.setVal(TPL_GRAVITY, gravitySG, DECIMALS_SG);
    engine.setVal(TPL_GRAVITY_CORR, corrGravitySG, DECIMALS_SG);
  } else {
    engine.setVal(TPL_GRAVITY, convertToPlato(gravitySG), DECIMALS_PLATO);
    engine.setVal(TPL_GRAVITY_CORR, convertToPlato(corrGravitySG),
                  DECIMALS_PLATO);
  }

  engine.setVal(TPL_GRAVITY_G, gravitySG, DECIMALS_SG);
  engine.setVal(TPL_GRAVITY_P, convertToPlato(gravitySG), DECIMALS_PLATO);
  engine.setVal(TPL_GRAVITY_CORR_G, corrGravitySG, DECIMALS_SG);
  engine.setVal(TPL_GRAVITY_CORR_P, convertToPlato(corrGravitySG),
                DECIMALS_PLATO);
  engine.setVal(TPL_GRAVITY_UNIT, myConfig.getGravityFormat());

  engine.setVal(TPL_APP_VER, CFG_APPVER);
  engine.setVal(TPL_APP_BUILD, CFG_GITREV);

#if LOG_LEVEL == 6
  dumpAll();
#endif
}
#endif  // GRAVITYMON

// EOF
