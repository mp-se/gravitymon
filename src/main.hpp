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
#ifndef SRC_MAIN_HPP_
#define SRC_MAIN_HPP_

#include <Arduino.h>
#include <stdlib.h>
#include <espframework.hpp>

enum RunMode {
    measurementMode = 0,
    configurationMode = 1,
    wifiSetupMode = 2,
    storageMode = 3
};

extern RunMode runMode;

constexpr auto DECIMALS_SG = 4;
constexpr auto DECIMALS_PLATO = 2;
constexpr auto DECIMALS_TEMP = 2;
constexpr auto DECIMALS_RUNTIME = 2;
constexpr auto DECIMALS_TILT = 3;
constexpr auto DECIMALS_PRESSURE = 3;
constexpr auto DECIMALS_BATTERY = 2;

#endif  // SRC_MAIN_HPP_
