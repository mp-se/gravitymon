/*
  Copyright (c) 2021-2026 Magnus
  https://github.com/mp-se/gravitymon

  Licensed under the GNU General Public License v3.0 (GPL v3) for open source use,
  or a Commercial License for proprietary use. See LICENSE and LICENSE_COMMERCIAL.
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
