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
#ifndef SRC_MAIN_GRAVITYMON_HPP_
#define SRC_MAIN_GRAVITYMON_HPP_

#if defined(GRAVITYMON)

#include <main.hpp>
#include <templating.hpp>

enum RunMode {
  measurementMode = 0,
  configurationMode = 1,
  wifiSetupMode = 2,
  storageMode = 3
};

void setupTemplateEngineGravity(TemplatingEngine& engine, float angle,
                                float gravitySG, float corrGravitySG,
                                float tempC, float runTime, float voltage);
void runGpioHardwareTests();

#if defined(D1_MINI)
// Hardware config for ESP8266-d1-mini, iSpindel hardware
// ------------------------------------------------------
#define PIN_SDA D3
#define PIN_SCL D4
#define PIN_CFG1 D8
#define PIN_CFG2 D7
#define PIN_DS D6
#define PIN_VOLT PIN_A0
#define CFG_FILENAMEBIN "firmware.bin"

#elif defined(WEMOS_D1_MINI32)
// Hardware config for ESP32-d1-mini, iSpindel hardware
// ------------------------------------------------------
#define PIN_SDA D3
#define PIN_SCL D4
#define PIN_DS D6
#define PIN_CFG1 D8
#define PIN_CFG2 D7
#define PIN_VOLT PIN_A0
#define ENABLE_BLE
#define CFG_FILENAMEBIN "firmware32.bin"

#elif defined(LOLIN_C3_MINI)
// Hardware config for ESP32-c3-mini, iSpindel hardware
// ------------------------------------------------------
#if defined(JTAG_DEBUG)
#define PIN_SDA 8
#define PIN_SCL 9
#warning "ESP32C3 JTAG debugging enabled, using GYRO on GPIO 8/9"
#else
#define PIN_SDA 7
#define PIN_SCL 6
#endif  // JTAG_DEBUG
#define PIN_DS A0
#define PIN_VOLT A3

#define PIN_CFG1 A5
#define PIN_CFG2 A4
#define CFG_FILENAMEBIN "firmware32c3.bin"

#define ENABLE_BLE

#elif defined(LOLIN_S2_MINI)
// Hardware config for ESP32-s2-mini, iSpindel hardware
// ------------------------------------------------------
#define PIN_SDA A17
#define PIN_SCL A15
#define PIN_CFG1 A11
#define PIN_CFG2 A10
#define PIN_DS A8
#define PIN_VOLT A2
#define CFG_FILENAMEBIN "firmware32s2.bin"

#elif defined(LOLIN_S3_MINI)
// Hardware config for ESP32-s3-mini, iSpindel hardware
// ------------------------------------------------------
#define PIN_SDA A17
#define PIN_SCL A15
#define PIN_CFG1 A10
#define PIN_CFG2 A9
#define PIN_DS A12
#define PIN_VOLT A1
#define CFG_FILENAMEBIN "firmware32s3.bin"

#elif defined(LOLIN32_LITE) && defined(FLOATY)
// Hardware config for ESP32-lite, Floaty hardware
// ------------------------------------------------------
#define PIN_SDA A17
#define PIN_SCL A19
#define PIN_DS A3
#define PIN_VOLT A7
#define PIN_CFG1 A14
#define PIN_CFG2 A13
#define PIN_VCC A5
#define PIN_GND A18
#define CFG_FILENAMEBIN "firmware32lite.bin"

#elif defined(OLIMEX_ESP32_C3_DEVKIT_LIPO)
// Hardware config for ESP32C3 from Olimex with build in Charger
// (ESP32-C3-DevKit-Lipo)
// ------------------------------------------------------
#define PIN_SCL GPIO0
#define PIN_SDA GPIO1
#define PIN_DS GPIO5  // implemetation with DS18 not tested
#define PIN_VOLT GPIO3
#define PIN_CFG1 GPIO4
#define PIN_CFG2 GPIO20
#define CFG_FILENAMEBIN "custom-olimex_esp32_c3_devkit_lipo.bin"

#elif defined(SUPER_MINI_ESP32C3)
// Hardware config for SUPER_MINI_ESP32C3, iSpindel hardware
// ------------------------------------------------------
#define PIN_SDA 7
#define PIN_SCL 6
#define PIN_DS A0
#define PIN_VOLT A3

#define PIN_CFG1 A5
#define PIN_CFG2 A4
#define CFG_FILENAMEBIN "custom-super_mini_esp32c3.bin"

#else
#warning "Unknown board type"
#endif

#if !defined(ESP8266) && !defined(ESP32) && !defined(ESP32C2) && \
    !defined(ESP32S3) && !defined(ESP32S2)
#error "Target CPU is not defined or unknown!"
#endif

#endif  // GRAVITYMON

#endif  // SRC_MAIN_GRAVITYMON_HPP_
