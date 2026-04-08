/*
  Copyright (c) 2021-2026 Magnus
  https://github.com/mp-se/gravitymon

  Licensed under the GNU General Public License v3.0 (GPL v3) for open source use,
  or a Commercial License for proprietary use. See LICENSE and LICENSE_COMMERCIAL.
*/


#if defined(GRAVITYMON)

#include <battery.hpp>
#include <main.hpp>
#include <push_gravitymon.hpp>
#include <pushtarget.hpp>

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif

void setupTemplateEngineGravity(GravitymonConfig* config,
                                TemplatingEngine& engine, float angle,
                                float velocity, float gravitySG,
                                float corrGravitySG, float tempC, float runTime,
                                float voltage) {
  // Names
  engine.setVal(TPL_MDNS, config->getMDNS());
  engine.setVal(TPL_ID, config->getID());
  engine.setVal(TPL_TOKEN, config->getToken());
  engine.setVal(TPL_TOKEN2, config->getToken2());

  // Temperature
  if (config->isTempFormatC()) {
    engine.setVal(TPL_TEMP, tempC, DECIMALS_TEMP);
  } else {
    engine.setVal(TPL_TEMP, convertCtoF(tempC), DECIMALS_TEMP);
  }

  engine.setVal(TPL_TEMP_C, tempC, DECIMALS_TEMP);
  engine.setVal(TPL_TEMP_F, convertCtoF(tempC), DECIMALS_TEMP);
  engine.setVal(TPL_TEMP_UNITS, config->getTempFormat());

  // Battery & Timer
  engine.setVal(TPL_BATTERY, voltage, DECIMALS_BATTERY);
  engine.setVal(TPL_SLEEP_INTERVAL, config->getSleepInterval());

  engine.setVal(TPL_BATTERY_PERCENT,
                getBatteryPercentage(voltage, BatteryType::LithiumIon));

  // Performance metrics
  engine.setVal(TPL_RUN_TIME, runTime, DECIMALS_RUNTIME);
  engine.setVal(TPL_RSSI, WiFi.RSSI());

  // Angle/Tilt
  engine.setVal(TPL_TILT, angle, DECIMALS_TILT);
  engine.setVal(TPL_ANGLE, angle, DECIMALS_TILT);
  engine.setVal(TPL_VELOCITY, velocity, 1);

  // Gravity options
  if (config->isGravitySG()) {
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
  engine.setVal(TPL_GRAVITY_UNIT, config->getGravityUnit());

  engine.setVal(TPL_APP_VER, CFG_APPVER);
  engine.setVal(TPL_APP_BUILD, CFG_GITREV);
}

#endif  // GRAVITYMON

// EOF
