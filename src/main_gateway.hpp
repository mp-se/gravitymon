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
#ifndef SRC_MAIN_GATEWAY_HPP_
#define SRC_MAIN_GATEWAY_HPP_

#if defined(GATEWAY)

#include <templating.hpp>

enum RunMode {
  measurementMode = 0,
  wifiSetupMode = 1,
};

void setupTemplateEngineGravityGateway(TemplatingEngine& engine, float angle,
                                       float gravitySG, float tempC,
                                       float voltage, int interval,
                                       const char* id, const char* token,
                                       const char* name);

void setupTemplateEnginePressureGateway(TemplatingEngine& engine,
                                        float pressurePsi, float pressurePsi1,
                                        float tempC, float voltage,
                                        int interval, const char* id,
                                        const char* token, const char* name);

// Defines that needs to be there for the common base but not used in the
// gateway
#if defined(LOLIN_D32_PRO)
#define PIN_VOLT T0
#elif defined(LOLIN_S3)
#define PIN_VOLT A3
#endif

#endif  // GATEWAY

#endif  // SRC_MAIN_GATEWAY_HPP_
