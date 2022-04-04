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
    "\"name\" : \"${mdns}\", "
    "\"ID\": \"${id}\", "
    "\"token\" : \"${token}\", "
    "\"interval\": ${sleep-interval}, "
    "\"temperature\": ${temp}, "
    "\"temp_units\": \"${temp-unit}\", "
    "\"gravity\": ${gravity}, "
    "\"angle\": ${angle}, "
    "\"battery\": ${battery}, "
    "\"rssi\": ${rssi}, "
    "\"corr-gravity\": ${corr-gravity}, "
    "\"gravity-unit\": \"${gravity-unit}\", "
    "\"run-time\": ${run-time} "
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

const char brewfatherFormat[] PROGMEM =
    "{"
    "\"name\": \"${mdns}\","
    "\"temp\": ${temp}, "
    "\"aux_temp\": 0, "
    "\"ext_temp\": 0, "
    "\"temp_unit\": \"${temp-unit}\", "
    "\"gravity\": ${gravity}, "
    "\"gravity_unit\": \"${gravity-unit}\", "
    "\"pressure\": 0, "
    "\"pressure_unit\": \"PSI\", "
    "\"ph\": 0, "
    "\"bpm\": 0, "
    "\"comment\": \"\", "
    "\"beer\": \"\", "
    "\"battery\": ${battery}"
    "}";

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

//
// Initialize the variables
//
void TemplatingEngine::initialize(float angle, float gravitySG,
                                  float corrGravitySG, float tempC,
                                  float runTime) {
  // Names
  setVal(TPL_MDNS, myConfig.getMDNS());
  setVal(TPL_ID, myConfig.getID());
  setVal(TPL_TOKEN, myConfig.getToken());
  setVal(TPL_TOKEN2, myConfig.getToken2());

  // Temperature
  if (myConfig.isTempC()) {
    setVal(TPL_TEMP, tempC, 1);
  } else {
    setVal(TPL_TEMP, convertCtoF(tempC), 1);
  }

  setVal(TPL_TEMP_C, tempC, 1);
  setVal(TPL_TEMP_F, convertCtoF(tempC), 1);
  setVal(TPL_TEMP_UNITS, myConfig.getTempFormat());

  // Battery & Timer
  setVal(TPL_BATTERY, myBatteryVoltage.getVoltage());
  setVal(TPL_SLEEP_INTERVAL, myConfig.getSleepInterval());

  // Performance metrics
  setVal(TPL_RUN_TIME, runTime, 1);
  setVal(TPL_RSSI, WiFi.RSSI());

  // Angle/Tilt
  setVal(TPL_TILT, angle);
  setVal(TPL_ANGLE, angle);

  // Gravity options
  if (myConfig.isGravitySG()) {
    setVal(TPL_GRAVITY, gravitySG, 4);
    setVal(TPL_GRAVITY_CORR, corrGravitySG, 4);
  } else {
    setVal(TPL_GRAVITY, convertToPlato(gravitySG), 1);
    setVal(TPL_GRAVITY_CORR, convertToPlato(corrGravitySG), 1);
  }

  setVal(TPL_GRAVITY_G, gravitySG, 4);
  setVal(TPL_GRAVITY_P, convertToPlato(gravitySG), 1);
  setVal(TPL_GRAVITY_CORR_G, corrGravitySG, 4);
  setVal(TPL_GRAVITY_CORR_P, convertToPlato(corrGravitySG), 1);
  setVal(TPL_GRAVITY_UNIT, myConfig.getGravityFormat());

#if LOG_LEVEL == 6
  // dumpAll();
#endif
}

//
// Create the data using defined template.
//
const String& TemplatingEngine::create(TemplatingEngine::Templates idx) {
  String fname;
  baseTemplate.reserve(600);

  // Load templates from memory
  switch (idx) {
    case TEMPLATE_HTTP1:
      baseTemplate = String(iSpindleFormat);
      fname = TPL_FNAME_HTTP1;
      break;
    case TEMPLATE_HTTP2:
      baseTemplate = String(iSpindleFormat);
      fname = TPL_FNAME_HTTP2;
      break;
    case TEMPLATE_HTTP3:
      baseTemplate = String(iHttpGetFormat);
      fname = TPL_FNAME_HTTP3;
      break;
    case TEMPLATE_BREWFATHER:
      baseTemplate = String(brewfatherFormat);
      // fname = TPL_FNAME_BREWFATHER;
      break;
    case TEMPLATE_INFLUX:
      baseTemplate = String(influxDbFormat);
      fname = TPL_FNAME_INFLUXDB;
      break;
    case TEMPLATE_MQTT:
      baseTemplate = String(mqttFormat);
      fname = TPL_FNAME_MQTT;
      break;
  }

  // TODO: Add code to load templates from disk if they exist.
  File file = LittleFS.open(fname, "r");
  if (file) {
    char buf[file.size() + 1];
    memset(&buf[0], 0, file.size() + 1);
    file.readBytes(&buf[0], file.size());
    baseTemplate = String(&buf[0]);
    file.close();
    Log.notice(F("TPL : Template loaded from disk %s." CR), fname.c_str());
  }

#if LOG_LEVEL == 6
  // Log.verbose(F("TPL : Base '%s'." CR), baseTemplate.c_str());
#endif

  // Insert data into template.
  transform(baseTemplate);

#if LOG_LEVEL == 6
  // Log.verbose(F("TPL : Transformed '%s'." CR), baseTemplate.c_str());
#endif

  return baseTemplate;
}

// EOF
