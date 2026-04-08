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

// EOF
