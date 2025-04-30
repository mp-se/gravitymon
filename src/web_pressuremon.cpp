/*
MIT License

Copyright (c) 2022-2025 Magnus

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
#if defined(PRESSUREMON)

#include <battery.hpp>
#include <config.hpp>
#include <helper.hpp>
#include <pressure.hpp>
#include <tempsensor.hpp>
#include <webserver.hpp>

constexpr auto PARAM_PRESSURE = "pressure";
constexpr auto PARAM_PRESSURE1 = "pressure1";
constexpr auto PARAM_TEMP = "temp";
// constexpr auto PARAM_TEMP1 = "temp1";
constexpr auto PARAM_SELF_SENSOR_CONNECTED = "sensor_connected";
constexpr auto PARAM_SELF_SENSOR_CONFIGURED = "sensor_configured";
constexpr auto PARAM_SELF_TEMP_CONNECTED = "temp_connected";
constexpr auto PARAM_ONEWIRE = "onewire";

void BrewingWebServer::doWebCalibrateStatus(JsonObject &obj) {
  if (myPressureSensor.isActive() || myPressureSensor1.isActive()) {
    obj[PARAM_SUCCESS] = true;
    obj[PARAM_MESSAGE] = "Calibration completed";
  } else {
    obj[PARAM_SUCCESS] = false;
    obj[PARAM_MESSAGE] = "Calibration failed, no sensors connected";
  }
}

void BrewingWebServer::doWebConfigWrite() {
  Log.notice(
      F("WEB : Configuring pressure sensors after configuration update" CR));

  myPressureSensor.setup(0, &Wire);
  // myPressureSensor1.setup(1, &Wire1);
}

void BrewingWebServer::doWebStatus(JsonObject &obj) {
  // float pressure, pressure1, temp, temp1;
  float pressure, pressure1, temp;

  pressure = myPressureSensor.getPressurePsi();
  // pressure1 = myPressureSensor1.getPressurePsi();
  pressure1 = NAN;

  temp = myTempSensor.getTempC();
  // temp = myPressureSensor.getTemperatureC();
  // temp1 = myPressureSensor1.getTemperatureC();
  // temp1 = NAN;

  if (!isnan(pressure)) {
    if (myConfig.isPressureBar()) {
      pressure = convertPsiPressureToBar(pressure);
    } else if (myConfig.isPressureKpa()) {
      pressure = convertPsiPressureToKPa(pressure);
    }

    obj[PARAM_PRESSURE] = serialized(String(pressure, DECIMALS_PRESSURE));
  }

  if (!isnan(pressure1)) {
    if (myConfig.isPressureBar()) {
      pressure = convertPsiPressureToBar(pressure1);
    } else if (myConfig.isPressureKpa()) {
      pressure = convertPsiPressureToKPa(pressure1);
    }

    obj[PARAM_PRESSURE1] = serialized(String(pressure1, DECIMALS_PRESSURE));
  }

  // if (!isnan(temp)) {
  if (myConfig.isTempUnitF()) {
    temp = convertCtoF(temp);
  }

  obj[PARAM_TEMP] = serialized(String(temp, DECIMALS_TEMP));

  obj[CONFIG_PRESSURE_UNIT] = myConfig.getPressureUnit();
  obj[PARAM_TEMP_UNIT] = String(myConfig.getTempUnit());

  obj[PARAM_SELF][PARAM_SELF_TEMP_CONNECTED] = myTempSensor.isSensorAttached();
  obj[PARAM_SELF][PARAM_SELF_SENSOR_CONNECTED] =
      !isnan(pressure) || !isnan(pressure1);
  obj[PARAM_SELF][PARAM_SELF_SENSOR_CONFIGURED] =
      myConfig.getPressureSensorType(0) != PressureSensorType::SensorNone ||
      myConfig.getPressureSensorType(1) != PressureSensorType::SensorNone;
  obj[PARAM_SELF][PARAM_SELF_BATTERY_LEVEL] = true;  // TODO; Fix this!
}

void BrewingWebServer::doTaskSensorCalibration() {
  if (myPressureSensor.isActive()) {
    myPressureSensor.calibrate();
  } else {
    Log.warning(
        F("WEB : First sensor not connnected, skipping calibration" CR));
  }

  // if (myPressureSensor1.isActive()) {
  //   myPressureSensor1.calibrate();
  // } else {
  //   Log.warning(
  //       F("WEB : Second sensor not connnected, skipping calibration" CR));
  // }
}

void BrewingWebServer::doTaskPushTestSetup(TemplatingEngine &engine,
                                           BrewingPush &push) {
  // float pressure, pressure1, temp, temp1;
  float pressure, pressure1, temp;

  pressure = myPressureSensor.getPressurePsi();
  // pressure1 = myPressureSensor1.getPressurePsi();
  pressure1 = NAN;

  temp = myTempSensor.getTempC();

  setupTemplateEnginePressure(engine, pressure, pressure1, temp, 1.0,
                              myBatteryVoltage.getVoltage());

  Log.notice(F("WEB : Running scheduled push test for %s" CR),
             _pushTestTarget.c_str());

  if (!_pushTestTarget.compareTo(PARAM_FORMAT_POST) &&
      myConfig.hasTargetHttpPost()) {
    String tpl = push.getTemplate(BrewingPush::PRESSURE_TEMPLATE_HTTP1);
    String doc = engine.create(tpl.c_str());

    if (myConfig.isHttpPostSSL() && myConfig.isSkipSslOnTest())
      Log.notice(F("PUSH: SSL enabled, skip run when not in gravity mode." CR));
    else
      push.sendHttpPost(doc);
    _pushTestEnabled = true;
  } else if (!_pushTestTarget.compareTo(PARAM_FORMAT_POST2) &&
             myConfig.hasTargetHttpPost2()) {
    String tpl = push.getTemplate(BrewingPush::PRESSURE_TEMPLATE_HTTP2);
    String doc = engine.create(tpl.c_str());
    if (myConfig.isHttpPost2SSL() && myConfig.isSkipSslOnTest())
      Log.notice(F("PUSH: SSL enabled, skip run when not in gravity mode." CR));
    else
      push.sendHttpPost2(doc);
    _pushTestEnabled = true;
  } else if (!_pushTestTarget.compareTo(PARAM_FORMAT_GET) &&
             myConfig.hasTargetHttpGet()) {
    String tpl = push.getTemplate(BrewingPush::PRESSURE_TEMPLATE_HTTP3);
    String doc = engine.create(tpl.c_str());
    if (myConfig.isHttpGetSSL() && myConfig.isSkipSslOnTest())
      Log.notice(F("PUSH: SSL enabled, skip run when not in gravity mode." CR));
    else
      push.sendHttpGet(doc);
    _pushTestEnabled = true;
  } else if (!_pushTestTarget.compareTo(PARAM_FORMAT_INFLUXDB) &&
             myConfig.hasTargetInfluxDb2()) {
    String tpl = push.getTemplate(BrewingPush::PRESSURE_TEMPLATE_INFLUX);
    String doc = engine.create(tpl.c_str());
    if (myConfig.isHttpInfluxDb2SSL() && myConfig.isSkipSslOnTest())
      Log.notice(F("PUSH: SSL enabled, skip run when not in gravity mode." CR));
    else
      push.sendInfluxDb2(doc);
    _pushTestEnabled = true;
  } else if (!_pushTestTarget.compareTo(PARAM_FORMAT_MQTT) &&
             myConfig.hasTargetMqtt()) {
    String tpl = push.getTemplate(BrewingPush::PRESSURE_TEMPLATE_MQTT);
    String doc = engine.create(tpl.c_str());
    if (myConfig.isMqttSSL() && myConfig.isSkipSslOnTest())
      Log.notice(F("PUSH: SSL enabled, skip run when not in gravity mode." CR));
    else
      push.sendMqtt(doc);
    _pushTestEnabled = true;
  }

  engine.freeMemory();
  push.clearTemplate();
}

// constexpr auto PARAM_I2C_1 = "i2c_1";

void BrewingWebServer::doTaskHardwareScanning(JsonObject &obj) {
  // JsonArray i2c1 = obj[PARAM_I2C_1].to<JsonArray>();

  // for (int i = 1, j = 0; i < 128; i++) {
  //   // The i2c_scanner uses the return value of
  //   // the Write.endTransmisstion to see if
  //   // a device did acknowledge to the address.
  //   Wire1.beginTransmission(i);
  //   int err = Wire1.endTransmission();

  //   if (err == 0) {
  //     Log.notice(F("WEB : Found device at 0x%02X." CR), i);
  //     i2c1[j][PARAM_ADRESS] = "0x" + String(i, 16);
  //     j++;
  //   }
  // }

  JsonArray onew = obj[PARAM_ONEWIRE].to<JsonArray>();

  for (int i = 0, j = 0; i < mySensors.getDS18Count(); i++) {
    DeviceAddress adr;
    mySensors.getAddress(&adr[0], i);
    Log.notice(F("WEB : Found onewire device %d." CR), i);
    onew[j][PARAM_ADRESS] = String(adr[0], 16) + String(adr[1], 16) +
                            String(adr[2], 16) + String(adr[3], 16) +
                            String(adr[4], 16) + String(adr[5], 16) +
                            String(adr[6], 16) + String(adr[7], 16);
    switch (adr[0]) {
      case DS18S20MODEL:
        onew[j][PARAM_FAMILY] = "DS18S20";
        break;
      case DS18B20MODEL:
        onew[j][PARAM_FAMILY] = "DS18B20";
        break;
      case DS1822MODEL:
        onew[j][PARAM_FAMILY] = "DS1822";
        break;
      case DS1825MODEL:
        onew[j][PARAM_FAMILY] = "DS1825";
        break;
      case DS28EA00MODEL:
        onew[j][PARAM_FAMILY] = "DS28EA00";
        break;
    }
    onew[j][PARAM_RESOLUTION] = mySensors.getResolution();
    j++;
  }
}

#endif  // PRESSUREMON

// EOF
