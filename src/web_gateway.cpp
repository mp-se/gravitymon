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
#include <web_gateway.hpp>
#include <uptime.hpp>

constexpr auto PARAM_GRAVITY_DEVICE = "gravity_device";
constexpr auto PARAM_DEVICE = "device";
constexpr auto PARAM_ENDPOINT = "endpoint";
constexpr auto PARAM_GRAVITY = "gravity";
constexpr auto PARAM_TEMP = "temp";
constexpr auto PARAM_UPDATE_TIME = "update_time";
constexpr auto PARAM_PUSH_TIME = "push_time";
constexpr auto PARAM_UPTIME_SECONDS = "uptime_seconds";
constexpr auto PARAM_UPTIME_MINUTES = "uptime_minutes";
constexpr auto PARAM_UPTIME_HOURS = "uptime_hours";
constexpr auto PARAM_UPTIME_DAYS = "uptime_days";

void BrewingWebServer::doWebCalibrateStatus(JsonObject &obj) {}

void BrewingWebServer::doWebConfigWrite() {}

void BrewingWebServer::doWebStatus(JsonObject &obj) {
}

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
  obj[CONFIG_GRAVITY_FORMAT] = String(myConfig.getGravityFormat());

  JsonArray devices = obj[PARAM_GRAVITY_DEVICE].to<JsonArray>();

  // Get data from BLE
  int j = 0;

  for (int i = 0; i < NO_GRAVITYMON; i++) {
    GravitymonData gd = bleScanner.getGravitymonData(i);
    if (gd.id != "") {
      devices[j][PARAM_DEVICE] = gd.id;
      devices[j][PARAM_GRAVITY] = gd.gravity;
      devices[j][PARAM_TEMP] = gd.tempC;
      devices[j][PARAM_UPDATE_TIME] = gd.getUpdateAge();
      devices[j][PARAM_PUSH_TIME] = gd.getPushAge();
      devices[j][PARAM_ENDPOINT] = "ble";
      j++;
    }
  }

  // Get data from WIFI
  for (int i = 0; i < NO_GRAVITYMON; i++) {
    GravitymonData gd = getGravitymonData(i);
    if (gd.id != "") {
      devices[j][PARAM_DEVICE] = gd.id;
      devices[j][PARAM_GRAVITY] = gd.gravity;
      devices[j][PARAM_TEMP] = gd.tempC;
      devices[j][PARAM_UPDATE_TIME] = gd.getUpdateAge();
      devices[j][PARAM_PUSH_TIME] = gd.getPushAge();
      devices[j][PARAM_ENDPOINT] = "wifi";
      j++;
    }
  }

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

void GatewayWebServer::webHandleRemotePost(AsyncWebServerRequest *request,
                                           JsonVariant &json) {
  Log.notice(F("WEB : webServer callback for /post." CR));
  JsonObject obj = json.as<JsonObject>();

  // TODO: Add support for pressure JSON data as well.

  /* Expected format
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

  String id = !obj[PARAM_BLE_ID].isNull() ? obj[PARAM_BLE_ID].as<String>() : "";
  String token =
      !obj[PARAM_BLE_TOKEN].isNull() ? obj[PARAM_BLE_TOKEN].as<String>() : "";
  String name =
      !obj[PARAM_BLE_NAME].isNull() ? obj[PARAM_BLE_NAME].as<String>() : "";
  int interval =
      !obj[PARAM_BLE_INTERVAL].isNull() ? obj[PARAM_BLE_INTERVAL].as<int>() : 0;
  float temp = !obj[PARAM_BLE_TEMPERATURE].isNull()
                   ? obj[PARAM_BLE_TEMPERATURE].as<float>()
                   : 0.0;
  String tempUnits = !obj[PARAM_BLE_TEMP_UNITS].isNull()
                         ? obj[PARAM_BLE_TEMP_UNITS].as<String>()
                         : "";
  float gravity = !obj[PARAM_BLE_GRAVITY].isNull()
                      ? obj[PARAM_BLE_GRAVITY].as<float>()
                      : 0.0;
  float angle =
      !obj[PARAM_BLE_ANGLE].isNull() ? obj[PARAM_BLE_ANGLE].as<float>() : 0.0;
  float battery = !obj[PARAM_BLE_BATTERY].isNull()
                      ? obj[PARAM_BLE_BATTERY].as<float>()
                      : 0.0;
  int rssi = !obj[PARAM_BLE_RSSI].isNull() ? obj[PARAM_BLE_RSSI].as<int>() : 0;

  int idx = findGravitymonId(id);
  if (idx >= 0) {
    GravitymonData &data = getGravitymonData(idx);
    Log.info(F("Web : Received post from %s." CR), id.c_str());

    if (tempUnits == "C")
      data.tempC = temp;
    else
      data.tempC = convertFtoC(temp);
    data.gravity = gravity;
    data.angle = angle;
    data.battery = battery;
    data.id = id;
    data.name = name;
    data.interval = interval;
    data.token = token;
    data.rssi = rssi;
    // data.address = "";
    data.type = "Http";
    data.setUpdated();
    request->send(200);
  } else {
    Log.error(F("Web : Max devices reached - no more devices available." CR));
  }

  request->send(422);
}

#endif  // GATEWAY

// EOF
