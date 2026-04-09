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


#ifndef SRC_BATTERY_HPP_
#define SRC_BATTERY_HPP_

#include <Arduino.h>

enum BatteryType {
  LiPo = 0,
  LithiumIon = 1  // 18650 battery
};

class BatteryConfigInterface {
 public:
  virtual float getVoltageFactor() const = 0;
  virtual BatteryType getBatteryType() const = 0;
};

class BatteryVoltage {
 private:
  float _batteryVoltage = 0;
  int _pin;
  BatteryConfigInterface *_batteryConfig = nullptr;

 public:
  explicit BatteryVoltage(BatteryConfigInterface *batteryConfig, int pin = -1);
  void read();
  float getVoltage() const { return _batteryVoltage; }
};

float getBatteryPercentage(float value, BatteryType type);

extern BatteryVoltage myBatteryVoltage;

#endif  // SRC_BATTERY_HPP_

// EOF
