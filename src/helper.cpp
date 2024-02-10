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
#include <Ticker.h>

#include <helper.hpp>
#include <main.hpp>

extern "C" {
void write_bytes(int fd, char* buf, int n) { EspSerial.print(*buf); }
}

void runGpioHardwareTests() {
#if defined(RUN_HARDWARE_TEST)
  int max = 10;

  Log.notice(F("HELP: Configuring GPIO for output." CR));
  pinMode(PIN_SDA, OUTPUT);
  pinMode(PIN_SCL, OUTPUT);
  pinMode(PIN_CFG1, OUTPUT);
  pinMode(PIN_CFG2, OUTPUT);
  pinMode(PIN_DS, OUTPUT);
  pinMode(PIN_VOLT, OUTPUT);
  delay(100);
  digitalWrite(PIN_SDA, LOW);
  digitalWrite(PIN_SCL, LOW);
  digitalWrite(PIN_CFG1, LOW);
  digitalWrite(PIN_CFG2, LOW);
  digitalWrite(PIN_DS, LOW);
  digitalWrite(PIN_VOLT, LOW);
  delay(100);

  int sleep = 700;

  Log.notice(F("HELP: Testing SDA." CR));
  for (int i = 0; i < max; i++) {
    digitalWrite(PIN_SDA, i % 2);
    delay(sleep);
  }
  digitalWrite(PIN_SDA, LOW);

  Log.notice(F("HELP: Testing SCL." CR));
  for (int i = 0; i < max; i++) {
    digitalWrite(PIN_SCL, i % 2);
    delay(sleep);
  }
  digitalWrite(PIN_SCL, LOW);

  Log.notice(F("HELP: Testing CFG1." CR));
  for (int i = 0; i < max; i++) {
    digitalWrite(PIN_CFG1, i % 2);
    delay(sleep);
  }
  digitalWrite(PIN_CFG1, LOW);

  Log.notice(F("HELP: Testing CFG2." CR));
  for (int i = 0; i < max; i++) {
    digitalWrite(PIN_CFG2, i % 2);
    delay(sleep);
  }
  digitalWrite(PIN_CFG2, LOW);

  Log.notice(F("HELP: Testing DS." CR));
  for (int i = 0; i < max; i++) {
    digitalWrite(PIN_DS, i % 2);
    delay(sleep);
  }
  digitalWrite(PIN_DS, LOW);

  Log.notice(F("HELP: Testing VOLT." CR));
  for (int i = 0; i < max; i++) {
    digitalWrite(PIN_VOLT, i % 2);
    delay(sleep);
  }
  digitalWrite(PIN_VOLT, LOW);

  Log.notice(F("HELP: Tests finished, enter waiting for reset." CR));
  while (true) delay(sleep);
#endif
}

bool checkPinConnected() {
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
}

void printBuildOptions() {
  Log.notice(F("Build options: %s (%s) LOGLEVEL %d "
#if defined(ESP8266)
               "ESP8266 "
#elif defined(ESP32LITE)
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

// EOF
