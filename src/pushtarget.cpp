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
#if defined(ESP8266)
#include <ESP8266HTTPClient.h>
#include <ESP8266mDNS.h>
#else  // defined (ESP32)
#include <HTTPClient.h>
#endif
#include <MQTT.h>

#include <config.hpp>
#include <helper.hpp>
#include <pushtarget.hpp>
#include <wifi.hpp>

PushTarget myPushTarget;

//
// Send the data to targets
//
void PushTarget::send(float angle, float gravitySG, float corrGravitySG,
                      float tempC, float runTime, bool force) {
  uint32_t timePassed = abs((int32_t)(millis() - _ms));
  uint32_t interval = myConfig.getSleepInterval() * 1000;

  if ((timePassed < interval) && !force) {
#if LOG_LEVEL == 6 && !defined(PUSH_DISABLE_LOGGING)
    Log.verbose(F("PUSH: Timer has not expired %l vs %l." CR), timePassed,
                interval);
#endif
    return;
  }

  _ms = millis();

#if defined(ESP8266)
  if (ESP.getFreeContStack() < 1500) {
    Log.error(F("PUSH: Low on memory, skipping push since it will crasch. "
                "(stack=%d, heap=%d)." CR),
              ESP.getFreeContStack(), ESP.getFreeHeap());
    myWifi.closeWifiClient();
    return;
  }
#endif

  TemplatingEngine engine;
  engine.initialize(angle, gravitySG, corrGravitySG, tempC, runTime);

  if (myConfig.isBrewfatherActive()) {
    LOG_PERF_START("push-brewfather");
    sendBrewfather(engine);
    LOG_PERF_STOP("push-brewfather");
  }

  if (myConfig.isHttpActive()) {
    LOG_PERF_START("push-http");
    sendHttp(engine, 0);
    LOG_PERF_STOP("push-http");
  }

  if (myConfig.isHttpActive2()) {
    LOG_PERF_START("push-http2");
    sendHttp(engine, 1);
    LOG_PERF_STOP("push-http2");
  }

  if (myConfig.isInfluxDb2Active()) {
    LOG_PERF_START("push-influxdb2");
    sendInfluxDb2(engine);
    LOG_PERF_STOP("push-influxdb2");
  }

  if (myConfig.isMqttActive()) {
    LOG_PERF_START("push-mqtt");
    sendMqtt(engine);
    LOG_PERF_STOP("push-mqtt");
  }

  LOG_PERF_PUSH();
}

//
// Send to influx db v2
//
void PushTarget::sendInfluxDb2(TemplatingEngine& engine) {
#if !defined(PUSH_DISABLE_LOGGING)
  Log.notice(F("PUSH: Sending values to influxdb2." CR));
#endif

  String serverPath =
      String(myConfig.getInfluxDb2PushUrl()) +
      "/api/v2/write?org=" + String(myConfig.getInfluxDb2PushOrg()) +
      "&bucket=" + String(myConfig.getInfluxDb2PushBucket());
  String doc = engine.create(TemplatingEngine::TEMPLATE_INFLUX);

  HTTPClient http;
  http.begin(myWifi.getWifiClient(), serverPath);

#if LOG_LEVEL == 6 && !defined(PUSH_DISABLE_LOGGING)
  Log.verbose(F("PUSH: url %s." CR), serverPath.c_str());
  Log.verbose(F("PUSH: data %s." CR), doc.c_str());
#endif

  // Send HTTP POST request
  String auth = "Token " + String(myConfig.getInfluxDb2PushToken());
  http.addHeader(F("Authorization"), auth.c_str());
  int httpResponseCode = http.POST(doc);

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
void PushTarget::sendBrewfather(TemplatingEngine& engine) {
#if !defined(PUSH_DISABLE_LOGGING)
  Log.notice(F("PUSH: Sending values to brewfather" CR));
#endif

  String serverPath = myConfig.getBrewfatherPushUrl();
  String doc = engine.create(TemplatingEngine::TEMPLATE_BREWFATHER);

  // Your Domain name with URL path or IP address with path
  HTTPClient http;
  http.begin(myWifi.getWifiClient(), serverPath);

#if LOG_LEVEL == 6 && !defined(PUSH_DISABLE_LOGGING)
  Log.verbose(F("PUSH: url %s." CR), serverPath.c_str());
  Log.verbose(F("PUSH: json %s." CR), doc.c_str());
#endif

  // Send HTTP POST request
  http.addHeader(F("Content-Type"), F("application/json"));
  int httpResponseCode = http.POST(doc);

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
void PushTarget::sendHttp(TemplatingEngine& engine, int index) {
#if !defined(PUSH_DISABLE_LOGGING)
  Log.notice(F("PUSH: Sending values to http (%s)" CR),
             index ? "http2" : "http1");
#endif

  String serverPath, doc;

  if (index == 0) {
    serverPath = myConfig.getHttpPushUrl();
    doc = engine.create(TemplatingEngine::TEMPLATE_HTTP1);
  } else {
    serverPath = myConfig.getHttpPushUrl2();
    doc = engine.create(TemplatingEngine::TEMPLATE_HTTP2);
  }

  HTTPClient http;
  if (serverPath.startsWith("https://")) {
    myWifi.getWifiClientSecure().setInsecure();
    Log.notice(F("PUSH: HTTP, SSL enabled without validation." CR));
    http.begin(myWifi.getWifiClientSecure(), serverPath);
  } else {
    http.begin(myWifi.getWifiClient(), serverPath);
  }

#if LOG_LEVEL == 6 && !defined(PUSH_DISABLE_LOGGING)
  Log.verbose(F("PUSH: url %s." CR), serverPath.c_str());
  Log.verbose(F("PUSH: json %s." CR), doc.c_str());
#endif

  // Send HTTP POST request
  http.addHeader(F("Content-Type"), F("application/json"));
  int httpResponseCode = http.POST(doc);

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
void PushTarget::sendMqtt(TemplatingEngine& engine) {
#if !defined(PUSH_DISABLE_LOGGING)
  Log.notice(F("PUSH: Sending values to mqtt." CR));
#endif

  MQTTClient mqtt(512);
  String url = myConfig.getMqttUrl();
  String doc = engine.create(TemplatingEngine::TEMPLATE_MQTT);
  int port = myConfig.getMqttPort();

  // if (url.endsWith(":8883")) {
  if (port > 8000) {
    // Allow secure channel, but without certificate validation
    myWifi.getWifiClientSecure().setInsecure();
    Log.notice(F("PUSH: MQTT, SSL enabled without validation." CR));
    mqtt.begin(url.c_str(), port, myWifi.getWifiClientSecure());
  } else {
    mqtt.begin(myConfig.getMqttUrl(), port, myWifi.getWifiClient());
  }

  mqtt.connect(myConfig.getMDNS(), myConfig.getMqttUser(),
               myConfig.getMqttPass());

#if LOG_LEVEL == 6 && !defined(PUSH_DISABLE_LOGGING)
  Log.verbose(F("PUSH: url %s." CR), myConfig.getMqttUrl());
  Log.verbose(F("PUSH: data %s." CR), doc.c_str());
#endif

  // Send MQQT message(s)
  mqtt.setTimeout(10);  // 10 seconds timeout

  int lines = 1;
  // Find out how many lines are in the document. Each line is one
  // topic/message. | is used as new line.
  for (unsigned int i = 0; i < doc.length() - 1; i++) {
    if (doc.charAt(i) == '|') lines++;
  }

  int index = 0;
  while (lines) {
    int next = doc.indexOf('|', index);
    String line = doc.substring(index, next);

    // Each line equals one topic post, format is <topic>:<value>
    String topic = line.substring(0, line.indexOf(":"));
    String value = line.substring(line.indexOf(":") + 1);
#if LOG_LEVEL == 6 && !defined(PUSH_DISABLE_LOGGING)
    Log.verbose(F("PUSH: topic '%s', value '%s'." CR), topic.c_str(),
                value.c_str());
#endif
    if (mqtt.publish(topic, value)) {
      Log.notice(F("PUSH: MQTT publish successful on %s" CR), topic.c_str());
    } else {
      Log.error(F("PUSH: MQTT publish failed err=%d, ret=%d" CR),
                mqtt.lastError(), mqtt.returnCode());
    }

    index = next + 1;
    lines--;
  }

  mqtt.disconnect();
  myWifi.closeWifiClient();
}

// EOF
