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
#ifndef SRC_BLE_GATEWAY_HPP_
#define SRC_BLE_GATEWAY_HPP_

#if defined(GATEWAY)

#include <NimBLEAdvertisedDevice.h>
#include <NimBLEDevice.h>
#include <NimBLEScan.h>
#include <NimBLEUtils.h>

#include <queue>
#include <string>
#include <vector>

constexpr auto PARAM_BLE_ID = "ID";
constexpr auto PARAM_BLE_TEMP = "temp";
constexpr auto PARAM_BLE_TEMPERATURE = "temperature";
constexpr auto PARAM_BLE_GRAVITY = "gravity";
constexpr auto PARAM_BLE_ANGLE = "angle";
constexpr auto PARAM_BLE_BATTERY = "battery";
constexpr auto PARAM_BLE_RSSI = "RSSI";
constexpr auto PARAM_BLE_NAME = "name";
constexpr auto PARAM_BLE_TOKEN = "token";
constexpr auto PARAM_BLE_INTERVAL = "interval";
constexpr auto PARAM_BLE_TEMP_UNITS = "temp_units";

class BleDeviceCallbacks : public NimBLEScanCallbacks {
  void onResult(const NimBLEAdvertisedDevice *advertisedDevice) override;
};

enum TiltColor {
  None = -1,
  Red = 0,
  Green = 1,
  Black = 2,
  Purple = 3,
  Orange = 4,
  Blue = 5,
  Yellow = 6,
  Pink = 7
};

class TiltData {
 public:
  // Data points
  float tempF = 0;
  float gravity = 0;
  int txPower = 0;
  int rssi = 0;

  // Internal stuff
  bool updated = false;
  struct tm timeinfoUpdated;
  uint32_t timeUpdated = 0;
  uint32_t timePushed = 0;

  void setUpdated() {
    updated = true;
    timeUpdated = millis();
    getLocalTime(&timeinfoUpdated);
  }

  void setPushed() {
    updated = false;
    timePushed = millis();
  }

  uint32_t getUpdateAge() { return (millis() - timeUpdated) / 1000; }
  uint32_t getPushAge() { return (millis() - timePushed) / 1000; }
};

class GravitymonData {
 public:
  // Data points
  float tempC = 0;
  float gravity = 0;
  float angle = 0;
  float battery = 0;
  int txPower = 0;
  int rssi = 0;
  String id = "";
  String name = "";
  String token = "";
  int interval = 0;

  // Internal stuff
  NimBLEAddress address;
  String type = "";
  String data = "";
  bool updated = false;
  struct tm timeinfoUpdated;
  uint32_t timeUpdated = 0;
  uint32_t timePushed = 0;

  void setUpdated() {
    updated = true;
    timeUpdated = millis();
    getLocalTime(&timeinfoUpdated);
  }

  void setPushed() {
    updated = false;
    timePushed = millis();
  }

  uint32_t getUpdateAge() { return (millis() - timeUpdated) / 1000; }
  uint32_t getPushAge() { return (millis() - timePushed) / 1000; }
};

class PressuremonData {
 public:
  // Data points
  float tempC = 0;
  float pressure = 0;
  float pressure1 = 0;
  float battery = 0;
  int txPower = 0;
  int rssi = 0;
  String id = "";
  String name = "";
  String token = "";
  int interval = 0;

  // Internal stuff
  NimBLEAddress address;
  String type = "";
  String data = "";
  bool updated = false;
  struct tm timeinfoUpdated;
  uint32_t timeUpdated = 0;
  uint32_t timePushed = 0;

  void setUpdated() {
    updated = true;
    timeUpdated = millis();
    getLocalTime(&timeinfoUpdated);
  }

  void setPushed() {
    updated = false;
    timePushed = millis();
  }

  uint32_t getUpdateAge() { return (millis() - timeUpdated) / 1000; }
  uint32_t getPushAge() { return (millis() - timePushed) / 1000; }
};

class ChamberData {
 public:
  // Data points
  float chamberTempC = 0;
  float beerTempC = 0;
  int txPower = 0;
  int rssi = 0;
  String id = "";

  // Internal stuff
  NimBLEAddress address;
  String type = "";
  String data = "";
  bool updated = false;
  struct tm timeinfoUpdated;
  uint32_t timeUpdated = 0;

  void setUpdated() {
    updated = true;
    timeUpdated = millis();
    getLocalTime(&timeinfoUpdated);
  }

  uint32_t getUpdateAge() { return (millis() - timeUpdated) / 1000; }
};

const auto NO_TILT_COLORS =
    8;  // Number of tilt devices that can be managed (one per color)
const auto NO_GRAVITYMON =
    8;  // Number of gravitymon devices that can be handled
const auto NO_PRESSUREMON =
    8;                      // Number of pressuremon devices that can be handled
const auto NO_CHAMBER = 8;  // Number of chamber devices that can be handled

class BleScanner {
 public:
  BleScanner();
  void init();
  void deInit();

  bool scan();
  void setScanTime(int scanTime) { _scanTime = scanTime; }
  void setAllowActiveScan(bool activeScan) { _activeScan = activeScan; }

  TiltColor proccesTiltBeacon(const std::string &advertStringHex,
                              const int8_t &currentRSSI);

  void proccesGravitymonBeacon(const std::string &advertStringHex,
                               NimBLEAddress address);
  void processGravitymonEddystoneBeacon(NimBLEAddress address,
                                        const std::vector<uint8_t> &payload);

  void proccesPressuremonBeacon(const std::string &advertStringHex,
                                NimBLEAddress address);
  void processPressuremonEddystoneBeacon(NimBLEAddress address,
                                         const std::vector<uint8_t> &payload);

  void proccesChamberBeacon(const std::string &advertStringHex,
                            NimBLEAddress address);

  TiltData &getTiltData(TiltColor col) { return _tilt[col]; }

  int findGravitymonId(String id) {
    for (int i = 0; i < NO_GRAVITYMON; i++)
      if (_gravitymon[i].id == id || _gravitymon[i].id == "") return i;
    return -1;
  }
  GravitymonData &getGravitymonData(int idx) { return _gravitymon[idx]; }

  int findPressuremonId(String id) {
    for (int i = 0; i < NO_PRESSUREMON; i++)
      if (_pressuremon[i].id == id || _pressuremon[i].id == "") return i;
    return -1;
  }
  PressuremonData &getPressuremonData(int idx) { return _pressuremon[idx]; }

  int findChamberId(String id) {
    for (int i = 0; i < NO_CHAMBER; i++)
      if (_chamber[i].id == id || _chamber[i].id == "") return i;
    return -1;
  }
  ChamberData &getChamberData(int idx) { return _chamber[idx]; }

  const char *getTiltColorAsString(TiltColor col);

 private:
  int _scanTime = 5;
  bool _activeScan = false;

  BLEScan *_bleScan = nullptr;

  BleDeviceCallbacks *_deviceCallbacks = nullptr;

  TiltData _tilt[NO_TILT_COLORS];
  GravitymonData _gravitymon[NO_GRAVITYMON];
  PressuremonData _pressuremon[NO_PRESSUREMON];
  ChamberData _chamber[NO_CHAMBER];

  TiltColor uuidToTiltColor(std::string uuid);
};

extern BleScanner bleScanner;

#endif  // GATEWAY

#endif  // SRC_BLE_GATEWAY_HPP_
