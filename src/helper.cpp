/*
  Copyright (c) 2021-2026 Magnus
  https://github.com/mp-se/gravitymon

  Licensed under the GNU General Public License v3.0 (GPL v3) for open source use,
  or a Commercial License for proprietary use. See LICENSE and LICENSE_COMMERCIAL.
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
