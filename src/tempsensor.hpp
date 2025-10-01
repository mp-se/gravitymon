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
#ifndef SRC_TEMPSENSOR_HPP_
#define SRC_TEMPSENSOR_HPP_

#include <Arduino.h>
#include <DallasTemperature.h>
#include <OneWire.h>

#include <memory>

constexpr auto INVALID_TEMPERATURE = -273.0f;

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

 public:
  explicit TempSensor(TempSensorConfigInterface *tempSensorConfig,
                      SecondayTempSensorInterface *secondary = nullptr)
      : _tempSensorConfig(tempSensorConfig), _secondary(secondary) {}

  void setup(int pin);
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
