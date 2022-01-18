/*
MIT License

Copyright (c) 2021-22 Magnus

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
#include <ESP8266HTTPClient.h>
#include <MQTT.h>

#include <config.hpp>
#include <gyro.hpp>
#include <pushtarget.hpp>
#include <wifi.hpp>

PushTarget myPushTarget;

//
// Send the data to targets
//
void PushTarget::send(float angle, float gravity, float corrGravity,
                      float tempC, float runTime, bool force) {
  uint32_t timePassed = abs((int32_t)(millis() - ms));
  uint32_t interval = myConfig.getSleepInterval() * 1000;

  if ((timePassed < interval) && !force) {
#if LOG_LEVEL == 6 && !defined(PUSH_DISABLE_LOGGING)
    Log.verbose(F("PUSH: Timer has not expired %l vs %l." CR), timePassed,
                interval);
#endif
    return;
  }

  ms = millis();

  if (myConfig.isBrewfatherActive()) {
    LOG_PERF_START("push-brewfather");
    sendBrewfather(angle, gravity, corrGravity, tempC);
    LOG_PERF_STOP("push-brewfather");
  }

  if (myConfig.isHttpActive()) {
    LOG_PERF_START("push-http");
    sendHttp(myConfig.getHttpPushUrl(), angle, gravity, corrGravity, tempC,
             runTime);
    LOG_PERF_STOP("push-http");
  }

  if (myConfig.isHttpActive2()) {
    LOG_PERF_START("push-http2");
    sendHttp(myConfig.getHttpPushUrl2(), angle, gravity, corrGravity, tempC,
             runTime);
    LOG_PERF_STOP("push-http2");
  }

  if (myConfig.isInfluxDb2Active()) {
    LOG_PERF_START("push-influxdb2");
    sendInfluxDb2(angle, gravity, corrGravity, tempC, runTime);
    LOG_PERF_STOP("push-influxdb2");
  }

  if (myConfig.isMqttActive()) {
    LOG_PERF_START("push-mqtt");
    sendMqtt(angle, gravity, corrGravity, tempC, runTime);
    LOG_PERF_STOP("push-mqtt");
  }
}

//
// Send to influx db v2
//
void PushTarget::sendInfluxDb2(float angle, float gravity, float corrGravity,
                               float tempC, float runTime) {
#if !defined(PUSH_DISABLE_LOGGING)
  Log.notice(
      F("PUSH: Sending values to influxdb2 angle=%F, gravity=%F, temp=%F." CR),
      angle, gravity, tempC);
#endif

  HTTPClient http;
  String serverPath =
      String(myConfig.getInfluxDb2PushUrl()) +
      "/api/v2/write?org=" + String(myConfig.getInfluxDb2PushOrg()) +
      "&bucket=" + String(myConfig.getInfluxDb2PushBucket());

  http.begin(myWifi.getWifiClient(), serverPath);

  float temp = myConfig.isTempC() ? tempC : convertCtoF(tempC);
  gravity = myConfig.isGravityTempAdj() ? corrGravity : gravity;

  // Create body for influxdb2
  char buf[1024];
  if (myConfig.isGravitySG()) {
    snprintf(&buf[0], sizeof(buf),
             "measurement,host=%s,device=%s,temp-format=%c,gravity-format=%s "
             "gravity=%.4f,corr-gravity=%.4f,angle=%.2f,temp=%.2f,battery=%.2f,"
             "rssi=%d\n",
             myConfig.getMDNS(), myConfig.getID(), myConfig.getTempFormat(),
             "G", gravity, corrGravity, angle, temp,
             myBatteryVoltage.getVoltage(), WiFi.RSSI());
  } else {
    snprintf(&buf[0], sizeof(buf),
             "measurement,host=%s,device=%s,temp-format=%c,gravity-format=%s "
             "gravity=%.1f,corr-gravity=%.1f,angle=%.2f,temp=%.2f,battery=%.2f,"
             "rssi=%d\n",
             myConfig.getMDNS(), myConfig.getID(), myConfig.getTempFormat(),
             "G", convertToPlato(gravity), convertToPlato(corrGravity), angle,
             convertCtoF(temp), myBatteryVoltage.getVoltage(), WiFi.RSSI());
  }

#if LOG_LEVEL == 6 && !defined(PUSH_DISABLE_LOGGING)
  Log.verbose(F("PUSH: url %s." CR), serverPath.c_str());
  Log.verbose(F("PUSH: data %s." CR), &buf[0]);
#endif

  // Send HTTP POST request
  String auth = "Token " + String(myConfig.getInfluxDb2PushToken());
  http.addHeader(F("Authorization"), auth.c_str());
  int httpResponseCode = http.POST(&buf[0]);

  if (httpResponseCode == 204) {
    Log.notice(F("PUSH: InfluxDB2 push successful, response=%d" CR),
               httpResponseCode);
  } else {
    Log.error(F("PUSH: InfluxDB2 push failed, response=%d" CR),
              httpResponseCode);
  }

  http.end();
  myWifi.closeWifiClient();
}

//
// Send data to brewfather
//
void PushTarget::sendBrewfather(float angle, float gravity, float corrGravity,
                                float tempC) {
#if !defined(PUSH_DISABLE_LOGGING)
  Log.notice(F("PUSH: Sending values to brewfather angle=%F, gravity=%F, "
               "corr-gravity=%F, temp=%F." CR),
             angle, gravity, corrGravity, tempC);
#endif

  DynamicJsonDocument doc(300);
  //
  //  {
  //  "name": "YourDeviceName", // Required field, this will be the ID in
  //  Brewfather "temp": 20.32, "aux_temp": 15.61, // Fridge Temp
  //  "ext_temp": 6.51, // Room Temp
  //  "temp_unit": "C", // C, F, K
  //  "gravity": 1.042,
  //  "gravity_unit": "G", // G, P
  //  "pressure": 10,
  //  "pressure_unit": "PSI", // PSI, BAR, KPA
  //  "ph": 4.12,
  //  "bpm": 123, // Bubbles Per Minute
  //  "comment": "Hello World",
  //  "beer": "Pale Ale"
  //  "battery": 4.98
  //  }
  //
  float temp = myConfig.isTempC() ? tempC : convertCtoF(tempC);

  doc["name"] = myConfig.getMDNS();
  doc["temp"] = reduceFloatPrecision(temp, 1);
  doc["temp_unit"] = String(myConfig.getTempFormat());
  doc["battery"] = reduceFloatPrecision(myBatteryVoltage.getVoltage(), 2);
  if (myConfig.isGravitySG()) {
    doc["gravity"] = reduceFloatPrecision(
        myConfig.isGravityTempAdj() ? corrGravity : gravity, 4);
  } else {
    doc["gravity"] = reduceFloatPrecision(
        convertToPlato(myConfig.isGravityTempAdj() ? corrGravity : gravity), 1);
  }
  doc["gravity_unit"] = myConfig.isGravitySG() ? "G" : "P";

  HTTPClient http;
  String serverPath = myConfig.getBrewfatherPushUrl();

  // Your Domain name with URL path or IP address with path
  http.begin(myWifi.getWifiClient(), serverPath);
  String json;
  serializeJson(doc, json);
#if LOG_LEVEL == 6 && !defined(PUSH_DISABLE_LOGGING)
  Log.verbose(F("PUSH: url %s." CR), serverPath.c_str());
  Log.verbose(F("PUSH: json %s." CR), json.c_str());
#endif

  // Send HTTP POST request
  http.addHeader(F("Content-Type"), F("application/json"));
  int httpResponseCode = http.POST(json);

  if (httpResponseCode == 200) {
    Log.notice(F("PUSH: Brewfather push successful, response=%d" CR),
               httpResponseCode);
  } else {
    Log.error(F("PUSH: Brewfather push failed, response=%d" CR),
              httpResponseCode);
  }

  http.end();
  myWifi.closeWifiClient();
}

//
// Send data to http target
//
void PushTarget::createIspindleFormat(DynamicJsonDocument &doc, float angle,
                                      float gravity, float corrGravity,
                                      float tempC, float runTime) {
  float temp = myConfig.isTempC() ? tempC : convertCtoF(tempC);

  // Use iSpindle format for compatibility
  doc["name"] = myConfig.getMDNS();
  doc["ID"] = myConfig.getID();
  doc["token"] = "gravitmon";
  doc["interval"] = myConfig.getSleepInterval();
  doc["temperature"] = reduceFloatPrecision(temp, 1);
  doc["temp-units"] = String(myConfig.getTempFormat());
  if (myConfig.isGravitySG()) {
    doc["gravity"] = reduceFloatPrecision(
        myConfig.isGravityTempAdj() ? corrGravity : gravity, 4);
    doc["corr-gravity"] = reduceFloatPrecision(corrGravity, 4);
  } else {
    doc["gravity"] = reduceFloatPrecision(
        convertToPlato(myConfig.isGravityTempAdj() ? corrGravity : gravity), 1);
    doc["corr-gravity"] = reduceFloatPrecision(convertToPlato(corrGravity), 1);
  }
  doc["angle"] = reduceFloatPrecision(angle, 2);
  doc["battery"] = reduceFloatPrecision(myBatteryVoltage.getVoltage(), 2);
  doc["rssi"] = WiFi.RSSI();

  // Some additional information
  doc["gravity-unit"] = myConfig.isGravitySG() ? "G" : "P";
  doc["run-time"] = reduceFloatPrecision(runTime, 2);
}

//
// Send data to http target
//
void PushTarget::sendHttp(String serverPath, float angle, float gravity,
                          float corrGravity, float tempC, float runTime) {
#if !defined(PUSH_DISABLE_LOGGING)
  Log.notice(F("PUSH: Sending values to http angle=%F, gravity=%F, "
               "corr-gravity=%F, temp=%F." CR),
             angle, gravity, corrGravity, tempC);
#endif

  DynamicJsonDocument doc(256);
  createIspindleFormat(doc, angle, gravity, corrGravity, tempC, runTime);

  HTTPClient http;

  if (serverPath.startsWith("https://")) {
    myWifi.getWifiClientSecure().setInsecure();
    Log.notice(F("PUSH: HTTP, SSL enabled without validation." CR));
    http.begin(myWifi.getWifiClientSecure(), serverPath);
  } else {
    http.begin(myWifi.getWifiClient(), serverPath);
  }

  String json;
  serializeJson(doc, json);
#if LOG_LEVEL == 6 && !defined(PUSH_DISABLE_LOGGING)
  Log.verbose(F("PUSH: url %s." CR), serverPath.c_str());
  Log.verbose(F("PUSH: json %s." CR), json.c_str());
#endif

  // Send HTTP POST request
  http.addHeader(F("Content-Type"), F("application/json"));
  int httpResponseCode = http.POST(json);

  if (httpResponseCode == 200) {
    Log.notice(F("PUSH: HTTP push successful, response=%d" CR),
               httpResponseCode);
  } else {
    Log.error(F("PUSH: HTTP push failed, response=%d" CR), httpResponseCode);
  }

  http.end();
  myWifi.closeWifiClient();
}

//
// Send data to http target
//
void PushTarget::sendMqtt(float angle, float gravity, float corrGravity,
                          float tempC, float runTime) {
#if !defined(PUSH_DISABLE_LOGGING)
  Log.notice(F("PUSH: Sending values to mqtt angle=%F, gravity=%F, "
               "corr-gravity=%F, temp=%F." CR),
             angle, gravity, corrGravity, tempC);
#endif

  DynamicJsonDocument doc(256);
  createIspindleFormat(doc, angle, gravity, corrGravity, tempC, runTime);

  MQTTClient mqtt(512);  // Maximum message size
  String url = myConfig.getMqttUrl();

  if (url.endsWith(":8883")) {
    // Allow secure channel, but without certificate validation
    myWifi.getWifiClientSecure().setInsecure();
    Log.notice(F("PUSH: MQTT, SSL enabled without validation." CR));
    url.replace(":8883", "");
    mqtt.begin(url.c_str(), 8883, myWifi.getWifiClientSecure());
  } else {
    mqtt.begin(myConfig.getMqttUrl(), myWifi.getWifiClient());
  }

  mqtt.connect(myConfig.getMDNS(), myConfig.getMqttUser(),
               myConfig.getMqttPass());

  String json;
  serializeJson(doc, json);
#if LOG_LEVEL == 6 && !defined(PUSH_DISABLE_LOGGING)
  Log.verbose(F("PUSH: url %s." CR), myConfig.getMqttUrl());
  Log.verbose(F("PUSH: json %s." CR), json.c_str());
#endif

  // Send MQQT message
  mqtt.setTimeout(10);  // 10 seconds timeout
  if (mqtt.publish(myConfig.getMqttTopic(), json)) {
    Log.notice(F("PUSH: MQTT publish successful" CR));
  } else {
    Log.error(F("PUSH: MQTT publish failed err=%d, ret=%d" CR),
              mqtt.lastError(), mqtt.returnCode());
  }

  mqtt.disconnect();
  myWifi.closeWifiClient();
}

// EOF
