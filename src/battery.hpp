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
#ifndef SRC_BATTERY_HPP_
#define SRC_BATTERY_HPP_

#include <Arduino.h>

enum BatteryType {
  LiPo = 0,
  LithiumIon = 1  // 18650 battery
};

class BatteryConfigInterface {
 public:
  virtual int getVoltagePin() const = 0;
  virtual float getVoltageFactor() const = 0;
  virtual BatteryType getBatteryType() const = 0;
};

class BatteryVoltage {
 private:
  float _batteryVoltage = 0;
  BatteryConfigInterface *_batteryConfig = nullptr;

 public:
  explicit BatteryVoltage(BatteryConfigInterface *batteryConfig);
  void read();
  float getVoltage() const { return _batteryVoltage; }
};

float getBatteryPercentage(float value, BatteryType type);

extern BatteryVoltage myBatteryVoltage;

#endif  // SRC_BATTERY_HPP_

// EOF
