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
#ifndef SRC_MAIN_HPP_
#define SRC_MAIN_HPP_

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ArduinoLog.h>
#include <stdlib.h>

enum RunMode {
  gravityMode = 0,
  configurationMode = 1,
  wifiSetupMode = 2,
  storageMode = 3
};
extern RunMode runMode;

#if defined(ESP8266)
// Hardware config for ESP8266-d1, iSpindel hardware
// ------------------------------------------------------
#include <LittleFS.h>
#define ESP_RESET ESP.reset
#define PIN_SDA D3
#define PIN_SCL D4
#define PIN_CFG1 D8
#define PIN_CFG2 D7
#define PIN_DS D6
#define PIN_LED 2
#define PIN_VOLT PIN_A0
#elif defined(ESP32C3)
// Hardware config for ESP32-c3-mini, iSpindel hardware
// ------------------------------------------------------
#include <FS.h>
#include <LittleFS.h>

#include "esp32c3/rom/rtc.h"
#define ESPhttpUpdate httpUpdate
#define ESP_RESET ESP.restart
#define ESP8266WebServer WebServer
#if defined(JTAG_DEBUG)
#define PIN_SDA 8
#define PIN_SCL 9
#warning "ESP32C3 JTAG debugging enabled, using GYRO on GPIO 8/9"
#else
#define PIN_SDA 7
#define PIN_SCL 6
#endif  // JTAG_DEBUG
#define PIN_CFG1 A5
#define PIN_CFG2 A4
#define PIN_DS A3
#define PIN_VOLT A0
// This should be the LED_BUILTIN, but that is also connected SDA (Gyro) so we
// cannot use both. So we point LED to pin 8 which is not used.
#define PIN_LED 8
#elif defined(ESP32S2)
// Hardware config for ESP32-s2-mini, iSpindel hardware
// ------------------------------------------------------
#include <FS.h>
#include <LittleFS.h>

#include "esp32s2/rom/rtc.h"
#define ESPhttpUpdate httpUpdate
#define ESP_RESET ESP.restart
#define ESP8266WebServer WebServer
#define PIN_SDA A17
#define PIN_SCL A15
#define PIN_CFG1 A11
#define PIN_CFG2 A10
#define PIN_DS A8
#define PIN_VOLT A2
#define PIN_LED LED_BUILTIN
#elif defined(ESP32LITE)
// Hardware config for ESP32-lite, Floaty hardware
// ------------------------------------------------------
#include <FS.h>
#include <LittleFS.h>

#include "esp32/rom/rtc.h"
#define ESPhttpUpdate httpUpdate
#define ESP_RESET ESP.restart
#define ESP8266WebServer WebServer
#define PIN_SDA A17
#define PIN_SCL A19
#define PIN_DS A3
#define PIN_VOLT A7
#define PIN_CFG1 A14
#define PIN_CFG2 A13
#define PIN_VCC A5
#define PIN_GND A18
#define PIN_LED LED_BUILTIN
#else  // defined (ESP32)
// Hardware config for ESP32-d1-min, iSpindel hardware
// ------------------------------------------------------
#include <FS.h>
#include <LittleFS.h>

#include "esp32/rom/rtc.h"
#define ESPhttpUpdate httpUpdate
#define ESP_RESET ESP.restart
#define ESP8266WebServer WebServer
#define PIN_SDA D3
#define PIN_SCL D4
#define PIN_DS D6
#define PIN_CFG1 D8
#define PIN_CFG2 D7
#define PIN_LED LED_BUILTIN
#define PIN_VOLT PIN_A0
#endif

#if defined(USE_SERIAL_PINS) && (defined(ESP32C3) || defined(ESP32S2))
// #define EspSerial Serial0 // We cant use Serial on newer boards since this is
// using USBC port
#define EspSerial \
  Serial0  // We cant use Serial on newer boards since this is using USBC port
#else
#define EspSerial Serial
#endif

#endif  // SRC_MAIN_HPP_
