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
#ifndef SRC_WIFICONNECTION_HPP_
#define SRC_WIFICONNECTION_HPP_

#if defined(ESP8266)
#include <ESP8266HTTPClient.h>
#else
#include <HTTPClient.h>
#endif
#include <DNSServer.h>
#include <LittleFS.h>
#include <WiFiUdp.h>

#include <interface.hpp>

class WifiConnection {
 private:
  String _apSSID;
  String _apPWD;
  String _apMDNS;
  String _userSSID;
  String _userPWD;
  WifiConfig* _wifiConfig;
  DNSServer* _dnsServer = NULL;

  // Double reset
  uint32_t _timer = 0;
  uint32_t _timeout = 3000;  // 3 seconds
  uint8_t _resetCounter = 0;
  const uint8_t _minResetCount = 2;

  void connectAsync(int wifiIndex);
  bool waitForConnection(int maxTime);
  void readReset();
  void writeReset();

 public:
  WifiConnection(WifiConfig* cfg, String apSSID, String apPWD, String apMDNS,
                 String userSSID = "", String userPWD = "");
  void init();
  void timeSync(String timeZone = "");

  bool connect();
  bool disconnect();
  bool isConnected();
  bool isDoubleResetDetected();
  void stopDoubleReset();
  void startWifiAP();
  bool hasConfig();
  String getIPAddress();

  void loop();
};

#endif  // SRC_WIFICONNECTION_HPP_

// EOF
