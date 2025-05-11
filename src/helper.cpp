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
#include <Ticker.h>

#include <helper.hpp>
#include <log.hpp>
#include <main.hpp>

extern "C" {
void write_bytes(int fd, char* buf, int n) { EspSerial.print(*buf); }
}

bool checkPinConnected() {
#if defined(PIN_CFG1) && defined(PIN_CFG2)
#if defined(ESP8266)
  pinMode(PIN_CFG1, INPUT);
#else
  pinMode(PIN_CFG1, INPUT_PULLDOWN);
#endif
  pinMode(PIN_CFG2, OUTPUT);
  delay(5);
  digitalWrite(PIN_CFG2, 1);
  delay(5);
  int i = digitalRead(PIN_CFG1);
  digitalWrite(PIN_CFG2, 0);
  return i == LOW ? false : true;
#else
  return false;
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
#if defined(FLOATY)
               "FLOATY "
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
float convertPsiPressureToKPa(float psi) { return psi * 68.947572932 * 1000; }
float convertPaPressureToPsi(float pa) { return pa * 0.000145038; }
float convertPaPressureToBar(float pa) { return pa / 100000; }

// EOF
