/*
MIT License

Copyright (c) 2025 Magnus

This code is based on application note from vendor with adaption to Arduino.

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

#include <XIDIBEI.hpp>
#include <log.hpp>

// Max pressure is in kPA
XIDIBEI::XIDIBEI(uint16_t maxPressure, TwoWire *wire) {
  _wire = wire;
  _maxPressure = maxPressure;
}

bool XIDIBEI::begin() {
  _wire->begin();
  _wire->beginTransmission(XIDIBEI_I2C_ADDRESS);
  uint8_t ret = _wire->endTransmission();
  Log.verbose(F("XIDI: Checking senor at %x returned %d." CR),
              XIDIBEI_I2C_ADDRESS, ret);
  return ret != 0 ? false : true;
}

bool XIDIBEI::read(float &pressure, float &temperature) {
  pressure = NAN;
  temperature = NAN;

  _wire->beginTransmission(XIDIBEI_I2C_ADDRESS);
  _wire->write(0x30);
  _wire->write(0x0A);  // Accuire pressure and temperature data
  if (_wire->endTransmission() != 0) {
    Log.error(F("XIDI: Failed to start accuire process." CR));
    return false;
  }

  // Option 1: Check when conversion is done
  /*
  uint8_t status;
  uint32_t timeout = millis() + 50;

  do {
    if (millis() > timeout) {
      Log.error(F("XIDI: Timeout while waiting for accuire to finish." CR));
      return false;
    }

    delay(5);

    _wire->beginTransmission(XIDIBEI_I2C_ADDRESS);
    _wire->write(0x30);

    if (_wire->endTransmission() != 0) {
      Log.error(F("XIDI: Failed to check status." CR));
      return false;
    }

    if (_wire->requestFrom(XIDIBEI_I2C_ADDRESS, 1) != 1) {
      Log.error(F("XIDI: Failed to start status read." CR));
      return false;
    }

    status = _wire->read();
  } while ((status & 0x08));  // Bit 3 should be off for conversion to be
  completed*/

  // Option 2: Wait for conversion to complete, simple approach
  delay(50);

  _wire->beginTransmission(XIDIBEI_I2C_ADDRESS);
  _wire->write(0x06);

  if (_wire->endTransmission() != 0) {
    Log.error(F("XIDI: Failed to init data transfer." CR));
    return false;
  }

  if (_wire->requestFrom(XIDIBEI_I2C_ADDRESS, 5) != 5) {
    Log.error(F("XIDI: Failed to start data read." CR));
    return false;
  }

  uint8_t pressureData[3], temperatureData[2];

  pressureData[0] = _wire->read();  // High bits
  pressureData[1] = _wire->read();
  pressureData[2] = _wire->read();  // Low bits

  temperatureData[0] = _wire->read();  // High bits
  temperatureData[1] = _wire->read();  // Low bits

  int32_t rawPressure = static_cast<uint32_t>(pressureData[0]) << 16 |
                        static_cast<uint32_t>(pressureData[1]) << 8 |
                        static_cast<uint32_t>(pressureData[2]);
  int16_t rawTemperature = static_cast<uint16_t>(temperatureData[0]) << 8 |
                           static_cast<uint16_t>(temperatureData[1]);

  Log.verbose(F("Raw Pressure: %x, Raw Temperature %x." CR), rawPressure,
              rawTemperature);

  // Convert from 24bit signed to 32bit signed
  rawPressure =
      rawPressure & 0x00800000 ? rawPressure |= 0xFF000000 : rawPressure;

  pressure = static_cast<float>(rawPressure) / 0x800000;
  temperature = static_cast<float>(rawTemperature) / 0x100;

  pressure = pressure * _maxPressure;

  Log.verbose(F("XIDI: Pressure: %F, Temperature %F." CR), pressure,
              temperature);
  return true;
}

#endif  // PRESSUREMON

// EOF
