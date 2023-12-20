/*
MIT License

Copyright (c) 2021-2023 Magnus

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
#if defined(ESP32) && !defined(ESP32S2)

#include <ble.hpp>
#include <string>

// Tilt UUID variants and data format, based on tilt-sim
//
// https://github.com/spouliot/tilt-sim
//
// Tilt data format is described here. Only SG and Temp is transmitted over BLE.
// https://kvurd.com/blog/tilt-hydrometer-ibeacon-data-format/

class CharacteristicCallbacks : public NimBLECharacteristicCallbacks {
 private:
  volatile bool _isRead = false;

 public:
  void clearReadFlag() { _isRead = false; }
  bool isRead() { return _isRead; }

  void onRead(NimBLECharacteristic* pCharacteristic) {
    // Log.info(F("BLE : Remote reading data" CR));
    _isRead = true;
  }
};

static CharacteristicCallbacks myCharCallbacks;

void BleSender::init() {
  if (_initFlag) return;

  BLEDevice::init("gravitymon");
  _advertising = BLEDevice::getAdvertising();

#if defined(ESP32C3_REV1)
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P6);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P6);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN, ESP_PWR_LVL_P6);
#else
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P9);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN, ESP_PWR_LVL_P9);
#endif

  _initFlag = true;
}

void BleSender::sendEddystone(float battery, float tempC, float gravity,
                              float angle) {
  Log.info(F("Starting eddystone data transmission" CR));

  char beacon_data[25];

  uint16_t g = gravity * 10000;
  uint16_t t = tempC * 1000;
  uint16_t b = battery * 1000;
  uint16_t a = angle * 100;
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
  beacon_data[6] = (g >> 8);
  beacon_data[7] = (g & 0xFF);
  beacon_data[8] = (a >> 8);
  beacon_data[9] = (a & 0xFF);
  beacon_data[10] = ((chipId & 0xFF000000) >> 24);
  beacon_data[11] = ((chipId & 0xFF0000) >> 16);
  beacon_data[12] = ((chipId & 0xFF00) >> 8);
  beacon_data[13] = (chipId & 0xFF);

  BLEAdvertisementData advData = BLEAdvertisementData();
  BLEAdvertisementData respData = BLEAdvertisementData();

  respData.setFlags(0x06);
  respData.setCompleteServices(BLEUUID("feaa"));
  respData.setServiceData(BLEUUID("feaa"), std::string(beacon_data, 14));

  advData.setName("gravitymon");
  _advertising->setAdvertisementData(advData);
  _advertising->setScanResponseData(respData);

  _advertising->start();
  delay(_beaconTime);
  _advertising->stop();
}

void BleSender::sendTiltData(String& color, float tempF, float gravSG,
                             bool tiltPro) {
  Log.info(F("BLE : Starting tilt data transmission" CR));

  if (!color.compareTo("red"))
    _uuid = BLEUUID::fromString("A495BB10-C5B1-4B44-B512-1370F02D74DE");
  else if (!color.compareTo("green"))
    _uuid = BLEUUID::fromString("A495BB20-C5B1-4B44-B512-1370F02D74DE");
  else if (!color.compareTo("black"))
    _uuid = BLEUUID::fromString("A495BB30-C5B1-4B44-B512-1370F02D74DE");
  else if (!color.compareTo("purple"))
    _uuid = BLEUUID::fromString("A495BB40-C5B1-4B44-B512-1370F02D74DE");
  else if (!color.compareTo("orange"))
    _uuid = BLEUUID::fromString("A495BB50-C5B1-4B44-B512-1370F02D74DE");
  else if (!color.compareTo("blue"))
    _uuid = BLEUUID::fromString("A495BB60-C5B1-4B44-B512-1370F02D74DE");
  else if (!color.compareTo("yellow"))
    _uuid = BLEUUID::fromString("A495BB70-C5B1-4B44-B512-1370F02D74DE");
  else  // if (_color.compareTo("pink"))
    _uuid = BLEUUID::fromString("A495BB80-C5B1-4B44-B512-1370F02D74DE");

  uint16_t gravity = gravSG * 1000;  // SG * 1000 or SG * 10000 for Tilt Pro/HD
  uint16_t temperature = tempF;      // Deg F _or_ Deg F * 10 for Tilt Pro/HD

  if (tiltPro) {
    gravity = gravSG * 10000;
    temperature = tempF * 10;
  }

  BLEBeacon beacon = BLEBeacon();
  beacon.setManufacturerId(0x4C00);
  beacon.setProximityUUID(_uuid);
  beacon.setMajor(temperature);
  beacon.setMinor(gravity);

  BLEAdvertisementData advData = BLEAdvertisementData();
  advData.setFlags(0x04);
  advData.setManufacturerData(beacon.getData());

  _advertising->setAdvertisementData(advData);
  _advertising->setAdvertisementType(BLE_GAP_CONN_MODE_NON);

  _advertising->start();
  delay(_beaconTime);
  _advertising->stop();
}

void BleSender::sendGravitymonData(String payload) {
  Log.info(F("BLE : Updating data for gravitymon data transmission" CR));

  _advertising->stop();

  if (!_server) {
    Log.info(
        F("BLE : Creating BLE server for gravitymon data transmission" CR));

    _uuid = BLEUUID::fromString("180a");
    _server = BLEDevice::createServer();
    _service = _server->createService(_uuid);
    _characteristic = _service->createCharacteristic(
        BLEUUID::fromString("2ac4"),
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::BROADCAST);
    _characteristic->setCallbacks(&myCharCallbacks);
    _service->start();
    _advertising->addServiceUUID(_uuid);
    _advertising->setScanResponse(true);
    _advertising->setMinPreferred(0x06);
    _advertising->setMaxPreferred(0x12);
  }

  myCharCallbacks.clearReadFlag();

  if (payload.length() > 510) {
    writeErrorLog("BLE : Payload is to long for sending over BLE");
    payload = "{\"error\":\"payload to long\"}";
  }

  _characteristic->setValue(payload);
  _advertising->start();
}

bool BleSender::isGravitymonDataSent() { return myCharCallbacks.isRead(); }

#endif  // ESP32 && !ESP32S2
