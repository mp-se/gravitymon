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
#include <config.hpp>
#include <templating.hpp>

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#else  // defined (ESP32)
#include <WiFi.h>
#endif

// Use iSpindle format for compatibility, HTTP POST
const char iSpindleFormat[] PROGMEM =
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

const char bleFormat[] PROGMEM =
    "{"
    "\"name\":\"${mdns}\","
    "\"ID\":\"${id}\","
    "\"token\":\"${token}\","
    "\"interval\":${sleep-interval},"
    "\"temperature\":${temp},"
    "\"temp_units\":\"${temp-unit}\","
    "\"gravity\":${gravity},"
    "\"angle\":${angle},"
    "\"battery\":${battery},"
    "\"RSSI\":0,"
    "\"corr-gravity\":${corr-gravity},"
    "\"gravity-unit\":\"${gravity-unit}\","
    "\"run-time\":${run-time}"
    "}";

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

void TemplatingEngine::initialize(float angle, float gravitySG,
                                  float corrGravitySG, float tempC,
                                  float runTime, float voltage) {
  // Names
  setVal(TPL_MDNS, myConfig.getMDNS());
  setVal(TPL_ID, myConfig.getID());
  setVal(TPL_TOKEN, myConfig.getToken());
  setVal(TPL_TOKEN2, myConfig.getToken2());

  // Temperature
  if (myConfig.isTempC()) {
    setVal(TPL_TEMP, tempC, DECIMALS_TEMP);
  } else {
    setVal(TPL_TEMP, convertCtoF(tempC), DECIMALS_TEMP);
  }

  setVal(TPL_TEMP_C, tempC, DECIMALS_TEMP);
  setVal(TPL_TEMP_F, convertCtoF(tempC), DECIMALS_TEMP);
  setVal(TPL_TEMP_UNITS, myConfig.getTempFormat());

  // Battery & Timer
  setVal(TPL_BATTERY, voltage, DECIMALS_BATTERY);
  setVal(TPL_SLEEP_INTERVAL, myConfig.getSleepInterval());

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

  setVal(TPL_BATTERY_PERCENT, charge);

  // Performance metrics
  setVal(TPL_RUN_TIME, runTime, DECIMALS_RUNTIME);
  setVal(TPL_RSSI, WiFi.RSSI());

  // Angle/Tilt
  setVal(TPL_TILT, angle, DECIMALS_TILT);
  setVal(TPL_ANGLE, angle, DECIMALS_TILT);

  // Gravity options
  if (myConfig.isGravitySG()) {
    setVal(TPL_GRAVITY, gravitySG, DECIMALS_SG);
    setVal(TPL_GRAVITY_CORR, corrGravitySG, DECIMALS_SG);
  } else {
    setVal(TPL_GRAVITY, convertToPlato(gravitySG), DECIMALS_PLATO);
    setVal(TPL_GRAVITY_CORR, convertToPlato(corrGravitySG), DECIMALS_PLATO);
  }

  setVal(TPL_GRAVITY_G, gravitySG, DECIMALS_SG);
  setVal(TPL_GRAVITY_P, convertToPlato(gravitySG), DECIMALS_PLATO);
  setVal(TPL_GRAVITY_CORR_G, corrGravitySG, DECIMALS_SG);
  setVal(TPL_GRAVITY_CORR_P, convertToPlato(corrGravitySG), DECIMALS_PLATO);
  setVal(TPL_GRAVITY_UNIT, myConfig.getGravityFormat());

  setVal(TPL_APP_VER, CFG_APPVER);
  setVal(TPL_APP_BUILD, CFG_GITREV);

#if LOG_LEVEL == 6
  // dumpAll();
#endif
}

// the useDefaultTemplate param is there to support unit tests.
const char* TemplatingEngine::create(TemplatingEngine::Templates idx,
                                     bool useDefaultTemplate) {
  String fname;
  _baseTemplate.reserve(600);

  // Load templates from memory
  switch (idx) {
    case TEMPLATE_HTTP1:
      _baseTemplate = String(iSpindleFormat);
      fname = TPL_FNAME_HTTP1;
      break;
    case TEMPLATE_HTTP2:
      _baseTemplate = String(iSpindleFormat);
      fname = TPL_FNAME_HTTP2;
      break;
    case TEMPLATE_HTTP3:
      _baseTemplate = String(iHttpGetFormat);
      fname = TPL_FNAME_HTTP3;
      break;
    case TEMPLATE_INFLUX:
      _baseTemplate = String(influxDbFormat);
      fname = TPL_FNAME_INFLUXDB;
      break;
    case TEMPLATE_MQTT:
      _baseTemplate = String(mqttFormat);
      fname = TPL_FNAME_MQTT;
      break;
    case TEMPLATE_BLE:
      _baseTemplate = String(bleFormat);
      fname =
          "/dummy";  // this file should not exist, use standard template only
      break;
  }

  if (!useDefaultTemplate) {
    File file = LittleFS.open(fname, "r");
    if (file) {
      char buf[file.size() + 1];
      memset(&buf[0], 0, file.size() + 1);
      file.readBytes(&buf[0], file.size());
      _baseTemplate = String(&buf[0]);
      file.close();
      Log.notice(F("TPL : Template loaded from disk %s." CR), fname.c_str());
    }
  }

#if LOG_LEVEL == 6
  Log.verbose(F("TPL : Base '%s'." CR), _baseTemplate.c_str());
#endif

  // Insert data into template.
  transform();
  _baseTemplate.clear();

  if (_output) {
#if LOG_LEVEL == 6
    Log.verbose(F("TPL : Transformed '%s'." CR), _output);
#endif
    return _output;
  }

  return "";
}

// added to support more unit test scenarios.
const char* TemplatingEngine::create(const char* formatTemplate) {
  _baseTemplate = String(formatTemplate);

  // Insert data into template.
  transform();
  _baseTemplate.clear();

  if (_output) return _output;

  return "";
}

// EOF
