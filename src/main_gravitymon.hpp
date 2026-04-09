/*
 * GravityMon
 * Copyright (c) 2021-2026 Magnus
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Alternatively, this software may be used under the terms of a
 * commercial license. See LICENSE_COMMERCIAL for details.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */


#ifndef SRC_MAIN_GRAVITYMON_HPP_
#define SRC_MAIN_GRAVITYMON_HPP_

#if defined(GRAVITYMON)

#include <Arduino.h>

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
#define CFG_PCB "iSpindel"

// #elif defined(WEMOS_D1_MINI32) // Support removed for this board
// // Hardware config for ESP32-d1-mini, iSpindel hardware
// // ------------------------------------------------------
// #define PIN_SDA D3
// #define PIN_SCL D4
// #define PIN_DS D6
// #define PIN_CFG1 D8
// #define PIN_CFG2 D7
// #define PIN_VOLT PIN_A0
// #define CFG_FILENAMEBIN "firmware32.bin"

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
#define PIN_CHARGING A1  // Needs to be an Analog and RTC connected pin
#define CFG_FILENAMEBIN "firmware32c3.bin"
#define CFG_PCB "iSpindel"

#elif defined(LOLIN_C3_PICO)
// Hardware config for ESP32-c3-pico, iSpindel hardware
// ------------------------------------------------------
#define PIN_SDA 7
#define PIN_SCL 6
#define PIN_DS A0
#define PIN_VOLT A3
#define PIN_CFG1 A5
#define PIN_CFG2 A4
#define PIN_CHARGING 10  // Needs to be an Analog and RTC connected pin
#define CFG_FILENAMEBIN "firmware32c3pico.bin"
#define CFG_PCB "iSpindel"

#elif defined(WAVESHARE_ESP32C3_ZERO)
// Hardware config for ESP32-c3-zero, iSpindel hardware
// ------------------------------------------------------
// IO10 is connected to RBG LED
// A2. IO8, IO9 are strapping pins
#define PIN_SDA IO6
#define PIN_SCL IO7
#define PIN_DS IO5
#define PIN_VOLT A0
#define PIN_CFG1 A1
#define PIN_CFG2 A3
#define PIN_CHARGING A4  // Needs to be an Analog and RTC connected pin
#define CFG_FILENAMEBIN "firmware32c3zero.bin"
#define CFG_PCB "Gravitymon"

#elif defined(LOLIN_S2_MINI)
// Hardware config for ESP32-s2-mini, iSpindel hardware
// ------------------------------------------------------
#define PIN_SDA A17
#define PIN_SCL A15
#define PIN_CFG1 A11
#define PIN_CFG2 A10
#define PIN_DS A8
#define PIN_VOLT A2
#define PIN_CHARGING A6  // Needs to be an Analog and RTC connected pin
#define CFG_FILENAMEBIN "firmware32s2.bin"
#define CFG_PCB "iSpindel"

#elif defined(LOLIN_S3_MINI)
// Hardware config for ESP32-s3-mini, iSpindel hardware
// ------------------------------------------------------
#define PIN_SDA A17
#define PIN_SCL A15
#define PIN_CFG1 A10
#define PIN_CFG2 A9
#define PIN_DS A12
#define PIN_VOLT A1
#define PIN_CHARGING A11  // Needs to be an Analog and RTC connected pin
#define CFG_FILENAMEBIN "firmware32s3.bin"
#define CFG_PCB "iSpindel"

#elif defined(OLIMEX_ESP32C3_DEVKIT_LIPO)
// Hardware config for ESP32C3 from Olimex with build in Charger
// (ESP32-C3-DevKit-Lipo)
// ------------------------------------------------------
#define PIN_SCL IO0
#define PIN_SDA IO1
#define PIN_DS IO5  // implemetation with DS18 not tested
#define PIN_VOLT IO3
#define PIN_CFG1 IO4
#define PIN_CFG2 IO20
#define CFG_FILENAMEBIN "custom-olimex_esp32c3_devkit_lipo.bin"
#define CFG_PCB ""

#elif defined(TENSTAR_ESP32C3_SUPER_MINI)
// Hardware config for SUPER_MINI_ESP32C3, iSpindel hardware
// ------------------------------------------------------
// IO8 is LED pin
// A2, IO8, IO9 is strapping pin
#define PIN_SDA IO7
#define PIN_SCL IO6
#define PIN_DS A0
#define PIN_DS2 A5
#define PIN_VOLT A3
#define PIN_CHARGING A4  // Needs to be an Analog and RTC connected pin
#define PIN_CFG1 IO10
#define PIN_CFG2 A1
#define CFG_FILENAMEBIN "firmware32c3supermini.bin"
#define CFG_PCB ""

#else
#warning "Unknown board type"
#endif

#if !defined(ESP8266) && !defined(ESP32) && !defined(ESP32C2) && \
    !defined(ESP32S3) && !defined(ESP32S2)
#error "Target CPU is not defined or unknown!"
#endif

#if !defined(PIN_DS2)
#define PIN_DS2 -1
#endif

#endif  // GRAVITYMON

#endif  // SRC_MAIN_GRAVITYMON_HPP_
