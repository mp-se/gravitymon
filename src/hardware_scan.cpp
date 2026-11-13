/*
 * GravityMon
 * Copyright (c) 2021-2026 Magnus
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Alternatively, this software may be used under the terms of a
 * commercial license. See LICENSE_COMMERCIAL for details.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#include <Wire.h>

#include <hardware_scan.hpp>
#include <log.hpp>

#if !defined(ESP8266) && ESP_ARDUINO_VERSION_MAJOR >= 3  // For Arduino Core 3.x
#include <esp_chip_info.h>
#include <esp_heap_caps.h>
#endif

#if defined(GRAVITYMON)
#include <gyro.hpp>
#include <tempsensor.hpp>

constexpr auto PARAM_GYRO = "gyro";
constexpr auto PARAM_ONEWIRE = "onewire";
constexpr auto PARAM_TEMP_SENSOR = "temp_sensor";
constexpr auto PARAM_DS18B20_DETECTED = "ds18b20_detected";
constexpr auto PARAM_DS18B20_COUNTERFEIT = "ds18b20_counterfeit";
constexpr auto PARAM_DS18B20_TYPE = "ds18b20_type";
#endif

void scanHardwareCommon(JsonObject &obj) {
  // Scan the i2c bus for devices, initialized in gyro.cpp
  JsonArray i2c = obj[PARAM_I2C].to<JsonArray>();

  for (int i = 1, j = 0; i < 128 && j < 32; i++) {  // Limit to max 32 devices
    // Abort if we get a timeout error (5)
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(i);
    int err = Wire.endTransmission();

    // Log.notice(F("WEB : Scanning 0x%x, response %d." CR), i, err);
    char addr_str[8];
    snprintf(addr_str, sizeof(addr_str), "0x%x", i);

    if (err == 0) {
      Log.notice(F("WEB : Found device at %s." CR), addr_str);
      i2c[j][PARAM_ADRESS] = addr_str;
      i2c[j][PARAM_BUS] = "Wire";
      j++;
    } else if (err == 5) {
      Log.notice(F("WEB : Timeout error at %s." CR), addr_str);
      i2c[j][PARAM_ADRESS] = addr_str;
      i2c[j][PARAM_BUS] = "Timeout error, aborting scan";
      j++;
      break;
    } else {
      // Ignore the other errors, we are just scanning for devices
    }
  }

  JsonObject cpu = obj[PARAM_CHIP].to<JsonObject>();

#if defined(ESP8266)
  cpu[PARAM_FAMILY] = "ESP8266";
#else
  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);

  cpu[PARAM_REVISION] = chip_info.revision;
  cpu[PARAM_CORES] = chip_info.cores;

  JsonArray feature = cpu[PARAM_FEATURES].to<JsonArray>();

  if (chip_info.features & CHIP_FEATURE_EMB_FLASH)
    feature.add("Embedded flash");
  if (chip_info.features & CHIP_FEATURE_WIFI_BGN) feature.add("2.4Ghz WIFI");
  if (chip_info.features & CHIP_FEATURE_BLE) feature.add("Bluetooth LE");
  if (chip_info.features & CHIP_FEATURE_BT) feature.add("Bluetooth Classic");
  if (chip_info.features & CHIP_FEATURE_IEEE802154)
    feature.add("IEEE 802.15.4/LR-WPAN");
  if (chip_info.features & CHIP_FEATURE_EMB_PSRAM)
    feature.add("Embedded PSRAM");
  if (heap_caps_get_free_size(MALLOC_CAP_RTCRAM) > 0) {
    feature.add("Embedded RTC RAM");
  }

  switch (chip_info.model) {
    case CHIP_ESP32:
      cpu[PARAM_FAMILY] = "ESP32";
      break;
    case CHIP_ESP32S2:
      cpu[PARAM_FAMILY] = "ESP32S2";
      break;
    case CHIP_ESP32S3:
      cpu[PARAM_FAMILY] = "ESP32S3";
      break;
    case CHIP_ESP32C3:
      cpu[PARAM_FAMILY] = "ESP32C3";
      break;
    case CHIP_ESP32H2:
      cpu[PARAM_FAMILY] = "ESP32H2";
      break;
    default:
      cpu[PARAM_FAMILY] = String(chip_info.model);
      break;
  }
#endif
}

#if defined(GRAVITYMON)

void scanHardwareGravitymon(JsonObject &obj, bool classifySensor) {
#if defined(ESP8266)
  JsonArray onew = obj[PARAM_ONEWIRE].to<JsonArray>();

  const uint8_t owPins[] = {/*D0,*/ D1, D2, D3, D4,
                            D5,         D6, D7, D8 /*, RX, TX*/};

  for (uint8_t i = 0, j = 0; i < sizeof(owPins); i++) {
    // Log.notice(F("WEB : Scanning onewire pin %d." CR), owPins[i]);
    uint8_t addr[8];
    OneWire ds(owPins[i]);

    if (ds.search(&addr[0])) {
      Log.notice(F("WEB : Found onewire on pin %d." CR), owPins[i]);
      onew[j] = owPins[i];
    }
  }
#endif

  obj[PARAM_DS18B20_DETECTED] = myTempSensor.searchForSensors();
  obj[PARAM_DS18B20_COUNTERFEIT] = myTempSensor.checkForCounterfeit();
  if (classifySensor) {
    obj[PARAM_DS18B20_TYPE] = myTempSensor.classifySensor();
  }

  JsonArray sensor = obj[PARAM_TEMP_SENSOR].to<JsonArray>();

  for (int i = 0, j = 0; i < myTempSensor.getSensorCount(); i++) {
    DeviceAddress adr;
    myTempSensor.getSensorAddress(&adr[0], i);
    Log.notice(F("WEB : Found onewire device %d." CR), i);
    sensor[j][PARAM_ADRESS] = String(adr[0], 16) + String(adr[1], 16) +
                              String(adr[2], 16) + String(adr[3], 16) +
                              String(adr[4], 16) + String(adr[5], 16) +
                              String(adr[6], 16) + String(adr[7], 16);
    switch (adr[0]) {
      case DS18S20MODEL:
        sensor[j][PARAM_FAMILY] = "DS18S20";
        break;
      case DS18B20MODEL:
        sensor[j][PARAM_FAMILY] = "DS18B20";
        break;
      case DS1822MODEL:
        sensor[j][PARAM_FAMILY] = "DS1822";
        break;
      case DS1825MODEL:
        sensor[j][PARAM_FAMILY] = "DS1825";
        break;
      case DS28EA00MODEL:
        sensor[j][PARAM_FAMILY] = "DS28EA00";
        break;
    }
    sensor[j][PARAM_RESOLUTION] = myTempSensor.getSensorResolution();
    j++;
  }

  // Test the gyro
  obj[PARAM_GYRO][PARAM_FAMILY] = myGyro.getGyroFamily();
  JsonObject gyro = obj[PARAM_GYRO];
  myGyro.getGyroTestResult(gyro);
}

#endif  // GRAVITYMON

// EOF
