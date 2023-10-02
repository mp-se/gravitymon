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
#ifndef SRC_WIFIMANAGER_HPP_
#define SRC_WIFIMANAGER_HPP_

#include <Arduino.h>

#ifdef ESP8266
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#else  // ESP32
#include <WebServer.h>
#include <WiFi.h>
#endif

#include <DNSServer.h>

#include <memory>
#undef min
#undef max
#include <algorithm>

#ifdef ESP8266
extern "C" {
#include <user_interface.h>
}
#define ESP_getChipId() (ESP.getChipId())
#else  // ESP32
#include <esp_wifi.h>
uint32_t getChipID();
uint32_t getChipOUI();
#define ESP_getChipId() getChipID()
#define ESP_getChipOUI() getChipOUI()
#endif

typedef struct {
  IPAddress _ap_static_ip;
  IPAddress _ap_static_gw;
  IPAddress _ap_static_sn;
} WiFi_AP_IPConfig;

typedef struct {
  IPAddress _sta_static_ip;
  IPAddress _sta_static_gw;
  IPAddress _sta_static_sn;
  IPAddress _sta_static_dns1;
  IPAddress _sta_static_dns2;
} WiFi_STA_IPConfig;

#define WFM_LABEL_BEFORE 1
#define WFM_LABEL_AFTER 2
#define WFM_NO_LABEL 0

const char WM_HTTP_200[] PROGMEM =
    "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
const char WM_HTTP_HEAD_START[] PROGMEM =
    "<!DOCTYPE html><html lang='en'><head><meta name='viewport' "
    "content='width=device-width, initial-scale=1, "
    "user-scalable=no'/><title>{v}</title>";
const char WM_HTTP_STYLE[] PROGMEM =
    "<style>div{padding:2px;font-size:1em;}body,textarea,input,select{"
    "background: 0;border-radius: 0;font: 16px sans-serif;margin: "
    "0}textarea,input,select{outline: 0;font-size: 14px;border: 1px solid "
    "#ccc;padding: 8px;width: 90%}.btn a{text-decoration: "
    "none}.container{margin: auto;width: "
    "90%}@media(min-width:1200px){.container{margin: auto;width: "
    "30%}}@media(min-width:768px) and (max-width:1200px){.container{margin: "
    "auto;width: 50%}}.btn,h2{font-size: 2em}h1{font-size: "
    "3em}.btn{background: #0ae;border-radius: 4px;border: 0;color: "
    "#fff;cursor: pointer;display: inline-block;margin: 2px 0;padding: 10px "
    "14px 11px;width: 100%}.btn:hover{background: "
    "#09d}.btn:active,.btn:focus{background: #08b}label>*{display: "
    "inline}form>*{display: block;margin-bottom: "
    "10px}textarea:focus,input:focus,select:focus{border-color: "
    "#5ab}.msg{background: #def;border-left: 5px solid #59d;padding: "
    "1.5em}.q{float: right;width: 64px;text-align: right}.l{background: "
    "url('data:image/"
    "png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAMAAABEpIrGAAAALVBMVEX///"
    "8EBwfBwsLw8PAzNjaCg4NTVVUjJiZDRUUUFxdiZGSho6OSk5Pg4eFydHTCjaf3AAAAZElEQVQ4"
    "je2NSw7AIAhEBamKn97/"
    "uMXEGBvozkWb9C2Zx4xzWykBhFAeYp9gkLyZE0zIMno9n4g19hmdY39scwqVkOXaxph0ZCXQcq"
    "xSpgQpONa59wkRDOL93eAXvimwlbPbwwVAegLS1HGfZAAAAABJRU5ErkJggg==') "
    "no-repeat left center;background-size: 1em}input[type='checkbox']{float: "
    "left;width: 20px}.table td{padding:.5em;text-align:left}.table "
    "tbody>:nth-child(2n-1){background:#ddd}fieldset{border-radius:0.5rem;"
    "margin:0px;}</style>";
const char WM_HTTP_SCRIPT[] PROGMEM =
    "<script>function "
    "c(l){document.getElementById('s').value=l.innerText||l.textContent;"
    "document.getElementById('p').focus();document.getElementById('s1').value="
    "l.innerText||l.textContent;document.getElementById('p1').focus();document."
    "getElementById('timezone').value=timezone.name();}</script>";
const char WM_HTTP_HEAD_END[] PROGMEM =
    "</head><body><div class='container'><div "
    "style='text-align:left;display:inline-block;min-width:260px;'>";
const char WM_FLDSET_START[] PROGMEM = "<fieldset>";
const char WM_FLDSET_END[] PROGMEM = "</fieldset>";
const char WM_HTTP_PORTAL_OPTIONS[] PROGMEM =
    "<form action='/wifi' method='get'><button "
    "class='btn'>Configuration</button></form><br/><form action='/i' "
    "method='get'><button class='btn'>Information</button></form><br/><form "
    "action='/close' method='get'><button class='btn'>Exit "
    "Portal</button></form><br/>";
const char WM_HTTP_ITEM[] PROGMEM =
    "<div><a href='#p' onclick='c(this)'>{v}</a>&nbsp;<span class='q "
    "{i}'>{r}%</span></div>";
const char JSON_ITEM[] PROGMEM =
    "{\"SSID\":\"{v}\", \"Encryption\":{i}, \"Quality\":\"{r}\"}";
const char WM_HTTP_FORM_START[] PROGMEM =
    "<form method='get' "
    "action='wifisave'><fieldset><div><label>SSID</label><input id='s' "
    "name='s' length=32 "
    "placeholder='SSID'><div></div></div><div><label>Password</label><input "
    "id='p' name='p' length=64 "
    "placeholder='password'><div></div></div><div><label>SSID1</label><input "
    "id='s1' name='s1' length=32 "
    "placeholder='SSID1'><div></div></div><div><label>Password</label><input "
    "id='p1' name='p1' length=64 "
    "placeholder='password1'><div></div></div></fieldset>";
const char WM_HTTP_FORM_LABEL_BEFORE[] PROGMEM =
    "<div><label for='{i}'>{p}</label><input id='{i}' name='{n}' length={l} "
    "placeholder='{p}' value='{v}' {c}><div></div></div>";
const char WM_HTTP_FORM_LABEL_AFTER[] PROGMEM =
    "<div><input id='{i}' name='{n}' length={l} placeholder='{p}' value='{v}' "
    "{c}><label for='{i}'>{p}</label><div></div></div>";
const char WM_HTTP_FORM_LABEL[] PROGMEM = "<label for='{i}'>{p}</label>";
const char WM_HTTP_FORM_PARAM[] PROGMEM =
    "<input id='{i}' name='{n}' length={l} placeholder='{p}' value='{v}' {c}>";
const char WM_HTTP_FORM_END[] PROGMEM =
    "<button class='btn' type='submit'>Save</button></form>";
const char WM_HTTP_SAVED[] PROGMEM =
    "<div class='msg'><b>Credentials Saved</b><br>Try connecting ESP to the "
    "{x}/{x1} network. Wait around 10 seconds then check <a href='/'>if it's "
    "OK.</a> <p/>The {v} AP will run on the same WiFi channel of the {x}/{x1} "
    "AP. You may have to manually reconnect to the {v} AP.</div>";
const char WM_HTTP_END[] PROGMEM = "</div></body></html>";
const char WM_HTTP_HEAD_CL[] PROGMEM = "Content-Length";
const char WM_HTTP_HEAD_CT[] PROGMEM = "text/html";
const char WM_HTTP_HEAD_CT2[] PROGMEM = "text/plain";

const char WM_HTTP_CACHE_CONTROL[] PROGMEM = "Cache-Control";
const char WM_HTTP_NO_STORE[] PROGMEM = "no-cache, no-store, must-revalidate";
const char WM_HTTP_PRAGMA[] PROGMEM = "Pragma";
const char WM_HTTP_NO_CACHE[] PROGMEM = "no-cache";
const char WM_HTTP_EXPIRES[] PROGMEM = "Expires";
const char WM_HTTP_CORS[] PROGMEM = "Access-Control-Allow-Origin";
const char WM_HTTP_CORS_ALLOW_ALL[] PROGMEM = "*";

#define WIFI_MANAGER_MAX_PARAMS 10

typedef struct {
  const char* _id;
  const char* _placeholder;
  char* _value;
  int _length;
  int _labelPlacement;
} WMParam_Data;

class ESP_WMParameter {
 public:
  explicit ESP_WMParameter(const char* custom);
  ESP_WMParameter(const char* id, const char* placeholder,
                  const char* defaultValue, const int& length,
                  const char* custom = "",
                  const int& labelPlacement = WFM_LABEL_BEFORE);
  explicit ESP_WMParameter(const WMParam_Data& WMParam_data);
  ~ESP_WMParameter() {
    if (_WMParam_data._value != NULL) {
      delete[] _WMParam_data._value;
    }
  }
  void setWMParam_Data(const WMParam_Data& WMParam_data) {
    memcpy(&_WMParam_data, &WMParam_data, sizeof(_WMParam_data));
  }
  void getWMParam_Data(WMParam_Data& WMParam_data) {
    memcpy(&WMParam_data, &_WMParam_data, sizeof(WMParam_data));
  }
  const char* getValue() { return _WMParam_data._value; }
  const char* getID() { return _WMParam_data._id; }
  const char* getPlaceholder() { return _WMParam_data._placeholder; }
  int getValueLength() { return _WMParam_data._length; }
  int getLabelPlacement() { return _WMParam_data._labelPlacement; }
  const char* getCustomHTML() { return _customHTML; }

 private:
  WMParam_Data _WMParam_data;
  const char* _customHTML = 0;

  void init(const char* id, const char* placeholder, const char* defaultValue,
            const int& length, const char* custom, const int& labelPlacement);

  friend class ESP_WiFiManager;
};

#define DEFAULT_PORTAL_TIMEOUT 60000L

class ESP_WiFiManager {
 public:
  explicit ESP_WiFiManager(const char* iHostname = "");
  ~ESP_WiFiManager();

  bool autoConnect();
  bool autoConnect(char const* apName, char const* apPassword = NULL);
  bool startConfigPortal();
  bool startConfigPortal(char const* apName, char const* apPassword = NULL);
  String getConfigPortalSSID() { return _apName; }
  String getConfigPortalPW() { return _apPassword; }
  void resetSettings();
  void setConfigPortalTimeout(const uint32_t& seconds);
  void setTimeout(const uint32_t& seconds);
  void setConnectTimeout(const uint32_t& seconds);
  void setMinimumSignalQuality(const int& quality = 8);
  int setConfigPortalChannel(const int& channel = 1);
  void setAPStaticIPConfig(const IPAddress& ip, const IPAddress& gw,
                           const IPAddress& sn);
  void setAPStaticIPConfig(const WiFi_AP_IPConfig& WM_AP_IPconfig);
  void getAPStaticIPConfig(WiFi_AP_IPConfig& WM_AP_IPconfig);
  void setSTAStaticIPConfig(const IPAddress& ip, const IPAddress& gw,
                            const IPAddress& sn);
  void setSTAStaticIPConfig(const WiFi_STA_IPConfig& WM_STA_IPconfig);
  void getSTAStaticIPConfig(WiFi_STA_IPConfig& WM_STA_IPconfig);
  void setAPCallback(void (*func)(ESP_WiFiManager*));
  void setSaveConfigCallback(void (*func)());
  bool addParameter(ESP_WMParameter* p);
  void setCustomHeadElement(const char* element);
  int scanWifiNetworks(int** indicesptr);
  void setCredentials(const char* ssid, const char* pwd, const char* ssid1,
                      const char* pwd1) {
    _ssid = String(ssid);
    _pass = String(pwd);
    _ssid1 = String(ssid1);
    _pass1 = String(pwd1);
  }

  void setCredentials(String& ssid, String& pwd, String& ssid1, String& pwd1) {
    _ssid = ssid;
    _pass = pwd;
    _ssid1 = ssid1;
    _pass1 = pwd1;
  }

  String getSSID() { return _ssid; }
  String getPW() { return _pass; }
  String getSSID1() { return _ssid1; }
  String getPW1() { return _pass1; }

#define MAX_WIFI_CREDENTIALS 2

  String getSSID(const uint8_t& index) {
    if (index == 0)
      return _ssid;
    else if (index == 1)
      return _ssid1;
    else
      return String("");
  }

  String getPW(const uint8_t& index) {
    if (index == 0)
      return _pass;
    else if (index == 1)
      return _pass1;
    else
      return String("");
  }

  ESP_WMParameter** getParameters();
  int getParametersCount();

  const char* getStatus(const int& status);

#ifdef ESP32
  String getStoredWiFiSSID();
  String getStoredWiFiPass();
#endif

  inline String WiFi_SSID() {
#ifdef ESP8266
    return WiFi.SSID();
#else
    return getStoredWiFiSSID();
#endif
  }

  inline String WiFi_Pass() {
#ifdef ESP8266
    return WiFi.psk();
#else
    return getStoredWiFiPass();
#endif
  }

  void setHostname() {
    if (RFC952_hostname[0] != 0) {
#if ESP8266
      WiFi.hostname(RFC952_hostname);
#else
      WiFi.setHostname(RFC952_hostname);
#endif
    }
  }

 private:
  std::unique_ptr<DNSServer> dnsServer;

#ifdef ESP8266
  std::unique_ptr<ESP8266WebServer> server;
#else  // ESP32
  std::unique_ptr<WebServer> server;
#endif

#define RFC952_HOSTNAME_MAXLEN 24
  char RFC952_hostname[RFC952_HOSTNAME_MAXLEN + 1];

  char* getRFC952_hostname(const char* iHostname);
  void setupConfigPortal();

  const char* _apName = "no-net";
  const char* _apPassword = NULL;

  String _ssid = "";
  String _pass = "";
  String _ssid1 = "";
  String _pass1 = "";
  uint32_t _configPortalTimeout = 0;
  uint32_t _connectTimeout = 0;
  uint32_t _configPortalStart = 0;

  int numberOfNetworks = 0;
  int* networkIndices = NULL;

#define MIN_WIFI_CHANNEL 1
#define MAX_WIFI_CHANNEL 11

  int _WiFiAPChannel = 1;

  WiFi_AP_IPConfig _WiFi_AP_IPconfig;

  WiFi_STA_IPConfig _WiFi_STA_IPconfig = {
      IPAddress(0, 0, 0, 0), IPAddress(192, 168, 2, 1),
      IPAddress(255, 255, 255, 0), IPAddress(192, 168, 2, 1),
      IPAddress(8, 8, 8, 8)};

  int _paramsCount = 0;
  int _minimumQuality = -1;

  const char* _customHeadElement = "";
  int status = WL_IDLE_STATUS;
  void setWifiStaticIP();
  int reconnectWifi();
  int connectWifi(const String& ssid = "", const String& pass = "");
  uint8_t waitForConnectResult();
  void handleRoot();
  void handleWifi();
  void handleWifiSave();
  void handleServerClose();
  void handleInfo();
  void handleState();
  void handleScan();
  void handleReset();
  void handleNotFound();
  bool captivePortal();
  void reportStatus(String& page);

  const byte DNS_PORT = 53;

  int getRSSIasQuality(const int& RSSI);
  bool isIp(const String& str);
  String toStringIp(const IPAddress& ip);

  bool connect = false;
  bool stopConfigPortal = false;

  void (*_apcallback)(ESP_WiFiManager*) = NULL;
  void (*_savecallback)() = NULL;

  int _max_params = 0;
  ESP_WMParameter** _params = NULL;

  template <typename Generic>
  void DEBUG_WM(Generic text);

  template <class T>
  auto optionalIPFromString(T* obj, const char* s)
      -> decltype(obj->fromString(s)) {
    return obj->fromString(s);
  }

  auto optionalIPFromString(...) -> bool {
    Log.notice(
        F("WM  : NO fromString METHOD ON IPAddress, you need ESP8266 core "
          "2.1.0 or newer for Custom IP configuration to work." CR));
    return false;
  }
};

#endif  // SRC_WIFIMANAGER_HPP_
