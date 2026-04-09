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


#include <math.h>

#include <log.hpp>
#include <tempsensor.hpp>

void TempSensor::setup(int pin, int pin2) {
  if (!setupSensor(pin)) {
    if (pin2 >= 0) {
      setupSensor(pin2);
    }
  }

  // Set the temp sensor adjustment values
  _tempSensorAdjC = _tempSensorConfig->getTempSensorAdjC();

#if LOG_LEVEL == 6
  Log.verbose(F("TSEN: Adjustment values for temp sensor %F C." CR),
              _tempSensorAdjC);
#endif

  _initialized = true;
}

bool TempSensor::setupSensor(int pin) {
  _onewire.reset(new OneWire(pin));
  _sensors.reset(new DallasTemperature(_onewire.get()));

#if LOG_LEVEL == 6
  Log.verbose(F("TSEN: Looking for temp sensors." CR));
#endif
  _sensors->begin();

  if (_sensors->getDS18Count()) {
    Log.notice(
        F("TSEN: Found %d temperature sensor(s). Using %d bit on pin=%d" CR),
        _sensors->getDS18Count(), _tempSensorConfig->getTempSensorResolution(),
        pin);
  } else {
    Log.warning(F("TSEN: No temp sensors found on pin=%d" CR), pin);
    return false;
  }

  return true;
}

void TempSensor::readSensor(bool useGyro) {
  if (!_initialized) {
    _temperatureC = NAN;
    return;
  }

  if (useGyro) {
    // When using the gyro temperature only the first read value will be
    // accurate so we will use this for processing.
    if (_secondary) {
      _temperatureC = _secondary->getInitialSensorTempC();
      _hasSensor = true;
    } else {
      _temperatureC = NAN;
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
    _temperatureC = NAN;
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
