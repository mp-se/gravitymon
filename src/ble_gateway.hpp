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

#include <measurement.hpp>
#include <queue>
#include <string>
#include <vector>

class BleDeviceCallbacks : public NimBLEScanCallbacks {
  void onResult(const NimBLEAdvertisedDevice *advertisedDevice) override;
};

class BleScanner {
 public:
  BleScanner();
  void init();
  void deInit();

  bool scan();
  void setScanTime(int scanTime) { _scanTime = scanTime; }
  void setAllowActiveScan(bool activeScan) { _activeScan = activeScan; }

  void proccesTiltBeacon(const std::string &advertStringHex,
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

 private:
  int _scanTime = 5;
  bool _activeScan = false;

  BLEScan *_bleScan = nullptr;

  BleDeviceCallbacks *_deviceCallbacks = nullptr;
  TiltColor uuidToTiltColor(std::string uuid);
};

extern BleScanner bleScanner;

#endif  // GATEWAY

#endif  // SRC_BLE_GATEWAY_HPP_
