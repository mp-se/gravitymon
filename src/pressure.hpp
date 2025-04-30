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

#include <log.hpp>
#include <memory>

enum PressureSensorType {
  SensorNone = 0,

  SensorXidibeiXDB401_I2C_KPa_200 = 1,    // 0-0.2 MPa
  SensorXidibeiXDB401_I2C_KPa_400 = 2,    // 0-0.4 MPa
  SensorXidibeiXDB401_I2C_KPa_500 = 3,    // 0-0.5 MPa
  SensorXidibeiXDB401_I2C_KPa_600 = 4,    // 0-0.6 MPa
  SensorXidibeiXDB401_I2C_KPa_800 = 5,    // 0-0.8 MPa
  SensorXidibeiXDB401_I2C_KPa_1000 = 6,   // 0-1 MPa
  SensorXidibeiXDB401_I2C_KPa_1200 = 7,   // 0-1.2 MPa
  SensorXidibeiXDB401_I2C_KPa_1500 = 8,   // 0-1.5 MPa
  SensorXidibeiXDB401_I2C_KPa_1600 = 9,   // 0-1.6 MPa
  SensorXidibeiXDB401_I2C_KPa_2000 = 10,  // 0-2 MPa
  SensorXidibeiXDB401_I2C_KPa_2500 = 11,  // 0-2.5 MPa
  SensorXidibeiXDB401_I2C_KPa_3000 = 12,  // 0-3 MPa
  SensorXidibeiXDB401_I2C_KPa_3500 = 13,  // 0-3.5 MPa
  SensorXidibeiXDB401_I2C_KPa_4000 = 14,  // 0-4 MPa

  SensorXidibeiXDB401_Analog_KPa_200 = 101,   // 0-0.2 MPa
  SensorXidibeiXDB401_Analog_KPa_400 = 102,   // 0-0.4 MPa
  SensorXidibeiXDB401_Analog_KPa_500 = 103,   // 0-0.5 MPa
  SensorXidibeiXDB401_Analog_KPa_600 = 104,   // 0-0.6 MPa
  SensorXidibeiXDB401_Analog_KPa_800 = 105,   // 0-0.8 MPa
  SensorXidibeiXDB401_Analog_KPa_1000 = 106,  // 0-1 MPa
  SensorXidibeiXDB401_Analog_KPa_1200 = 107,  // 0-1.2 MPa
  SensorXidibeiXDB401_Analog_KPa_1500 = 108,  // 0-1.5 MPa
  SensorXidibeiXDB401_Analog_KPa_1600 = 109,  // 0-1.6 MPa
  SensorXidibeiXDB401_Analog_KPa_2000 = 110,  // 0-2 MPa
  SensorXidibeiXDB401_Analog_KPa_2500 = 111,  // 0-2.5 MPa
  SensorXidibeiXDB401_Analog_KPa_3000 = 112,  // 0-3 MPa
  SensorXidibeiXDB401_Analog_KPa_3500 = 113,  // 0-3.5 MPa
  SensorXidibeiXDB401_Analog_KPa_4000 = 114,  // 0-4 MPa
};

// Inteface towards config class for pressure sensor related settings
class PressureConfigInterface {
 public:
  virtual bool isPressureBar() const = 0;
  virtual bool isPressureKpa() const;
  virtual bool isPressurePsi() const;

  virtual const char* getPressureUnit() const;
  virtual char getTempUnit() const;

  virtual PressureSensorType getPressureSensorType(int idx) const;
};

class PressureSensorInterface {
 protected:
  PressureConfigInterface* _pressureConfig;

 public:
  explicit PressureSensorInterface(PressureConfigInterface* pressureConfig) {
    _pressureConfig = pressureConfig;
  }

  virtual bool read(bool validate = true) = 0;
  virtual bool isActive();
  virtual float getPressurePsi(bool doCorrection = true);
  virtual float getTemperatureC();
  virtual void calibrate();
  virtual float getAnalogVoltage();
};

class PressureSensor {
 private:
  PressureConfigInterface* _pressureConfig;
  std::unique_ptr<PressureSensorInterface> _impl;

  int _idx = 0;

 public:
  explicit PressureSensor(PressureConfigInterface* pressureConfig) {
    _pressureConfig = pressureConfig;
  }

  void setup(uint8_t idx, TwoWire* wire);

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

extern PressureSensor myPressureSensor;
extern PressureSensor myPressureSensor1;

#endif  // PRESSUREMON

#endif  // SRC_PRESSURE_HPP_

// EOF
