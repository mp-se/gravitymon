/*
MIT License

Copyright (c) 2022 Magnus

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

#include <battery.hpp>
#include <utils.hpp>
#include <log.hpp>
#include <history.hpp>
#include <helper.hpp>

BatteryVoltage myBatteryVoltage;

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

test(helper_floatHistory) { 
  LittleFS.begin();

  FloatHistoryLog hist("/history.log");
  hist.addEntry(1.0);
  hist.addEntry(1.0);
  float f = hist.getAverage();
  assertEqual(f, 1.0);
}

// EOF
