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

#include <calc.hpp>
#include <config.hpp>
#include <gyro.hpp>
#include <webserver.hpp>
#include <tempsensor.hpp>

constexpr auto PARAM_ANGLE = "angle";
constexpr auto PARAM_TEMP = "temp";
constexpr auto PARAM_ISPINDEL_CONFIG = "ispindel_config";
constexpr auto PARAM_GRAVITYMON1_CONFIG = "gravitymon1_config";
constexpr auto PARAM_SELF_GYRO_CONNECTED = "gyro_connected";
constexpr auto PARAM_SELF_GYRO_MOVING = "gyro_moving";
constexpr auto PARAM_SELF_GYRO_CALIBRATION = "gyro_calibration";
constexpr auto PARAM_SELF_GRAVITY_FORMULA = "gravity_formula";
constexpr auto PARAM_GYRO = "gyro";
constexpr auto PARAM_SELF_TEMP_CONNECTED = "temp_connected";
constexpr auto PARAM_ONEWIRE = "onewire";

void BrewingWebServer::doWebCalibrateStatus(JsonObject &obj) {
  if (myGyro.isConnected()) {
    obj[PARAM_SUCCESS] = true;
    obj[PARAM_MESSAGE] = "Calibration completed";
  } else {
    obj[PARAM_SUCCESS] = false;
    obj[PARAM_MESSAGE] = "Calibration failed, no gyro connected";
  }
}

void BrewingWebServer::doWebConfigWrite() {}

void BrewingWebServer::doWebStatus(JsonObject &obj) {
  double angle = 0;  // Indicate we have no valid gyro value

  if (myGyro.hasValue()) angle = myGyro.getAngle();

  double tempC = myTempSensor.getTempC();
  double gravity = calculateGravity(angle, tempC);

  obj[CONFIG_GRAVITY_FORMAT] = String(myConfig.getGravityFormat());

  obj[PARAM_ANGLE] = serialized(String(angle, DECIMALS_TILT));

  if (myConfig.isGravityTempAdj()) {
    gravity = gravityTemperatureCorrectionC(
        gravity, tempC, myConfig.getDefaultCalibrationTemp());
  }
  if (myConfig.isGravityPlato()) {
    obj[CONFIG_GRAVITY] =
        serialized(String(convertToPlato(gravity), DECIMALS_PLATO));
  } else {
    obj[CONFIG_GRAVITY] = serialized(String(gravity, DECIMALS_SG));
  }

  if (myConfig.isTempFormatC()) {
    obj[PARAM_TEMP] = serialized(String(tempC, DECIMALS_TEMP));
  } else {
    obj[PARAM_TEMP] = serialized(String(convertCtoF(tempC), DECIMALS_TEMP));
  }

  obj[PARAM_GRAVITYMON1_CONFIG] = LittleFS.exists("/gravitymon.json");
  obj[PARAM_SELF][PARAM_SELF_TEMP_CONNECTED] = myTempSensor.isSensorAttached();

#if defined(ESP8266)
  obj[PARAM_ISPINDEL_CONFIG] = LittleFS.exists("/config.json");
#else
  obj[PARAM_ISPINDEL_CONFIG] = false;
#endif

#if defined(ESP32LITE)
  obj[PARAM_HARDWARE] = "floaty";
  obj[PARAM_SELF][PARAM_SELF_BATTERY_LEVEL] =
      true;  // No hardware support for these
  obj[PARAM_SELF][PARAM_SELF_TEMP_CONNECTED] = true;
#else
  obj[PARAM_HARDWARE] = "ispindel";
  obj[PARAM_SELF][PARAM_SELF_TEMP_CONNECTED] = myTempSensor.isSensorAttached();
#endif

  obj[PARAM_SELF][PARAM_SELF_GRAVITY_FORMULA] =
      strlen(myConfig.getGravityFormula()) > 0 ? true : false;
  obj[PARAM_SELF][PARAM_SELF_GYRO_CALIBRATION] = myConfig.hasGyroCalibration();
  obj[PARAM_SELF][PARAM_SELF_GYRO_CONNECTED] = myGyro.isConnected();
  obj[PARAM_SELF][PARAM_SELF_GYRO_MOVING] = myGyro.isSensorMoving();
}

void BrewingWebServer::doTaskSensorCalibration() {
  if (myGyro.isConnected()) {
    myGyro.calibrateSensor();
  } else {
    Log.error(F("WEB : No gyro connected, skipping calibration" CR));
  }
}

void BrewingWebServer::doTaskPushTestSetup(TemplatingEngine &engine,
                                           BrewingPush &push) {
  float angle = myGyro.getAngle();
  float tempC = myTempSensor.getTempC();
  float gravitySG = calculateGravity(angle, tempC);
  float corrGravitySG = gravityTemperatureCorrectionC(
      gravitySG, tempC, myConfig.getDefaultCalibrationTemp());
  setupTemplateEngineGravity(engine, angle, gravitySG, corrGravitySG, tempC,
                             1.0, myBatteryVoltage.getVoltage());
}

void BrewingWebServer::doTaskHardwareScanning(JsonObject &obj) {
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

  // Test the gyro
  switch (myGyro.getGyroID()) {
    case 0x34:
      obj[PARAM_GYRO][PARAM_FAMILY] = "MPU6050";
      break;
    case 0x38:
      obj[PARAM_GYRO][PARAM_FAMILY] = "MPU6500";
      break;
    default:
      obj[PARAM_GYRO][PARAM_FAMILY] = "0x" + String(myGyro.getGyroID(), 16);
      break;
  }
}

#endif  // GRAVITYMON

// EOF
