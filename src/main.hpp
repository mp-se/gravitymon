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
