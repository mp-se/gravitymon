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


#include <config_brewing.hpp>
#include <main.hpp>

BrewingConfig::BrewingConfig(String baseMDNS, String fileName)
    : BaseConfig(baseMDNS, fileName) {}

void BrewingConfig::createJson(JsonObject& doc) const {
  createJsonBase(doc);
  createJsonWifi(doc);
  createJsonOta(doc);
  createJsonPush(doc);

  doc[CONFIG_USE_WIFI_DIRECT] = isWifiDirect();
  doc[CONFIG_TOKEN] = getToken();
  doc[CONFIG_TOKEN2] = getToken2();
  doc[CONFIG_SLEEP_INTERVAL] = getSleepInterval();
  doc[CONFIG_BATTERY_TYPE] = static_cast<int>(getBatteryType());
  doc[CONFIG_VOLTAGE_FACTOR] =
      serialized(String(getVoltageFactor(), DECIMALS_BATTERY));
  doc[CONFIG_VOLTAGE_CONFIG] =
      serialized(String(getVoltageConfig(), DECIMALS_BATTERY));
  doc[CONFIG_TEMP_ADJ] = serialized(String(getTempSensorAdjC(), DECIMALS_TEMP));

  doc[CONFIG_SKIP_SSL_ON_TEST] = isSkipSslOnTest();
  doc[CONFIG_PUSH_INTERVAL_POST] = this->getPushIntervalPost();
  doc[CONFIG_PUSH_INTERVAL_POST2] = this->getPushIntervalPost2();
  doc[CONFIG_PUSH_INTERVAL_GET] = this->getPushIntervalGet();
  doc[CONFIG_PUSH_INTERVAL_INFLUX] = this->getPushIntervalInflux();
  doc[CONFIG_PUSH_INTERVAL_MQTT] = this->getPushIntervalMqtt();

  doc[CONFIG_TEMPSENSOR_RESOLUTION] = this->getTempSensorResolution();
}

void BrewingConfig::parseJson(JsonObject& doc) {
  parseJsonBase(doc);
  parseJsonWifi(doc);
  parseJsonOta(doc);
  parseJsonPush(doc);

  if (!doc[CONFIG_USE_WIFI_DIRECT].isNull())
    setWifiDirect(doc[CONFIG_USE_WIFI_DIRECT]);

  if (!doc[CONFIG_TOKEN].isNull()) setToken(doc[CONFIG_TOKEN]);
  if (!doc[CONFIG_TOKEN2].isNull()) setToken2(doc[CONFIG_TOKEN2]);

  if (!doc[CONFIG_SLEEP_INTERVAL].isNull())
    setSleepInterval(doc[CONFIG_SLEEP_INTERVAL].as<int>());
  if (!doc[CONFIG_VOLTAGE_FACTOR].isNull())
    setVoltageFactor(doc[CONFIG_VOLTAGE_FACTOR].as<float>());
  if (!doc[CONFIG_VOLTAGE_CONFIG].isNull())
    setVoltageConfig(doc[CONFIG_VOLTAGE_CONFIG].as<float>());
  if (!doc[CONFIG_TEMP_ADJ].isNull())
    setTempSensorAdjC(doc[CONFIG_TEMP_ADJ].as<float>());
  if (!doc[CONFIG_BATTERY_TYPE].isNull())
    setBatteryType(doc[CONFIG_BATTERY_TYPE].as<int>());

  if (!doc[CONFIG_SKIP_SSL_ON_TEST].isNull())
    setSkipSslOnTest(doc[CONFIG_SKIP_SSL_ON_TEST].as<bool>());

  if (!doc[CONFIG_PUSH_INTERVAL_POST].isNull())
    this->setPushIntervalPost(doc[CONFIG_PUSH_INTERVAL_POST].as<int>());
  if (!doc[CONFIG_PUSH_INTERVAL_POST2].isNull())
    this->setPushIntervalPost2(doc[CONFIG_PUSH_INTERVAL_POST2].as<int>());
  if (!doc[CONFIG_PUSH_INTERVAL_GET].isNull())
    this->setPushIntervalGet(doc[CONFIG_PUSH_INTERVAL_GET].as<int>());
  if (!doc[CONFIG_PUSH_INTERVAL_INFLUX].isNull())
    this->setPushIntervalInflux(doc[CONFIG_PUSH_INTERVAL_INFLUX].as<int>());
  if (!doc[CONFIG_PUSH_INTERVAL_MQTT].isNull())
    this->setPushIntervalMqtt(doc[CONFIG_PUSH_INTERVAL_MQTT].as<int>());

  if (!doc[CONFIG_TEMPSENSOR_RESOLUTION].isNull())
    this->setTempSensorResolution(doc[CONFIG_TEMPSENSOR_RESOLUTION].as<int>());
}

// EOF
