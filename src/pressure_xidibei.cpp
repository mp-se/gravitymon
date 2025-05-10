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
#if defined(PRESSUREMON)

#include <helper.hpp>
#include <pressure_xidibei.hpp>

constexpr auto XIDIBEI_IIC_CALIBRATION_COUNT = 5;

bool XIDIBEIPressureSensor::setup(float maxPressure, TwoWire *wire,
                                  uint8_t idx) {
  _pressureCorrection = _pressureConfig->getPressureSensorCorrection(idx);
  _temperatureCorrection = _pressureConfig->getTemperatureSensorCorrection(idx);
  _maxPressure = maxPressure;
  _idx = idx;
  _xidibeiSensor.reset(new XIDIBEI(_maxPressure, wire));
  _sensorActive = _xidibeiSensor->begin();
  Log.notice(
      F("PRES: XIDIBEI sensor initialized = %s, max pressure = %F, pressure "
        "correction = %F, temperature correction = %F (%d)" CR),
      _sensorActive ? "true" : "false", _maxPressure, _pressureCorrection,
      _temperatureCorrection, _idx);
  return _sensorActive;
}

void XIDIBEIPressureSensor::calibrate() {
  Log.notice(F("PRES: Starting auto calibration (%d)." CR), _idx);
  float zero = 0;

  for (int i = 0; i < XIDIBEI_IIC_CALIBRATION_COUNT; i++) {
    read(false);
    float f = getPressurePsi(false);
    zero += f;
    Log.notice(F("PRES: Step %d, Pressure = %F, sum %F (%d)." CR), i + 1, f,
               zero, _idx);
    delay(500);
  }

  Log.notice(F("PRES: Measured difference, ave %F, sum %F (%d)." CR),
             zero / XIDIBEI_IIC_CALIBRATION_COUNT, zero, _idx);
  _pressureConfig->setPressureSensorCorrection(
      -(zero / XIDIBEI_IIC_CALIBRATION_COUNT), _idx);
  _pressureConfig->saveFile();
  _pressureCorrection = _pressureConfig->getPressureSensorCorrection(_idx);
}

float XIDIBEIPressureSensor::getTemperatureC() {
  return _temperature + _temperatureCorrection;
}

float XIDIBEIPressureSensor::getPressurePsi(bool doCorrection) {
  if (doCorrection) return _pressure + _pressureCorrection;
  return _pressure;
}

bool XIDIBEIPressureSensor::read(bool validate) {
  float pressure;

  // Returns temperature in C and pressure in kPa
  bool b = _xidibeiSensor->read(pressure, _temperature);
  _pressure = convertPaPressureToPsi(pressure * 1000);

  if (validate) {
    if (_pressure > _maxPressure) {
      Log.warning(
          F("PRES: Read pressure is invalid and out of range %F (%d)." CR),
          _pressure, _idx);
      _pressure = NAN;
      return false;
    }
  }

  return b;
}

#endif  // PRESSUREMON

// EOF
