/*
  Copyright (c) 2021-2026 Magnus
  https://github.com/mp-se/gravitymon

  Licensed under the GNU General Public License v3.0 (GPL v3) for open source use,
  or a Commercial License for proprietary use. See LICENSE and LICENSE_COMMERCIAL.
*/


#include <AUnit.h>

#include <ArduinoJson.h>

test(json_FormattingString) {
  String out;
  JsonDocument doc;

  doc["key-string"] = "string-1";
  serializeJson(doc, out);

  assertEqual(out, "{\"key-string\":\"string-1\"}");
}

test(json_FormattingString2) {
  String in = "{\"key-string\":\"string-1\"}";
  JsonDocument doc;

  deserializeJson(doc, in);
  String val = doc["key-string"];
  assertEqual(val, "string-1");
}

test(json_FormattingFloat) {
  String out;
  JsonDocument doc;

  doc["key-float"] = 1.1234;
  serializeJson(doc, out);

  assertEqual(out, "{\"key-float\":1.1234}");
}

test(json_FormattingFloat2) {
  String in = "{\"key-float\":1.1234}";
  JsonDocument doc;

  deserializeJson(doc, in);
  float val = doc["key-float"];
  float val2 = 1.1234;
  assertEqual(val, val2);
}

// EOF
