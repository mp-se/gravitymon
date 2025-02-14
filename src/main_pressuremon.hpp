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
#ifndef SRC_MAIN_PRESSUREMON_HPP_
#define SRC_MAIN_PRESSUREMON_HPP_

#if defined(PRESSUREMON)

#include <main.hpp>
#include <templating.hpp>

enum RunMode {
  measurementMode = 0,
  configurationMode = 1,
  wifiSetupMode = 2,
};

void setupTemplateEnginePressure(TemplatingEngine& engine, float pressurePsi,
                                 float pressurePsi1, float tempC,
                                 float runTime, float voltage);

float convertPsiPressureToBar(float psi);
float convertPsiPressureToKPa(float psi);
float convertPaPressureToPsi(float pa);
float convertPaPressureToBar(float pa);

#if defined(LOLIN_S3_MINI)
// Hardware config for ESP32-s3-mini, pressuremon hardware
// ------------------------------------------------------
#define PIN_SDA SDA
#define PIN_SCL SCL
#define PIN_SDA1 A17
#define PIN_SCL1 A15

#define PIN_CFG1 A10
#define PIN_CFG2 A9
#define PIN_VOLT A1
#define PIN_DS A3

#elif defined(LOLIN_S2_MINI)
// Hardware config for ESP32-s2-mini, pressuremon hardware
// ------------------------------------------------------
#define PIN_SDA SDA
#define PIN_SCL SCL
#define PIN_SDA1 A17
#define PIN_SCL1 A15

#define PIN_CFG1 A10
#define PIN_CFG2 A11
#define PIN_VOLT A2
#define PIN_DS A4

#elif defined(LOLIN_C3_MINI)
// Hardware config for ESP32-c3-mini, pressuremon hardware
// ------------------------------------------------------
#define PIN_SDA SDA
#define PIN_SCL SCL
#define PIN_SDA1 SCK
#define PIN_SCL1 2

#define PIN_CFG1 3
#define PIN_CFG2 4
#define PIN_VOLT 7
#define PIN_DS 5

#elif defined(LOLIN_D32_PRO)
// Hardware config for ESP32 pro, pressuremon hardware
// ------------------------------------------------------
#define PIN_SDA SDA
#define PIN_SCL SCL
#define PIN_SDA1 MISO
#define PIN_SCL1 SCK

#define PIN_CFG1 DAC1
#define PIN_CFG2 DAC2
#define PIN_VOLT A7
#define PIN_DS 5 // SS PIN

#elif defined(LOLIN_S3_PRO)
// Hardware config for ESP32-s3 pro, pressuremon hardware
// ------------------------------------------------------
#define PIN_SDA SDA
#define PIN_SCL SCL
#define PIN_SDA1 8
#define PIN_SCL1 3

#define PIN_CFG1 17
#define PIN_CFG2 18
#define PIN_VOLT 3
#define PIN_DS 1
#endif

#endif  // PRESSUREMON

#endif  // SRC_MAIN_PRESSUREMON_HPP_
