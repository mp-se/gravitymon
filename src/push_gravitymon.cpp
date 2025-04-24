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
#if defined(GRAVITYMON)

#include <pushtarget.hpp>

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif

void setupTemplateEngineGravity(TemplatingEngine& engine, float angle,
                                float gravitySG, float corrGravitySG,
                                float tempC, float runTime, float voltage) {
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
  engine.setVal(TPL_SLEEP_INTERVAL,
                myConfig.GravitymonConfig::getSleepInterval());

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
  engine.setVal(TPL_GRAVITY_UNIT, myConfig.getGravityUnit());

  engine.setVal(TPL_APP_VER, CFG_APPVER);
  engine.setVal(TPL_APP_BUILD, CFG_GITREV);
}

#endif  // GRAVITYMON

// EOF
