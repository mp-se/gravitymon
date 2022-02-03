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
#include <ESP8266mDNS.h>
#else  // defined (ESP32)
#endif
#include <MQTT.h>

#include <config.hpp>
#include <helper.hpp>
#include <pushtarget.hpp>
#include <wifi.hpp>

//
// Send the data to targets
//
void PushTarget::send(float angle, float gravitySG, float corrGravitySG,
                      float tempC, float runTime) {
  printHeap("PUSH");
  http.setReuse(false);
  httpSecure.setReuse(false);

  TemplatingEngine engine;
  engine.initialize(angle, gravitySG, corrGravitySG, tempC, runTime);

  if (myConfig.isBrewfatherActive()) {
    LOG_PERF_START("push-brewfather");
    sendBrewfather(engine);
    LOG_PERF_STOP("push-brewfather");
  }

  if (myConfig.isHttpActive()) {
    LOG_PERF_START("push-http");
    sendHttp(engine, myConfig.isHttpSSL(), 0);
    LOG_PERF_STOP("push-http");
  }

  if (myConfig.isHttp2Active()) {
    LOG_PERF_START("push-http2");
    sendHttp(engine, myConfig.isHttp2SSL(), 1);
    LOG_PERF_STOP("push-http2");
  }

  if (myConfig.isInfluxDb2Active()) {
    LOG_PERF_START("push-influxdb2");
    sendInfluxDb2(engine);
    LOG_PERF_STOP("push-influxdb2");
  }

  if (myConfig.isMqttActive()) {
    LOG_PERF_START("push-mqtt");
    sendMqtt(engine, myConfig.isMqttSSL());
    LOG_PERF_STOP("push-mqtt");
  }
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

  http.begin(wifi, serverPath);
  http.setTimeout(myHardwareConfig.getPushTimeout());

#if LOG_LEVEL == 6 && !defined(PUSH_DISABLE_LOGGING)
  Log.verbose(F("PUSH: url %s." CR), serverPath.c_str());
  Log.verbose(F("PUSH: data %s." CR), doc.c_str());
#endif

  String auth = "Token " + String(myConfig.getInfluxDb2PushToken());
  http.addHeader(F("Authorization"), auth.c_str());
  int httpResponseCode = http.POST(doc);

  if (httpResponseCode == 204) {
    Log.notice(F("PUSH: InfluxDB2 push successful, response=%d" CR),
               httpResponseCode);
  } else {
    ErrorFileLog errLog;
    errLog.addEntry("PUSH: Influxdb push failed response=" +
                    String(httpResponseCode));
  }

  http.end();
  wifi.stop();
  tcp_cleanup();
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

  http.begin(wifi, serverPath);
  http.setTimeout(myHardwareConfig.getPushTimeout());

#if LOG_LEVEL == 6 && !defined(PUSH_DISABLE_LOGGING)
  Log.verbose(F("PUSH: url %s." CR), serverPath.c_str());
  Log.verbose(F("PUSH: json %s." CR), doc.c_str());
#endif

  http.addHeader(F("Content-Type"), F("application/json"));
  int httpResponseCode = http.POST(doc);

  if (httpResponseCode == 200) {
    Log.notice(F("PUSH: Brewfather push successful, response=%d" CR),
               httpResponseCode);
  } else {
    ErrorFileLog errLog;
    errLog.addEntry("PUSH: Brewfather push failed response=" +
                    String(httpResponseCode));
  }

  http.end();
  wifi.stop();
  tcp_cleanup();
}

//
//
//
void PushTarget::addHttpHeader(HTTPClient& http, String header) {
  if (!header.length()) return;

  int i = header.indexOf(":");
  if (i) {
    String name = header.substring(0, i);
    String value = header.substring(i + 1);
    value.trim();
    Log.notice(F("PUSH: Adding header '%s': '%s'" CR), name.c_str(),
               value.c_str());
    http.addHeader(name, value);
  } else {
    ErrorFileLog errLog;
    errLog.addEntry("PUSH: Unable to set header, invalid value " + header);
  }
}

//
// Send data to http target
//
void PushTarget::sendHttp(TemplatingEngine& engine, bool isSecure, int index) {
#if !defined(PUSH_DISABLE_LOGGING)
  Log.notice(F("PUSH: Sending values to http (%s)" CR),
             index ? "http2" : "http");
#endif
  String serverPath, doc;

  if (index == 0) {
    serverPath = myConfig.getHttpUrl();
    doc = engine.create(TemplatingEngine::TEMPLATE_HTTP1);
  } else {
    serverPath = myConfig.getHttp2Url();
    doc = engine.create(TemplatingEngine::TEMPLATE_HTTP2);
  }

  int httpResponseCode;

#if LOG_LEVEL == 6 && !defined(PUSH_DISABLE_LOGGING)
  Log.verbose(F("PUSH: url %s." CR), serverPath.c_str());
  Log.verbose(F("PUSH: json %s." CR), doc.c_str());
#endif

  if (isSecure) {
    Log.notice(F("PUSH: HTTP, SSL enabled without validation." CR));
    wifiSecure.setInsecure();
    httpSecure.begin(wifiSecure, serverPath);
    httpSecure.setTimeout(myHardwareConfig.getPushTimeout());

    if (index == 0) {
      addHttpHeader(httpSecure, myConfig.getHttpHeader(0));
      addHttpHeader(httpSecure, myConfig.getHttpHeader(1));
    } else {
      addHttpHeader(httpSecure, myConfig.getHttp2Header(0));
      addHttpHeader(httpSecure, myConfig.getHttp2Header(1));
    }

    httpResponseCode = httpSecure.POST(doc);
  } else {
    http.begin(wifi, serverPath);
    http.setTimeout(myHardwareConfig.getPushTimeout());

    if (index == 0) {
      addHttpHeader(http, myConfig.getHttpHeader(0));
      addHttpHeader(http, myConfig.getHttpHeader(1));
    } else {
      addHttpHeader(http, myConfig.getHttp2Header(0));
      addHttpHeader(http, myConfig.getHttp2Header(1));
    }

    httpResponseCode = http.POST(doc);
  }

  if (httpResponseCode == 200) {
    Log.notice(F("PUSH: HTTP push successful, response=%d" CR),
               httpResponseCode);
  } else {
    ErrorFileLog errLog;
    errLog.addEntry(
        "PUSH: HTTP push failed response=" + String(httpResponseCode) +
        String(index == 0 ? " (http)" : " (http2)"));
  }

  if (isSecure) {
    httpSecure.end();
    wifiSecure.stop();
  } else {
    http.end();
    wifi.stop();
  }
  tcp_cleanup();
}

//
// Send data to http target
//
void PushTarget::sendMqtt(TemplatingEngine& engine, bool isSecure) {
#if !defined(PUSH_DISABLE_LOGGING)
  Log.notice(F("PUSH: Sending values to mqtt." CR));
#endif

  MQTTClient mqtt(512);
  String url = myConfig.getMqttUrl();
  String doc = engine.create(TemplatingEngine::TEMPLATE_MQTT);
  int port = myConfig.getMqttPort();

  if (myConfig.isMqttSSL()) {
    Log.notice(F("PUSH: MQTT, SSL enabled without validation." CR));
    wifiSecure.setInsecure();
    mqtt.begin(url.c_str(), port, wifiSecure);
  } else {
    mqtt.begin(myConfig.getMqttUrl(), port, wifi);
  }

  mqtt.connect(myConfig.getMDNS(), myConfig.getMqttUser(),
               myConfig.getMqttPass());

#if LOG_LEVEL == 6 && !defined(PUSH_DISABLE_LOGGING)
  Log.verbose(F("PUSH: url %s." CR), myConfig.getMqttUrl());
  Log.verbose(F("PUSH: data %s." CR), doc.c_str());
#endif

  // Send MQQT message(s)
  mqtt.setTimeout(myHardwareConfig.getPushTimeout());  // 10 seconds timeout

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
      ErrorFileLog errLog;
      errLog.addEntry("PUSH: MQTT push on " + topic +
                      " failed error=" + String(mqtt.lastError()));
    }

    index = next + 1;
    lines--;
  }

  mqtt.disconnect();
  if (isSecure) {
    wifiSecure.stop();
  } else {
    wifi.stop();
  }
  tcp_cleanup();
}

// EOF
