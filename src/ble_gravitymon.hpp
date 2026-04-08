/*
  Copyright (c) 2021-2026 Magnus
  https://github.com/mp-se/gravitymon

  Licensed under the GNU General Public License v3.0 (GPL v3) for open source use,
  or a Commercial License for proprietary use. See LICENSE and LICENSE_COMMERCIAL.
*/


#ifndef SRC_BLE_GRAVITYMON_HPP_
#define SRC_BLE_GRAVITYMON_HPP_

#if defined(ENABLE_BLE) && defined(GRAVITYMON)

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
  void sendTiltData(String& color, float tempF, float gravSG, bool tiltPro);
  void sendRaptV1Data(float batteryPercentage, float tempC, float gravSG,
                      float angle);
  void sendRaptV2Data(
      float batteryPercentage, float tempC, float gravSG, float angle,
      float velocity);  // If velocity = NAN velocity valid flag is false
  void sendEddystoneData(float battery, float tempC, float gravSG, float angle);
  void sendCustomBeaconData(float battery, float tempC, float gravSG,
                            float angle);
};

#endif  // ENABLE_BLE && GRAVITYMON

#endif  // SRC_BLE_GRAVITYMON_HPP_
