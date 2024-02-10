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
#else
#include <WiFi.h>
#endif

#include <espframework.hpp>
#include <log.hpp>
#include <wificonnection.hpp>

const int NTP_PACKET_SIZE =
    48;  // NTP time stamp is in the first 48 bytes of the message

const char *resetFilename = "/reset.dat";

WifiConnection::WifiConnection(WifiConfig *cfg, String apSSID, String apPWD,
                               String apMDNS, String userSSID, String userPWD) {
  _wifiConfig = cfg;
  _apSSID = apSSID;
  _apPWD = apPWD;
  _apMDNS = apMDNS;
  _userSSID = userSSID;
  _userPWD = userPWD;
}

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
  if (_userSSID.length()) return true;
  if (strlen(_wifiConfig->getWifiSSID(0))) return true;
#if defined(ESP8266)
  String ssid = WiFi.SSID();
  String pwd = WiFi.psk();
  if (ssid.length()) {
    Log.notice(F("WIFI: Found credentials in EEPORM." CR));
    _wifiConfig->setWifiSSID(ssid, 0);

    if (WiFi.psk().length()) _wifiConfig->setWifiPass(WiFi.psk(), 0);

    _wifiConfig->saveFile();
    return true;
  }
#else
#endif
  return false;
}

bool WifiConnection::isConnected() { return WiFi.status() == WL_CONNECTED; }

String WifiConnection::getIPAddress() { return WiFi.localIP().toString(); }

bool WifiConnection::isDoubleResetDetected() {
  if (_userSSID.length())
    return false;  // Ignore this if we have hardcoded settings.

  return _resetCounter > _minResetCount;
}

void WifiConnection::stopDoubleReset() {
  Log.notice(F("WIFI: Stop double reset detection." CR));
  _resetCounter = 0;
  writeReset();
}

void WifiConnection::startWifiAP() {
  IPAddress local(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);

  if (!WiFi.softAPConfig(local, local, subnet)) {
    Log.notice(F("WIFI: Failed to configure access point." CR));
    return;
  }

  if (!WiFi.softAP(_apSSID, _apPWD)) {
    Log.notice(F("WIFI: Failed to create access point." CR));
    return;
  }

  Log.notice(F("WIFI: Starting dns server." CR));
  _dnsServer = new DNSServer();
  if(_dnsServer) {
    _dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
    if(!_dnsServer->start(53, "*", local)) {
      Log.error(F("WIFI: Failed to start dns server." CR));
    }
  } else {
      Log.error(F("WIFI: Failed to create dns server." CR));
  }

  Log.notice(F("WIFI: Access point created %s." CR),
             WiFi.softAPIP().toString().c_str());
}

void WifiConnection::loop() {
  if (abs((int32_t)(millis() - _timer)) > _timeout) {
    _timer = millis();
    _resetCounter = 0;
    writeReset();
  }

  if(_dnsServer) {
    _dnsServer->processNextRequest();
  }
}

void WifiConnection::connectAsync(int wifiIndex) {
  WiFi.persistent(true);
  WiFi.mode(WIFI_STA);
#if defined(ESP32C3)
  Log.notice(F("WIFI: Reducing wifi power for c3 chip." CR));
  WiFi.setTxPower(WIFI_POWER_8_5dBm);  // Required for ESP32C3 Mini
#endif
  if (_userSSID.length()) {
    Log.notice(F("WIFI: Connecting to wifi using hardcoded settings %s." CR),
               _userSSID.c_str());
    WiFi.begin(_userSSID.c_str(), _userPWD.c_str());
  } else {
    Log.notice(F("WIFI: Connecting to wifi (%d) using stored settings %s." CR),
               wifiIndex, _wifiConfig->getWifiSSID(wifiIndex));
    WiFi.begin(_wifiConfig->getWifiSSID(wifiIndex),
               _wifiConfig->getWifiPass(wifiIndex));
  }
}

bool WifiConnection::waitForConnection(int maxTime) {
#if LOG_LEVEL == 6
  WiFi.printDiag(EspSerial);
#endif
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);

    if (i % 10) EspSerial.print(".");

    if (i++ > (maxTime * 10)) {
      Log.error(F("WIFI: Failed to connect to wifi %d" CR), WiFi.status());
      WiFi.disconnect();
      EspSerial.print(CR);
      return false;  // Return to main that we have failed to connect.
    }
  }
  EspSerial.print(CR);
  Log.notice(F("WIFI: Connected to wifi %s ip=%s." CR), WiFi.SSID().c_str(),
             getIPAddress().c_str());
  Log.notice(F("WIFI: Using mDNS name %s." CR), _wifiConfig->getMDNS());
  return true;
}

bool WifiConnection::connect() {
  int timeout = _wifiConfig->getWifiConnectionTimeout();

  connectAsync(0);
  if (!waitForConnection(timeout)) {
    Log.warning(F("WIFI: Failed to connect to first SSID %s." CR),
                _wifiConfig->getWifiSSID(0));

    if (strlen(_wifiConfig->getWifiSSID(1))) {
      connectAsync(1);

      if (waitForConnection(timeout)) {
        Log.notice(F("WIFI: Connected to second SSID %s." CR),
                   _wifiConfig->getWifiSSID(1));
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
  return WiFi.disconnect(true);  // Erase WIFI credentials
}

void WifiConnection::timeSync() {
  configTime(0 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  Log.notice(F("WIFI: Waiting for NTP sync."));
  time_t now = time(nullptr);

  while (now < 8 * 3600 * 2) {
    delay(500);
    EspSerial.print(".");
    now = time(nullptr);
  }

  EspSerial.print(CR);

  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Log.notice(F("WIFI: Current time: %s."), asctime(&timeinfo));
}

// EOF
