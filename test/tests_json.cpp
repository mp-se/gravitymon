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

#include <ArduinoJson.h>

test(json_FormattingString) {
  String out;
  DynamicJsonDocument doc(500);

  doc["key-string"] = "string-1";
  serializeJson(doc, out);

  assertEqual(out, "{\"key-string\":\"string-1\"}");
}

test(json_FormattingString2) {
  String in = "{\"key-string\":\"string-1\"}";
  DynamicJsonDocument doc(500);

  deserializeJson(doc, in);
  String val = doc["key-string"];
  assertEqual(val, "string-1");
}

test(json_FormattingFloat) {
  String out;
  DynamicJsonDocument doc(500);

  doc["key-float"] = 1.1234;
  serializeJson(doc, out);

  assertEqual(out, "{\"key-float\":1.1234}");
}

test(json_FormattingFloat2) {
  String in = "{\"key-float\":1.1234}";
  DynamicJsonDocument doc(500);

  deserializeJson(doc, in);
  float val = doc["key-float"];
  float val2 = 1.1234;
  assertEqual(val, val2);
}

// EOF
