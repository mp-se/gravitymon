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
#ifndef SRC_HARDWARE_SCAN_HPP_
#define SRC_HARDWARE_SCAN_HPP_

#include <ArduinoJson.h>

// Hardware scan, shared by the web api (/api/hardware) and the hardware test.
// Does not depend on the web server or wifi.

constexpr auto PARAM_I2C = "i2c";
constexpr auto PARAM_ADRESS = "adress";
constexpr auto PARAM_BUS = "bus";
constexpr auto PARAM_RESOLUTION = "resolution";
constexpr auto PARAM_FAMILY = "family";
constexpr auto PARAM_CHIP = "chip";
constexpr auto PARAM_REVISION = "revision";
constexpr auto PARAM_CORES = "cores";
constexpr auto PARAM_FEATURES = "features";

// Scans the i2c bus (initialized in gyro.cpp) and reads the chip information.
void scanHardwareCommon(JsonObject &obj);

#if defined(GRAVITYMON)
// Scans onewire pins (ESP8266), the DS18B20 sensor(s) and the gyro.
// classifySensor=false skips the DS18B20 family classification, that uses
// undocumented commands and can change the calibration of clone sensors.
void scanHardwareGravitymon(JsonObject &obj, bool classifySensor = true);
#endif

#endif  // SRC_HARDWARE_SCAN_HPP_

// EOF
