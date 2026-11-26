/*
  Copyright (c) 2021-2026 Magnus
  https://github.com/mp-se/gravitymon

  Licensed under the GNU General Public License v3.0 (GPL v3) for open source use,
  or a Commercial License for proprietary use. See LICENSE and LICENSE_COMMERCIAL.
*/


#include <AUnit.h>

#include <gyro.hpp>
#include <config_gravitymon.hpp>

extern GravitymonConfig myConfig;
extern GyroSensor myGyro;

test(gyro_connectGyro) {
  myGyro.setup(GyroMode::GYRO_CONTINUOUS, true);
  assertEqual(myGyro.isConnected(), true);
}

test(gyro_readGyro) { 
  myGyro.setup(GyroMode::GYRO_CONTINUOUS, true);
  assertEqual(myGyro.read(), true);
}

test(gyro_readGyroTemp) {
  myGyro.setup(GyroMode::GYRO_CONTINUOUS, true);
  float f = NAN;
  assertNotEqual(myGyro.getInitialSensorTempC(), f);
  assertNotEqual(myGyro.getSensorTempC(), f);
}

test(gyro_tiltLegacy) {
  // Flat on the table, Z up
  assertNear(calculateTiltAngle(0, 0, 1, false, nullptr), 90.0, 0.01);
  // Standing up, Y is the axis
  assertNear(calculateTiltAngle(0, 1, 0, false, nullptr), 0.0, 0.01);
  // 45 degrees
  assertNear(calculateTiltAngle(0, 0.7071, 0.7071, false, nullptr), 45.0, 0.01);
  // Swapped, X is the axis
  assertNear(calculateTiltAngle(1, 0, 0, true, nullptr), 0.0, 0.01);
  assertNear(calculateTiltAngle(1, 0, 0, false, nullptr), 90.0, 0.01);
  // No data
  assertNear(calculateTiltAngle(0, 0, 0, false, nullptr), 0.0, 0.01);
}

test(gyro_tiltAxisCalibration) {
  // Axis equal to Y and no bias should match the legacy calculation
  GyroAxisData ident = {0, 1, 0, 0, 0, 0};
  assertNear(calculateTiltAngle(0, 0.7071, 0.7071, false, &ident), 45.0, 0.01);
  assertNear(calculateTiltAngle(0.3, 0.5, 0.8, false, &ident),
             calculateTiltAngle(0.3, 0.5, 0.8, false, nullptr), 0.001);

  // Bias is removed before the angle is calculated
  GyroAxisData bias = {0, 1, 0, 0.03, -0.02, 0.05};
  assertNear(calculateTiltAngle(0.03, 0.7071 - 0.02, 0.7071 + 0.05, false,
                                &bias),
             45.0, 0.01);

  // Sensor mounted 2.5 degrees off around X, the case axis is then
  // (0, cos(2.5), sin(2.5)) in sensor coordinates. A case tilted 40 degrees
  // gives the same angle regardless of how its rolled.
  GyroAxisData axis = {0, 0.99905, 0.04362, 0, 0, 0};
  // roll 0:   gravity is 40 degrees from the axis towards +Z
  assertNear(calculateTiltAngle(0, 0.73728, 0.67559, false, &axis), 40.0, 0.02);
  // roll 180: gravity is 40 degrees from the axis towards -Z
  assertNear(calculateTiltAngle(0, 0.79335, -0.60876, false, &axis), 40.0,
             0.02);
  // Without the calibration the two readings are 5 degrees apart
  assertNear(calculateTiltAngle(0, 0.73728, 0.67559, false, nullptr), 42.5,
             0.02);
  assertNear(calculateTiltAngle(0, 0.79335, -0.60876, false, nullptr), 37.5,
             0.02);
}

test(gyro_axisCalibrationConfig) {
  GyroAxisData saved = myConfig.getGyroAxisCalibration();

  myConfig.setGyroAxisCalibration({0, 0, 0, 0, 0, 0});
  assertEqual(myConfig.hasGyroAxisCalibration(), false);

  JsonDocument doc;
  JsonObject obj = doc.to<JsonObject>();
  obj[CONFIG_GYRO_AXIS_CALIBRATION]["nx"] = 0.01;
  obj[CONFIG_GYRO_AXIS_CALIBRATION]["ny"] = 0.999;
  // Incomplete data should be ignored
  myConfig.parseJson(obj);
  assertEqual(myConfig.hasGyroAxisCalibration(), false);

  obj[CONFIG_GYRO_AXIS_CALIBRATION]["nz"] = 0.04;
  obj[CONFIG_GYRO_AXIS_CALIBRATION]["bx"] = 0.03;
  obj[CONFIG_GYRO_AXIS_CALIBRATION]["by"] = -0.02;
  obj[CONFIG_GYRO_AXIS_CALIBRATION]["bz"] = 0.05;
  myConfig.parseJson(obj);
  assertEqual(myConfig.hasGyroAxisCalibration(), true);
  assertNear(myConfig.getGyroAxisCalibration().ny, 0.999, 0.0001);
  assertNear(myConfig.getGyroAxisCalibration().by, -0.02, 0.0001);

  JsonDocument doc2;
  JsonObject obj2 = doc2.to<JsonObject>();
  myConfig.createJson(obj2);
  assertNear(obj2[CONFIG_GYRO_AXIS_CALIBRATION]["bz"].as<float>(), 0.05,
             0.0001);

  myConfig.setGyroAxisCalibration(saved);
}

// EOF
