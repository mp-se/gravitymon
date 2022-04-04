/*
MIT License

Copyright (c) 2021-22 Magnus

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
#else  // defined (ESP32)
#include <HTTPUpdate.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#endif
#include <incbin.h>

#include <config.hpp>
#include <main.hpp>
#include <wifi.hpp>

// Settings for DRD
#define ESP_DRD_USE_LITTLEFS true
#define ESP_DRD_USE_SPIFFS false
#define ESP_DRD_USE_EEPROM false
#include <ESP_DoubleResetDetector.h>
#define DRD_TIMEOUT 3
#define DRD_ADDRESS 0

// Settings for WIFI Manager
#define USE_ESP_WIFIMANAGER_NTP false
#define USE_CLOUDFLARE_NTP false
#define USING_CORS_FEATURE false
#define NUM_WIFI_CREDENTIALS 1
#define USE_STATIC_IP_CONFIG_IN_CP false
#define _WIFIMGR_LOGLEVEL_ 3
#include <ESP_WiFiManager.h>
// Override the look and feel of the standard ui (hide secondary forms)
const char WM_HTTP_FORM_START[] PROGMEM =
    "<form method='get' "
    "action='wifisave'><fieldset><div><label>SSID</label><input id='s' "
    "name='s' length=32 "
    "placeholder='SSID'><div></div></div><div><label>Password</label><input "
    "id='p' name='p' length=64 placeholder='password'><div></div></div><div "
    "hidden><label>SSID1</label><input id='s1' name='s1' length=32 "
    "placeholder='SSID1'><div></div></div><div "
    "hidden><label>Password</label><input id='p1' name='p1' length=64 "
    "placeholder='password1'><div></div></div></fieldset>";
ESP_WiFiManager *myWifiManager;
DoubleResetDetector *myDRD;

WifiConnection myWifi;

const char *userSSID = USER_SSID;
const char *userPWD = USER_SSID_PWD;

//
// Initialize
//
void WifiConnection::init() {
  myDRD = new DoubleResetDetector(DRD_TIMEOUT, DRD_ADDRESS);
}

//
// Check if we have a valid wifi configuration
//
bool WifiConnection::hasConfig() {
  if (strlen(myConfig.getWifiSSID())) return true;
  if (strlen(userSSID)) return true;

  // Check if there are stored WIFI Settings we can use.
  String ssid = WiFi.SSID();
  if (ssid.length()) {
    Log.notice(F("WIFI: Found credentials in EEPORM." CR));
    myConfig.setWifiSSID(WiFi.SSID());
    myConfig.setWifiPass(WiFi.psk());
    myConfig.saveFile();
    return true;
  }
  return false;
}

//
// Check if the wifi is connected
//
bool WifiConnection::isConnected() { return WiFi.status() == WL_CONNECTED; }

//
// Get the IP adress
//
String WifiConnection::getIPAddress() { return WiFi.localIP().toString(); }

//
// Additional method to detect double reset.
//
bool WifiConnection::isDoubleResetDetected() {
  if (strlen(userSSID))
    return false;  // Ignore this if we have hardcoded settings.
  return myDRD->detectDoubleReset();
}

//
// Stop double reset detection
//
void WifiConnection::stopDoubleReset() { myDRD->stop(); }

//
// Start the wifi manager
//
void WifiConnection::startPortal() {
  Log.notice(F("WIFI: Starting Wifi config portal." CR));

  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  myWifiManager = new ESP_WiFiManager(WIFI_MDNS);
  myWifiManager->setMinimumSignalQuality(-1);
  myWifiManager->setConfigPortalChannel(0);
  myWifiManager->setConfigPortalTimeout(
      myHardwareConfig.getWifiPortalTimeout());

  String mdns("<p>Default mDNS name is: http://");
  mdns += myConfig.getMDNS();
  mdns += ".local<p>";
  ESP_WMParameter deviceName(mdns.c_str());
  myWifiManager->addParameter(&deviceName);

  if (myWifiManager->startConfigPortal(WIFI_DEFAULT_SSID, WIFI_DEFAULT_PWD)) {
    Log.notice(F("WIFI: Exited portal, connected to wifi. Rebooting..." CR));
    myConfig.setWifiSSID(myWifiManager->getSSID());
    myConfig.setWifiPass(myWifiManager->getPW());
    myConfig.saveFile();
  } else {
    Log.notice(
        F("WIFI: Exited portal, no connection to wifi. Rebooting..." CR));
  }

  stopDoubleReset();
  delay(500);
  ESP_RESET();
}

//
// Call the wifi manager in loop
//
void WifiConnection::loop() { myDRD->loop(); }

//
// Connect to last known access point, non blocking mode.
//
void WifiConnection::connectAsync() {
  WiFi.persistent(true);
  WiFi.mode(WIFI_STA);
  if (strlen(userSSID)) {
    Log.notice(F("WIFI: Connecting to wifi using hardcoded settings %s." CR),
               userSSID);
    WiFi.begin(userSSID, userPWD);
  } else {
    Log.notice(F("WIFI: Connecting to wifi using stored settings %s." CR),
               myConfig.getWifiSSID());
    WiFi.begin(myConfig.getWifiSSID(), myConfig.getWifiPass());
  }
}

//
// Blocks until wifi connection has been found
//
bool WifiConnection::waitForConnection(int maxTime) {
#if DEBUG_LEVEL == 6
  WiFi.printDiag(Serial);
#endif
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);

    if (i % 10) Serial.print(".");

    if (i++ >
        (maxTime * 10)) {  // Try for maxTime seconds. Since delay is 100ms.
      ErrorFileLog errLog;
      errLog.addEntry("WIFI: Failed to connect to wifi " +
                      String(WiFi.status()));
      WiFi.disconnect();
      Serial.print(CR);
      return false;  // Return to main that we have failed to connect.
    }
  }
  Serial.print(CR);
  Log.notice(F("WIFI: Connected to wifi ip=%s." CR), getIPAddress().c_str());
  Log.notice(F("WIFI: Using mDNS name %s." CR), myConfig.getMDNS());
  return true;
}

//
// Connect to last known access point, blocking mode.
//
bool WifiConnection::connect() {
  connectAsync();
  return waitForConnection(20);  // 20 seconds.
}

//
// This will erase the stored credentials and forcing the WIFI manager to AP
// mode.
//
bool WifiConnection::disconnect() {
  Log.notice(F("WIFI: Erasing stored WIFI credentials." CR));
  // Erase WIFI credentials
  return WiFi.disconnect(true);
}

#if defined(ACTIVATE_OTA)

//
//
//
bool WifiConnection::updateFirmware() {
  if (!_newFirmware) {
    Log.notice(F("WIFI: No newer version exist, skipping update." CR));
    return false;
  }
#if LOG_LEVEL == 6
  Log.verbose(F("WIFI: Updating firmware." CR));
#endif

  WiFiClient wifi;
  WiFiClientSecure wifiSecure;
  HTTPUpdateResult ret;
  String serverPath = myConfig.getOtaURL();
#if defined(ESP8266)
  serverPath += "firmware.bin";
#else  // defined (ESP32)
  serverPath += "firmware32.bin";
#endif

  if (serverPath.startsWith("https://")) {
    wifiSecure.setInsecure();
    Log.notice(F("WIFI: OTA, SSL enabled without validation." CR));
    ret = ESPhttpUpdate.update(wifiSecure, serverPath);
  } else {
    ret = ESPhttpUpdate.update(wifi, serverPath);
  }

  switch (ret) {
    case HTTP_UPDATE_FAILED: {
      ErrorFileLog errLog;
      errLog.addEntry("WIFI: OTA update failed " +
                      String(ESPhttpUpdate.getLastError()));
    } break;
    case HTTP_UPDATE_NO_UPDATES:
      break;
    case HTTP_UPDATE_OK: {
      Log.notice("WIFI: OTA Update sucesfull, rebooting.");
      delay(100);
      ESP_RESET();
      break;
    }
  }

  return false;
}

//
// Download and save file
//
void WifiConnection::downloadFile(HTTPClient &http, String &fname) {
#if LOG_LEVEL == 6 && !defined(WIFI_DISABLE_LOGGING)
  Log.verbose(F("WIFI: Download file %s." CR), fname);
#endif
  int httpResponseCode = http.GET();

  if (httpResponseCode == 200) {
    File f = LittleFS.open(fname, "w");
    http.writeToStream(&f);
    f.close();
    Log.notice(F("WIFI: Downloaded file %s." CR), fname.c_str());
  } else {
    ErrorFileLog errLog;
    errLog.addEntry("WIFI: Failed to download html-file " +
                    String(httpResponseCode));
  }
}

//
// Check what firmware version is available over OTA
//
bool WifiConnection::checkFirmwareVersion() {
#if LOG_LEVEL == 6 && !defined(WIFI_DISABLE_LOGGING)
  Log.verbose(F("WIFI: Checking if new version exist." CR));
#endif
  WiFiClient wifi;
  WiFiClientSecure wifiSecure;
  HTTPClient http;
  String serverPath = myConfig.getOtaURL();
  serverPath += "version.json";

  // Your Domain name with URL path or IP address with path
  if (myConfig.isOtaSSL()) {
    wifiSecure.setInsecure();
    Log.notice(F("WIFI: OTA, SSL enabled without validation." CR));
    http.begin(wifiSecure, serverPath);
  } else {
    http.begin(wifi, serverPath);
  }

  // Send HTTP GET request
  DynamicJsonDocument ver(300);
  int httpResponseCode = http.GET();

  if (httpResponseCode == 200) {
    Log.notice(F("WIFI: Found version.json, response=%d" CR), httpResponseCode);

    String payload = http.getString();
#if LOG_LEVEL == 6 && !defined(WIFI_DISABLE_LOGGING)
    Log.verbose(F("WIFI: Payload %s." CR), payload.c_str());
#endif
    DeserializationError err = deserializeJson(ver, payload);
    if (err) {
      ErrorFileLog errLog;
      errLog.addEntry(F("WIFI: Failed to parse version.json"));
    } else {
#if LOG_LEVEL == 6 && !defined(WIFI_DISABLE_LOGGING)
      Log.verbose(F("WIFI: Project %s version %s." CR),
                  (const char *)ver["project"], (const char *)ver["version"]);
#endif
      int newVer[3];
      int curVer[3];

      if (parseFirmwareVersionString(newVer, (const char *)ver["version"])) {
        if (parseFirmwareVersionString(curVer, CFG_APPVER)) {
#if LOG_LEVEL == 6 && !defined(WIFI_DISABLE_LOGGING)
          Log.verbose(F("WIFI: OTA checking new=%d.%d.%d cur=%d.%d.%d" CR),
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
      if (!ver["html"].isNull() && _newFirmware) {
        Log.notice(
            F("WIFI: OTA checking if html files should be downloaded." CR));
        JsonArray htmlFiles = ver["html"].as<JsonArray>();
        for (JsonVariant v : htmlFiles) {
          String s = v;
#if LOG_LEVEL == 6
          Log.verbose(F("WIFI: OTA listed html file %s" CR), s.c_str());
#endif
          downloadFile(http, s);
        }
      }
    }
  } else {
    Log.error(F("WIFI: OTA error checking version.json, response=%d" CR),
              httpResponseCode);
  }
  http.end();

#if LOG_LEVEL == 6
  Log.verbose(F("WIFI: OTA found new version %s." CR),
              _newFirmware ? "true" : "false");
#endif

  return _newFirmware;
}

//
// Parse a version string in the format M.m.p (eg. 1.2.10)
//
bool WifiConnection::parseFirmwareVersionString(int (&num)[3],
                                                const char *version) {
#if LOG_LEVEL == 6 && !defined(WIFI_DISABLE_LOGGING)
  Log.verbose(F("WIFI: Parsing version number string %s." CR), version);
#endif
  char temp[80];
  char *s;
  char *p = &temp[0];
  int i = 0;

  // strcpy(&temp[0], version);
  snprintf(&temp[0], sizeof(temp), "%s", version);

  while ((s = strtok_r(p, ".", &p)) != NULL) {
    num[i++] = atoi(s);
  }

  return true;
}

#endif  // ACTIVATE_OTA

// EOF
