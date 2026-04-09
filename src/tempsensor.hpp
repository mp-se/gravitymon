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


#ifndef SRC_TEMPSENSOR_HPP_
#define SRC_TEMPSENSOR_HPP_

#include <Arduino.h>
#include <DallasTemperature.h>
#include <OneWire.h>

#include <memory>

class SecondayTempSensorInterface {
 public:
  virtual float getInitialSensorTempC() const = 0;
};

class TempSensorConfigInterface {
 public:
  virtual int getTempSensorResolution() const = 0;
  virtual float getTempSensorAdjC() const = 0;
};

class TempSensor {
 private:
  std::unique_ptr<DallasTemperature> _sensors;
  std::unique_ptr<OneWire> _onewire;
  TempSensorConfigInterface *_tempSensorConfig = nullptr;
  SecondayTempSensorInterface *_secondary = nullptr;

  bool _hasSensor = false;
  float _tempSensorAdjC = 0;
  float _temperatureC = 0;
  bool _initialized = false;

  bool setupSensor(int pin);

 public:
  explicit TempSensor(TempSensorConfigInterface *tempSensorConfig,
                      SecondayTempSensorInterface *secondary = nullptr)
      : _tempSensorConfig(tempSensorConfig), _secondary(secondary) {}

  void setup(int pin, int pin2 = -1);
  void readSensor(bool useGyro = false);
  bool isSensorAttached() { return _hasSensor; }
  float getTempC() const { return _temperatureC + _tempSensorAdjC; }

  int getSensorCount() { return _sensors ? _sensors->getDS18Count() : 0; }
  void getSensorAddress(uint8_t *deviceAddress, int index) {
    if (_sensors) {
      _sensors->getAddress(deviceAddress, index);
    }
  }
  int getSensorResolution() { return _sensors ? _sensors->getResolution() : 0; }
};

extern TempSensor myTempSensor;

#endif  // SRC_TEMPSENSOR_HPP_

// EOF
