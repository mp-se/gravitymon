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
#if defined(GRAVITYMON)

#include <config_gravitymon.hpp>
#include <log.hpp>
#include <main.hpp>

GravitymonConfig::GravitymonConfig(String baseMDNS, String fileName)
    : BrewingConfig(baseMDNS, fileName) {}

void GravitymonConfig::createJson(JsonObject& doc) const {
  BrewingConfig::createJson(doc);

  doc[CONFIG_BLE_TILT_COLOR] = getBleTiltColor();
  doc[CONFIG_GRAVITY_FORMULA] = getGravityFormula();
  doc[CONFIG_GRAVITY_UNIT] = String(getGravityUnit());
  doc[CONFIG_GYRO_TEMP] = isGyroTemp();
  doc[CONFIG_GYRO_FILTER] = isGyroFilter();
  doc[CONFIG_GYRO_TYPE] = getGyroType();
  doc[CONFIG_GYRO_SWAP_XY] = isGyroSwapXY();
  doc[CONFIG_STORAGE_SLEEP] = isStorageSleep();
  doc[CONFIG_GRAVITY_TEMP_ADJ] = isGravityTempAdj();

  JsonObject cal = doc[CONFIG_GYRO_CALIBRATION].to<JsonObject>();
  cal["ax"] = _gyroCalibration.ax;
  cal["ay"] = _gyroCalibration.ay;
  cal["az"] = _gyroCalibration.az;
  cal["gx"] = _gyroCalibration.gx;
  cal["gy"] = _gyroCalibration.gy;
  cal["gz"] = _gyroCalibration.gz;

  JsonArray fdArray = doc[CONFIG_FORMULA_DATA].to<JsonArray>();
  for (int i = 0; i < FORMULA_DATA_SIZE; i++) {
    fdArray[i]["a"] = serialized(String(_formulaData.a[i], DECIMALS_TILT));
    fdArray[i]["g"] = serialized(String(_formulaData.g[i], DECIMALS_SG));
  }

  doc[CONFIG_GYRO_READ_COUNT] = this->getGyroReadCount();
  // doc[CONFIG_GYRO_READ_DELAY] = this->getGyroReadDelay();
  doc[CONFIG_GYRO_MOVING_THREASHOLD] = this->getGyroSensorMovingThreashold();
  doc[CONFIG_FORMULA_DEVIATION] =
      serialized(String(this->getMaxFormulaCreationDeviation(), DECIMALS_TILT));
  doc[CONFIG_FORMULA_CALIBRATION_TEMP] = this->getDefaultCalibrationTemp();
  doc[CONFIG_IGNORE_LOW_ANGLES] = this->isIgnoreLowAngles();
  doc[CONFIG_BLE_FORMAT] = getGravitymonBleFormat();
  doc[CONFIG_BATTERY_SAVING] = this->isBatterySaving();
  doc[CONFIG_CHARGING_PIN_ENABLED] = this->isPinChargingMode();
}

void GravitymonConfig::parseJson(JsonObject& doc) {
  BrewingConfig::parseJson(doc);

  if (!doc[CONFIG_BLE_TILT_COLOR].isNull())
    setBleTiltColor(doc[CONFIG_BLE_TILT_COLOR]);
  if (!doc[CONFIG_GRAVITY_FORMULA].isNull())
    setGravityFormula(doc[CONFIG_GRAVITY_FORMULA]);
  if (!doc[CONFIG_GRAVITY_TEMP_ADJ].isNull())
    setGravityTempAdj(doc[CONFIG_GRAVITY_TEMP_ADJ].as<bool>());
  if (!doc[CONFIG_GYRO_TEMP].isNull())
    setGyroTemp(doc[CONFIG_GYRO_TEMP].as<bool>());
  if (!doc[CONFIG_GYRO_SWAP_XY].isNull())
    setGyroSwapXY(doc[CONFIG_GYRO_SWAP_XY].as<bool>());
  if (!doc[CONFIG_STORAGE_SLEEP].isNull())
    setStorageSleep(doc[CONFIG_STORAGE_SLEEP].as<bool>());
  if (!doc[CONFIG_GRAVITY_UNIT].isNull()) {
    String s = doc[CONFIG_GRAVITY_UNIT];
    setGravityUnit(s.charAt(0));
  }
  if (!doc["gravity_format"].isNull()) {  // Legacy support for gravity_format
    String s = doc["gravity_format"];
    setGravityUnit(s.charAt(0));
  }
  if (!doc[CONFIG_GYRO_FILTER].isNull())
    setGyroFilter(doc[CONFIG_GYRO_FILTER].as<bool>());
  if (!doc[CONFIG_GYRO_TYPE].isNull())
    setGyroType(doc[CONFIG_GYRO_TYPE].as<int>());

  if (!doc[CONFIG_GYRO_CALIBRATION]["ax"].isNull())
    _gyroCalibration.ax = doc[CONFIG_GYRO_CALIBRATION]["ax"];
  if (!doc[CONFIG_GYRO_CALIBRATION]["ay"].isNull())
    _gyroCalibration.ay = doc[CONFIG_GYRO_CALIBRATION]["ay"];
  if (!doc[CONFIG_GYRO_CALIBRATION]["az"].isNull())
    _gyroCalibration.az = doc[CONFIG_GYRO_CALIBRATION]["az"];
  if (!doc[CONFIG_GYRO_CALIBRATION]["gx"].isNull())
    _gyroCalibration.gx = doc[CONFIG_GYRO_CALIBRATION]["gx"];
  if (!doc[CONFIG_GYRO_CALIBRATION]["gy"].isNull())
    _gyroCalibration.gy = doc[CONFIG_GYRO_CALIBRATION]["gy"];
  if (!doc[CONFIG_GYRO_CALIBRATION]["gz"].isNull())
    _gyroCalibration.gz = doc[CONFIG_GYRO_CALIBRATION]["gz"];

  if (!doc[CONFIG_FORMULA_DATA].isNull()) {
    JsonArray array = doc[CONFIG_FORMULA_DATA].as<JsonArray>();
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

  if (!doc[CONFIG_GYRO_READ_COUNT].isNull())
    this->setGyroReadCount(doc[CONFIG_GYRO_READ_COUNT].as<int>());
  // if (!doc[CONFIG_GYRO_READ_DELAY].isNull())
  //  this->setGyroReadDelay(doc[CONFIG_GYRO_READ_DELAY].as<int>());
  if (!doc[CONFIG_GYRO_MOVING_THREASHOLD].isNull())
    this->setGyroSensorMovingThreashold(
        doc[CONFIG_GYRO_MOVING_THREASHOLD].as<int>());
  if (!doc[CONFIG_FORMULA_DEVIATION].isNull())
    this->setMaxFormulaCreationDeviation(
        doc[CONFIG_FORMULA_DEVIATION].as<float>());
  if (!doc[CONFIG_FORMULA_CALIBRATION_TEMP].isNull())
    this->SetDefaultCalibrationTemp(
        doc[CONFIG_FORMULA_CALIBRATION_TEMP].as<float>());
  if (!doc[CONFIG_IGNORE_LOW_ANGLES].isNull())
    setIgnoreLowAngles(doc[CONFIG_IGNORE_LOW_ANGLES].as<bool>());
  if (!doc[CONFIG_BLE_FORMAT].isNull())
    setGravitymonBleFormat(doc[CONFIG_BLE_FORMAT].as<int>());
  if (!doc[CONFIG_BATTERY_SAVING].isNull())
    setBatterySaving(doc[CONFIG_BATTERY_SAVING].as<bool>());
  if (!doc[CONFIG_CHARGING_PIN_ENABLED].isNull())
    setPinChargingMode(doc[CONFIG_CHARGING_PIN_ENABLED].as<bool>());
}

void GravitymonConfig::migrateSettings() {
  constexpr auto CFG_FILENAME_OLD = "/gravitymon.json";

  if (!LittleFS.exists(CFG_FILENAME_OLD)) {
    return;
  }

  File configFile = LittleFS.open(CFG_FILENAME_OLD, "r");

  if (!configFile) {
    Serial.println("Failed to open old config file");
    return;
  }

  JsonDocument doc;
  JsonDocument doc2;

  DeserializationError err = deserializeJson(doc, configFile);
  configFile.close();

  if (err) {
    Serial.println("Failed to open parse old config file");
    return;
  }

  JsonObject obj = doc.as<JsonObject>();
  JsonObject obj2 = doc2.as<JsonObject>();

  serializeJson(obj, EspSerial);
  EspSerial.print(CR);

  for (JsonPair kv : obj) {
    String k = kv.key().c_str();
    k.replace("-", "_");
    if (k != "formula_calculation_data") obj2[k] = obj[kv.key().c_str()];
  }

  obj2[CONFIG_BLE_TILT_COLOR] = obj["ble"];

  JsonArray fdArray = obj2[CONFIG_FORMULA_DATA].to<JsonArray>();
  for (int i = 0; i < FORMULA_DATA_SIZE; i++) {
    String num(i + 1);
    fdArray[i]["a"] = obj["formula-calculation-data"]["a" + num];
    fdArray[i]["g"] = obj["formula-calculation-data"]["g" + num];
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

  obj2[CONFIG_PUSH_INTERVAL_POST] = obj["int-http1"];
  obj2[CONFIG_PUSH_INTERVAL_POST2] = obj["int-http2"];
  obj2[CONFIG_PUSH_INTERVAL_GET] = obj["int-http3"];
  obj2[CONFIG_PUSH_INTERVAL_INFLUX] = obj["int-influx"];
  obj2[CONFIG_PUSH_INTERVAL_MQTT] = obj["int-mqtt"];

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

void GravitymonConfig::migrateHwSettings() {
  constexpr auto CFG_FILENAME_HW_OLD = "/hardware.json";

  if (!LittleFS.exists(CFG_FILENAME_HW_OLD)) {
    return;
  }

  File configFile = LittleFS.open(CFG_FILENAME_HW_OLD, "r");

  if (!configFile) {
    Serial.println("Failed to open old hw config file");
    return;
  }

  JsonDocument doc;
  JsonDocument doc2;

  DeserializationError err = deserializeJson(doc, configFile);
  configFile.close();

  if (err) {
    Serial.println("Failed to open parse old hw config file");
    return;
  }

  JsonObject obj = doc.as<JsonObject>();
  JsonObject obj2 = doc2.as<JsonObject>();

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

#endif  // GRAVITYMON

// EOF
