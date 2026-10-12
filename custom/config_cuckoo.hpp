/*
 * GravityMon - CuckooTilt customisation
 * Copyright (c) 2021-2026 Magnus
 *
 * Licensed under the GNU General Public License v3.0 (GPL v3) for open source
 * use, or a Commercial License for proprietary use. See LICENSE and
 * LICENSE_COMMERCIAL.
 */
#ifndef CUSTOM_CONFIG_CUCKOO_HPP_
#define CUSTOM_CONFIG_CUCKOO_HPP_

#include <LittleFS.h>

#include <config_gravitymon.hpp>
#include <log.hpp>
#include <pushtarget.hpp>
#include <templating.hpp>

constexpr auto CONFIG_HTTP_POST_SLEEP_INTERVAL = "http_post_sleep_interval";
constexpr auto CONFIG_BLE_SLEEP_INTERVAL = "ble_sleep_interval";
constexpr auto CONFIG_USE_HTTP_POST = "use_http_post";
constexpr auto CONFIG_USE_BLE = "use_ble";

constexpr auto CUCKOO_OTA_URL = "http://api.ckbrew.com/rest/public/";
constexpr auto CUCKOO_HTTP_POST_URL =
    "http://api.ckbrew.com/rest/public/deviceData";

constexpr auto TPL_HTTP_POST_SLEEP_INTERVAL = "${http-post-sleep-interval}";
constexpr auto TPL_BLE_SLEEP_INTERVAL = "${ble-sleep-interval}";

// Default payload for the two HTTP post targets, expected by the CuckooTilt
// backend. Same as the standard iSpindle compatible format with a few extra
// fields.
constexpr auto CUCKOO_HTTP_POST_FORMAT =
    "{"
    "\"name\": \"${mdns}\", "
    "\"ID\": \"${id}\", "
    "\"token\": \"${token}\", "
    "\"interval\": ${sleep-interval}, "
    "\"http-post-sleep-interval\": ${http-post-sleep-interval}, "
    "\"ble-sleep-interval\": ${ble-sleep-interval}, "
    "\"temperature\": ${temp}, "
    "\"temp_units\": \"${temp-unit}\", "
    "\"gravity\": ${gravity}, "
    "\"velocity\": ${velocity}, "
    "\"angle\": ${angle}, "
    "\"battery\": ${battery}, "
    "\"RSSI\": ${rssi}, "
    "\"corr-gravity\": ${corr-gravity}, "
    "\"gravity-unit\": \"${gravity-unit}\", "
    "\"ip-address\": \"${ip-address}\", "
    "\"app-ver\": \"${app-ver}\", "
    "\"run-mode\": \"${run-mode}\", "
    "\"run-time\": ${run-time} "
    "}";

// Manufacturer configuration for CuckooTilt.
//
// 1. Defaults: set in the constructor, i.e. before main calls loadFile(), so
//    anything stored on the device (or changed by the user) still overrides
//    them. A factory reset removes the file and the defaults apply again.
//
// 2. Extra settings: separate send intervals and on/off switches for HTTP post
//    and Bluetooth. They are saved in the config file and exposed through the
//    config API (createJson/parseJson are virtual in the base class). The UI
//    uses them to calculate sleep_interval and the push skip intervals.
//
// 3. Default payload: see writeDefaultTemplates().
class CuckooConfig : public GravitymonConfig {
 private:
  int _httpPostSleepInterval = 900;  // seconds
  int _bleSleepInterval = 120;       // seconds
  bool _useHttpPost = true;
  bool _useBle = false;

 public:
  CuckooConfig(String baseMDNS, String fileName)
      : GravitymonConfig(baseMDNS, fileName) {
    setStorageSleep(true);
    setBatterySaving(false);
    setVoltageConfig(4.3);
    setTempSensorResolution(11);
    setGravityFormula(
        "0.979742642057399+0.0005883931405567239*tilt+0."
        "000008876047085889598*tilt*tilt");

    // Manufacturer server: firmware updates and the data endpoint. These are
    // what the manufacturer's own firmware has built in.
    setOtaURL(CUCKOO_OTA_URL);
    setTargetHttpPost(CUCKOO_HTTP_POST_URL);
    setHeader1HttpPost("Content-Type: application/json");

    // The setters above flag the config as changed, but these are defaults
    // and not user changes, so nothing needs to be saved yet.
    _saveNeeded = false;
  }

  int getHttpPostSleepInterval() const { return _httpPostSleepInterval; }
  void setHttpPostSleepInterval(int v) {
    if (v > 0) {
      _httpPostSleepInterval = v;
      _saveNeeded = true;
    }
  }

  int getBleSleepInterval() const { return _bleSleepInterval; }
  void setBleSleepInterval(int v) {
    if (v > 0) {
      _bleSleepInterval = v;
      _saveNeeded = true;
    }
  }

  bool getUseHttpPost() const { return _useHttpPost; }
  void setUseHttpPost(bool b) {
    _useHttpPost = b;
    _saveNeeded = true;
  }

  bool getUseBle() const { return _useBle; }
  void setUseBle(bool b) {
    _useBle = b;
    _saveNeeded = true;
  }

  // Replaces BrewingConfig::isWifiPushActive(): the HTTP post targets only
  // count when the HTTP post switch is on. Not virtual in the base class, so
  // this is used where the code works on a CuckooConfig (main_cuckoo.cpp).
  bool isWifiPushActive() const {
    return (_useHttpPost && (hasTargetHttpPost() || hasTargetHttpPost2())) ||
           hasTargetHttpGet() || hasTargetInfluxDb2() || hasTargetMqtt() ||
           isWifiDirect();
  }

  // Template variables used by CUCKOO_HTTP_POST_FORMAT, in addition to the
  // ones set by setupTemplateEngineGravity(). A disabled channel reports 0.
  void setupTemplateEngine(TemplatingEngine& engine) const {
    engine.setVal(TPL_HTTP_POST_SLEEP_INTERVAL,
                  _useHttpPost ? _httpPostSleepInterval : 0);
    engine.setVal(TPL_BLE_SLEEP_INTERVAL, _useBle ? _bleSleepInterval : 0);
  }

  // Stores CUCKOO_HTTP_POST_FORMAT as the format for the HTTP post targets.
  // Templates are files and a factory reset removes them, so without force
  // this only sets the format on first boot / after a factory reset and keeps
  // any format the user has saved. With force the format is fixed and
  // replaced on every boot. Call after the file system is mounted.
  void writeDefaultTemplates(bool force) {
    for (const char* fname : {TPL_GRAVITY_FNAME_POST, TPL_GRAVITY_FNAME_POST2}) {
      if (!force && LittleFS.exists(fname)) continue;

      File file = LittleFS.open(fname, "w");
      if (file) {
        file.print(CUCKOO_HTTP_POST_FORMAT);
        file.close();
        Log.notice(F("CFG : Stored default format in %s." CR), fname);
      } else {
        Log.error(F("CFG : Failed to store default format in %s." CR), fname);
      }
    }
  }

  void createJson(JsonObject& doc) const override {
    GravitymonConfig::createJson(doc);

    doc[CONFIG_HTTP_POST_SLEEP_INTERVAL] = getHttpPostSleepInterval();
    doc[CONFIG_BLE_SLEEP_INTERVAL] = getBleSleepInterval();
    doc[CONFIG_USE_HTTP_POST] = getUseHttpPost();
    doc[CONFIG_USE_BLE] = getUseBle();
  }

  void parseJson(JsonObject& doc) override {
    GravitymonConfig::parseJson(doc);

    if (!doc[CONFIG_HTTP_POST_SLEEP_INTERVAL].isNull())
      setHttpPostSleepInterval(doc[CONFIG_HTTP_POST_SLEEP_INTERVAL].as<int>());
    if (!doc[CONFIG_BLE_SLEEP_INTERVAL].isNull())
      setBleSleepInterval(doc[CONFIG_BLE_SLEEP_INTERVAL].as<int>());
    if (!doc[CONFIG_USE_HTTP_POST].isNull())
      setUseHttpPost(doc[CONFIG_USE_HTTP_POST].as<bool>());
    if (!doc[CONFIG_USE_BLE].isNull())
      setUseBle(doc[CONFIG_USE_BLE].as<bool>());
  }
};

#endif  // CUSTOM_CONFIG_CUCKOO_HPP_
