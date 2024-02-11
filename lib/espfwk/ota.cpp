/*
MIT License

Copyright (c) 2021-2024 Magnus

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
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266httpUpdate.h>
#else
#include <HTTPUpdate.h>
#include <WiFi.h>
#define ESPhttpUpdate httpUpdate
#endif

#include <espframework.hpp>
#include <log.hpp>
#include <ota.hpp>

OtaUpdate::OtaUpdate(OtaConfig *cfg, String ver) {
  _otaConfig = cfg;
  _curVer = ver;
}

bool OtaUpdate::updateFirmware() {
  if (!_newFirmware) {
    Log.notice(F("OTA : No newer version exist, skipping update." CR));
    return false;
  }
#if LOG_LEVEL == 6
  Log.verbose(F("OTA : Updating firmware." CR));
#endif

  WiFiClient wifi;
  WiFiClientSecure wifiSecure;
  HTTPUpdateResult ret;
  String serverPath = _otaConfig->getOtaURL();
  serverPath += "firmware.bin";

  if (serverPath.startsWith("https://")) {
    wifiSecure.setInsecure();
    Log.notice(F("OTA: OTA, SSL enabled without validation." CR));
    ret = ESPhttpUpdate.update(wifiSecure, serverPath);
  } else {
    ret = ESPhttpUpdate.update(wifi, serverPath);
  }

  switch (ret) {
    case HTTP_UPDATE_FAILED: {
      Log.error(F("OTA : OTA update failed %d" CR),
                ESPhttpUpdate.getLastError());
    } break;
    case HTTP_UPDATE_NO_UPDATES:
      break;
    case HTTP_UPDATE_OK: {
      Log.notice(F("OTA : OTA Update sucesfull, rebooting." CR));
      delay(100);
      ESP_RESET();
      break;
    }
  }

  return false;
}

void OtaUpdate::downloadFile(HTTPClient &http, String &fname) {
  int httpResponseCode = http.GET();

  if (httpResponseCode == 200) {
    File f = LittleFS.open(fname, "w");
    http.writeToStream(&f);
    f.close();
    Log.notice(F("OTA : Downloaded file %s." CR), fname.c_str());
  } else {
    Log.error(F("OTA : Failed to download file %s" CR), httpResponseCode);
  }
}

bool OtaUpdate::checkFirmwareVersion() {
  if (!_otaConfig->isOtaActive()) {
    Log.verbose(F("OTA : No OTA url defined, skipping." CR));
    return false;
  }

#if LOG_LEVEL == 6
  Log.verbose(F("OTA : Checking if new version exist, current ver=%s." CR),
              _curVer.c_str());
#endif
  WiFiClient wifi;
  WiFiClientSecure wifiSecure;
  HTTPClient http;
  String serverPath = _otaConfig->getOtaURL();
  serverPath += "version.json";

  // Your Domain name with URL path or IP address with path
  if (_otaConfig->isOtaSSL()) {
    wifiSecure.setInsecure();
    Log.notice(F("OTA : OTA, SSL enabled without validation." CR));
    http.begin(wifiSecure, serverPath);
  } else {
    http.begin(wifi, serverPath);
  }

  // Send HTTP GET request
  DynamicJsonDocument ver(300);
  int httpResponseCode = http.GET();

  if (httpResponseCode == 200) {
    Log.notice(F("OTA : Found version.json, response=%d" CR), httpResponseCode);

    String payload = http.getString();
#if LOG_LEVEL == 6
    Log.verbose(F("OTA : Payload %s." CR), payload.c_str());
#endif
    DeserializationError err = deserializeJson(ver, payload);
    if (err) {
      Log.error(F("OTA : Failed to parse version.json" CR));
    } else {
#if LOG_LEVEL == 6
      Log.verbose(F("OTA : Project %s version %s." CR),
                  (const char *)ver["project"], (const char *)ver["version"]);
#endif
      int newVer[3];
      int curVer[3];

      if (parseFirmwareVersionString(newVer, (const char *)ver["version"])) {
        if (parseFirmwareVersionString(curVer, _curVer.c_str())) {
#if LOG_LEVEL == 6
          Log.verbose(F("OTA : OTA checking new=%d.%d.%d cur=%d.%d.%d" CR),
                      newVer[0], newVer[1], newVer[2], curVer[0], curVer[1],
                      curVer[2]);
#endif
          // Compare major version
          if (newVer[0] > curVer[0]) _newFirmware = true;
          // Compare minor version
          else if (newVer[0] == curVer[0] && newVer[1] > curVer[1])
            _newFirmware = true;
          // Compare patch version
          else if (newVer[0] == curVer[0] && newVer[1] == curVer[1] &&
                   newVer[2] > curVer[2])
            _newFirmware = true;
        }
      }

      // Download new html files to filesystem if they are present.
      if (!ver["files"].isNull() && _newFirmware) {
        Log.notice(F("OTA : OTA checking if files should be downloaded." CR));
        JsonArray htmlFiles = ver["files"].as<JsonArray>();
        for (JsonVariant v : htmlFiles) {
          String s = v;
#if LOG_LEVEL == 6
          Log.verbose(F("OTA : Dowloading file %s" CR), s.c_str());
#endif
          downloadFile(http, s);
        }
      }
    }
  } else {
    Log.error(F("OTA : OTA error checking version.json, response=%d" CR),
              httpResponseCode);
  }
  http.end();

#if LOG_LEVEL == 6
  Log.verbose(F("OTA : New version %s." CR),
              _newFirmware ? "found" : "NOT found");
#endif

  return _newFirmware;
}

bool OtaUpdate::parseFirmwareVersionString(int (&num)[3], const char *version) {
#if LOG_LEVEL == 6
  Log.verbose(F("OTA : Parsing version number string %s." CR), version);
#endif
  char temp[80];
  char *s;
  char *p = &temp[0];
  int i = 0;

  snprintf(temp, sizeof(temp), "%s", version);

  while ((s = strtok_r(p, ".", &p)) != NULL) {
    num[i++] = atoi(s);
  }

  return true;
}

// EOF
