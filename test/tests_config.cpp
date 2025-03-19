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

GravmonConfig myConfig("test", "test.cfg");

test(config_defaultValues) {
  assertEqual(myConfig.getGravityFormat(), 'G');
  assertEqual(myConfig.getTempFormat(), 'C');
  assertEqual(myConfig.getSleepInterval(), 900);
  assertEqual(myConfig.getTempSensorAdjC(), 0.0);
  float f = 4.15;
  assertEqual(myConfig.getVoltageConfig(), f);
}

test(config_advDefaultValues) {
  assertEqual(myConfig.getDefaultCalibrationTemp(), 20.0);
  assertEqual(myConfig.getGyroReadCount(), 50);
  assertEqual(myConfig.getGyroReadDelay(), 3150);
  assertEqual(myConfig.getGyroSensorMovingThreashold(), 500);
  assertEqual(myConfig.getMaxFormulaCreationDeviation(), 3.0);
  assertEqual(myConfig.getPushIntervalPost(), 0);
  assertEqual(myConfig.getPushIntervalPost2(), 0);
  assertEqual(myConfig.getPushIntervalGet(), 0);
  assertEqual(myConfig.getPushIntervalMqtt(), 0);
  assertEqual(myConfig.getPushIntervalInflux(), 0);
  assertEqual(myConfig.getPushTimeout(), 10);
  assertEqual(myConfig.getTempSensorResolution(), 9);
  assertEqual(myConfig.getWifiConnectionTimeout(), 20);
  assertEqual(myConfig.getWifiPortalTimeout(), 120);
  assertEqual(myConfig.isIgnoreLowAnges(), false);
}

test(config_tempFormat) {
  myConfig.setTempFormat('F');
  assertEqual(myConfig.getTempFormat(), 'F');
  myConfig.setTempFormat('C');
  assertEqual(myConfig.getTempFormat(), 'C');
  myConfig.setTempFormat('X');
  assertEqual(myConfig.getTempFormat(), 'C');
}

test(config_gravityFormat) {
  myConfig.setGravityFormat('P');
  assertEqual(myConfig.getGravityFormat(), 'P');
  myConfig.setGravityFormat('G');
  assertEqual(myConfig.getGravityFormat(), 'G');
  myConfig.setGravityFormat('X');
  assertEqual(myConfig.getGravityFormat(), 'G');
}

// EOF
