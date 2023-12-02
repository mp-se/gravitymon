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

BLEServer* bleServer = NULL;

class CharacteristicCallbacks : public NimBLECharacteristicCallbacks {
 private:
  volatile bool _isRead = false;

 public:
  void clearReadFlag() { _isRead = false; }
  bool isRead() { return _isRead; }

  void onRead(NimBLECharacteristic* pCharacteristic) {
    Log.info(F("BLE : Remote reading data" CR));
    _isRead = true;
  }
};

static CharacteristicCallbacks myCharCallbacks;

BleSender::BleSender() {
  BLEDevice::init("gravitymon");
  _advertising = BLEDevice::getAdvertising();

  // boost power to maximum, these might be changed once battery life using BLE
  // has been tested.
#if defined(ESP32C3_REV1)
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P6);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P6);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN, ESP_PWR_LVL_P6);
#else
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P9);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN, ESP_PWR_LVL_P9);
#endif
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

  if (tiltPro) {  // Note! Experimental, have not figured out how the receiver
                  // recognise between standard and Pro/HD
    gravity = gravSG * 10000;
    temperature = tempF * 10;
  }

  BLEBeacon oBeacon = BLEBeacon();
  oBeacon.setManufacturerId(
      0x4C00);  // fake Apple 0x004C LSB (ENDIAN_CHANGE_U16!)
  oBeacon.setProximityUUID(_uuid);
  oBeacon.setMajor(temperature);
  oBeacon.setMinor(gravity);
  std::string strServiceData = "";
  strServiceData += static_cast<char>(26);    // Len
  strServiceData += static_cast<char>(0xFF);  // Type
  strServiceData += oBeacon.getData();

  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
  oAdvertisementData.setFlags(0x04);  // BR_EDR_NOT_SUPPORTED 0x04
  oAdvertisementData.addData(strServiceData);

  BLEAdvertisementData oScanResponseData = BLEAdvertisementData();
  _advertising->setAdvertisementData(oAdvertisementData);
  _advertising->setScanResponseData(oScanResponseData);
  _advertising->setAdvertisementType(BLE_GAP_CONN_MODE_NON);

  _advertising->start();
  delay(200);
  _advertising->stop();
}

void BleSender::sendGravitymonData(String payload) {
  if (!bleServer) {  // Initialize server if not already done
    Log.info(
        F("BLE : Creating BLE server for gravitymon data transmission" CR));

    _uuid = BLEUUID::fromString("0000180a-0000-0000-0000-94b47730ed7a");
    bleServer = BLEDevice::createServer();
    _service = bleServer->createService(_uuid);
    _characteristic = _service->createCharacteristic(
        BLEUUID::fromString("00002903-0000-0000-0000-94b47730ed7a"),
        NIMBLE_PROPERTY::READ);
    _characteristic->setCallbacks(&myCharCallbacks);
    _service->start();
    _advertising->addServiceUUID(_uuid);
    _advertising->setScanResponse(true);
    _advertising->setMinPreferred(0x06);
    _advertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
  }

  Log.info(F("BLE : Updating data for gravitymon data transmission" CR));
  myCharCallbacks.clearReadFlag();

  if (payload.length() > 510) {
    writeErrorLog("BLE : Payload is to long for sending over BLE");
    payload = "{\"error\":\"payload to long\"}";
  }

  _characteristic->setValue(payload);
}

bool BleSender::isGravitymonDataSent() { return myCharCallbacks.isRead(); }

#endif  // ESP32 && !ESP32S2
