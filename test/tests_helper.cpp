/*
  Copyright (c) 2021-2026 Magnus
  https://github.com/mp-se/gravitymon

  Licensed under the GNU General Public License v3.0 (GPL v3) for open source use,
  or a Commercial License for proprietary use. See LICENSE and LICENSE_COMMERCIAL.
*/


#include <AUnit.h>

#include <battery.hpp>
#include <utils.hpp>
#include <helper.hpp>
#include <log.hpp>
#include <config_gravitymon.hpp>

extern GravitymonConfig myConfig;

test(helper_convertToPlato) {
  double p = convertToPlato(1.008);
  char buffer[20];
  String s = convertFloatToString(p, &buffer[0], 2);
  s.trim();
  assertEqual(s, "2.06");
}

test(helper_convertToSG) {
  double p = convertToSG(2.06);
  char buffer[20];
  String s = convertFloatToString(p, &buffer[0], 3);
  s.trim();
  assertEqual(s, "1.008");
}

test(helper_convertCtoF) {
  float t = convertCtoF(20.0);
  assertEqual(t, 68.0);
}

test(helper_convertFtoC) {
  float t = convertFtoC(68.0);
  assertEqual(t, 20.0);
}

test(helper_urlEncode) {
  String s = urlencode("Hello world");
  assertEqual(s, "Hello%20world");
}

test(helper_urlDecode) {
  String s = urldecode("Hello%20world");
  assertEqual(s, "Hello world");
}

test(helper_convertFloatToString) {
  char buffer[20];
  String s = convertFloatToString(20.2, &buffer[0], 2);
  s.trim();
  assertEqual(s, "20.20");
}

test(helper_reduceFloatPrecision1) {
  float v = 20.233;
  float f = reduceFloatPrecision(v, 2);
  v = 20.23;
  assertEqual(f, v);
}

test(helper_reduceFloatPrecision2) {
  float v = 20.238;
  float f = reduceFloatPrecision(v, 2);
  v = 20.24;
  assertEqual(f, v);
}

test(helper_reduceFloatPrecision3) {
  float v = 20.23844;
  float f = reduceFloatPrecision(v, 3);
  v = 20.238;
  assertEqual(f, v);
}

test(helper_reduceFloatPrecision4) {
  float v = 20.238444;
  float f = reduceFloatPrecision(v, 4);
  v = 20.2384;
  assertEqual(f, v);
}

test(helper_readBatteryVoltage) {
  myBatteryVoltage.read();
  float f = myBatteryVoltage.getVoltage();
  assertMoreOrEqual(f, 2.0);
}

test(helper_chipRevision) { // Include for code coverage
  detectChipRevision();
  assertEqual(1,1);
}

test(helper_resetReason) { // Include for code coverage
  checkResetReason();
  assertEqual(1,1);
}

test(helper_writeErrorLog) { // Include for code coverage
  writeErrorLog("Write to error log.");
  assertEqual(1,1);
}

// EOF
