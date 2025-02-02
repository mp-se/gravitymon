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
#include <pressure.hpp>
#include <webserver.hpp>
#include <tempsensor.hpp>

constexpr auto PARAM_PRESSURE = "pressure";
constexpr auto PARAM_PRESSURE1 = "pressure1";
constexpr auto PARAM_TEMP = "temp";
// constexpr auto PARAM_TEMP1 = "temp1";
constexpr auto PARAM_SELF_SENSOR_CONNECTED = "sensor_connected";
constexpr auto PARAM_SELF_SENSOR_CONFIGURED = "sensor_configured";
constexpr auto PARAM_SELF_TEMP_CONNECTED = "temp_connected";
constexpr auto PARAM_ONEWIRE = "onewire";

void BrewingWebServer::doWebCalibrateStatus(JsonObject &obj) {
  if (myPressureSensor[0].isActive() || myPressureSensor[1].isActive()) {
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

  myPressureSensor[0].setup(0, &Wire);
  // myPressureSensor[1].setup(1, &Wire1);
}

void BrewingWebServer::doWebStatus(JsonObject &obj) {
  // float pressure, pressure1, temp, temp1;
  float pressure, pressure1, temp;

  pressure = myPressureSensor[0].getPressurePsi();
  // pressure1 = myPressureSensor[1].getPressurePsi();
  pressure1 = NAN;

  temp = myTempSensor.getTempC();
  // temp = myPressureSensor[0].getTemperatureC();
  // temp1 = myPressureSensor[1].getTemperatureC();
  // temp1 = NAN;

  if (!isnan(pressure)) {
    if (myConfig.isPressureBar()) {
      pressure = convertPsiPressureToBar(pressure);
    } else if (myConfig.isPressureKpa()) {
      pressure = convertPsiPressureToKPa(pressure);
    }

    obj[PARAM_PRESSURE] = pressure;
  }

  if (!isnan(pressure1)) {
    if (myConfig.isPressureBar()) {
      pressure = convertPsiPressureToBar(pressure1);
    } else if (myConfig.isPressureKpa()) {
      pressure = convertPsiPressureToKPa(pressure1);
    }

    obj[PARAM_PRESSURE1] = pressure1;
  }

  // if (!isnan(temp)) {
  if (myConfig.isTempUnitF()) {
    temp = convertCtoF(temp);
  }

  obj[PARAM_TEMP] = temp;
  // }

  // if (!isnan(temp1)) {
  //   if (myConfig.isTempUnitF()) {
  //     temp1 = convertCtoF(temp1);
  //   }

  //   obj[PARAM_TEMP1] = temp1;
  // }

  obj[CONFIG_PRESSURE_UNIT] = myConfig.getPressureUnit();
  obj[PARAM_TEMP_UNIT] = String(myConfig.getTempUnit());

  // obj[PARAM_SELF][PARAM_SELF_TEMP_CONNECTED] = !isnan(temp) || !isnan(temp1);
  obj[PARAM_SELF][PARAM_SELF_TEMP_CONNECTED] = myTempSensor.isSensorAttached();
  obj[PARAM_SELF][PARAM_SELF_SENSOR_CONNECTED] =
      !isnan(pressure) || !isnan(pressure1);
  obj[PARAM_SELF][PARAM_SELF_SENSOR_CONFIGURED] =
      myConfig.getPressureSensorType(0) != PressureSensorType::SensorNone ||
      myConfig.getPressureSensorType(1) != PressureSensorType::SensorNone;
  obj[PARAM_SELF][PARAM_SELF_BATTERY_LEVEL] = true;  // TODO; Fix this!
}

void BrewingWebServer::doTaskSensorCalibration() {
  if (myPressureSensor[0].isActive()) {
    myPressureSensor[0].calibrate();
  } else {
    Log.warning(
        F("WEB : First sensor not connnected, skipping calibration" CR));
  }

  // if (myPressureSensor[1].isActive()) {
  //   myPressureSensor[1].calibrate();
  // } else {
  //   Log.warning(
  //       F("WEB : Second sensor not connnected, skipping calibration" CR));
  // }
}

void BrewingWebServer::doTaskPushTestSetup(TemplatingEngine &engine,
                                           BrewingPush &push) {
  // float pressure, pressure1, temp, temp1;
  float pressure, pressure1, temp;

  pressure = myPressureSensor[0].getPressurePsi();
  // pressure1 = myPressureSensor[1].getPressurePsi();
  pressure1 = NAN;

  temp = myTempSensor.getTempC();
  // temp = myPressureSensor[0].getTemperatureC();
  // temp1 = myPressureSensor[1].getTemperatureC();
  // temp1 = NAN;

  setupTemplateEnginePressure(engine, pressure, pressure1, temp,1.0,
                              myBatteryVoltage.getVoltage());
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
