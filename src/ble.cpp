/*
MIT License

Copyright (c) 2021-22 Magnus

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

//
// Create ble sender
//
BleSender::BleSender(const char* color) {
  BLEDevice::init("");

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

  _advertising = BLEDevice::getAdvertising();
  _color = color;

  if (!_color.compareTo("red"))
    _uuid = BLEUUID::fromString("A495BB10-C5B1-4B44-B512-1370F02D74DE");
  else if (!_color.compareTo("green"))
    _uuid = BLEUUID::fromString("A495BB20-C5B1-4B44-B512-1370F02D74DE");
  else if (!_color.compareTo("black"))
    _uuid = BLEUUID::fromString("A495BB30-C5B1-4B44-B512-1370F02D74DE");
  else if (!_color.compareTo("purple"))
    _uuid = BLEUUID::fromString("A495BB40-C5B1-4B44-B512-1370F02D74DE");
  else if (!_color.compareTo("orange"))
    _uuid = BLEUUID::fromString("A495BB50-C5B1-4B44-B512-1370F02D74DE");
  else if (!_color.compareTo("blue"))
    _uuid = BLEUUID::fromString("A495BB60-C5B1-4B44-B512-1370F02D74DE");
  else if (!_color.compareTo("yellow"))
    _uuid = BLEUUID::fromString("A495BB70-C5B1-4B44-B512-1370F02D74DE");
  else  // if (_color.compareTo("pink"))
    _uuid = BLEUUID::fromString("A495BB80-C5B1-4B44-B512-1370F02D74DE");
}

//
// Send temp and gravity via BLE
//
void BleSender::sendData(float tempF, float gravSG) {
  uint16_t gravity = gravSG * 1000;  // SG * 1000 or SG * 10000 for Tilt Pro/HD
  uint16_t temperature = tempF;      // Deg F _or_ Deg F * 10 for Tilt Pro/HD

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
  delay(100);
  _advertising->stop();
  delay(100);
}

#endif  // ESP32 && !ESP32S2
