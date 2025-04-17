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

#include <ArduinoJson.h>

#include <ble.hpp>
#include <config.hpp>
#include <measurement.hpp>
#include <uptime.hpp>
#include <web_gateway.hpp>

constexpr auto PARAM_GRAVITY_DEVICE = "gravity_device";
constexpr auto PARAM_PRESSURE_DEVICE = "pressure_device";
constexpr auto PARAM_TEMPERATURE_DEVICE = "temperature_device";
constexpr auto PARAM_DEVICE = "device";
constexpr auto PARAM_TYPE = "type";
constexpr auto PARAM_SOURCE = "source";
constexpr auto PARAM_GRAVITY = "gravity";
constexpr auto PARAM_PRESSURE = "pressure";
constexpr auto PARAM_PRESSURE1 = "pressure1";
constexpr auto PARAM_TEMP = "temp";
constexpr auto PARAM_CHAMBER_TEMP = "chamber_temp";
constexpr auto PARAM_BEER_TEMP = "beer_temp";
constexpr auto PARAM_UPDATE_TIME = "update_time";
constexpr auto PARAM_PUSH_TIME = "push_time";
constexpr auto PARAM_UPTIME_SECONDS = "uptime_seconds";
constexpr auto PARAM_UPTIME_MINUTES = "uptime_minutes";
constexpr auto PARAM_UPTIME_HOURS = "uptime_hours";
constexpr auto PARAM_UPTIME_DAYS = "uptime_days";

void BrewingWebServer::doWebCalibrateStatus(JsonObject &obj) {}

void BrewingWebServer::doWebConfigWrite() {}

void BrewingWebServer::doWebStatus(JsonObject &obj) {}

void BrewingWebServer::doTaskSensorCalibration() {}

void BrewingWebServer::doTaskPushTestSetup(TemplatingEngine &engine,
                                           BrewingPush &push) {
  // TODO Add support for push test related to gateway
}

void BrewingWebServer::doTaskHardwareScanning(JsonObject &obj) {}

// Methods specific for GatewayWebServer

GatewayWebServer::GatewayWebServer(WebConfig *config)
    : BrewingWebServer(config) {}

void GatewayWebServer::doWebStatus(JsonObject &obj) {
  JsonArray gravityDevices = obj[PARAM_GRAVITY_DEVICE].to<JsonArray>();
  JsonArray pressureDevices = obj[PARAM_PRESSURE_DEVICE].to<JsonArray>();
  JsonArray temperatureDevices = obj[PARAM_TEMPERATURE_DEVICE].to<JsonArray>();
  int gravIdx = 0, pressIdx = 0, tempIdx = 0;

  for (int i = 0; i < myMeasurementList.size(); i++) {
    MeasurementEntry *entry = myMeasurementList.getMeasurementEntry(i);

    switch (entry->getType()) {
      case MeasurementType::Gravitymon: {
        Log.notice("Loop: Processing Gravitymon data %d." CR, i);
        const GravityData *gd = entry->getGravityData();

        gravityDevices[gravIdx][PARAM_DEVICE] = gd->getId();
        gravityDevices[gravIdx][PARAM_GRAVITY] = gd->getGravity();
        gravityDevices[gravIdx][PARAM_TEMP] = gd->getTempC();
        gravityDevices[gravIdx][PARAM_UPDATE_TIME] = entry->getUpdateAge();
        gravityDevices[gravIdx][PARAM_PUSH_TIME] = entry->getPushAge();
        gravityDevices[gravIdx][PARAM_SOURCE] = gd->getSourceAsString();
        gravityDevices[gravIdx][PARAM_TYPE] = gd->getTypeAsString();
        gravIdx++;
      } break;

      case MeasurementType::Pressuremon: {
        Log.notice("Loop: Processing Pressuremon data %d." CR, i);
        const PressureData *pd = entry->getPressureData();

        pressureDevices[pressIdx][PARAM_DEVICE] = pd->getId();
        pressureDevices[pressIdx][PARAM_PRESSURE] = pd->getPressure();
        pressureDevices[pressIdx][PARAM_PRESSURE1] = pd->getPressure1();
        pressureDevices[pressIdx][PARAM_TEMP] = pd->getTempC();
        pressureDevices[pressIdx][PARAM_UPDATE_TIME] = entry->getUpdateAge();
        pressureDevices[pressIdx][PARAM_PUSH_TIME] = entry->getPushAge();
        pressureDevices[pressIdx][PARAM_SOURCE] = pd->getSourceAsString();
        pressureDevices[pressIdx][PARAM_TYPE] = pd->getTypeAsString();
        pressIdx++;
      } break;

      case MeasurementType::Tilt:
      case MeasurementType::TiltPro: {
        Log.notice("Loop: Processing Tilt data %d." CR, i);
        const TiltData *td = entry->getTiltData();

        gravityDevices[gravIdx][PARAM_DEVICE] = td->getId();
        gravityDevices[gravIdx][PARAM_GRAVITY] = td->getGravity();
        gravityDevices[gravIdx][PARAM_TEMP] = td->getTempC();
        gravityDevices[gravIdx][PARAM_UPDATE_TIME] = entry->getUpdateAge();
        gravityDevices[gravIdx][PARAM_PUSH_TIME] = entry->getPushAge();
        gravityDevices[gravIdx][PARAM_SOURCE] = td->getSourceAsString();
        gravityDevices[gravIdx][PARAM_TYPE] = td->getTypeAsString();
        gravIdx++;
      } break;

      case MeasurementType::Chamber: {
        Log.notice("Loop: Processing Tilt data %d." CR, i);
        const ChamberData *cd = entry->getChamberData();

        temperatureDevices[tempIdx][PARAM_DEVICE] = cd->getId();
        temperatureDevices[tempIdx][PARAM_CHAMBER_TEMP] = cd->getChamberTempC();
        temperatureDevices[tempIdx][PARAM_BEER_TEMP] = cd->getBeerTempC();
        temperatureDevices[tempIdx][PARAM_UPDATE_TIME] = entry->getUpdateAge();
        temperatureDevices[tempIdx][PARAM_PUSH_TIME] = entry->getPushAge();
        temperatureDevices[tempIdx][PARAM_SOURCE] = cd->getSourceAsString();
        temperatureDevices[tempIdx][PARAM_TYPE] = cd->getTypeAsString();
        tempIdx++;
      } break;
    }
  }

  // Add other params
  obj[CONFIG_GRAVITY_UNIT] = String(myConfig.getGravityUnit());
  obj[CONFIG_PRESSURE_UNIT] = myConfig.getPressureUnit();
  obj[PARAM_UPTIME_SECONDS] = myUptime.getSeconds();
  obj[PARAM_UPTIME_MINUTES] = myUptime.getMinutes();
  obj[PARAM_UPTIME_HOURS] = myUptime.getHours();
  obj[PARAM_UPTIME_DAYS] = myUptime.getDays();
}

bool GatewayWebServer::setupWebServer(const char *serviceName) {
  bool b = BrewingWebServer::setupWebServer(serviceName);

  AsyncCallbackJsonWebHandler *handler;
  handler = new AsyncCallbackJsonWebHandler(
      "/post", std::bind(&GatewayWebServer::webHandleRemotePost, this,
                         std::placeholders::_1, std::placeholders::_2));
  _server->addHandler(handler);
  return b;
}

constexpr auto GRAVMON_NAME = "name";
constexpr auto GRAVMON_ID = "ID";
constexpr auto GRAVMON_TOKEN = "token";
constexpr auto GRAVMON_INTERVAL = "interval";
constexpr auto GRAVMON_TEMPERATURE = "temperature";
constexpr auto GRAVMON_TEMPERATURE_UNIT = "temp_units";
constexpr auto GRAVMON_GRAVITY = "gravity";
constexpr auto GRAVMON_ANGLE = "angle";
constexpr auto GRAVMON_BATTERY = "battery";
constexpr auto GRAVMON_RSSI = "RSSI";

constexpr auto PRESSMON_NAME = "name";
constexpr auto PRESSMON_ID = "id";
constexpr auto PRESSMON_TOKEN = "token";
constexpr auto PRESSMON_INTERVAL = "interval";
constexpr auto PRESSMON_TEMPERATURE = "temperature";
constexpr auto PRESSMON_TEMPERATURE_UNIT = "temperature-unit";
constexpr auto PRESSMON_PRESSURE = "pressure";
constexpr auto PRESSMON_PRESSURE1 = "pressure1";
constexpr auto PRESSMON_PRESSURE_UNIT = "pressure-unit";
constexpr auto PRESSMON_BATTERY = "battery";
constexpr auto PRESSMON_RSSI = "rssi";

void GatewayWebServer::webHandleRemotePost(AsyncWebServerRequest *request,
                                           JsonVariant &json) {
  Log.notice(F("WEB : webServer callback for /post." CR));
  JsonObject obj = json.as<JsonObject>();

  /* Expected gravitymon format.
  {
    "name": "gravitymon-gwfa413c",
    "ID": "fa413c",
    "token": "",
    "interval": 900,
    "temperature": 20.1,
    "temp_units": "C",
    "gravity": 1.015,
    "angle": 35,
    "battery": 4,
    "RSSI": -79
  }*/

  /* Expected pressuremon format.
  {
    "name": "test",
    "id": "e17d44",
    "token": "",
    "interval": 300,
    "temperature": 22.3,
    "temperature-unit": "C",
    "pressure": 1.001,
    "pressure-unit": "PSI",
    "battery": 3.85,
    "rssi": -76.2
  }*/

  if (!obj[GRAVMON_GRAVITY].isNull() || !obj[PRESSMON_PRESSURE].isNull()) {
    String json;
    json.reserve(200);
    serializeJson(obj, json);
    _postData.push(json);
    request->send(200);
    return;
  }

  request->send(422);
}

void GatewayWebServer::loop() {
  // Process data that was received in posts
  while (!_postData.empty()) {
    String data = _postData.front();

    Serial.printf("Processing data: %s\n", data.c_str());

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, data);
    JsonObject obj = doc.as<JsonObject>();

    // Process gravitymon data
    if (!obj[GRAVMON_GRAVITY].isNull()) {
      String id = !obj[GRAVMON_ID].isNull() ? obj[GRAVMON_ID].as<String>() : "";
      String token =
          !obj[GRAVMON_TOKEN].isNull() ? obj[GRAVMON_TOKEN].as<String>() : "";
      String name =
          !obj[GRAVMON_NAME].isNull() ? obj[GRAVMON_NAME].as<String>() : "";
      int interval =
          !obj[GRAVMON_INTERVAL].isNull() ? obj[GRAVMON_INTERVAL].as<int>() : 0;
      float temp = !obj[GRAVMON_TEMPERATURE].isNull()
                       ? obj[GRAVMON_TEMPERATURE].as<float>()
                       : 0.0;
      String tempUnits = !obj[GRAVMON_TEMPERATURE_UNIT].isNull()
                             ? obj[GRAVMON_TEMPERATURE_UNIT].as<String>()
                             : "";
      float gravity = !obj[GRAVMON_GRAVITY].isNull()
                          ? obj[GRAVMON_GRAVITY].as<float>()
                          : 0.0;
      float angle =
          !obj[GRAVMON_ANGLE].isNull() ? obj[GRAVMON_ANGLE].as<float>() : 0.0;
      float battery = !obj[GRAVMON_BATTERY].isNull()
                          ? obj[GRAVMON_BATTERY].as<float>()
                          : 0.0;
      int rssi = !obj[GRAVMON_RSSI].isNull() ? obj[GRAVMON_RSSI].as<int>() : 0;

      std::unique_ptr<MeasurementBaseData> gravityData;
      gravityData.reset(new GravityData(MeasurementSource::HttpPost, id, name,
                                        token, temp, gravity, angle, battery, 0,
                                        rssi, interval));

      Log.info(F("WEB : Update data for gravitymon %s." CR),
               gravityData->getId());
      myMeasurementList.updateData(gravityData);
    }

    // Process pressuremon data
    if (!obj[PRESSMON_PRESSURE].isNull()) {
      String id =
          !obj[PRESSMON_ID].isNull() ? obj[PRESSMON_ID].as<String>() : "";
      String token =
          !obj[PRESSMON_TOKEN].isNull() ? obj[PRESSMON_TOKEN].as<String>() : "";
      String name =
          !obj[PRESSMON_NAME].isNull() ? obj[PRESSMON_NAME].as<String>() : "";
      int interval = !obj[PRESSMON_INTERVAL].isNull()
                         ? obj[PRESSMON_INTERVAL].as<int>()
                         : 0;
      float temp = !obj[PRESSMON_TEMPERATURE].isNull()
                       ? obj[PRESSMON_TEMPERATURE].as<float>()
                       : 0.0;
      String tempUnits = !obj[PRESSMON_PRESSURE_UNIT].isNull()
                             ? obj[PRESSMON_PRESSURE_UNIT].as<String>()
                             : "";
      float pressure = !obj[PRESSMON_PRESSURE].isNull()
                           ? obj[PRESSMON_PRESSURE].as<float>()
                           : 0.0;
      float pressure1 = !obj[PRESSMON_PRESSURE1].isNull()
                            ? obj[PRESSMON_PRESSURE1].as<float>()
                            : 0.0;
      float battery = !obj[PRESSMON_BATTERY].isNull()
                          ? obj[PRESSMON_BATTERY].as<float>()
                          : 0.0;
      int rssi =
          !obj[PRESSMON_RSSI].isNull() ? obj[PRESSMON_RSSI].as<int>() : 0;

      std::unique_ptr<MeasurementBaseData> pressureData;
      pressureData.reset(new PressureData(MeasurementSource::HttpPost, id, name,
                                          token, temp, pressure, pressure1,
                                          battery, 0, rssi, interval));

      Log.info(F("WEB : Update data for pressuremon %s." CR),
               pressureData->getId());
      myMeasurementList.updateData(pressureData);
    }

    _postData.pop();
  }
}

#endif  // GATEWAY

// EOF
