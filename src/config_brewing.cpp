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
#include <config_brewing.hpp>

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
  doc[CONFIG_VOLTAGE_FACTOR] =
      serialized(String(getVoltageFactor(), DECIMALS_BATTERY));
  doc[CONFIG_VOLTAGE_CONFIG] =
      serialized(String(getVoltageConfig(), DECIMALS_BATTERY));
  doc[CONFIG_VOLTAGE_PIN] = getVoltagePin();
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

  if (!doc[CONFIG_SKIP_SSL_ON_TEST].isNull())
    setSkipSslOnTest(doc[CONFIG_SKIP_SSL_ON_TEST].as<bool>());
  if (!doc[CONFIG_VOLTAGE_PIN].isNull())
    setVoltagePin(doc[CONFIG_VOLTAGE_PIN].as<int>());

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
