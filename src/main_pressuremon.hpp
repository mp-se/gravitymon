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
                                 float pressurePsi1, float tempC, float runTime,
                                 float voltage);

float convertPsiPressureToBar(float psi);
float convertPsiPressureToKPa(float psi);
float convertPaPressureToPsi(float pa);
float convertPaPressureToBar(float pa);

#if defined(LOLIN_S3_MINI)
// Hardware config for ESP32-s3-mini, pressuremon hardware
// ------------------------------------------------------
#define PIN_SDA 35
#define PIN_SCL 36
#define PIN_SDA1 6
#define PIN_SCL1 5
#define PIN_VOLT 2
#define PIN_DS 10

#elif defined(LOLIN_S2_MINI)
// Hardware config for ESP32-s2-mini, pressuremon hardware
// ------------------------------------------------------
#define PIN_SDA 33
#define PIN_SCL 35
#define PIN_SDA1 6
#define PIN_SCL1 4
#define PIN_VOLT 3
#define PIN_DS 12

#elif defined(LOLIN_C3_MINI)
// Hardware config for ESP32-c3-mini, pressuremon hardware
// ------------------------------------------------------
#define PIN_SDA 8
#define PIN_SCL 10
#define PIN_SDA1 1
#define PIN_SCL1 2
#define PIN_VOLT 3
#define PIN_DS 5
#endif

#endif  // PRESSUREMON

#endif  // SRC_MAIN_PRESSUREMON_HPP_
