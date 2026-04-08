/*
  Copyright (c) 2021-2026 Magnus
  https://github.com/mp-se/gravitymon

  Licensed under the GNU General Public License v3.0 (GPL v3) for open source use,
  or a Commercial License for proprietary use. See LICENSE and LICENSE_COMMERCIAL.
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
