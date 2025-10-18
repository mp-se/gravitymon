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
#include <log.hpp>
#include <tempsensor.hpp>

void TempSensor::setup(int pin) {
  _onewire.reset(new OneWire(pin));
  _sensors.reset(new DallasTemperature(_onewire.get()));

#if LOG_LEVEL == 6
  Log.verbose(F("TSEN: Looking for temp sensors." CR));
#endif
  _sensors->begin();

  if (_sensors->getDS18Count()) {
    Log.notice(F("TSEN: Found %d temperature sensor(s). Using %d bit" CR),
               _sensors->getDS18Count(),
               _tempSensorConfig->getTempSensorResolution());
  } else {
    Log.warning(F("TSEN: No temp sensors found" CR));
  }

  // Set the temp sensor adjustment values
  _tempSensorAdjC = _tempSensorConfig->getTempSensorAdjC();

#if LOG_LEVEL == 6
  Log.verbose(F("TSEN: Adjustment values for temp sensor %F C." CR),
              _tempSensorAdjC);
#endif
}

void TempSensor::readSensor(bool useGyro) {
  if (useGyro) {
    // When using the gyro temperature only the first read value will be
    // accurate so we will use this for processing.
    if (_secondary) {
      _temperatureC = _secondary->getInitialSensorTempC();
      _hasSensor = true;
    } else {
      _temperatureC = INVALID_TEMPERATURE;
      _hasSensor = false;
    }
#if LOG_LEVEL == 6
    Log.verbose(F("TSEN: Reciving temp value for gyro sensor %F C." CR),
                _temperatureC);
#endif
    return;
  }

  // If we dont have sensors just return 0
  if (!_sensors->getDS18Count()) {
#if LOG_LEVEL == 6
    Log.notice(F("TSEN: No temperature sensors found. Skipping read." CR));
#endif
    _temperatureC = INVALID_TEMPERATURE;
    return;
  }

  // Read the sensors
  _sensors->setResolution(_tempSensorConfig->getTempSensorResolution());
  _sensors->requestTemperatures();

  if (_sensors->getDS18Count() >= 1) {
    _temperatureC = _sensors->getTempCByIndex(0);
    _hasSensor = true;

#if LOG_LEVEL == 6
    Log.verbose(F("TSEN: Reciving temp value for DS18B20 sensor %F C." CR),
                _temperatureC);
#endif
  }
}

// EOF
