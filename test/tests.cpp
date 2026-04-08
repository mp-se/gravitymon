/*
  Copyright (c) 2021-2026 Magnus
  https://github.com/mp-se/gravitymon

  Licensed under the GNU General Public License v3.0 (GPL v3) for open source use,
  or a Commercial License for proprietary use. See LICENSE and LICENSE_COMMERCIAL.
*/


#include <Arduino.h>
#include <main.hpp>
#include <helper.hpp>
#include <AUnit.h>
#include <config_gravitymon.hpp>

using aunit::Printer;
using aunit::TestRunner;
using aunit::Verbosity;

GravitymonConfig myConfig("test", "test.cfg");
GyroSensor myGyro(&myConfig);
BatteryVoltage myBatteryVoltage(&myConfig);
TempSensor myTempSensor(&myConfig, &myGyro);

void setup() {
  Serial.begin(115200);
  Serial.println("Gravitymon - Unit Test Build");

  delay(2000);
  Printer::setPrinter(&Serial);
  // TestRunner::setVerbosity(Verbosity::kAll);

  // TestRunner::exclude("battery_*");
  // TestRunner::exclude("calc_*");
  // TestRunner::exclude("ble_*");
  // TestRunner::exclude("config_*");
  // TestRunner::exclude("gyro_*");
  // TestRunner::exclude("helper_*");
  // TestRunner::exclude("json_*");
  // TestRunner::exclude("template_*");
  // TestRunner::exclude("tempsensor_*");
  // TestRunner::exclude("webserver_*");
  // TestRunner::exclude("wifi_*");
}

void loop() {
  TestRunner::run();
  delay(10);
}

// EOF
