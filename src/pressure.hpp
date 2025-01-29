/*
MIT License

Copyright (c) 2025 Magnus

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
#ifndef SRC_PRESSURE_HPP_
#define SRC_PRESSURE_HPP_

#if defined(PRESSUREMON)

#include <Arduino.h>
#include <Wire.h>

#include <config.hpp>
#include <log.hpp>
#include <memory>

class PressureSensorInterface {
 public:
  PressureSensorInterface() = default;

  virtual bool read();

  virtual bool isActive();

  virtual float getPressurePsi(bool doCorrection = true);
  virtual float getTemperatureC();

  virtual void calibrate();

  virtual float getAnalogVoltage();
};

class PressureSensor {
 private:
  std::unique_ptr<PressureSensorInterface> _impl;
  int _idx = 0;

 public:
  void setup(uint8_t idx, TwoWire *wire);

  bool read();
  bool isActive();

  float getPressurePsi(bool doCorrection = true);
  float getTemperatureC();
  float getTemperatureF() { return (getTemperatureC() * 1.8) + 32.0; }

  void calibrate();

  float getPressure(
      bool doCorrection = true);  // Returns in chosen device format
  float getTemperature();         // Returns in chosen device format

  float getAnalogVoltage();
};

extern PressureSensor myPressureSensor[MAX_SENSOR_DEVICES];

#endif  // PRESSUREMON

#endif  // SRC_PRESSURE_HPP_

// EOF
