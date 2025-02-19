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
#if defined(GATEWAY)

#include <ble_gateway.hpp>
#include <cstdio>
#include <log.hpp>
#include <string>
#include <utils.hpp>
#include <vector>

BleScanner bleScanner;

constexpr auto TILT_COLOR_RED_UUID = "a495bb10c5b14b44b5121370f02d74de";
constexpr auto TILT_COLOR_GREEN_UUID = "a495bb20c5b14b44b5121370f02d74de";
constexpr auto TILT_COLOR_BLACK_UUID = "a495bb30c5b14b44b5121370f02d74de";
constexpr auto TILT_COLOR_PURPLE_UUID = "a495bb40c5b14b44b5121370f02d74de";
constexpr auto TILT_COLOR_ORANGE_UUID = "a495bb50c5b14b44b5121370f02d74de";
constexpr auto TILT_COLOR_BLUE_UUID = "a495bb60c5b14b44b5121370f02d74de";
constexpr auto TILT_COLOR_YELLOW_UUID = "a495bb70c5b14b44b5121370f02d74de";
constexpr auto TILT_COLOR_PINK_UUID = "a495bb80c5b14b44b5121370f02d74de";

constexpr auto SERV_UUID = "180A";
constexpr auto SERV2_UUID = "1801";
constexpr auto CHAR_UUID = "2AC4";

void BleDeviceCallbacks::onResult(
    const NimBLEAdvertisedDevice *advertisedDevice) {
  Log.notice(F("BLE : %s,%s" CR),
            advertisedDevice->getAddress().toString().c_str(),
            advertisedDevice->getName().c_str());

  if (advertisedDevice->getName() == "gravitymon") {
    bool eddyStone = false;

    // Print out the advertised services
    for (int i = 0; i < advertisedDevice->getServiceDataCount(); i++)
      // Log.notice(F("BLE : Service: %d %s %s" CR), i,
      //            advertisedDevice->getServiceDataUUID(i).toString().c_str(),
      //            advertisedDevice->getServiceData(i).c_str());

      // Check if we have a gravitymon eddy stone beacon.
      for (int i = 0; i < advertisedDevice->getServiceDataCount(); i++) {
        if (advertisedDevice->getServiceDataUUID(i).toString() ==
            "0xfeaa") {  // id for eddystone beacon
          eddyStone = true;
        }
      }

    if (eddyStone) {
      Log.notice(F("BLE : Processing gravitymon eddy stone beacon" CR));
      bleScanner.processGravitymonEddystoneBeacon(
          advertisedDevice->getAddress(), advertisedDevice->getPayload());
    }

    return;
  }

  // Check if we have a tilt iBeacon to process

  if (advertisedDevice->getManufacturerData().length() >= 24) {
    if (advertisedDevice->getManufacturerData()[0] == 0x4c &&
        advertisedDevice->getManufacturerData()[1] == 0x00 &&
        advertisedDevice->getManufacturerData()[2] == 0x02 &&
        advertisedDevice->getManufacturerData()[3] == 0x15) {
      Log.notice(F("BLE : Advertised iBeacon TILT Device: %s" CR),
                 advertisedDevice->getAddress().toString().c_str());

      bleScanner.proccesTiltBeacon(advertisedDevice->getManufacturerData(),
                                   advertisedDevice->getRSSI());
    }
  }

  // Check if we have a gravmon iBeacon to process

  if (advertisedDevice->getManufacturerData().length() >= 24) {
    if (advertisedDevice->getManufacturerData()[0] == 0x4c &&
        advertisedDevice->getManufacturerData()[1] == 0x00 &&
        advertisedDevice->getManufacturerData()[2] == 0x03 &&
        advertisedDevice->getManufacturerData()[3] == 0x15) {
      Log.notice(F("BLE : Advertised iBeacon GRAVMON/PRESMON Device: %s" CR),
                 advertisedDevice->getAddress().toString().c_str());

      bleScanner.proccesGravitymonBeacon(
          advertisedDevice->getManufacturerData(),
          advertisedDevice->getAddress());
      bleScanner.proccesPressuremonBeacon(
          advertisedDevice->getManufacturerData(),
          advertisedDevice->getAddress());
    }
  }

  else if (advertisedDevice->getName() == "pressuremon") {
    bool eddyStone = false;

    // Print out the advertised services
    for (int i = 0; i < advertisedDevice->getServiceDataCount(); i++)
      // Log.notice(F("BLE : Service: %d %s %s" CR), i,
      //            advertisedDevice->getServiceDataUUID(i).toString().c_str(),
      //            advertisedDevice->getServiceData(i).c_str());

      // Check if we have a gravitymon eddy stone beacon.
      for (int i = 0; i < advertisedDevice->getServiceDataCount(); i++) {
        if (advertisedDevice->getServiceDataUUID(i).toString() ==
            "0xfeaa") {  // id for eddystone beacon
          eddyStone = true;
        }
      }

    if (eddyStone) {
      Log.notice(F("BLE : Processing pressuremon eddy stone beacon" CR));
      bleScanner.processPressuremonEddystoneBeacon(
          advertisedDevice->getAddress(), advertisedDevice->getPayload());
    }

    return;
  }

  // Check if we have a presmon iBeacon to process
  if (advertisedDevice->getManufacturerData().length() >= 24) {
    if (advertisedDevice->getManufacturerData()[0] == 0x4c &&
        advertisedDevice->getManufacturerData()[1] == 0x00 &&
        advertisedDevice->getManufacturerData()[2] == 0x03 &&
        advertisedDevice->getManufacturerData()[3] == 0x15) {
      Log.notice(F("BLE : Advertised iBeacon PRESMON Device: %s" CR),
                 advertisedDevice->getAddress().toString().c_str());

      bleScanner.proccesPressuremonBeacon(
          advertisedDevice->getManufacturerData(),
          advertisedDevice->getAddress());
    }
  }
}

void BleClientCallbacks::onConnect(NimBLEClient *client) {
  // Log.notice(F("BLE : Client connected"));
}

void BleScanner::proccesGravitymonBeacon(const std::string &advertStringHex,
                                         NimBLEAddress address) {
  const char *payload = advertStringHex.c_str();

  float battery;
  float temp;
  float gravity;
  float angle;
  uint32_t chipId;

  chipId = (*(payload + 12) << 24) | (*(payload + 13) << 16) |
           (*(payload + 14) << 8) | *(payload + 15);
  angle = static_cast<float>((*(payload + 16) << 8) | *(payload + 17)) / 100;
  battery = static_cast<float>((*(payload + 18) << 8) | *(payload + 19)) / 1000;
  gravity =
      static_cast<float>((*(payload + 20) << 8) | *(payload + 21)) / 10000;
  temp = static_cast<float>((*(payload + 22) << 8) | *(payload + 23)) / 1000;

  char chip[20];
  snprintf(&chip[0], sizeof(chip), "%6x", chipId);

  int idx = findGravitymonId(chip);
  if (idx >= 0) {
    GravitymonData &data = getGravitymonData(idx);
    data.tempC = temp;
    data.gravity = gravity;
    data.angle = angle;
    data.battery = battery;
    data.id = chip;
    data.address = address;
    data.type = "Beacon";
    data.setUpdated();
  } else {
    Log.error(F("BLE : Max devices reached - no more devices available." CR));
  }
}

void BleScanner::processGravitymonEddystoneBeacon(
    NimBLEAddress address, const std::vector<uint8_t> &payload) {
  //                                                                      <--------------
  //                                                                      beacon
  //                                                                      data
  //                                                                      ------------>
  // 0b 09 67 72 61 76 69 74 79 6d 6f 6e 02 01 06 03 03 aa fe 11 16 aa fe 20 00
  // 0c 8b 10 8b 00 00 30 39 00 00 16 2e

  float battery;
  float temp;
  float gravity;
  float angle;
  uint32_t chipId;

  battery = static_cast<float>((payload[25] << 8) | payload[26]) / 1000;
  temp = static_cast<float>((payload[27] << 8) | payload[28]) / 1000;
  gravity = static_cast<float>((payload[29] << 8) | payload[30]) / 10000;
  angle = static_cast<float>((payload[31] << 8) | payload[32]) / 100;
  chipId = (payload[33] << 24) | (payload[34] << 16) | (payload[35] << 8) |
           (payload[36]);

  char chip[20];
  snprintf(&chip[0], sizeof(chip), "%6x", chipId);

  int idx = findGravitymonId(chip);
  if (idx >= 0) {
    GravitymonData &data = getGravitymonData(idx);
    data.tempC = temp;
    data.gravity = gravity;
    data.angle = angle;
    data.battery = battery;
    data.id = chip;

    data.address = address;
    data.type = "EddyStone";
    data.setUpdated();
  } else {
    Log.error(F("BLE : Max devices reached - no more devices available." CR));
  }
}

void BleScanner::proccesPressuremonBeacon(const std::string &advertStringHex,
                                          NimBLEAddress address) {
  const char *payload = advertStringHex.c_str();

  if (*(payload + 4) == 'P' && *(payload + 5) == 'R' && *(payload + 6) == 'E' &&
      *(payload + 7) == 'S') {
    float battery;
    float temp;
    float pressure;
    float pressure1;
    uint32_t chipId;

    chipId = (*(payload + 12) << 24) | (*(payload + 13) << 16) |
             (*(payload + 14) << 8) | *(payload + 15);
    pressure =
        static_cast<float>((*(payload + 16) << 8) | *(payload + 17)) / 100;
    pressure1 =
        static_cast<float>((*(payload + 18) << 8) | *(payload + 19)) / 10000;
    battery =
        static_cast<float>((*(payload + 20) << 8) | *(payload + 21)) / 1000;
    temp = static_cast<float>((*(payload + 22) << 8) | *(payload + 23)) / 1000;

    char chip[20];
    snprintf(&chip[0], sizeof(chip), "%6x", chipId);

    int idx = findPressuremonId(chip);
    if (idx >= 0) {
      PressuremonData &data = getPressuremonData(idx);
      data.tempC = temp;
      data.pressure = pressure;
      data.pressure1 = pressure1;
      data.battery = battery;
      data.id = chip;
      data.address = address;
      data.type = "Beacon";
      data.setUpdated();
    } else {
      Log.error(F("BLE : Max devices reached - no more devices available." CR));
    }
  }
}

void BleScanner::processPressuremonEddystoneBeacon(
    NimBLEAddress address, const std::vector<uint8_t> &payload) {
  //                                                                      <--------------
  //                                                                      beacon
  //                                                                      data
  //                                                                      ------------>
  // 0b 09 67 72 61 76 69 74 79 6d 6f 6e 02 01 06 03 03 aa fe 11 16 aa fe 20 00
  // 0c 8b 10 8b 00 00 30 39 00 00 16 2e

  float battery;
  float temp;
  float pressure;
  float pressure1;
  uint32_t chipId;

  battery = static_cast<float>((payload[25] << 8) | payload[26]) / 1000;
  temp = static_cast<float>((payload[27] << 8) | payload[28]) / 1000;
  pressure = static_cast<float>((payload[29] << 8) | payload[30]) / 10000;
  pressure1 = static_cast<float>((payload[31] << 8) | payload[32]) / 100;
  chipId = (payload[33] << 24) | (payload[34] << 16) | (payload[35] << 8) |
           (payload[36]);

  char chip[20];
  snprintf(&chip[0], sizeof(chip), "%6x", chipId);

  int idx = findPressuremonId(chip);
  if (idx >= 0) {
    PressuremonData &data = getPressuremonData(idx);
    data.tempC = temp / 1000;
    data.pressure = pressure / 10000;
    data.pressure1 = pressure1 / 10000;
    data.battery = battery / 1000;
    data.id = chip;

    data.address = address;
    data.type = "EddyStone";
    data.setUpdated();
  } else {
    Log.error(F("BLE : Max devices reached - no more devices available." CR));
  }
}

BleScanner::BleScanner() {
  _deviceCallbacks = new BleDeviceCallbacks();
  _clientCallbacks = new BleClientCallbacks();
}

void BleScanner::init() {
  NimBLEDevice::init("");
  _bleScan = NimBLEDevice::getScan();
  _bleScan->setScanCallbacks(_deviceCallbacks);
  _bleScan->setMaxResults(0);
  _bleScan->setActiveScan(_activeScan);

  _bleScan->setInterval(
      97);  // Select prime numbers to reduce risk of frequency beat pattern
            // with ibeacon advertisement interval
  _bleScan->setWindow(37);  // Set to less or equal setInterval value. Leave
                            // reasonable gap to allow WiFi some time.
  scan();
}

void BleScanner::deInit() { NimBLEDevice::deinit(); }

bool BleScanner::scan() {
  if (!_bleScan) return false;

  if (_bleScan->isScanning()) return true;

  _bleScan->clearResults();

  // Mark all tilt data as invalid
  for (int i = 0; i < NO_TILT_COLORS; i++) {
    _tilt[i].updated = false;
  }

  // Mark all gravitymon data as invalid
  for (int i = 0; i < NO_GRAVITYMON; i++) {
    _gravitymon[i].updated = false;
  }

  Log.notice(F("BLE : Starting %s scan." CR),
             _activeScan ? "ACTIVE" : "PASSIVE");
  _bleScan->setActiveScan(_activeScan);

  NimBLEScanResults foundDevices =
      _bleScan->getResults(_scanTime * 1000, false);
  Log.notice(F("BLE : Scanning completed, found %d results." CR),
             foundDevices.getCount());
  _bleScan->clearResults();  // delete results scan buffer to release memory
  return true;
}

TiltColor BleScanner::proccesTiltBeacon(const std::string &advertStringHex,
                                        const int8_t &currentRSSI) {
  TiltColor color;

  // Check that this is an iBeacon packet
  if (advertStringHex[0] != 0x4c || advertStringHex[1] != 0x00 ||
      advertStringHex[2] != 0x02 || advertStringHex[3] != 0x15)
    return TiltColor::None;

  // The advertisement string is the "manufacturer data" part of the following:
  // Advertised Device: Name: Tilt, Address: 88:c2:55:ac:26:81, manufacturer
  // data: 4c000215a495bb40c5b14b44b5121370f02d74de005004d9c5
  // 4c000215a495bb40c5b14b44b5121370f02d74de005004d9c5
  // ????????iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiittttggggXR
  // **********----------**********----------**********
  char hexCode[3] = {'\0'};
  char colorArray[33] = {'\0'};
  char tempArray[5] = {'\0'};
  char gravityArray[5] = {'\0'};
  char txPowerArray[3] = {'\0'};

  for (int i = 4; i < advertStringHex.length(); i++) {
    snprintf(hexCode, sizeof(hexCode), "%.2x", advertStringHex[i]);
    // Indices 4 - 19 each generate two characters of the color array
    if ((i > 3) && (i < 20)) {
      strncat(colorArray, hexCode, 2);
    }
    // Indices 20-21 each generate two characters of thfe temperature array
    if (i == 20 || i == 21) {
      strncat(tempArray, hexCode, 2);
    }
    // Indices 22-23 each generate two characters of the sp_gravity array
    if (i == 22 || i == 23) {
      strncat(gravityArray, hexCode, 2);
    }
    // Index 24 contains the tx_pwr (which is used by recent tilts to indicate
    // battery age)
    if (i == 24) {
      strncat(txPowerArray, hexCode, 2);
    }
  }

  color = uuidToTiltColor(colorArray);
  if (color == TiltColor::None) {
    return TiltColor::None;
  }

  uint16_t temp = std::strtoul(tempArray, nullptr, 16);
  uint16_t gravity = std::strtoul(gravityArray, nullptr, 16);
  uint8_t txPower = std::strtoul(txPowerArray, nullptr, 16);

  float gravityFactor = 1000;
  float tempFactor = 1;

  if (gravity >= 5000) {  // check for tilt PRO
    gravityFactor = 10000;
    tempFactor = 10;
  }

  // Log.notice(
  //     F("BLE : Tilt data received Temp=%sF, SG=%s, TxPower=%d, Pro=%s" CR),
  //     String(temp / tempFactor, 1).c_str(),
  //     String(gravity / gravityFactor, 4).c_str(), txPower,
  //     gravity >= 5000 ? "yes" : "no");

  TiltData &data = getTiltData(color);
  data.gravity = gravity / gravityFactor;
  data.tempF = temp / tempFactor;
  data.txPower = txPower;
  data.rssi = currentRSSI;
  data.setUpdated();
  return color;
}

TiltColor BleScanner::uuidToTiltColor(std::string uuid) {
  if (uuid == TILT_COLOR_RED_UUID) {
    return TiltColor::Red;
  } else if (uuid == TILT_COLOR_GREEN_UUID) {
    return TiltColor::Green;
  } else if (uuid == TILT_COLOR_BLACK_UUID) {
    return TiltColor::Black;
  } else if (uuid == TILT_COLOR_PURPLE_UUID) {
    return TiltColor::Purple;
  } else if (uuid == TILT_COLOR_ORANGE_UUID) {
    return TiltColor::Orange;
  } else if (uuid == TILT_COLOR_BLUE_UUID) {
    return TiltColor::Blue;
  } else if (uuid == TILT_COLOR_YELLOW_UUID) {
    return TiltColor::Yellow;
  } else if (uuid == TILT_COLOR_PINK_UUID) {
    return TiltColor::Pink;
  }
  return TiltColor::None;
}

const char *BleScanner::getTiltColorAsString(TiltColor col) {
  if (col == TiltColor::Red) {
    return "Red";
  } else if (col == TiltColor::Green) {
    return "Green";
  } else if (col == TiltColor::Black) {
    return "Black";
  } else if (col == TiltColor::Purple) {
    return "Purple";
  } else if (col == TiltColor::Orange) {
    return "Orange";
  } else if (col == TiltColor::Blue) {
    return "Blue";
  } else if (col == TiltColor::Yellow) {
    return "Yellow";
  } else if (col == TiltColor::Pink) {
    return "Pink";
  }
  return "";
}

#endif  // GATEWAY
