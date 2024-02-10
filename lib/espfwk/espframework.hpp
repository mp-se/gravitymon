/*
MIT License

Copyright (c) 2021-2024 Magnus

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
#ifndef SRC_ESPFRAMEWORK_HPP_
#define SRC_ESPFRAMEWORK_HPP_

#include <Arduino.h>
#include <ArduinoJson.h>

#define ESPFWK_VER "1.0.0-alfa1"

void forcedReset();

#if defined(ESP8266)
#include <LittleFS.h>
#define ESP_RESET ESP.reset
#elif defined(ESP32C3)
#include <FS.h>
#include <LittleFS.h>
#include "esp32c3/rom/rtc.h"
#define ESP_RESET forcedReset
#elif defined(ESP32S2)
#include <FS.h>
#include <LittleFS.h>
#include "esp32s2/rom/rtc.h"
#define ESP_RESET forcedReset
#elif defined(ESP32S3)
#include <FS.h>
#include <LittleFS.h>
#include "esp32s3/rom/rtc.h"
#define ESP_RESET forcedReset
#elif defined(ESP32LITE)
#include <FS.h>
#include <LittleFS.h>
#include "esp32/rom/rtc.h"
#define ESP_RESET forcedReset
#else  // defined (ESP32)
#include <FS.h>
#include <LittleFS.h>
#include "esp32/rom/rtc.h"
#define ESP_RESET forcedReset
#endif

constexpr auto JSON_BUFFER_SIZE_S = 500;
constexpr auto JSON_BUFFER_SIZE_M = 1000;
constexpr auto JSON_BUFFER_SIZE_L = 3000;
constexpr auto JSON_BUFFER_SIZE_XL = 5000;

#endif  // SRC_ESPFRAMEWORK_HPP_

// EOF
