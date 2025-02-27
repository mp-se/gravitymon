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
#if defined(ENABLE_BLE) && defined(PRESSUREMON)

#include <ble_pressuremon.hpp>
#include <log.hpp>
#include <string>

void BleSender::init() {
  if (_initFlag) return;

  BLEDevice::init("pressuremon");
  _advertising = BLEDevice::getAdvertising();

  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P9);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN, ESP_PWR_LVL_P9);

  _initFlag = true;
}

/*void BleSender::sendEddystoneData(float battery, float tempC, float
pressurePsi, float pressurePsi1) { Log.info(F("Starting eddystone data
transmission" CR));

  char beacon_data[25];

  uint16_t p = pressurePsi * 100;
  uint16_t p1 = pressurePsi1 * 100;
  uint16_t t = tempC * 1000;
  uint16_t b = battery * 1000;
  uint32_t chipId = 0;

  for (int i = 0; i < 17; i = i + 8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }

  beacon_data[0] = 0x20;  // Eddystone Frame Type (Unencrypted Eddystone-TLM)
  beacon_data[1] = 0x00;  // TLM version
  beacon_data[2] = (b >> 8);
  beacon_data[3] = (b & 0xFF);
  beacon_data[4] = (t >> 8);
  beacon_data[5] = (t & 0xFF);
  beacon_data[6] = (p >> 8);
  beacon_data[7] = (p & 0xFF);
  beacon_data[8] = (p1 >> 8);
  beacon_data[9] = (p1 & 0xFF);
  beacon_data[10] = ((chipId & 0xFF000000) >> 24);
  beacon_data[11] = ((chipId & 0xFF0000) >> 16);
  beacon_data[12] = ((chipId & 0xFF00) >> 8);
  beacon_data[13] = (chipId & 0xFF);

  BLEAdvertisementData advData = BLEAdvertisementData();
  BLEAdvertisementData respData = BLEAdvertisementData();

  respData.setFlags(0x06);
  respData.setCompleteServices(BLEUUID("feaa"));
  respData.setServiceData(BLEUUID("feaa"), std::string(beacon_data, 14));

  advData.setName("pressuremon");
  _advertising->setAdvertisementData(advData);
  _advertising->setScanResponseData(respData);

  _advertising->start();
  delay(_beaconTime);
  _advertising->stop();
}*/

void BleSender::sendCustomBeaconData(float battery, float tempC,
                                     float pressurePsi, float pressurePsi1) {
  Log.info(F("Starting custom beacon data transmission" CR));

  _advertising->stop();

  uint16_t p = pressurePsi * 100;
  uint16_t p1 = pressurePsi1 * 100;
  uint16_t t = tempC * 1000;
  uint16_t b = battery * 1000;
  uint32_t chipId = 0;

  for (int i = 0; i < 17; i = i + 8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }

  std::string mf = "";

  mf += static_cast<char>(0x4C);  // Manuf ID (Apple)
  mf += static_cast<char>(0x00);
  mf += static_cast<char>(0x03);  // SubType (standards is 0x02)
  mf += static_cast<char>(0x15);  // SubType Length
  mf += "PRESMON.";
  mf += static_cast<char>(((chipId & 0xFF000000) >> 24));  // Chipid
  mf += static_cast<char>(((chipId & 0xFF0000) >> 16));
  mf += static_cast<char>(((chipId & 0xFF00) >> 8));
  mf += static_cast<char>((chipId & 0xFF));
  mf += static_cast<char>((p >> 8));  // Pressure (pressure*10000)
  mf += static_cast<char>((p & 0xFF));
  mf += static_cast<char>((p1 >> 8));  // Pressure1 (pressure1*10000)
  mf += static_cast<char>((p1 & 0xFF));
  mf += static_cast<char>((b >> 8));  // Battery (batt_v*1000)
  mf += static_cast<char>((b & 0xFF));
  mf += static_cast<char>((t >> 8));  // Temperature (temp_c*1000)
  mf += static_cast<char>((t & 0xFF));
  mf += static_cast<char>(0x00);  // Signal

#if LOG_LEVEL == 6
  dumpPayload(mf.c_str(), mf.length());
#endif

  BLEAdvertisementData advData = BLEAdvertisementData();
  advData.setFlags(0x04);
  advData.setManufacturerData(mf);
  _advertising->setAdvertisementData(advData);

  // _advertising->setAdvertisementType(BLE_GAP_CONN_MODE_NON);
  _advertising->setConnectableMode(BLE_GAP_CONN_MODE_NON);
  _advertising->start();
  delay(_beaconTime);
  _advertising->stop();
}

void BleSender::dumpPayload(const char* p, int len) {
  for (int i = 0; i < len; i++) {
    EspSerial.printf("%X%X ", (*(p + i) & 0xf0) >> 4, (*(p + i) & 0x0f));
  }
  EspSerial.println();
}

#endif  // ENABLE_BLE && PRESSUREMON
