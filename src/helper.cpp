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


#include <Ticker.h>

#include <helper.hpp>
#include <log.hpp>
#include <main.hpp>

#if ESP_ARDUINO_VERSION_MAJOR >= 3  // For Arduino Core 3.x
#define SOC_ADC_MAX_BITWIDTH 12
#endif

extern "C" {
void write_bytes(int fd, char* buf, int n) { EspSerial.print(*buf); }
}

bool checkPinConnected(int pin1, int pin2) {
#if defined(ESP8266)
  pinMode(pin1, INPUT);
#else
  pinMode(pin1, INPUT_PULLDOWN);
#endif
  pinMode(pin2, OUTPUT);
  delay(5);
  digitalWrite(pin2, 1);
  delay(5);
  int i = digitalRead(pin1);
  digitalWrite(pin2, 0);
  return i == LOW ? false : true;
}

bool checkPinCharging(int pin) {
#if defined(ESP8266)
  return false;
#else
  pinMode(pin, INPUT);
  delay(5);
  analogReadResolution(SOC_ADC_MAX_BITWIDTH);
  analogSetAttenuation(ADC_11db);
  int v = analogRead(pin);
  // Log.notice(F("Helper: Charging pin %d voltage %d" CR), pin, v);
  return v > 2500 ? true : false;  // > 2V on pin
#endif
}

void printBuildOptions() {
  Log.notice(F("Build options: %s (%s) LOGLEVEL %d "
#if defined(GRAVITYMON)
               "GRAVITYMON "
#endif
#if defined(PRESSUREMON)
               "PRESSUREMON "
#endif
#if defined(GATEWAY)
               "GATEWAY "
#endif
#if defined(ESP8266)
               "ESP8266 "
#else
               "ESP32 "
#endif
#ifdef SKIP_SLEEPMODE
               "SKIP_SLEEP "
#endif
#ifdef COLLECT_PERFDATA
               "PERFDATA "
#endif
               CR),
             CFG_APPVER, CFG_GITREV, LOG_LEVEL);
}

float convertPsiPressureToBar(float psi) { return psi * 0.0689475729; }
float convertPsiPressureToKPa(float psi) { return psi * 68.947572932 / 1000; }
float convertPaPressureToPsi(float pa) { return pa * 0.000145038; }
float convertPaPressureToBar(float pa) { return pa / 100000; }

// EOF
