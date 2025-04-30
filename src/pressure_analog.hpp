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
#ifndef SRC_PRESSURE_ANALOG_HPP_
#define SRC_PRESSURE_ANALOG_HPP_

#if defined(PRESSUREMON)

#include <ADS1115_WE.h>

#include <memory>
#include <pressure.hpp>

constexpr auto ADC_I2C_ADDRESS = 0x48;

class AnalogPressureSensor : public PressureSensorInterface {
 private:
  static std::unique_ptr<ADS1115_WE> _adcSensor;
  uint8_t _idx;
  float _pressureCorrection = 0;
  float _pressure, _voltage, _maxPressure;
  float _minV, _maxV, _minKpa, _maxKpa;
  int _adcChannel;
  bool _sensorActive = false;

  void adcSetup();
  void selectChannel();

 public:
  explicit AnalogPressureSensor(PressureConfigInterface* pressureConfig)
      : PressureSensorInterface(pressureConfig) {}

  bool setup(float minV, float maxV, float minKpa, float maxKpa,
             int _adcChannel, TwoWire* wire, uint8_t idx);
  bool read(bool validate = true);
  bool isActive() { return _sensorActive; }

  float getPressurePsi(bool doCorrection = true);
  float getTemperatureC();

  void calibrate();

  float getAnalogVoltage() { return _voltage; }
};

#endif  // PRESSUREMON

#endif  // SRC_PRESSURE_ANALOG_HPP_

// EOF
