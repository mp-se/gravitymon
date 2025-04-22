/*
MIT License

Copyright (c) 2022-2024 Magnus

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
#include <AUnit.h>

#include <config.hpp>

#if defined(GRAVITYMON)
GravitymonConfig myConfig("test", "test.cfg");
#elif defined(PRESSUREMON)
PressuremonConfig myConfig("test", "test.cfg");
#elif defined(GATEWAY)
GravmonGatewayConfig myConfig("test", "test.cfg");
#endif


test(config_defaultValues) {
  float f = 4.15;
  assertEqual(myConfig.getVoltageConfig(), f);
  f = 1.59;
  assertEqual(myConfig.getVoltageFactor(), f);
  assertEqual(myConfig.getVoltagePin(), PIN_VOLT);

  assertEqual(myConfig.getTempFormat(), 'C');
  assertEqual(myConfig.getTempSensorAdjC(), 0.0);
  assertEqual(myConfig.getSleepInterval(), 900);
  assertEqual(myConfig.getTempSensorResolution(), 9);

  assertEqual(myConfig.getPushIntervalPost(), 0);
  assertEqual(myConfig.getPushIntervalPost2(), 0);
  assertEqual(myConfig.getPushIntervalGet(), 0);
  assertEqual(myConfig.getPushIntervalMqtt(), 0);
  assertEqual(myConfig.getPushIntervalInflux(), 0);
  assertEqual(myConfig.getPushTimeout(), 10);

  assertEqual(myConfig.getToken(), "");
  assertEqual(myConfig.getToken2(), "");

  assertEqual(myConfig.isWifiDirect(), false);
  assertEqual(myConfig.getWifiConnectionTimeout(), 30);
  assertEqual(myConfig.getWifiPortalTimeout(), 120);
}

test(config_tempFormat) {
  myConfig.setTempFormat('F');
  assertEqual(myConfig.getTempFormat(), 'F');
  myConfig.setTempFormat('C');
  assertEqual(myConfig.getTempFormat(), 'C');
  myConfig.setTempFormat('X');
  assertEqual(myConfig.getTempFormat(), 'C');
}

test(config_tempSensorResolution) {
  myConfig.setTempSensorResolution(9); 
  assertEqual(myConfig.getTempSensorResolution(), 9);
  myConfig.setTempSensorResolution(8); 
  assertEqual(myConfig.getTempSensorResolution(), 9); 

  myConfig.setTempSensorResolution(12);
  assertEqual(myConfig.getTempSensorResolution(), 12);
  myConfig.setTempSensorResolution(13);
  assertEqual(myConfig.getTempSensorResolution(), 12);
}

#if defined(GRAVITYMON) || defined(PRESSUREMON)
test(config_batterySaving) {
  myConfig.setBatterySaving(false);
  assertEqual(myConfig.isBatterySaving(), false);
  myConfig.setBatterySaving(true);
  assertEqual(myConfig.isBatterySaving(), true);
}
#endif

#if defined(GRAVITYMON)
test(config_gravitymonValues) {
  assertEqual(myConfig.getDefaultCalibrationTemp(), 20.0);
  assertEqual(myConfig.getGyroReadCount(), 50);
  assertEqual(myConfig.getGyroReadDelay(), 3150);
  assertEqual(myConfig.getGyroSensorMovingThreashold(), 500);
  assertEqual(myConfig.getGravityUnit(), 'G');
  assertEqual(myConfig.isIgnoreLowAnges(), false);
  assertEqual(myConfig.isBatterySaving(), true);
  assertEqual(myConfig.isGyroTemp(), false);
  assertEqual(myConfig.isStorageSleep(), false);
  assertEqual(myConfig.isGyroDisabled(), false);
  assertEqual(myConfig.isGravityTempAdj(), false);
}

test(config_gravityFormat) {
  myConfig.setGravityUnit('P');
  assertEqual(myConfig.getGravityUnit(), 'P');
  myConfig.setGravityUnit('G');
  assertEqual(myConfig.getGravityUnit(), 'G');
  myConfig.setGravityUnit('X');
  assertEqual(myConfig.getGravityUnit(), 'G');
}

test(config_gyroDisabled) {
  myConfig.setGyroDisabled(true);
  assertEqual(myConfig.isGyroDisabled(), true);
  myConfig.setGyroDisabled(false);
  assertEqual(myConfig.isGyroDisabled(), false);
}

test(config_gravityFormula) {
  assertEqual(myConfig.getGravityFormula(), "");
  myConfig.setGravityFormula("2+2^2");
  assertEqual(myConfig.getGravityFormula(), "2+2^2");
}

test(config_tiltColor) {
  assertEqual(myConfig.getBleTiltColor(), "");

  myConfig.setBleTiltColor("Blue");
  assertEqual(myConfig.getBleTiltColor(), "Blue");

  myConfig.setBleTiltColor("");
  assertEqual(myConfig.getBleTiltColor(), "");
}

test(config_formulaDeviation) {
  float f = 0.01;
  assertEqual(myConfig.getMaxFormulaCreationDeviation(), f);

  myConfig.setMaxFormulaCreationDeviation(0.05);
  f = 0.05;
  assertEqual(myConfig.getMaxFormulaCreationDeviation(), f);

  myConfig.setMaxFormulaCreationDeviation(0.1);
  f = 0.1;
  assertEqual(myConfig.getMaxFormulaCreationDeviation(), f);
}

#endif // GRAVITYMON

// EOF
