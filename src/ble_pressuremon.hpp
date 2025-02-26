/*
MIT License

Copyright (c) 2025 Magnus

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
#ifndef SRC_BLE_PRESSUREMON_HPP_
#define SRC_BLE_PRESSUREMON_HPP_

#if defined(ENABLE_BLE) && defined(PRESSUREMON)

#include <NimBLEBeacon.h>
#include <NimBLEDevice.h>

class BleSender {
 private:
  BLEServer* _server = nullptr;
  BLEAdvertising* _advertising = nullptr;
  BLEService* _service = nullptr;
  BLECharacteristic* _characteristic = nullptr;
  BLEUUID _uuid;
  bool _initFlag = false;
  int _beaconTime = 1000;

  void dumpPayload(const char* payload, int len);

 public:
  BleSender() {}

  void init();

  // Beacons
  /*void sendEddystoneData(float battery, float tempC, float pressurePsi,
                         float pressurePsi1);*/
  void sendCustomBeaconData(float battery, float tempC, float pressurePsi,
                            float pressurePsi1);
};

#endif  // ENABLE_BLE && PRESSUREMON

#endif  // SRC_BLE_PRESSUREMON_HPP_
