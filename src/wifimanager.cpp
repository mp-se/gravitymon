/*
MIT License

This is based on library by Khoi Hoang
https://github.com/khoih-prog/ESP_WiFiManager Licensed under MIT license,
Version: 1.12.1

Copyright (c) 2023 Magnus

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
#include <Arduino.h>
#include <ArduinoLog.h>

#include <utility>
#include <wifimanager.hpp>

ESP_WMParameter::ESP_WMParameter(const char* custom) {
  _WMParam_data._id = NULL;
  _WMParam_data._placeholder = NULL;
  _WMParam_data._length = 0;
  _WMParam_data._value = NULL;
  _WMParam_data._labelPlacement = WFM_LABEL_BEFORE;
  _customHTML = custom;
}

ESP_WMParameter::ESP_WMParameter(const char* id, const char* placeholder,
                                 const char* defaultValue, const int& length,
                                 const char* custom,
                                 const int& labelPlacement) {
  init(id, placeholder, defaultValue, length, custom, labelPlacement);
}

ESP_WMParameter::ESP_WMParameter(const WMParam_Data& WMParam_data) {
  init(WMParam_data._id, WMParam_data._placeholder, WMParam_data._value,
       WMParam_data._length, "", WMParam_data._labelPlacement);
}

void ESP_WMParameter::init(const char* id, const char* placeholder,
                           const char* defaultValue, const int& length,
                           const char* custom, const int& labelPlacement) {
  _WMParam_data._id = id;
  _WMParam_data._placeholder = placeholder;
  _WMParam_data._length = length;
  _WMParam_data._labelPlacement = labelPlacement;
  _WMParam_data._value = new char[_WMParam_data._length + 1];

  if (_WMParam_data._value != NULL) {
    memset(_WMParam_data._value, 0, _WMParam_data._length + 1);

    if (defaultValue != NULL) {
      strncpy(_WMParam_data._value, defaultValue, _WMParam_data._length);
    }
  }

  _customHTML = custom;
}

ESP_WMParameter** ESP_WiFiManager::getParameters() { return _params; }

int ESP_WiFiManager::getParametersCount() { return _paramsCount; }

char* ESP_WiFiManager::getRFC952_hostname(const char* iHostname) {
  memset(RFC952_hostname, 0, sizeof(RFC952_hostname));

  size_t len = (RFC952_HOSTNAME_MAXLEN < strlen(iHostname))
                   ? RFC952_HOSTNAME_MAXLEN
                   : strlen(iHostname);

  size_t j = 0;

  for (size_t i = 0; i < len - 1; i++) {
    if (isalnum(iHostname[i]) || iHostname[i] == '-') {
      RFC952_hostname[j] = iHostname[i];
      j++;
    }
  }

  if (isalnum(iHostname[len - 1]) || (iHostname[len - 1] != '-'))
    RFC952_hostname[j] = iHostname[len - 1];

  return RFC952_hostname;
}

ESP_WiFiManager::ESP_WiFiManager(const char* iHostname) {
  _max_params = WIFI_MANAGER_MAX_PARAMS;
  _params = reinterpret_cast<ESP_WMParameter**>(
      malloc(_max_params * sizeof(ESP_WMParameter*)));

  if (iHostname[0] == 0) {
#ifdef ESP8266
    String _hostname = "ESP8266-" + String(ESP.getChipId(), HEX);
#else  // ESP32
    String _hostname = "ESP32-" + String(ESP_getChipId(), HEX);

#endif
    _hostname.toUpperCase();

    getRFC952_hostname(_hostname.c_str());

  } else {
    getRFC952_hostname(iHostname);
  }

  Log.notice(F("WM  : RFC925 hostname = %s" CR), &RFC952_hostname[0]);
  setHostname();
  WiFi.mode(WIFI_STA);
  networkIndices = NULL;
}

ESP_WiFiManager::~ESP_WiFiManager() {
  if (_params != NULL) {
    // Log.verbose(F("WM  : freeing allocated params!" CR));
    free(_params);
  }

  if (networkIndices) {
    free(networkIndices);
  }
}

bool ESP_WiFiManager::addParameter(ESP_WMParameter* p) {
  if (_paramsCount == _max_params) {
    // rezise the params array
    _max_params += WIFI_MANAGER_MAX_PARAMS;

    // Log.verbose(F("WM  : Increasing _max_params to: %d" CR), _max_params);

    ESP_WMParameter** new_params = reinterpret_cast<ESP_WMParameter**>(
        realloc(_params, _max_params * sizeof(ESP_WMParameter*)));

    if (new_params != NULL) {
      _params = new_params;
    } else {
      Log.error(F("WM  : failed to realloc params, size not increased!" CR));
      return false;
    }
  }

  _params[_paramsCount] = p;
  _paramsCount++;

  // Log.verbose(F("WM  : Adding parameter %s" CR), p->getID());
  return true;
}

void ESP_WiFiManager::setupConfigPortal() {
  stopConfigPortal = false;

  if (WiFi.getAutoConnect() == 0) WiFi.setAutoConnect(1);

  dnsServer.reset(new DNSServer());

#ifdef ESP8266
  server.reset(new ESP8266WebServer(80));
#else  // ESP32
  server.reset(new WebServer(80));
#endif

  if (_WiFi_AP_IPconfig._ap_static_ip) {
    Log.notice(F("WM  : Custom AP IP/GW/Subnet = %s, %s, %s" CR),
               _WiFi_AP_IPconfig._ap_static_ip.toString().c_str(),
               _WiFi_AP_IPconfig._ap_static_gw.toString().c_str(),
               _WiFi_AP_IPconfig._ap_static_sn.toString().c_str());

    WiFi.softAPConfig(_WiFi_AP_IPconfig._ap_static_ip,
                      _WiFi_AP_IPconfig._ap_static_gw,
                      _WiFi_AP_IPconfig._ap_static_sn);
  }

  if (dnsServer) {
    dnsServer->setErrorReplyCode(DNSReplyCode::NoError);

    if (!dnsServer->start(DNS_PORT, "*", WiFi.softAPIP())) {
      Log.error(F("WM  : Can't start DNS Server. No available socket." CR));
    }
  } else {
    Log.error(F("WM  : Can't initiate DNS Server. No enough space." CR));
  }

  _configPortalStart = millis();

  Log.notice(F("WM  : Configuring AP SSID = %s" CR), _apName);

  if (_apPassword != NULL) {
    if (strlen(_apPassword) < 8 || strlen(_apPassword) > 63) {
      Log.error(F("WM  : Invalid AccessPoint password. Ignoring" CR));
      _apPassword = NULL;
    }

    // Log.notice(F("WM  : AP PWD = %s" CR), _apPassword == NULL ? "<null>" :
    // _apPassword);
  }

  static int channel;

  if (_WiFiAPChannel == 0)
    channel = (_configPortalStart % MAX_WIFI_CHANNEL) + 1;
  else
    channel = _WiFiAPChannel;

  Log.notice(F("WM  : AP Channel = %d" CR), channel);
  WiFi.softAP(_apName, _apPassword, channel);
  delay(500);
  Log.notice(F("WM  : AP IP address = %s" CR),
             WiFi.softAPIP().toString().c_str());

  server->on("/", std::bind(&ESP_WiFiManager::handleRoot, this));
  server->on("/wifi", std::bind(&ESP_WiFiManager::handleWifi, this));
  server->on("/wifisave", std::bind(&ESP_WiFiManager::handleWifiSave, this));
  server->on("/close", std::bind(&ESP_WiFiManager::handleServerClose, this));
  server->on("/i", std::bind(&ESP_WiFiManager::handleInfo, this));
  server->on("/r", std::bind(&ESP_WiFiManager::handleReset, this));
  server->on("/state", std::bind(&ESP_WiFiManager::handleState, this));
  server->on("/scan", std::bind(&ESP_WiFiManager::handleScan, this));
  server->on("/fwlink", std::bind(&ESP_WiFiManager::handleRoot, this));
  server->onNotFound(std::bind(&ESP_WiFiManager::handleNotFound, this));
  server->begin();

  Log.notice(F("WM  : HTTP server for wifi portal is started." CR));
}

bool ESP_WiFiManager::autoConnect() {
#ifdef ESP8266
  String ssid = "ESP_" + String(ESP.getChipId());
#else  // ESP32
  String ssid = "ESP_" + String(ESP_getChipId());
#endif

  return autoConnect(ssid.c_str(), NULL);
}

#ifndef AUTOCONNECT_NO_INVALIDATE
#define AUTOCONNECT_NO_INVALIDATE true
#endif

bool ESP_WiFiManager::autoConnect(char const* apName, char const* apPassword) {
#if AUTOCONNECT_NO_INVALIDATE
  Log.notice(
      F("WM  : AutoConnect using previously saved SSID/PW, but keep previous "
        "settings." CR));
  connectWifi();
#else
  Log.notice(
      F("WM  : AutoConnect using previously saved SSID/PW, but invalidate "
        "previous settings." CR));
  connectWifi(WiFi_SSID(), WiFi_Pass());
#endif

  uint32_t startedAt = millis();

  while (millis() - startedAt < 10000) {
    delay(100);

    if (WiFi.status() == WL_CONNECTED) {
      float waited = (millis() - startedAt);

      Log.notice(F("WM  : Connected after waiting %F s." CR), waited / 1000);
      Log.notice(F("WM  : Local ip = %s" CR),
                 WiFi.localIP().toString().c_str());

      return true;
    }
  }

  return startConfigPortal(apName, apPassword);
}

bool ESP_WiFiManager::startConfigPortal() {
#ifdef ESP8266
  String ssid = "ESP_" + String(ESP.getChipId());
#else  // ESP32
  String ssid = "ESP_" + String(ESP_getChipId());
#endif
  ssid.toUpperCase();

  return startConfigPortal(ssid.c_str(), NULL);
}

bool ESP_WiFiManager::startConfigPortal(char const* apName,
                                        char const* apPassword) {
  // setup AP
  Log.notice(F("WM  : Creating AP for wifi portal." CR));

#if defined(ESP8266)
  int connRes = WiFi.waitForConnectResult();
#else  // ESP32
  int connRes = WiFi.waitForConnectResult(1000);
#endif
  // Log.verbose(F("WM  : WiFi.waitForConnectResult Done" CR));

  if (connRes == WL_CONNECTED) {
    // Log.verbose(F("WM  : WIFI connected." CR));
    WiFi.mode(WIFI_AP_STA);  // Dual mode works fine if it is connected to WiFi
  } else {
    // Log.verbose(F("WM  : WIFI not connected." CR));
    WiFi.mode(WIFI_AP);
  }

  _apName = apName;
  _apPassword = apPassword;

  // notify we entered AP mode
  if (_apcallback != NULL) {
    // Log.verbose(F("WM  : _apcallback" CR));
    _apcallback(this);
  }

  connect = false;

  setupConfigPortal();

  bool TimedOut = true;

  Log.notice(F("WM  : WIFI Portal started, enter loop." CR));

  while (_configPortalTimeout == 0 ||
         millis() < _configPortalStart + _configPortalTimeout) {
    dnsServer->processNextRequest();
    server->handleClient();

#if defined(ESP32S2) || defined(ESP32C3)
    delay(1);
#endif

    if (connect) {
      TimedOut = false;
      delay(500);

      Log.notice(F("WM  : Connecting to new AP." CR));

      if (connectWifi(_ssid, _pass) != WL_CONNECTED) {
        Log.error(F("WM  : Failed to connect to AP." CR));
        WiFi.mode(WIFI_AP);
      } else {
        if (_savecallback != NULL) {
          _savecallback();
        }

        break;
      }
    }

    if (stopConfigPortal) {
      Log.notice(F("WM  : Stopped wifi portal." CR));

      stopConfigPortal = false;
      break;
    }

    yield();
  }

  WiFi.mode(WIFI_STA);

  if (TimedOut) {
    setHostname();

    setWifiStaticIP();

    WiFi.begin();
    int connRes = waitForConnectResult();

    Log.error(F("WM  : WIFI connection timeout: %s" CR), getStatus(connRes));
  }

  server->stop();
  server.reset();
  dnsServer->stop();
  dnsServer.reset();

  return WiFi.status() == WL_CONNECTED;
}

void ESP_WiFiManager::setWifiStaticIP() {
  if (_WiFi_STA_IPconfig._sta_static_ip) {
    WiFi.config(_WiFi_STA_IPconfig._sta_static_ip,
                _WiFi_STA_IPconfig._sta_static_gw,
                _WiFi_STA_IPconfig._sta_static_sn);

    Log.notice(F("WM  : Using custom STA IP/GW/Subnet: %s" CR),
               WiFi.localIP().toString().c_str());
  }
}

int ESP_WiFiManager::reconnectWifi() {
  int connectResult;

  if ((connectResult = connectWifi(_ssid, _pass)) != WL_CONNECTED) {
    Log.error(F("WM  : Failed to connect to %s" CR), _ssid.c_str());

    if ((connectResult = connectWifi(_ssid1, _pass1)) != WL_CONNECTED) {
      Log.error(F("WM  : Failed to connect to %s" CR), _ssid1.c_str());
    } else {
      Log.error(F("WM  : Connected to %s" CR), _ssid1.c_str());
    }
  } else {
    Log.error(F("WM  : Connected to %s" CR), _ssid.c_str());
  }

  return connectResult;
}

int ESP_WiFiManager::connectWifi(const String& ssid, const String& pass) {
  if ((ssid != "") || ((ssid == "") && (WiFi_SSID() != ""))) {
    if (WiFi.status() == WL_CONNECTED) {
      Log.notice(F("WM  : Already connected. Bailing out." CR));
      return WL_CONNECTED;
    }

    if (ssid != "") resetSettings();

#ifdef ESP8266
    setWifiStaticIP();
#endif
    WiFi.mode(WIFI_AP_STA);
    setHostname();
#ifdef ESP32
    setWifiStaticIP();
#endif

    if (ssid != "") {
      Log.notice(F("WM  : Connect to new WiFi using new IP parameters" CR));

      WiFi.begin(ssid.c_str(), pass.c_str());
    } else {
      Log.notice(
          F("WM  : Connect to previous WiFi using new IP parameters" CR));

      WiFi.begin();
    }
  } else if (WiFi_SSID() == "") {
    Log.notice(F("WM  : No saved credentials" CR));
  }

  int connRes = waitForConnectResult();
  Log.notice(F("WM  : Connection result: %s" CR), getStatus(connRes));
  return connRes;
}

uint8_t ESP_WiFiManager::waitForConnectResult() {
  if (_connectTimeout == 0) {
    uint32_t startedAt = millis();

    WiFi.waitForConnectResult();
    float waited = (millis() - startedAt);
    Log.notice(F("WM  : Connected after waiting %F s" CR), waited / 1000);
    Log.notice(F("WM  : Local ip = %s" CR), WiFi.localIP().toString().c_str());
    return WiFi.status();
  } else {
    Log.notice(F("WM  : Waiting WiFi connection with time out." CR));

    uint32_t start = millis();
    bool keepConnecting = true;
    uint8_t status;

    while (keepConnecting) {
      status = WiFi.status();

      if (millis() > start + _connectTimeout) {
        keepConnecting = false;
        Log.error(F("WM  : WIFI connection timed out." CR));
      }

      if (status == WL_CONNECTED || status == WL_CONNECT_FAILED) {
        keepConnecting = false;
      }

      delay(100);
    }

    return status;
  }
}

const char* ESP_WiFiManager::getStatus(const int& status) {
  switch (status) {
    case WL_IDLE_STATUS:
      return "WL_IDLE_STATUS";
    case WL_NO_SSID_AVAIL:
      return "WL_NO_SSID_AVAIL";
    case WL_CONNECTED:
      return "WL_CONNECTED";
    case WL_CONNECT_FAILED:
      return "WL_CONNECT_FAILED";
    case WL_DISCONNECTED:
      return "WL_DISCONNECTED";
    default:
      return "UNKNOWN";
  }
}

void ESP_WiFiManager::resetSettings() {
#ifdef ESP8266
  WiFi.disconnect(true);
#else
  WiFi.disconnect(true, true);
  WiFi.begin("0", "0");
#endif
  delay(200);
  return;
}

void ESP_WiFiManager::setTimeout(const uint32_t& seconds) {
  setConfigPortalTimeout(seconds);
}

void ESP_WiFiManager::setConfigPortalTimeout(const uint32_t& seconds) {
  _configPortalTimeout = seconds * 1000;
}

void ESP_WiFiManager::setConnectTimeout(const uint32_t& seconds) {
  _connectTimeout = seconds * 1000;
}

int ESP_WiFiManager::setConfigPortalChannel(const int& channel) {
  if ((channel < MIN_WIFI_CHANNEL - 1) || (channel > MAX_WIFI_CHANNEL))
    _WiFiAPChannel = 1;
  else if ((channel >= MIN_WIFI_CHANNEL - 1) && (channel <= MAX_WIFI_CHANNEL))
    _WiFiAPChannel = channel;

  return _WiFiAPChannel;
}

void ESP_WiFiManager::setAPStaticIPConfig(const IPAddress& ip,
                                          const IPAddress& gw,
                                          const IPAddress& sn) {
  _WiFi_AP_IPconfig._ap_static_ip = ip;
  _WiFi_AP_IPconfig._ap_static_gw = gw;
  _WiFi_AP_IPconfig._ap_static_sn = sn;
}

void ESP_WiFiManager::setAPStaticIPConfig(
    const WiFi_AP_IPConfig& WM_AP_IPconfig) {
  memcpy(reinterpret_cast<void*>(&_WiFi_AP_IPconfig), &WM_AP_IPconfig,
         sizeof(_WiFi_AP_IPconfig));
}

void ESP_WiFiManager::getAPStaticIPConfig(WiFi_AP_IPConfig& WM_AP_IPconfig) {
  memcpy(reinterpret_cast<void*>(&WM_AP_IPconfig), &_WiFi_AP_IPconfig,
         sizeof(WM_AP_IPconfig));
}

void ESP_WiFiManager::setSTAStaticIPConfig(const IPAddress& ip,
                                           const IPAddress& gw,
                                           const IPAddress& sn) {
  _WiFi_STA_IPconfig._sta_static_ip = ip;
  _WiFi_STA_IPconfig._sta_static_gw = gw;
  _WiFi_STA_IPconfig._sta_static_sn = sn;
}

void ESP_WiFiManager::setSTAStaticIPConfig(
    const WiFi_STA_IPConfig& WM_STA_IPconfig) {
  memcpy(reinterpret_cast<void*>(&_WiFi_STA_IPconfig),
         reinterpret_cast<const void*>(&WM_STA_IPconfig),
         sizeof(_WiFi_STA_IPconfig));
}

void ESP_WiFiManager::getSTAStaticIPConfig(WiFi_STA_IPConfig& WM_STA_IPconfig) {
  memcpy(reinterpret_cast<void*>(&WM_STA_IPconfig), &_WiFi_STA_IPconfig,
         sizeof(WM_STA_IPconfig));
}

void ESP_WiFiManager::setMinimumSignalQuality(const int& quality) {
  _minimumQuality = quality;
}

void ESP_WiFiManager::reportStatus(String& page) {
  if (WiFi_SSID() != "") {
    page += F("Configured to connect to access point <b>");
    page += WiFi_SSID();

    if (WiFi.status() == WL_CONNECTED) {
      page += F(" and currently connected</b> on IP <a href=\"http://");
      page += WiFi.localIP().toString();
      page += F("/\">");
      page += WiFi.localIP().toString();
      page += F("</a>");
    } else {
      page += F(" but not currently connected</b> to network.");
    }
  } else {
    page += F("No network currently configured.");
  }
}

void ESP_WiFiManager::handleRoot() {
  Log.notice(F("WM  : Webserver, handle root." CR));

  _configPortalTimeout = 0;

  if (captivePortal()) {
    return;
  }

  server->sendHeader(FPSTR(WM_HTTP_CACHE_CONTROL), FPSTR(WM_HTTP_NO_STORE));

  server->sendHeader(FPSTR(WM_HTTP_PRAGMA), FPSTR(WM_HTTP_NO_CACHE));
  server->sendHeader(FPSTR(WM_HTTP_EXPIRES), "-1");

  String page = FPSTR(WM_HTTP_HEAD_START);

  page.replace("{v}", "Options");
  page += FPSTR(WM_HTTP_SCRIPT);
  page += FPSTR(WM_HTTP_STYLE);
  page += _customHeadElement;
  page += FPSTR(WM_HTTP_HEAD_END);
  page += "<h2>";
  page += _apName;

  if (WiFi_SSID() != "") {
    if (WiFi.status() == WL_CONNECTED) {
      page += " on ";
      page += WiFi_SSID();
    } else {
      page += " <s>on ";
      page += WiFi_SSID();
      page += "</s>";
    }
  }

  page += "</h2>";
  page += FPSTR(WM_HTTP_PORTAL_OPTIONS);
  page += F("<div class=\"msg\">");
  reportStatus(page);
  page += F("</div>");
  page += FPSTR(WM_HTTP_END);

  server->send(200, "text/html", page);
}

void ESP_WiFiManager::handleWifi() {
  Log.notice(F("WM  : WebServer, handle WiFi" CR));

  _configPortalTimeout = 0;
  server->sendHeader(FPSTR(WM_HTTP_CACHE_CONTROL), FPSTR(WM_HTTP_NO_STORE));
  server->sendHeader(FPSTR(WM_HTTP_PRAGMA), FPSTR(WM_HTTP_NO_CACHE));
  server->sendHeader(FPSTR(WM_HTTP_EXPIRES), "-1");

  String page = FPSTR(WM_HTTP_HEAD_START);

  page.replace("{v}", "Config ESP");
  page += FPSTR(WM_HTTP_SCRIPT);
  page += FPSTR(WM_HTTP_STYLE);
  page += _customHeadElement;
  page += FPSTR(WM_HTTP_HEAD_END);
  page += F("<h2>Configuration</h2>");

  numberOfNetworks = scanWifiNetworks(&networkIndices);

  if (numberOfNetworks == 0) {
    page += F("No network found. Refresh to scan again.");
  } else {
    page += FPSTR(WM_FLDSET_START);

    for (int i = 0; i < numberOfNetworks; i++) {
      if (networkIndices[i] == -1) continue;

      Log.notice(F("WM  : Index = %d" CR), i);
      Log.notice(F("WM  : SSID = %s" CR), WiFi.SSID(networkIndices[i]).c_str());
      Log.notice(F("WM  : RSSI = %d" CR), WiFi.RSSI(networkIndices[i]));

      int quality = getRSSIasQuality(WiFi.RSSI(networkIndices[i]));

      String item = FPSTR(WM_HTTP_ITEM);
      String rssiQ;
      rssiQ += quality;
      item.replace("{v}", WiFi.SSID(networkIndices[i]));
      item.replace("{r}", rssiQ);

#ifdef ESP8266
      if (WiFi.encryptionType(networkIndices[i]) != ENC_TYPE_NONE)
#else  // ESP32
      if (WiFi.encryptionType(networkIndices[i]) != WIFI_AUTH_OPEN)
#endif
      {
        item.replace("{i}", "l");
      } else {
        item.replace("{i}", "");
      }

      page += item;
      delay(0);
    }

    page += FPSTR(WM_FLDSET_END);

    page += "<br/>";
  }

  page +=
      "<small>*Hint: To reuse the saved WiFi credentials, leave SSID and PWD "
      "fields empty</small>";

  page += FPSTR(WM_HTTP_FORM_START);

  char parLength[2];

  page += FPSTR(WM_FLDSET_START);

  for (int i = 0; i < _paramsCount; i++) {
    if (_params[i] == NULL) {
      break;
    }

    String pitem;

    switch (_params[i]->getLabelPlacement()) {
      case WFM_LABEL_BEFORE:
        pitem = FPSTR(WM_HTTP_FORM_LABEL_BEFORE);
        break;

      case WFM_LABEL_AFTER:
        pitem = FPSTR(WM_HTTP_FORM_LABEL_AFTER);
        break;

      default:
        pitem = FPSTR(WM_HTTP_FORM_PARAM);
        break;
    }

    if (_params[i]->getID() != NULL) {
      pitem.replace("{i}", _params[i]->getID());
      pitem.replace("{n}", _params[i]->getID());
      pitem.replace("{p}", _params[i]->getPlaceholder());
      snprintf(parLength, sizeof(parLength), "%d",
               _params[i]->getValueLength());
      pitem.replace("{l}", parLength);
      pitem.replace("{v}", _params[i]->getValue());
      pitem.replace("{c}", _params[i]->getCustomHTML());
    } else {
      pitem = _params[i]->getCustomHTML();
    }

    page += pitem;
  }

  if (_paramsCount > 0) {
    page += FPSTR(WM_FLDSET_END);
  }

  if (_params[0] != NULL) {
    page += "<br/>";
  }

  Log.notice(F("WM  : Static IP = %s"),
             _WiFi_STA_IPconfig._sta_static_ip.toString().c_str());

  if (_WiFi_STA_IPconfig._sta_static_ip) {
    page += FPSTR(WM_FLDSET_START);

    String item = FPSTR(WM_HTTP_FORM_LABEL);
    item += FPSTR(WM_HTTP_FORM_PARAM);
    item.replace("{i}", "ip");
    item.replace("{n}", "ip");
    item.replace("{p}", "Static IP");
    item.replace("{l}", "15");
    item.replace("{v}", _WiFi_STA_IPconfig._sta_static_ip.toString());

    page += item;

    item = FPSTR(WM_HTTP_FORM_LABEL);
    item += FPSTR(WM_HTTP_FORM_PARAM);
    item.replace("{i}", "gw");
    item.replace("{n}", "gw");
    item.replace("{p}", "Gateway IP");
    item.replace("{l}", "15");
    item.replace("{v}", _WiFi_STA_IPconfig._sta_static_gw.toString());

    page += item;

    item = FPSTR(WM_HTTP_FORM_LABEL);
    item += FPSTR(WM_HTTP_FORM_PARAM);
    item.replace("{i}", "sn");
    item.replace("{n}", "sn");
    item.replace("{p}", "Subnet");
    item.replace("{l}", "15");
    item.replace("{v}", _WiFi_STA_IPconfig._sta_static_sn.toString());
    page += item;
    page += FPSTR(WM_FLDSET_END);
    page += "<br/>";
  }

  page += FPSTR(WM_HTTP_FORM_END);
  page += FPSTR(WM_HTTP_END);
  server->send(200, "text/html", page);
  Log.notice(F("WM  : Webserver, sent config page." CR));
}

void ESP_WiFiManager::handleWifiSave() {
  Log.notice(F("WM  : Webserver, handle WiFi save." CR));

  _ssid = server->arg("s").c_str();
  _pass = server->arg("p").c_str();

  _ssid1 = server->arg("s1").c_str();
  _pass1 = server->arg("p1").c_str();

  for (int i = 0; i < _paramsCount; i++) {
    if (_params[i] == NULL) {
      break;
    }

    String value = server->arg(_params[i]->getID()).c_str();
    value.toCharArray(_params[i]->_WMParam_data._value,
                      _params[i]->_WMParam_data._length);
    Log.notice(F("WM  : Parameter and value %s %s" CR), _params[i]->getID(),
               value.c_str());
  }

  if (server->arg("ip") != "") {
    String ip = server->arg("ip");
    optionalIPFromString(&_WiFi_STA_IPconfig._sta_static_ip, ip.c_str());
    // Log.verbose(F("WM  : New Static IP = %s" CR),
  }

  if (server->arg("gw") != "") {
    String gw = server->arg("gw");
    optionalIPFromString(&_WiFi_STA_IPconfig._sta_static_gw, gw.c_str());
    // Log.verbose(F("WM  : New Static Gateway = %s" CR),
  }

  if (server->arg("sn") != "") {
    String sn = server->arg("sn");
    optionalIPFromString(&_WiFi_STA_IPconfig._sta_static_sn, sn.c_str());
    // Log.verbose(F("WM  : New Static Netmask = %s" CR),
  }

  String page = FPSTR(WM_HTTP_HEAD_START);

  page.replace("{v}", "Credentials Saved");
  page += FPSTR(WM_HTTP_SCRIPT);
  page += FPSTR(WM_HTTP_STYLE);
  page += _customHeadElement;
  page += FPSTR(WM_HTTP_HEAD_END);
  page += FPSTR(WM_HTTP_SAVED);
  page.replace("{v}", _apName);
  page.replace("{x}", _ssid);
  page.replace("{x1}", _ssid1);

  page += FPSTR(WM_HTTP_END);

  server->send(200, "text/html", page);

  Log.notice(F("WM  : Sent wifi save page" CR));

  connect = true;
  _configPortalTimeout = DEFAULT_PORTAL_TIMEOUT;
}

void ESP_WiFiManager::handleServerClose() {
  Log.notice(F("WM  : Webserver, handle server Close" CR));

  server->sendHeader(FPSTR(WM_HTTP_CACHE_CONTROL), FPSTR(WM_HTTP_NO_STORE));
  server->sendHeader(FPSTR(WM_HTTP_PRAGMA), FPSTR(WM_HTTP_NO_CACHE));
  server->sendHeader(FPSTR(WM_HTTP_EXPIRES), "-1");

  String page = FPSTR(WM_HTTP_HEAD_START);

  page.replace("{v}", "Close Server");
  page += FPSTR(WM_HTTP_SCRIPT);
  page += FPSTR(WM_HTTP_STYLE);
  page += _customHeadElement;
  page += FPSTR(WM_HTTP_HEAD_END);
  page += F("<div class=\"msg\">");
  page += F("My network is <b>");
  page += WiFi_SSID();
  page += F("</b><br>");
  page += F("IP address is <b>");
  page += WiFi.localIP().toString();
  page += F("</b><br><br>");
  page += F("Portal closed...<br><br>");

  page += FPSTR(WM_HTTP_END);

  server->send(200, "text/html", page);

  stopConfigPortal = true;

  Log.notice(F("WM  : Sent server close page" CR));
  _configPortalTimeout = DEFAULT_PORTAL_TIMEOUT;
}

void ESP_WiFiManager::handleInfo() {
  Log.notice(F("WM  : WebServer, handle info." CR));

  _configPortalTimeout = 0;

  server->sendHeader(FPSTR(WM_HTTP_CACHE_CONTROL), FPSTR(WM_HTTP_NO_STORE));
  server->sendHeader(FPSTR(WM_HTTP_PRAGMA), FPSTR(WM_HTTP_NO_CACHE));
  server->sendHeader(FPSTR(WM_HTTP_EXPIRES), "-1");

  String page = FPSTR(WM_HTTP_HEAD_START);

  page.replace("{v}", "Info");
  page += FPSTR(WM_HTTP_SCRIPT);
  page += FPSTR(WM_HTTP_STYLE);
  page += _customHeadElement;
  page += FPSTR(WM_HTTP_HEAD_END);

  page += F("<h2>WiFi Information</h2>");

  reportStatus(page);

  page += FPSTR(WM_FLDSET_START);

  page += F("<h3>Device Data</h3>");
  page += F("<table class=\"table\">");
  page +=
      F("<thead><tr><th>Name</th><th>Value</th></tr></"
        "thead><tbody><tr><td>Chip ID</td><td>");

  page += F("0x");
#ifdef ESP8266
  page += String(ESP.getChipId(), HEX);
#else  // ESP32
  page += String(ESP_getChipId(), HEX);

  page += F("</td></tr>");
  page += F("<tr><td>Chip OUI</td><td>");
  page += F("0x");
  page += String(getChipOUI(), HEX);

  page += F("</td></tr>");
  page += F("<tr><td>Chip Model</td><td>");
  page += ESP.getChipModel();
  page += F(" Rev");
  page += ESP.getChipRevision();
#endif

  page += F("</td></tr>");
  page += F("<tr><td>Flash Chip ID</td><td>");

#ifdef ESP8266
  page += String(ESP.getFlashChipId(), HEX);
#else  // ESP32
  page += F("TODO");
#endif

  page += F("</td></tr>");
  page += F("<tr><td>IDE Flash Size</td><td>");
  page += ESP.getFlashChipSize();
  page += F(" bytes</td></tr>");
  page += F("<tr><td>Real Flash Size</td><td>");

#ifdef ESP8266
  page += ESP.getFlashChipRealSize();
#else  // ESP32
  // TODO
  page += F("TODO");
#endif

  page += F(" bytes</td></tr>");
  page += F("<tr><td>Access Point IP</td><td>");
  page += WiFi.softAPIP().toString();
  page += F("</td></tr>");
  page += F("<tr><td>Access Point MAC</td><td>");
  page += WiFi.softAPmacAddress();
  page += F("</td></tr>");

  page += F("<tr><td>SSID</td><td>");
  page += WiFi_SSID();
  page += F("</td></tr>");

  page += F("<tr><td>Station IP</td><td>");
  page += WiFi.localIP().toString();
  page += F("</td></tr>");

  page += F("<tr><td>Station MAC</td><td>");
  page += WiFi.macAddress();
  page += F("</td></tr>");
  page += F("</tbody></table>");

  page += FPSTR(WM_FLDSET_END);

  page += FPSTR(WM_HTTP_END);

  server->send(200, "text/html", page);

  Log.notice(F("WM  : Sent info page" CR));
}

void ESP_WiFiManager::handleState() {
  Log.notice(F("WM  : Webserver, handle state - json" CR));

  server->sendHeader(FPSTR(WM_HTTP_CACHE_CONTROL), FPSTR(WM_HTTP_NO_STORE));
  server->sendHeader(FPSTR(WM_HTTP_PRAGMA), FPSTR(WM_HTTP_NO_CACHE));
  server->sendHeader(FPSTR(WM_HTTP_EXPIRES), "-1");

  String page = F("{\"Soft_AP_IP\":\"");

  page += WiFi.softAPIP().toString();
  page += F("\",\"Soft_AP_MAC\":\"");
  page += WiFi.softAPmacAddress();
  page += F("\",\"Station_IP\":\"");
  page += WiFi.localIP().toString();
  page += F("\",\"Station_MAC\":\"");
  page += WiFi.macAddress();
  page += F("\",");

  if (WiFi.psk() != "") {
    page += F("\"Password\":true,");
  } else {
    page += F("\"Password\":false,");
  }

  page += F("\"SSID\":\"");
  page += WiFi_SSID();
  page += F("\"}");

  server->send(200, "application/json", page);

  Log.notice(F("WM  : Sent state page in json format" CR));
}

void ESP_WiFiManager::handleScan() {
  Log.notice(F("WM  : WebServer, handle scan" CR));

  _configPortalTimeout = 0;

  // Log.verbose(F("WM  : State-Json" CR));

  server->sendHeader(FPSTR(WM_HTTP_CACHE_CONTROL), FPSTR(WM_HTTP_NO_STORE));
  server->sendHeader(FPSTR(WM_HTTP_PRAGMA), FPSTR(WM_HTTP_NO_CACHE));
  server->sendHeader(FPSTR(WM_HTTP_EXPIRES), "-1");

  int n;
  int* indices;

  n = scanWifiNetworks(&indices);
  Log.notice(F("WM  : In handleScan, scanWifiNetworks done" CR));

  String page = F("{\"Access_Points\":[");

  for (int i = 0; i < n; i++) {
    if (indices[i] == -1) continue;

    if (i != 0) page += F(", ");

    Log.notice(F("WM  : Index = %d" CR), i);
    Log.notice(F("WM  : SSID = %s" CR), WiFi.SSID(indices[i]).c_str());
    Log.notice(F("WM  : RSSI = %d" CR), WiFi.RSSI(indices[i]));

    int quality = getRSSIasQuality(WiFi.RSSI(indices[i]));
    String item = FPSTR(JSON_ITEM);
    String rssiQ;

    rssiQ += quality;
    item.replace("{v}", WiFi.SSID(indices[i]));
    item.replace("{r}", rssiQ);

#ifdef ESP8266

    if (WiFi.encryptionType(indices[i]) != ENC_TYPE_NONE)
#else  // ESP32
    if (WiFi.encryptionType(indices[i]) != WIFI_AUTH_OPEN)
#endif
    {
      item.replace("{i}", "true");
    } else {
      item.replace("{i}", "false");
    }

    // LOGDEBUG(item);

    page += item;
    delay(0);
  }

  if (indices) {
    free(indices);  // indices array no longer required so free memory
  }

  page += F("]}");

  server->send(200, "application/json", page);

  Log.notice(F("WM  : Sent WiFiScan Data in Json format" CR));
}

void ESP_WiFiManager::handleReset() {
  Log.notice(F("WM  : Webserver, handle reset." CR));

  server->sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server->sendHeader("Pragma", "no-cache");
  server->sendHeader("Expires", "-1");

  String page = FPSTR(WM_HTTP_HEAD_START);

  page.replace("{v}", "WiFi Information");
  page += FPSTR(WM_HTTP_SCRIPT);
  page += FPSTR(WM_HTTP_STYLE);
  page += _customHeadElement;
  page += FPSTR(WM_HTTP_HEAD_END);
  page += F("Resetting");
  page += FPSTR(WM_HTTP_END);

  server->send(200, "text/html", page);

  Log.notice(F("WM  : Sent reset page" CR));
  delay(5000);
  resetSettings();
#ifdef ESP8266
  ESP.reset();
#else  // ESP32
  ESP.restart();
#endif

  delay(2000);
}

void ESP_WiFiManager::handleNotFound() {
  if (captivePortal()) {
    return;
  }

  String message = "File Not Found\n\n";

  message += "URI: ";
  message += server->uri();
  message += "\nMethod: ";
  message += (server->method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server->args();
  message += "\n";

  for (uint8_t i = 0; i < server->args(); i++) {
    message += " " + server->argName(i) + ": " + server->arg(i) + "\n";
  }

  server->sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server->sendHeader("Pragma", "no-cache");
  server->sendHeader("Expires", "-1");

  server->send(404, "text/plain", message);
}

bool ESP_WiFiManager::captivePortal() {
  Log.notice(F("WM  : captivePortal: hostHeader = %s" CR),
             server->hostHeader().c_str());

  if (!isIp(server->hostHeader())) {
    Log.notice(F("WM  : Request redirected to captive portal : %s"),
               server->client().localIP().toString().c_str());

    server->sendHeader(
        F("Location"),
        (String)F("http://") + toStringIp(server->client().localIP()), true);

    server->send(302, FPSTR(WM_HTTP_HEAD_CT2), "Redirect to captive portal.");
    return true;
  }

  return false;
}

void ESP_WiFiManager::setAPCallback(
    void (*func)(ESP_WiFiManager* myWiFiManager)) {
  _apcallback = func;
}

void ESP_WiFiManager::setSaveConfigCallback(void (*func)()) {
  _savecallback = func;
}

void ESP_WiFiManager::setCustomHeadElement(const char* element) {
  _customHeadElement = element;
}

int ESP_WiFiManager::scanWifiNetworks(int** indicesptr) {
  Log.notice(F("WM  : Scanning Network" CR));

  int n = WiFi.scanNetworks(false, true);

  Log.notice(F("WM  : scanWifiNetworks: Done, Scanned Networks n = %d" CR), n);

  if (n <= 0) {
    Log.notice(F("WM  : No network found" CR));

    return (0);
  } else {
    int* indices = reinterpret_cast<int*>(malloc(n * sizeof(int)));

    if (indices == NULL) {
      Log.error(F("WM  : Out of memory" CR));
      *indicesptr = NULL;

      return (0);
    }

    *indicesptr = indices;

    for (int i = 0; i < n; i++) {
      indices[i] = i;
    }

    // Log.verbose(F("WM  : Sorting" CR));
    for (int i = 0; i < n; i++) {
      for (int j = i + 1; j < n; j++) {
        if (WiFi.RSSI(indices[j]) > WiFi.RSSI(indices[i])) {
          std::swap(indices[i], indices[j]);
        }
      }
    }

    // Log.verbose(F("WM  : Removing Dup" CR));
    String cssid;

    for (int i = 0; i < n; i++) {
      if (indices[i] == -1) continue;

      cssid = WiFi.SSID(indices[i]);

      for (int j = i + 1; j < n; j++) {
        if (cssid == WiFi.SSID(indices[j])) {
          Log.verbose(F("WM  : DUP AP: %s" CR), WiFi.SSID(indices[j]).c_str());
          indices[j] = -1;  // set dup aps to index -1
        }
      }
    }

    for (int i = 0; i < n; i++) {
      if (indices[i] == -1) continue;  // skip dups

      int quality = getRSSIasQuality(WiFi.RSSI(indices[i]));

      if (!(_minimumQuality == -1 || _minimumQuality < quality)) {
        indices[i] = -1;
        // Log.verbose(F("WM  : Skipping low quality" CR));
      }
    }

    return (n);
  }
}

int ESP_WiFiManager::getRSSIasQuality(const int& RSSI) {
  int quality = 0;

  if (RSSI <= -100) {
    quality = 0;
  } else if (RSSI >= -50) {
    quality = 100;
  } else {
    quality = 2 * (RSSI + 100);
  }

  return quality;
}

bool ESP_WiFiManager::isIp(const String& str) {
  for (unsigned int i = 0; i < str.length(); i++) {
    int c = str.charAt(i);

    if (c != '.' && c != ':' && (c < '0' || c > '9')) {
      return false;
    }
  }

  return true;
}

String ESP_WiFiManager::toStringIp(const IPAddress& ip) {
  String res = "";

  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }

  res += String(((ip >> 8 * 3)) & 0xFF);

  return res;
}

#ifdef ESP32
String ESP_WiFiManager::getStoredWiFiSSID() {
  if (WiFi.getMode() == WIFI_MODE_NULL) {
    return String();
  }

  wifi_ap_record_t info;

  if (!esp_wifi_sta_get_ap_info(&info)) {
    return String(reinterpret_cast<char*>(info.ssid));
  } else {
    wifi_config_t conf;

    esp_wifi_get_config(WIFI_IF_STA, &conf);

    return String(reinterpret_cast<char*>(conf.sta.ssid));
  }

  return String();
}

String ESP_WiFiManager::getStoredWiFiPass() {
  if (WiFi.getMode() == WIFI_MODE_NULL) {
    return String();
  }

  wifi_config_t conf;
  esp_wifi_get_config(WIFI_IF_STA, &conf);
  return String(reinterpret_cast<char*>(conf.sta.password));
}

uint32_t getChipID() {
  uint64_t chipId64 = 0;

  for (int i = 0; i < 6; i++) {
    chipId64 |= (((uint64_t)ESP.getEfuseMac() >> (40 - (i * 8))) & 0xff)
                << (i * 8);
  }

  return (uint32_t)(chipId64 & 0xFFFFFF);
}

uint32_t getChipOUI() {
  uint64_t chipId64 = 0;

  for (int i = 0; i < 6; i++) {
    chipId64 |= (((uint64_t)ESP.getEfuseMac() >> (40 - (i * 8))) & 0xff)
                << (i * 8);
  }

  return (uint32_t)(chipId64 >> 24);
}

#endif  // #ifdef ESP32
