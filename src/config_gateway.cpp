/*
MIT License

Copyright (c) 2021-2025 Magnus

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
#if defined(GATEWAY)

#include <config.hpp>
#include <main.hpp>

GravmonGatewayConfig::GravmonGatewayConfig(String baseMDNS, String fileName)
    : BrewingConfig(baseMDNS, fileName) {}

void GravmonGatewayConfig::createJson(JsonObject& doc) {
  // Call base class functions
  createJsonBase(doc);
  createJsonWifi(doc);
  createJsonPush(doc);

  doc[CONFIG_GRAVITY_FORMAT] = String(getGravityFormat());
  doc[CONFIG_TIMEZONE] = getTimezone();
  doc[CONFIG_BLE_ACTIVE_SCAN] = getBleActiveScan();
  doc[CONFIG_BLE_SCAN_TIME] = getBleScanTime();
  doc[CONFIG_PUSH_RESEND_TIME] = getPushResendTime();
}

void GravmonGatewayConfig::parseJson(JsonObject& doc) {
  // Call base class functions
  parseJsonBase(doc);
  parseJsonWifi(doc);
  parseJsonPush(doc);

  if (!doc[CONFIG_GRAVITY_FORMAT].isNull()) {
    String s = doc[CONFIG_GRAVITY_FORMAT];
    setGravityFormat(s.charAt(0));
  }

  if (!doc[CONFIG_TIMEZONE].isNull()) setTimezone(doc[CONFIG_TIMEZONE]);
  if (!doc[CONFIG_BLE_ACTIVE_SCAN].isNull())
    setBleActiveScan(doc[CONFIG_BLE_ACTIVE_SCAN].as<bool>());
  if (!doc[CONFIG_BLE_SCAN_TIME].isNull())
    setBleScanTime(doc[CONFIG_BLE_SCAN_TIME].as<int>());
  if (!doc[CONFIG_PUSH_RESEND_TIME].isNull())
    setPushResendTime(doc[CONFIG_PUSH_RESEND_TIME].as<int>());
}

#endif  // GATEWAY

// EOF
