/*
  Copyright (c) 2021-2026 Magnus
  https://github.com/mp-se/gravitymon

  Licensed under the GNU General Public License v3.0 (GPL v3) for open source use,
  or a Commercial License for proprietary use. See LICENSE and LICENSE_COMMERCIAL.
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
