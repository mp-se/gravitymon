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
#else  // defined (ESP32)
#include <HTTPUpdate.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#endif

#include <config.hpp>
#include <main.hpp>
#include <wifi.hpp>
#include <wifimanager.hpp>
ESP_WiFiManager *myWifiManager = 0;
WifiConnection myWifi;

#if defined(USER_SSID)
const char *userSSID = USER_SSID;
const char *userPWD = USER_SSID_PWD;
#else
const char *userSSID = "";
const char *userPWD = "";
#endif

const char *resetFilename = "/reset.dat";

void WifiConnection::init() {
  readReset();
  Log.notice(F("WIFI: Current reset counter %u." CR), _resetCounter);
  _resetCounter++;
  writeReset();
}

void WifiConnection::readReset() {
  File file = LittleFS.open(resetFilename, "r");

  if (file) {
    file.read(reinterpret_cast<uint8_t *>(&this->_resetCounter),
              sizeof(_resetCounter));
    file.close();
  } else {
    Log.warning(F("WIFI: Failed to read reset counter." CR));
    _resetCounter = 0;
  }
}

void WifiConnection::writeReset() {
  File file = LittleFS.open(resetFilename, "w");

  if (file) {
    file.write(reinterpret_cast<uint8_t *>(&this->_resetCounter),
               sizeof(_resetCounter));
    file.close();
  } else {
    Log.warning(F("WIFI: Failed to write reset counter." CR));
    _resetCounter = 0;
  }
}

bool WifiConnection::hasConfig() {
  if (strlen(userSSID)) return true;
  if (strlen(myConfig.getWifiSSID(0))) return true;

    // Check if there are stored WIFI Settings we can use.
#if defined(ESP8266)
  String ssid = WiFi.SSID();
  String pwd = WiFi.psk();
#else
  if (myWifiManager == 0) myWifiManager = new ESP_WiFiManager(WIFI_MDNS);

  String ssid = myWifiManager->WiFi_SSID();
  String pwd = myWifiManager->WiFi_Pass();
#endif
  if (ssid.length()) {
    Log.notice(F("WIFI: Found stored credentials." CR));
    myConfig.setWifiSSID(ssid, 0);

    if (pwd.length()) myConfig.setWifiPass(pwd, 0);

    myConfig.saveFile();
    return true;
  }

  return false;
}

bool WifiConnection::isConnected() { return WiFi.status() == WL_CONNECTED; }

String WifiConnection::getIPAddress() { return WiFi.localIP().toString(); }

bool WifiConnection::isDoubleResetDetected() {
  if (strlen(userSSID))
    return false;  // Ignore this if we have hardcoded settings.

  return _resetCounter > _minResetCount;
}

void WifiConnection::stopDoubleReset() {
  Log.notice(F("WIFI: Stop double reset detection." CR));
  _resetCounter = 0;
  writeReset();
}

void WifiConnection::startPortal() {
  Log.notice(F("WIFI: Starting Wifi config portal." CR));

  stopDoubleReset();
  ledOn(LedColor::WHITE);  // White or Led ON

  if (myWifiManager == 0) myWifiManager = new ESP_WiFiManager(WIFI_MDNS);

  myWifiManager->setMinimumSignalQuality(-1);
  myWifiManager->setConfigPortalChannel(0);
  myWifiManager->setConfigPortalTimeout(myConfig.getWifiPortalTimeout());

  String mdns("<p>Default mDNS name is: http://");
  mdns += myConfig.getMDNS();
  mdns += ".local<p>";
  ESP_WMParameter deviceName(mdns.c_str());
  myWifiManager->addParameter(&deviceName);

#if defined(ESP32C3_REV1)
  Log.notice(F("WIFI: Reducing wifi power for c3 chip." CR));
  WiFi.setTxPower(WIFI_POWER_8_5dBm);  // Required for ESP32C3 Mini
#elif defined(ESP32C3)
  WiFi.setTxPower(WIFI_POWER_15dBm);
#endif

  myWifiManager->startConfigPortal(WIFI_DEFAULT_SSID, WIFI_DEFAULT_PWD);

  if (myWifiManager->getSSID(0).length()) {
    myConfig.setWifiSSID(myWifiManager->getSSID(0), 0);
    myConfig.setWifiPass(myWifiManager->getPW(0), 0);
    myConfig.setWifiSSID(myWifiManager->getSSID(1), 1);
    myConfig.setWifiPass(myWifiManager->getPW(1), 1);

    // If the same SSID has been used, lets delete the second
    if (!strcmp(myConfig.getWifiSSID(0), myConfig.getWifiSSID(1))) {
      myConfig.setWifiSSID("", 1);
      myConfig.setWifiPass("", 1);
    }

    Log.notice(F("WIFI: Stored SSID1:'%s' SSID2:'%s'" CR),
               myConfig.getWifiSSID(0), myConfig.getWifiSSID(1));
    myConfig.saveFile();
  } else {
    Log.notice(
        F("WIFI: Could not find first SSID so assuming we got a timeout." CR));
  }

  Log.notice(F("WIFI: Exited wifi config portal. Rebooting..." CR));
  ledOff();
  delay(500);
  ESP_RESET();
}

void WifiConnection::loop() {
  if (abs((int32_t)(millis() - _timer)) > _timeout) {
    _timer = millis();
    _resetCounter = 0;
    writeReset();
  }

  if(!WiFi.isConnected()) {
    if(_reconnectCounter>5) {
      Log.notice(F("WIFI: Failed to reconnect with wifi, rebooting..." CR));
      delay(500);
      ESP_RESET();
    }
    
    Log.notice(F("WIFI: Not connected, trying to reconnect %d..." CR), _reconnectCounter);
    _reconnectCounter++;
    WiFi.reconnect();
    delay(500);  
  } else {
    _reconnectCounter = 0;
  }
}

void WifiConnection::connectAsync(int wifiIndex) {
  WiFi.setHostname(myConfig.getMDNS());
  WiFi.persistent(true);
  WiFi.mode(WIFI_STA);

#if defined(ESP32C3_REV1)
  Log.notice(F("WIFI: Reducing wifi power for c3 chip." CR));
  WiFi.setTxPower(WIFI_POWER_8_5dBm);  // Required for ESP32C3 Mini
#elif defined(ESP32C3)
  WiFi.setTxPower(WIFI_POWER_13dBm);
#endif

  if (strlen(userSSID)) {
    Log.notice(F("WIFI: Connecting to wifi using hardcoded settings %s." CR),
               userSSID);
    WiFi.begin(userSSID, userPWD);
  } else {
    Log.notice(F("WIFI: Connecting to wifi (%d) using stored settings %s." CR),
               wifiIndex, myConfig.getWifiSSID(wifiIndex));
    WiFi.begin(myConfig.getWifiSSID(wifiIndex),
               myConfig.getWifiPass(wifiIndex));
  }
}

bool WifiConnection::waitForConnection(int maxTime) {
#if DEBUG_LEVEL == 6
  WiFi.printDiag(EspSerial);
#endif
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);

    if (i % 10) EspSerial.print(".");

    if (i++ >
        (maxTime * 10)) {  // Try for maxTime seconds. Since delay is 100ms.
      writeErrorLog("WIFI: Failed to connect to wifi %d", WiFi.status());
      stopDoubleReset();
      WiFi.disconnect();
      EspSerial.print(CR);
      return false;  // Return to main that we have failed to connect.
    }
  }
  EspSerial.print(CR);
  Log.notice(F("WIFI: Connected to wifi %s ip=%s, channel=%d." CR),
             WiFi.SSID().c_str(), getIPAddress().c_str(), WiFi.channel());
  Log.notice(F("WIFI: Using mDNS name %s." CR), myConfig.getMDNS());
  return true;
}

int WifiConnection::findStrongestNetwork() {
  if (!myConfig.dualWifiConfigured()) {
    Log.notice(F("WIFI: Only one wifi SSID is configured, skipping scan." CR));
    return -1;
  }

  Log.notice(F("WIFI: Scanning for wifi." CR));
  int noNetwork = WiFi.scanNetworks(false, false);
  int strenght[2] = {-100, -100};

  for (int i = 0; i < noNetwork; i++) {
    int rssi = WiFi.RSSI(i);
    String ssid = WiFi.SSID(i);

    if (ssid.compareTo(myConfig.getWifiSSID(0)))
      strenght[0] = rssi;
    else if (ssid.compareTo(myConfig.getWifiSSID(1)))
      strenght[1] = rssi;

#if LOG_LEVEL == 6
    Log.verbose(F("WIFI: Found %s %d." CR), ssid.c_str(), rssi);
#endif
  }

  if (strenght[0] == -100 && strenght[1] == -100)
    return -1;  // None of the stored networks can be seen

  if (strenght[0] >= strenght[1])
    return 0;  // First network is strongest or they have equal strength

  return 1;  // Second network is the strongest
}

#if defined(ESP8266)
String WifiConnection::getEncryptionType(uint8_t encryptionType) {
  switch (encryptionType) {
    case ENC_TYPE_NONE:
      return "None";
    case ENC_TYPE_WEP:
      return "WEP";
    case ENC_TYPE_TKIP:
      return "TKIP";
    case ENC_TYPE_CCMP:
      return "CCMP";
    case ENC_TYPE_AUTO:
      return "AUTO";
  }
  return "UNKNOWN";
}
#else
String WifiConnection::getEncryptionType(wifi_auth_mode_t encryptionType) {
  switch (encryptionType) {
    case WIFI_AUTH_OPEN:
      return "Open";
    case WIFI_AUTH_WEP:
      return "WEP";
    case WIFI_AUTH_WPA_PSK:
      return "WPA_PSK";
    case WIFI_AUTH_WPA2_PSK:
      return "WPA2_PSK";
    case WIFI_AUTH_WPA_WPA2_PSK:
      return "WPA_WPA2_PSK";
    case WIFI_AUTH_WPA2_ENTERPRISE:
      return "WPA2_ENTERPRISE";
    case WIFI_AUTH_WPA3_PSK:
      return "WPA3_PSK";
    case WIFI_AUTH_WPA2_WPA3_PSK:
      return "WPA2_WPA3_PSK";
    case WIFI_AUTH_WAPI_PSK:
      return "WPAI_PSK";
    case WIFI_AUTH_MAX:
      return "MAX";
  }

  return "UNKNOWN";
}
#endif

void WifiConnection::scanWifiNetworks() {
  Log.notice(F("WIFI: Scanning for wifi." CR));
  int noNetwork = WiFi.scanNetworks(false, false);

  for (int i = 0; i < noNetwork; i++) {
    int rssi = WiFi.RSSI(i);
    String ssid = WiFi.SSID(i);
    int channel = WiFi.channel(i);
#if defined(ESP8266)
    uint8_t encryptionType = WiFi.encryptionType(i);
#else
    wifi_auth_mode_t encryptionType = WiFi.encryptionType(i);
#endif

    Log.notice(F("WIFI: Found ssid=%s dBm=%d ch=%d %s." CR), ssid.c_str(), rssi,
               channel, getEncryptionType(encryptionType).c_str());
  }
}

bool WifiConnection::connect() {
  // scanWifiNetworks();

  /*
  // Alternative code for connecting to strongest wifi.
  // Takes approximatly 2 seconds to scan for available network
  int i = findStrongestNetwork();

  if (i != -1) {
    Log.notice(F("WIFI: Wifi %d:'%s' is strongest." CR), i,
  myConfig.getWifiSSID(i)); } else { i = 0; // Use first SSID as default.
  }

  connectAsync(i);
  return waitForConnection(myConfig.getWifiConnectTimeout());
  */

  // Alternative code for using seconday wifi settings as fallback.
  // If success to seconday is successful this is used as standard
  int timeout = myConfig.getWifiConnectTimeout();

  connectAsync(0);
  if (!waitForConnection(timeout)) {
    Log.warning(F("WIFI: Failed to connect to first SSID %s." CR),
                myConfig.getWifiSSID(0));

    if (strlen(myConfig.getWifiSSID(1))) {
      connectAsync(1);

      if (waitForConnection(timeout)) {
        Log.notice(F("WIFI: Connected to second SSID %s, making secondary "
                     "default." CR),
                   myConfig.getWifiSSID(1));

        myConfig.swapPrimaryWifi();
        myConfig.saveFile();
        return true;
      }
    }

    Log.warning(F("WIFI: Failed to connect to any SSID." CR));
    return false;
  }

  return true;
}

bool WifiConnection::disconnect() {
  Log.notice(F("WIFI: Erasing stored WIFI credentials." CR));
  // Erase WIFI credentials
  return WiFi.disconnect(true);
}

#if defined(ACTIVATE_OTA)
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
#elif defined(ESP32C3)
  serverPath += "firmware32c3.bin";
#elif defined(ESP32C3_REV1)
  serverPath += "firmware32c3v1.bin";
#elif defined(ESP32S2)
  serverPath += "firmware32s2.bin";
#elif defined(ESP32S3)
  serverPath += "firmware32s3.bin";
#elif defined(ESP32LITE)
  serverPath += "firmware32lite.bin";
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
      writeErrorLog("WIFI: OTA update failed %d", ESPhttpUpdate.getLastError());
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
    writeErrorLog("WIFI: Failed to download html-file %d", httpResponseCode);
  }
}

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
  DynamicJsonDocument ver(JSON_BUFFER_SIZE_SMALL);
  int httpResponseCode = http.GET();

  if (httpResponseCode == 200) {
    Log.notice(F("WIFI: Found version.json, response=%d" CR), httpResponseCode);

    String payload = http.getString();
#if LOG_LEVEL == 6 && !defined(WIFI_DISABLE_LOGGING)
    Log.verbose(F("WIFI: Payload %s." CR), payload.c_str());
#endif
    DeserializationError err = deserializeJson(ver, payload);
    if (err) {
      writeErrorLog("WIFI: Failed to parse version.json");
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

  if (_newFirmware) Log.notice(F("WIFI: Found new version." CR));

#if LOG_LEVEL == 6
  Log.verbose(F("WIFI: OTA found new version %s." CR),
              _newFirmware ? "true" : "false");
#endif

  return _newFirmware;
}

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
