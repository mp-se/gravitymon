/*
MIT License

Copyright (c) 2025 Magnus

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
#if defined(PRESSUREMON)

#include <pushtarget.hpp>

const char iHttpPostFormat[] PROGMEM =
    "{"
    "\"name\": \"${mdns}\", "
    "\"ID\": \"${id}\", "
    "\"token\": \"${token}\", "
    "\"interval\": ${sleep-interval}, "
    "\"temperature\": ${temp}, "
    // "\"temperature1\": ${temp1}, "
    "\"temperature-unit\": \"${temp-unit}\", "
    "\"pressure\": ${pressure}, "
    // "\"pressure1\": ${pressure1}, "
    "\"pressure-unit\": \"${pressure-unit}\", "
    "\"battery\": ${battery}, "
    "\"RSSI\": ${rssi}, "
    "\"run-time\": ${run-time} "
    "}";

const char iHttpGetFormat[] PROGMEM =
    "?name=${mdns}"
    "&id=${id}"
    "&token=${token2}"
    "&interval=${sleep-interval}"
    "&temperature=${temp}"
    // "&temperature1=${temp1}"
    "&temperature-unit=${temp-unit}"
    "&pressure=${pressure}"
    // "&pressure1=${pressure1}"
    "&pressure-unit=${pressure-unit}"
    "&battery=${battery}"
    "&rssi=${rssi}"
    "&run-time=${run-time}";

const char influxDbFormat[] PROGMEM =
    "measurement,host=${mdns},"
    "device=${id},"
    "temperature-unit=${temp-unit},"
    "pressure-unit=${pressure-unit} "
    "pressure=${pressure},"
    // "pressure1=${pressure1},"
    "temp=${temp},"
    // "temp1=${temp1},"
    "battery=${battery},"
    "rssi=${rssi}\n";

const char mqttFormat[] PROGMEM =
    "pressuremon/${mdns}/temperature:${temp}|"
    // "pressuremon/${mdns}/temperature1:${temp1}|"
    "pressuremon/${mdns}/temperature-unit:${temp-unit}|"
    "pressuremon/${mdns}/battery:${battery}|"
    "pressuremon/${mdns}/pressure:${pressure}|"
    // "pressuremon/${mdns}/pressure1:${pressure1}|"
    "pressuremon/${mdns}/pressure-unit:${pressure-unit}|"
    "pressuremon/${mdns}/interval:${sleep-interval}|"
    "pressuremon/${mdns}/RSSI:${rssi}|";

// constexpr auto TPL_TEMP1 = "${temp1}";
// constexpr auto TPL_TEMP1_C = "${temp1-c}";
// constexpr auto TPL_TEMP1_F = "${temp1-f}";
constexpr auto TPL_PRESSURE = "${pressure}";
constexpr auto TPL_PRESSURE1 = "${pressure1}";
constexpr auto TPL_PRESSURE_PSI = "${pressure-psi}";
constexpr auto TPL_PRESSURE1_PSI = "${pressure1-psi}";
constexpr auto TPL_PRESSURE_BAR = "${pressure-bar}";
constexpr auto TPL_PRESSURE1_BAR = "${pressure1-bar}";
constexpr auto TPL_PRESSURE_KPA = "${pressure-kpa}";
constexpr auto TPL_PRESSURE1_KPA = "${pressure1-kpa}";
constexpr auto TPL_PRESSURE_UNIT = "${pressure-unit}";  // PSI, BAR, KPA

void setupTemplateEnginePressure(TemplatingEngine& engine, float pressurePsi,
                                 float pressurePsi1, float tempC,
                                 float runTime, float voltage) {
  // Set values so that the payload is not invalid
  if (isnan(pressurePsi)) pressurePsi = 0;
  if (isnan(pressurePsi1)) pressurePsi1 = 0;
  if (isnan(tempC)) tempC = 0;
  // if (isnan(tempC1)) tempC1 = 0;

  // Names
  engine.setVal(TPL_MDNS, myConfig.getMDNS());
  engine.setVal(TPL_ID, myConfig.getID());
  engine.setVal(TPL_TOKEN, myConfig.getToken());
  engine.setVal(TPL_TOKEN2, myConfig.getToken2());

  // Temperature
  if (myConfig.isTempFormatC()) {
    engine.setVal(TPL_TEMP, tempC, DECIMALS_TEMP);
    // engine.setVal(TPL_TEMP1, tempC1, DECIMALS_TEMP);
  } else {
    engine.setVal(TPL_TEMP, convertCtoF(tempC), DECIMALS_TEMP);
    // engine.setVal(TPL_TEMP1, convertCtoF(tempC1), DECIMALS_TEMP);
  }

  engine.setVal(TPL_TEMP_C, tempC, DECIMALS_TEMP);
  engine.setVal(TPL_TEMP_F, convertCtoF(tempC), DECIMALS_TEMP);
  // engine.setVal(TPL_TEMP1_C, tempC1, DECIMALS_TEMP);
  // engine.setVal(TPL_TEMP1_F, convertCtoF(tempC1), DECIMALS_TEMP);
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

  engine.setVal(TPL_PRESSURE_PSI, pressurePsi, DECIMALS_PRESSURE);
  engine.setVal(TPL_PRESSURE1_PSI, pressurePsi1, DECIMALS_PRESSURE);
  engine.setVal(TPL_PRESSURE_BAR, convertPsiPressureToBar(pressurePsi),
                DECIMALS_PRESSURE);
  engine.setVal(TPL_PRESSURE1_BAR, convertPsiPressureToBar(pressurePsi1),
                DECIMALS_PRESSURE);
  engine.setVal(TPL_PRESSURE_KPA, convertPsiPressureToKPa(pressurePsi),
                DECIMALS_PRESSURE);
  engine.setVal(TPL_PRESSURE_KPA, convertPsiPressureToKPa(pressurePsi1),
                DECIMALS_PRESSURE);

  if (myConfig.isPressureBar()) {
    engine.setVal(TPL_PRESSURE, convertPsiPressureToBar(pressurePsi),
                  DECIMALS_PRESSURE);
    engine.setVal(TPL_PRESSURE1, convertPsiPressureToBar(pressurePsi1),
                  DECIMALS_PRESSURE);
  } else if (myConfig.isPressureKpa()) {
    engine.setVal(TPL_PRESSURE, convertPsiPressureToKPa(pressurePsi),
                  DECIMALS_PRESSURE);
    engine.setVal(TPL_PRESSURE1, convertPsiPressureToKPa(pressurePsi1),
                  DECIMALS_PRESSURE);
  } else {
    engine.setVal(TPL_PRESSURE, pressurePsi, DECIMALS_PRESSURE);
    engine.setVal(TPL_PRESSURE1, pressurePsi1, DECIMALS_PRESSURE);
  }

  engine.setVal(TPL_PRESSURE_UNIT, myConfig.getPressureUnit());

  engine.setVal(TPL_APP_VER, CFG_APPVER);
  engine.setVal(TPL_APP_BUILD, CFG_GITREV);
}

#endif  // PRESSUREMON

// EOF
