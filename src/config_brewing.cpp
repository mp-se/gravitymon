/*
  Copyright (c) 2021-2026 Magnus
  https://github.com/mp-se/gravitymon

  Licensed under the GNU General Public License v3.0 (GPL v3) for open source use,
  or a Commercial License for proprietary use. See LICENSE and LICENSE_COMMERCIAL.
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
