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
#if defined(ENABLE_BLE) && defined(GRAVITYMON)

#include <ble_gravitymon.hpp>
#include <log.hpp>
#include <string>

// Tilt UUID variants and data format, based on tilt-sim
//
// https://github.com/spouliot/tilt-sim
//
// Tilt data format is described here. Only SG and Temp is transmitted over BLE.
// https://kvurd.com/blog/tilt-hydrometer-ibeacon-data-format/

void BleSender::init() {
  if (_initFlag) return;

  BLEDevice::init("gravitymon");
  _advertising = BLEDevice::getAdvertising();

  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P9);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN, ESP_PWR_LVL_P9);

  _initFlag = true;
}

void BleSender::sendEddystoneData(float battery, float tempC, float gravSG,
                                  float angle) {
  Log.info(F("Starting eddystone data transmission" CR));

  char beacon_data[25];

  uint16_t g = gravSG * 10000;
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
  const NimBLEBeacon::BeaconData& beaconData = beacon.getData();
  // advData.setManufacturerData(beacon.getData());
  advData.setManufacturerData(std::string(
      reinterpret_cast<const char*>(&beaconData), sizeof(beaconData)));

  _advertising->setAdvertisementData(advData);
  // _advertising->setAdvertisementType(BLE_GAP_CONN_MODE_NON);
  _advertising->setConnectableMode(BLE_GAP_CONN_MODE_NON);

  _advertising->start();
  delay(_beaconTime);
  _advertising->stop();
}

void BleSender::sendRaptV1Data(float battery, float tempC, float gravSG, float angle) {
  Log.info(F("Starting rapt v1 beacon data transmission" CR));

  _advertising->stop();

  uint16_t t = (tempC + 273.15) * 128.0;
  uint16_t b = battery * 256;
  uint16_t a = angle * 16;
  uint32_t chipId = 0;

  union { // For mapping the raw float to bytes
      float f;
      uint8_t b[4];
  } floatUnion;

  for (int i = 0; i < 17; i = i + 8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }

  std::string mf = "";

  /*
    typedef struct __attribute__((packed)) {
        char prefix[4];        // RAPT
        uint8_t version;       // always 0x01
        uint8_t mac[6];
        uint16_t temperature;  // x / 128 - 273.15
        float gravity;         // / 1000
        int16_t x;             // x / 16
        int16_t y;             // x / 16
        int16_t z;             // x / 16
        int16_t battery;       // x / 256
    } RAPTPillMetricsV1;
  */

  mf += "RAPT";
  mf += static_cast<char>(0x01);  // Rapt v1

  mf += static_cast<char>(0x00);  // Mac adress, using that for ChipID
  mf += static_cast<char>(0x00);  // 
  mf += static_cast<char>(((chipId & 0xFF000000) >> 24));
  mf += static_cast<char>(((chipId & 0xFF0000) >> 16));
  mf += static_cast<char>(((chipId & 0xFF00) >> 8));
  mf += static_cast<char>((chipId & 0xFF));

  mf += static_cast<char>((t >> 8));  // Temperature
  mf += static_cast<char>((t & 0xFF));

  floatUnion.f = gravSG * 1000;  // Gravity
  mf += static_cast<char>(floatUnion.b[3]);
  mf += static_cast<char>(floatUnion.b[2]);
  mf += static_cast<char>(floatUnion.b[1]);
  mf += static_cast<char>(floatUnion.b[0]);

  mf += static_cast<char>((a >> 8));  // X, Angle
  mf += static_cast<char>((a & 0xFF));
  mf += static_cast<char>(0x00);  // Y
  mf += static_cast<char>(0x00); 
  mf += static_cast<char>(0x00);  // Z
  mf += static_cast<char>(0x00); 

  mf += static_cast<char>((b >> 8));  // Battery
  mf += static_cast<char>((b & 0xFF));

#if LOG_LEVEL == 6
  dumpPayload(mf.c_str(), mf.length());
#endif

  BLEAdvertisementData advData = BLEAdvertisementData();
  advData.setFlags(0x04);
  advData.setManufacturerData(mf);
  _advertising->setAdvertisementData(advData);

  _advertising->setConnectableMode(BLE_GAP_CONN_MODE_NON);
  _advertising->start();
  delay(_beaconTime);
  _advertising->stop();
}

void BleSender::sendRaptV2Data(float battery, float tempC, float gravSG, float angle, float velocity, bool velocityValid) {
  Log.info(F("Starting rapt v2 beacon data transmission" CR));

  _advertising->stop();

  union { // For mapping the raw float to bytes
      float f;
      uint8_t b[4];
  } floatUnion;

  uint16_t t = (tempC + 273.15) * 128.0;
  uint16_t b = battery * 256;
  uint16_t a = angle * 16;

  std::string mf = "";

  /*
        typedef struct __attribute__((packed)) {
            char prefix[4];        // RAPT
            uint8_t version;       // always 0x02
            uint8_t padding;       // always 0x00
            uint8_t gravity_velocity_valid; // boolean
            float gravity_velocity;
            uint16_t temperature;  // x / 128 - 273.15
            float gravity;         // / 1000
            int16_t x;             // x / 16
            int16_t y;             // x / 16
            int16_t z;             // x / 16
            int16_t battery;       // x / 256
        } RAPTPillMetricsV2;
  */

  mf += "RAPT";
  mf += static_cast<char>(0x02);  // Rapt v2
  mf += static_cast<char>(0x00);  // Padding

  mf += static_cast<char>(velocityValid ? 0x01 : 0x00);  // Valocity valid

  floatUnion.f = velocity;  // Velocity
  mf += static_cast<char>(floatUnion.b[3]);
  mf += static_cast<char>(floatUnion.b[2]);
  mf += static_cast<char>(floatUnion.b[1]);
  mf += static_cast<char>(floatUnion.b[0]);

  mf += static_cast<char>((t >> 8));  // Temperature
  mf += static_cast<char>((t & 0xFF));

  floatUnion.f = gravSG * 1000;  // Gravity
  mf += static_cast<char>(floatUnion.b[3]);
  mf += static_cast<char>(floatUnion.b[2]);
  mf += static_cast<char>(floatUnion.b[1]);
  mf += static_cast<char>(floatUnion.b[0]);

  mf += static_cast<char>((a >> 8));  // X, Angle
  mf += static_cast<char>((a & 0xFF));
  mf += static_cast<char>(0x00);  // Y
  mf += static_cast<char>(0x00); 
  mf += static_cast<char>(0x00);  // Z
  mf += static_cast<char>(0x00); 

  mf += static_cast<char>((b >> 8));  // Battery (batt_v*1000)
  mf += static_cast<char>((b & 0xFF));

#if LOG_LEVEL == 6
  dumpPayload(mf.c_str(), mf.length());
#endif

  BLEAdvertisementData advData = BLEAdvertisementData();
  advData.setFlags(0x04);
  advData.setManufacturerData(mf);
  _advertising->setAdvertisementData(advData);

  _advertising->setConnectableMode(BLE_GAP_CONN_MODE_NON);
  _advertising->start();
  delay(_beaconTime);
  _advertising->stop();
}

void BleSender::sendCustomBeaconData(float battery, float tempC, float gravSG,
                                     float angle) {
  Log.info(F("Starting custom beacon data transmission" CR));

  _advertising->stop();

  uint16_t g = gravSG * 10000;
  uint16_t t = tempC * 1000;
  uint16_t b = battery * 1000;
  uint16_t a = angle * 100;
  uint32_t chipId = 0;

  for (int i = 0; i < 17; i = i + 8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }

  std::string mf = "";

  mf += static_cast<char>(0x4C);  // Manuf ID (Apple)
  mf += static_cast<char>(0x00);
  mf += static_cast<char>(0x03);  // SubType (standards is 0x02)
  mf += static_cast<char>(0x15);  // SubType Length
  mf += "GRAVMON.";
  mf += static_cast<char>(((chipId & 0xFF000000) >> 24));  // Chipid
  mf += static_cast<char>(((chipId & 0xFF0000) >> 16));
  mf += static_cast<char>(((chipId & 0xFF00) >> 8));
  mf += static_cast<char>((chipId & 0xFF));
  mf += static_cast<char>((a >> 8));  // Angle (angle*100)
  mf += static_cast<char>((a & 0xFF));
  mf += static_cast<char>((b >> 8));  // Battery (batt_v*1000)
  mf += static_cast<char>((b & 0xFF));
  mf += static_cast<char>((g >> 8));  // Gravity (gravity_sg*10000)
  mf += static_cast<char>((g & 0xFF));
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

#endif  // ENABLE_BLE && GRAVITYMON
