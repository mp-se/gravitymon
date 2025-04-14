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
#include <helper.hpp>
#include <memory>
#include <pressure_analog.hpp>

std::unique_ptr<ADS1115_WE> AnalogPressureSensor::_adcSensor;

constexpr auto XIDIBEI_ANALOG_CALIBRATION_COUNT = 10;

bool AnalogPressureSensor::setup(float minV, float maxV, float minKpa,
                                 float maxKpa, int adcChannel, TwoWire *wire,
                                 uint8_t idx) {
  _pressureCorrection = myConfig.getPressureSensorCorrection(idx);

  _idx = idx;
  _minV = minV;
  _maxV = maxV;
  _minKpa = minKpa;
  _maxKpa = maxKpa;
  _adcChannel = adcChannel;

  if (_adcSensor == nullptr) {
    _adcSensor.reset(new ADS1115_WE(wire, ADC_I2C_ADDRESS));
    _sensorActive = _adcSensor->init();
    adcSetup();

    Log.notice(F("PRES: Analog (ADC) sensor initialized = %s, pressure "
                 "correction = %F (%d)" CR),
               _sensorActive ? "true" : "false", _pressureCorrection, _idx);
  } else {
    Log.notice(F("PRES: Analog (ADC) sensor initialized = true, pressure "
                 "correction = %F (%d)" CR),
               _pressureCorrection, _idx);
  }

  return _sensorActive;
}

void AnalogPressureSensor::adcSetup() {
  /* Set the voltage range of the ADC to adjust the gain
   * Please note that you must not apply more than VDD + 0.3V to the input pins!
   *
   * ADS1115_RANGE_6144  ->  +/- 6144 mV
   * ADS1115_RANGE_4096  ->  +/- 4096 mV
   * ADS1115_RANGE_2048  ->  +/- 2048 mV (default)
   * ADS1115_RANGE_1024  ->  +/- 1024 mV
   * ADS1115_RANGE_0512  ->  +/- 512 mV
   * ADS1115_RANGE_0256  ->  +/- 256 mV
   */
  _adcSensor->setVoltageRange_mV(
      ADS1115_RANGE_2048);  // comment line/change parameter to change range

  /* Set the inputs to be compared
   *
   *  ADS1115_COMP_0_1    ->  compares 0 with 1 (default)
   *  ADS1115_COMP_0_3    ->  compares 0 with 3
   *  ADS1115_COMP_1_3    ->  compares 1 with 3
   *  ADS1115_COMP_2_3    ->  compares 2 with 3
   *  ADS1115_COMP_0_GND  ->  compares 0 with GND
   *  ADS1115_COMP_1_GND  ->  compares 1 with GND
   *  ADS1115_COMP_2_GND  ->  compares 2 with GND
   *  ADS1115_COMP_3_GND  ->  compares 3 with GND
   */
  _adcSensor->setCompareChannels(
      ADS1115_COMP_0_GND);  // comment line/change parameter to change channel

  /* Set number of conversions after which the alert pin asserts
   * - or you can disable the alert
   *
   *  ADS1115_ASSERT_AFTER_1  -> after 1 conversion
   *  ADS1115_ASSERT_AFTER_2  -> after 2 conversions
   *  ADS1115_ASSERT_AFTER_4  -> after 4 conversions
   *  ADS1115_DISABLE_ALERT   -> disable comparator / alert pin (default)
   */
  // _adcSensor->setAlertPinMode(ADS1115_ASSERT_AFTER_1); //uncomment if you
  // want to change the default

  /* Set the conversion rate in SPS (samples per second)
   * Options should be self-explaining:
   *
   *  ADS1115_8_SPS
   *  ADS1115_16_SPS
   *  ADS1115_32_SPS
   *  ADS1115_64_SPS
   *  ADS1115_128_SPS (default)
   *  ADS1115_250_SPS
   *  ADS1115_475_SPS
   *  ADS1115_860_SPS
   */
  _adcSensor->setConvRate(
      ADS1115_64_SPS);  // uncomment if you want to change the default

  /* Set continuous or single shot mode:
   *
   *  ADS1115_CONTINUOUS  ->  continuous mode
   *  ADS1115_SINGLE     ->  single shot mode (default)
   */
  _adcSensor->setMeasureMode(
      ADS1115_CONTINUOUS);  // comment line/change parameter to change mode
}

void AnalogPressureSensor::selectChannel() {
  switch (_adcChannel) {
    case 0:
      _adcSensor->setCompareChannels(ADS1115_COMP_0_GND);
      break;

    case 1:
      _adcSensor->setCompareChannels(ADS1115_COMP_1_GND);
      break;

    case 2:
      _adcSensor->setCompareChannels(ADS1115_COMP_2_GND);
      break;

    case 3:
      _adcSensor->setCompareChannels(ADS1115_COMP_3_GND);
      break;
  }
}

void AnalogPressureSensor::calibrate() {
  Log.notice(F("PRES: Starting auto calibration (%d)." CR), _idx);
  float zero = 0;

  for (int i = 0; i < XIDIBEI_ANALOG_CALIBRATION_COUNT; i++) {
    read();
    float f = getPressurePsi(false);
    Log.notice(F("PRES: Step %d, Pressure = %F (%d)." CR), i + 1, f, _idx);
    zero += f;
    delay(500);
  }

  Log.notice(F("PRES: Measured difference %F (%d)." CR),
             zero / XIDIBEI_ANALOG_CALIBRATION_COUNT, _idx);
  myConfig.setPressureSensorCorrection(
      -(zero / XIDIBEI_ANALOG_CALIBRATION_COUNT), _idx);
  myConfig.saveFile();
  _pressureCorrection = myConfig.getPressureSensorCorrection(_idx);
}

float AnalogPressureSensor::getTemperatureC() { return NAN; }

float AnalogPressureSensor::getPressurePsi(bool doCorrection) {
  if (doCorrection) return _pressure + _pressureCorrection;
  return _pressure;
}

float mapFloat(float x, float in_min, float in_max, float out_min,
               float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

bool AnalogPressureSensor::read(bool validate) {
  // Returns temperature in C and pressure in kPa
  selectChannel();
  _voltage = _adcSensor->getResult_mV();

  float pressure =
      mapFloat(_voltage, _minV * 1000, _maxV * 1000, _minKpa, _maxKpa);
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

  return true;
}

#endif  // PRESSUREMON

// EOF
