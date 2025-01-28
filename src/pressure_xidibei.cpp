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

#include <config.hpp>
#include <pressure_xidibei.hpp>

bool XIDIBEIPressureSensor::setup(float maxPressure, TwoWire *wire,
                                  uint8_t idx) {
  _pressureCorrection = myConfig.getPressureSensorCorrection(idx);
  _temperatureCorrection = myConfig.getTemperatureSensorCorrection(idx);
  _idx = idx;
  _xidibeiSensor.reset(new XIDIBEI(maxPressure, wire));
  _sensorActive = _xidibeiSensor->begin();
  Log.notice(
      F("PRES: XIDIBEI sensor initialized = %s, max pressure = %F, pressure "
        "correction = %F, temperature correction = %F (%d)" CR),
      _sensorActive ? "true" : "false", maxPressure, _pressureCorrection,
      _temperatureCorrection, _idx);
  return _sensorActive;
}

void XIDIBEIPressureSensor::calibrateSensor() {
  Log.notice(F("PRES: Starting auto calibration (%d)." CR), _idx);
  float zero = 0;

  for (int i = 0; i < 10; i++) {
    readSensor();
    float f = getPressurePsi(false);
    Log.notice(F("PRES: Step %d, Pressure = %F (%d)." CR), i + 1, f, _idx);
    zero += f;
    delay(500);
  }

  Log.notice(F("PRES: Measured difference %F (%d)." CR), zero / 10, _idx);
  myConfig.setPressureSensorCorrection(zero / 10, _idx);
  myConfig.saveFile();
  _pressureCorrection = myConfig.getPressureSensorCorrection(_idx);
}

float XIDIBEIPressureSensor::getTemperatureC() {
  return _temperature + _temperatureCorrection;
}

float XIDIBEIPressureSensor::getPressurePsi(bool doCorrection) {
  if (doCorrection) return _pressure - _pressureCorrection;
  return _pressure;
}

bool XIDIBEIPressureSensor::readSensor() {
  float pressure;

  // Returns temperature in C and pressure in kPa
  bool b = _xidibeiSensor->readSensor(_temperature, pressure);
  _pressure = convertPaPressureToPsi(pressure * 1000);
  return b;
}

#endif  // PRESSUREMON

// EOF
