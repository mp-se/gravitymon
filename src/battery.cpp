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
#include <algorithm>
#include <battery.hpp>
#include <cmath>
#include <log.hpp>

// #define SIMULATE_VOLTAGE 3.9

BatteryVoltage::BatteryVoltage(BatteryConfigInterface *batteryConfig, int pin) {
  _batteryConfig = batteryConfig;
  _pin = pin;
#if defined(ESP8266)
  if(_pin > 0) {
    pinMode(_pin, INPUT);
  }
#else
  if(_pin > 0) {
    pinMode(_pin, INPUT);
    analogReadResolution(SOC_ADC_MAX_BITWIDTH);
    analogSetAttenuation(ADC_11db);
  }
#endif
}

void BatteryVoltage::read() {
  // The analog pin can only handle 3.3V maximum voltage so we need to reduce
  // the voltage (from max 5V)
  float factor = _batteryConfig->getVoltageFactor();  // Default value is 1.63
  int v = 0;

  if( _pin > 0) {
    v = analogRead(_pin);
  }

  // An ESP8266 has a ADC range of 0-1023 and a maximum voltage of 3.3V
  // An ESP32 has an ADC range of 0-4095 and a maximum voltage of 3.3V

  // Max input values per board (2.5V is the a good setting)
  // ESP32-S2: 2500mV
  // ESP32-S3: 3100mV
  // ESP32-C3: 2500mV
  // ESP32-C6: 3300mV

#if defined(ESP8266)
  _batteryVoltage = ((3.3 / 1023) * v) * factor;
#elif defined(ESP32S2)
  _batteryVoltage = ((2.5 / ((1 << SOC_ADC_MAX_BITWIDTH) - 1)) * v) * factor;
#elif defined(ESP32S3)
  _batteryVoltage = ((3.1 / ((1 << SOC_ADC_MAX_BITWIDTH) - 1)) * v) * factor;
#elif defined(ESP32C3)
  _batteryVoltage = ((2.5 / ((1 << SOC_ADC_MAX_BITWIDTH) - 1)) * v) * factor;
#endif

#if LOG_LEVEL == 6
  Log.verbose(
      F("BATT: Reading voltage level. Factor=%F Value=%d, Voltage=%F." CR),
      factor, v, _batteryVoltage);
#endif

#if defined(SIMULATE_VOLTAGE)
  _batteryVoltage = SIMULATE_VOLTAGE;
#endif
}

float getBatteryPercentage(float value, BatteryType type) {
  constexpr float MAX_VOLTAGE_LITHIUMION = 4.20;
  constexpr float MIN_VOLTAGE_LITHIUMION = 3.00;
  constexpr float MAX_VOLTAGE_LIPO = 4.20;
  constexpr float MIN_VOLTAGE_LIPO = 3.30;

  float percentage;

  switch (type) {
    case BatteryType::LiPo:
      if (value >= MAX_VOLTAGE_LIPO) return 100;
      if (value <= MIN_VOLTAGE_LIPO) return 0;

      percentage =
          ((value - MIN_VOLTAGE_LIPO) / (MAX_VOLTAGE_LIPO - MIN_VOLTAGE_LIPO)) *
          100.0;
      percentage = constrain(percentage, 0.0, 100.0);
      return round(percentage);

    default:
    case BatteryType::LithiumIon:
      if (value >= MAX_VOLTAGE_LITHIUMION) return 100.0f;
      if (value <= MIN_VOLTAGE_LITHIUMION) return 0.0f;

      percentage = ((value - MIN_VOLTAGE_LITHIUMION) /
                    (MAX_VOLTAGE_LITHIUMION - MIN_VOLTAGE_LITHIUMION)) *
                   100.0;
      percentage = constrain(percentage, 0.0, 100.0);
      return round(percentage);
  }

  return 0.0;
}

// EOF
