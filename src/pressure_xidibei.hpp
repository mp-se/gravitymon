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
#ifndef SRC_PRESSURE_XIDIBEI_HPP_
#define SRC_PRESSURE_XIDIBEI_HPP_

#if defined(PRESSUREMON)

#include <XIDIBEI.hpp>
#include <memory>
#include <pressure.hpp>

class XIDIBEIPressureSensor : public PressureSensorInterface {
 private:
  std::unique_ptr<XIDIBEI> _xidibeiSensor;
  uint8_t _idx;
  float _pressureCorrection = 0, _temperatureCorrection = 0, _maxPressure;
  float _pressure, _temperature;
  bool _sensorActive = false;

 public:
  XIDIBEIPressureSensor() {}

  bool setup(float maxPressure, TwoWire *wire, uint8_t idx);
  bool read();
  bool isActive() { return _sensorActive; }

  float getPressurePsi(bool doCorrection = true);
  float getTemperatureC();

  void calibrate();

  float getAnalogVoltage() { return NAN; }
};

#endif  // PRESSUREMON

#endif  // SRC_PRESSURE_XIDIBEI_HPP_

// EOF
