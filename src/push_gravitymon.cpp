/*
 * GravityMon
 * Copyright (c) 2021-2026 Magnus
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Alternatively, this software may be used under the terms of a
 * commercial license. See LICENSE_COMMERCIAL for details.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
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
