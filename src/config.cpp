/*
MIT License

Copyright (c) 2021-2024 Magnus

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
#include <basewebserver.hpp>
#include <config.hpp>
#include <log.hpp>
#include <main.hpp>
#include <resources.hpp>

GravmonConfig::GravmonConfig(String baseMDNS, String fileName)
    : BaseConfig(baseMDNS, fileName, JSON_BUFFER_SIZE_XL) {}

void GravmonConfig::createJson(JsonObject& doc) {
  // Call base class functions
  createJsonBase(doc);
  createJsonWifi(doc);
  createJsonOta(doc);
  createJsonPush(doc);

  doc[PARAM_BLE_TILT_COLOR] = getBleTiltColor();
  doc[PARAM_BLE_FORMAT] = getBleFormat();
  doc[PARAM_USE_WIFI_DIRECT] = isWifiDirect();
  doc[PARAM_TOKEN] = getToken();
  doc[PARAM_TOKEN2] = getToken2();
  doc[PARAM_SLEEP_INTERVAL] = getSleepInterval();
  doc[PARAM_VOLTAGE_FACTOR] =
      serialized(String(getVoltageFactor(), DECIMALS_BATTERY));
  doc[PARAM_VOLTAGE_CONFIG] =
      serialized(String(getVoltageConfig(), DECIMALS_BATTERY));
  doc[PARAM_GRAVITY_FORMULA] = getGravityFormula();
  doc[PARAM_GRAVITY_FORMAT] = String(getGravityFormat());
  doc[PARAM_TEMP_ADJ] = serialized(String(getTempSensorAdjC(), DECIMALS_TEMP));
  doc[PARAM_GRAVITY_TEMP_ADJ] = isGravityTempAdj();
  doc[PARAM_GYRO_TEMP] = isGyroTemp();
  doc[PARAM_GYRO_DISABLED] = isGyroDisabled();
  doc[PARAM_STORAGE_SLEEP] = isStorageSleep();
  doc[PARAM_SKIP_SSL_ON_TEST] = isSkipSslOnTest();
  doc[PARAM_VOLTAGE_PIN] = getVoltagePin();

  JsonObject cal = doc.createNestedObject(PARAM_GYRO_CALIBRATION);
  cal["ax"] = _gyroCalibration.ax;
  cal["ay"] = _gyroCalibration.ay;
  cal["az"] = _gyroCalibration.az;
  cal["gx"] = _gyroCalibration.gx;
  cal["gy"] = _gyroCalibration.gy;
  cal["gz"] = _gyroCalibration.gz;

  JsonArray fdArray = doc.createNestedArray(PARAM_FORMULA_DATA);
  for (int i = 0; i < FORMULA_DATA_SIZE; i++) {
    JsonObject fd = fdArray.createNestedObject();
    fd["a"] = serialized(String(_formulaData.a[i], DECIMALS_TILT));
    fd["g"] = serialized(String(_formulaData.g[i], DECIMALS_SG));
  }

  doc[PARAM_GYRO_READ_COUNT] = this->getGyroReadCount();
  // doc[PARAM_GYRO_READ_DELAY] = this->getGyroReadDelay();
  doc[PARAM_GYRO_MOVING_THREASHOLD] = this->getGyroSensorMovingThreashold();
  doc[PARAM_FORMULA_DEVIATION] =
      serialized(String(this->getMaxFormulaCreationDeviation(), DECIMALS_TILT));
  doc[PARAM_FORMULA_CALIBRATION_TEMP] = this->getDefaultCalibrationTemp();
  doc[PARAM_PUSH_INTERVAL_POST] = this->getPushIntervalPost();
  doc[PARAM_PUSH_INTERVAL_POST2] = this->getPushIntervalPost2();
  doc[PARAM_PUSH_INTERVAL_GET] = this->getPushIntervalGet();
  doc[PARAM_PUSH_INTERVAL_INFLUX] = this->getPushIntervalInflux();
  doc[PARAM_PUSH_INTERVAL_MQTT] = this->getPushIntervalMqtt();
  doc[PARAM_TEMPSENSOR_RESOLUTION] = this->getTempSensorResolution();
  doc[PARAM_IGNORE_LOW_ANGLES] = this->isIgnoreLowAnges();
  doc[PARAM_BATTERY_SAVING] = this->isBatterySaving();
}

void GravmonConfig::parseJson(JsonObject& doc) {
  // Call base class functions
  parseJsonBase(doc);
  parseJsonWifi(doc);
  parseJsonOta(doc);
  parseJsonPush(doc);

  if (!doc[PARAM_BLE_TILT_COLOR].isNull())
    setBleTiltColor(doc[PARAM_BLE_TILT_COLOR]);
  if (!doc[PARAM_BLE_FORMAT].isNull())
    setBleFormat(doc[PARAM_BLE_FORMAT].as<int>());

  if (!doc[PARAM_USE_WIFI_DIRECT].isNull())
    setWifiDirect(doc[PARAM_USE_WIFI_DIRECT]);

  if (!doc[PARAM_TOKEN].isNull()) setToken(doc[PARAM_TOKEN]);
  if (!doc[PARAM_TOKEN2].isNull()) setToken2(doc[PARAM_TOKEN2]);

  if (!doc[PARAM_SLEEP_INTERVAL].isNull())
    setSleepInterval(doc[PARAM_SLEEP_INTERVAL].as<int>());
  if (!doc[PARAM_VOLTAGE_FACTOR].isNull())
    setVoltageFactor(doc[PARAM_VOLTAGE_FACTOR].as<float>());
  if (!doc[PARAM_VOLTAGE_CONFIG].isNull())
    setVoltageConfig(doc[PARAM_VOLTAGE_CONFIG].as<float>());
  if (!doc[PARAM_GRAVITY_FORMULA].isNull())
    setGravityFormula(doc[PARAM_GRAVITY_FORMULA]);
  if (!doc[PARAM_GRAVITY_TEMP_ADJ].isNull())
    setGravityTempAdj(doc[PARAM_GRAVITY_TEMP_ADJ].as<bool>());
  if (!doc[PARAM_GYRO_TEMP].isNull())
    setGyroTemp(doc[PARAM_GYRO_TEMP].as<bool>());
  if (!doc[PARAM_STORAGE_SLEEP].isNull())
    setStorageSleep(doc[PARAM_STORAGE_SLEEP].as<bool>());
  if (!doc[PARAM_GRAVITY_FORMAT].isNull()) {
    String s = doc[PARAM_GRAVITY_FORMAT];
    setGravityFormat(s.charAt(0));
  }
  if (!doc[PARAM_GYRO_DISABLED].isNull())
    setGyroDisabled(doc[PARAM_GYRO_DISABLED].as<bool>());
  if (!doc[PARAM_TEMP_ADJ].isNull())
    setTempSensorAdjC(doc[PARAM_TEMP_ADJ].as<float>());
  if (!doc[PARAM_SKIP_SSL_ON_TEST].isNull())
    setSkipSslOnTest(doc[PARAM_SKIP_SSL_ON_TEST].as<bool>());
  if (!doc[PARAM_VOLTAGE_PIN].isNull())
    setVoltagePin(doc[PARAM_VOLTAGE_PIN].as<int>());

  if (!doc[PARAM_GYRO_CALIBRATION]["ax"].isNull())
    _gyroCalibration.ax = doc[PARAM_GYRO_CALIBRATION]["ax"];
  if (!doc[PARAM_GYRO_CALIBRATION]["ay"].isNull())
    _gyroCalibration.ay = doc[PARAM_GYRO_CALIBRATION]["ay"];
  if (!doc[PARAM_GYRO_CALIBRATION]["az"].isNull())
    _gyroCalibration.az = doc[PARAM_GYRO_CALIBRATION]["az"];
  if (!doc[PARAM_GYRO_CALIBRATION]["gx"].isNull())
    _gyroCalibration.gx = doc[PARAM_GYRO_CALIBRATION]["gx"];
  if (!doc[PARAM_GYRO_CALIBRATION]["gy"].isNull())
    _gyroCalibration.gy = doc[PARAM_GYRO_CALIBRATION]["gy"];
  if (!doc[PARAM_GYRO_CALIBRATION]["gz"].isNull())
    _gyroCalibration.gz = doc[PARAM_GYRO_CALIBRATION]["gz"];

  if (!doc[PARAM_FORMULA_DATA].isNull()) {
    JsonArray array = doc[PARAM_FORMULA_DATA].as<JsonArray>();
    int i = 0;

    for (JsonVariant v : array) {
      _formulaData.a[i] = v["a"].as<double>();
      _formulaData.g[i] = v["g"].as<double>();
      i++;
    }

    if (i != FORMULA_DATA_SIZE) {
      Log.warning(F("Size of formula array is not as expected (%d)" CR), i);
    }
  }

  if (!doc[PARAM_GYRO_READ_COUNT].isNull())
    this->setGyroReadCount(doc[PARAM_GYRO_READ_COUNT].as<int>());
  // if (!doc[PARAM_GYRO_READ_DELAY].isNull())
  //  this->setGyroReadDelay(doc[PARAM_GYRO_READ_DELAY].as<int>());
  if (!doc[PARAM_GYRO_MOVING_THREASHOLD].isNull())
    this->setGyroSensorMovingThreashold(
        doc[PARAM_GYRO_MOVING_THREASHOLD].as<int>());
  if (!doc[PARAM_FORMULA_DEVIATION].isNull())
    this->setMaxFormulaCreationDeviation(
        doc[PARAM_FORMULA_DEVIATION].as<float>());
  if (!doc[PARAM_FORMULA_CALIBRATION_TEMP].isNull())
    this->SetDefaultCalibrationTemp(
        doc[PARAM_FORMULA_CALIBRATION_TEMP].as<float>());
  if (!doc[PARAM_PUSH_INTERVAL_POST].isNull())
    this->setPushIntervalPost(doc[PARAM_PUSH_INTERVAL_POST].as<int>());
  if (!doc[PARAM_PUSH_INTERVAL_POST2].isNull())
    this->setPushIntervalPost2(doc[PARAM_PUSH_INTERVAL_POST2].as<int>());
  if (!doc[PARAM_PUSH_INTERVAL_GET].isNull())
    this->setPushIntervalGet(doc[PARAM_PUSH_INTERVAL_GET].as<int>());
  if (!doc[PARAM_PUSH_INTERVAL_INFLUX].isNull())
    this->setPushIntervalInflux(doc[PARAM_PUSH_INTERVAL_INFLUX].as<int>());
  if (!doc[PARAM_PUSH_INTERVAL_MQTT].isNull())
    this->setPushIntervalMqtt(doc[PARAM_PUSH_INTERVAL_MQTT].as<int>());
  if (!doc[PARAM_TEMPSENSOR_RESOLUTION].isNull())
    this->setTempSensorResolution(doc[PARAM_TEMPSENSOR_RESOLUTION].as<int>());
  if (!doc[PARAM_IGNORE_LOW_ANGLES].isNull())
    setIgnoreLowAnges(doc[PARAM_IGNORE_LOW_ANGLES].as<bool>());
  if (!doc[PARAM_BATTERY_SAVING].isNull())
    setBatterySaving(doc[PARAM_BATTERY_SAVING].as<bool>());
}

void GravmonConfig::migrateSettings() {
  constexpr auto CFG_FILENAME_OLD = "/gravitymon.json";

  if (!LittleFS.exists(CFG_FILENAME_OLD)) {
    return;
  }

  File configFile = LittleFS.open(CFG_FILENAME_OLD, "r");

  if (!configFile) {
    Serial.println("Failed to open old config file");
    return;
  }

  DynamicJsonDocument doc(JSON_BUFFER_SIZE_L);
  DynamicJsonDocument doc2(JSON_BUFFER_SIZE_L);

  DeserializationError err = deserializeJson(doc, configFile);
  configFile.close();

  if (err) {
    Serial.println("Failed to open parse old config file");
    return;
  }

  JsonObject obj = doc.as<JsonObject>();
  JsonObject obj2 = doc2.createNestedObject();

  serializeJson(obj, EspSerial);
  EspSerial.print(CR);

  for (JsonPair kv : obj) {
    String k = kv.key().c_str();
    k.replace("-", "_");
    if (k != "formula_calculation_data") obj2[k] = obj[kv.key().c_str()];
  }

  obj2[PARAM_BLE_TILT_COLOR] = obj["ble"];

  JsonArray fdArray = obj2.createNestedArray(PARAM_FORMULA_DATA);
  for (int i = 0; i < FORMULA_DATA_SIZE; i++) {
    JsonObject fd = fdArray.createNestedObject();
    String num(i + 1);
    fd["a"] = obj["formula-calculation-data"]["a" + num];
    fd["g"] = obj["formula-calculation-data"]["g" + num];
  }

  obj2[PARAM_HTTP_POST_TARGET] = obj["http-push"];
  obj2[PARAM_HTTP_POST_HEADER1] = obj["http-push-h1"];
  obj2[PARAM_HTTP_POST_HEADER2] = obj["http-push-h2"];
  obj2[PARAM_HTTP_POST2_TARGET] = obj["http-push2"];
  obj2[PARAM_HTTP_POST2_HEADER1] = obj["http-push2-h1"];
  obj2[PARAM_HTTP_POST2_HEADER2] = obj["http-push2-h2"];
  obj2[PARAM_HTTP_GET_TARGET] = obj["http-push3"];
  obj2[PARAM_INFLUXDB2_TARGET] = obj["influxdb2-push"];
  obj2[PARAM_INFLUXDB2_TOKEN] = obj["influxdb2-auth"];
  obj2[PARAM_MQTT_TARGET] = obj["mqtt-push"];

  obj2[PARAM_PUSH_INTERVAL_POST] = obj["int-http1"];
  obj2[PARAM_PUSH_INTERVAL_POST2] = obj["int-http2"];
  obj2[PARAM_PUSH_INTERVAL_GET] = obj["int-http3"];
  obj2[PARAM_PUSH_INTERVAL_INFLUX] = obj["int-influx"];
  obj2[PARAM_PUSH_INTERVAL_MQTT] = obj["int-mqtt"];

  obj.clear();
#if LOG_LEVEL == 6
  serializeJson(obj2, EspSerial);
  EspSerial.print(CR);
#endif
  parseJson(obj2);
  obj2.clear();

  if (saveFile()) {
    LittleFS.remove(CFG_FILENAME_OLD);
  }

  Log.notice(F("CFG : Migrated old config /gravitymon.json." CR));
}

void GravmonConfig::migrateHwSettings() {
  constexpr auto CFG_FILENAME_HW_OLD = "/hardware.json";

  if (!LittleFS.exists(CFG_FILENAME_HW_OLD)) {
    return;
  }

  File configFile = LittleFS.open(CFG_FILENAME_HW_OLD, "r");

  if (!configFile) {
    Serial.println("Failed to open old hw config file");
    return;
  }

  DynamicJsonDocument doc(JSON_BUFFER_SIZE_L);
  DynamicJsonDocument doc2(JSON_BUFFER_SIZE_L);

  DeserializationError err = deserializeJson(doc, configFile);
  configFile.close();

  if (err) {
    Serial.println("Failed to open parse old hw config file");
    return;
  }

  JsonObject obj = doc.as<JsonObject>();
  JsonObject obj2 = doc2.createNestedObject();

  serializeJson(obj, EspSerial);
  EspSerial.print(CR);

  for (JsonPair kv : obj) {
    String k = kv.key().c_str();
    k.replace("-", "_");
    obj2[k] = obj[kv.key().c_str()];
  }

  obj.clear();
#if LOG_LEVEL == 6
  serializeJson(obj2, EspSerial);
  EspSerial.print(CR);
#endif
  obj2.clear();

  if (saveFile()) {
    LittleFS.remove(CFG_FILENAME_HW_OLD);
  }

  Log.notice(F("CFG : Migrated old hw config /hardware.json." CR));
}

// EOF
