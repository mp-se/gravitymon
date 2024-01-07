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
#if defined(ESP8266)
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#else  // defined (ESP32)
#include <HTTPClient.h>
#include <WiFi.h>
#endif

#include <config.hpp>
#include <gyro.hpp>
#include <helper.hpp>
#include <main.hpp>
#include <perf.hpp>
#include <tempsensor.hpp>
#include <wifi.hpp>

#if defined(COLLECT_PERFDATA)

PerfLogging myPerfLogging;

void PerfLogging::clear() {
  // Clear the measurements
  if (first == 0) return;

  PerfEntry* pe = first;

  do {
    pe->max = 0;
    pe->start = 0;
    pe->end = 0;
    pe->mA = 0;
    pe->V = 0;
    pe = pe->next;
  } while (pe != 0);
}

void PerfLogging::start(const char* key) {
  PerfEntry* pe = add(key);
  pe->start = millis();
}

void PerfLogging::stop(const char* key) {
  PerfEntry* pe = find(key);

  if (pe != 0) {
    pe->end = millis();

    uint32_t t = pe->end - pe->start;

    if (t > pe->max) pe->max = t;
  }
}

void PerfLogging::print() {
  PerfEntry* pe = first;

  while (pe != 0) {
    Log.notice(F("PERF: %s %ums" CR), pe->key, pe->max);
    pe = pe->next;
  }
}

void PerfLogging::pushInflux() {
  if (!myConfig.isInfluxDb2Active()) return;

  if (myConfig.isInfluxSSL()) {
    Log.warning(
        F("PERF: InfluxDB2 with SSL is not supported when pushing performance "
          "data, skipping" CR));
    return;
  }

  WiFiClient wifi;
  HTTPClient http;
  String serverPath =
      String(myConfig.getInfluxDb2PushUrl()) +
      "/api/v2/write?org=" + String(myConfig.getInfluxDb2PushOrg()) +
      "&bucket=" + String(myConfig.getInfluxDb2PushBucket());

  http.begin(wifi, serverPath);

  // Create body for influxdb2, format used
  // key,host=mdns value=0.0
  String body;
  body.reserve(500);

  // Create the payload with performance data.
  // ------------------------------------------------------------------------------------------
  PerfEntry* pe = first;
  char buf[150];
  snprintf(&buf[0], sizeof(buf), "perf,host=%s,device=%s ", myConfig.getMDNS(),
           myConfig.getID());
  body += &buf[0];

  while (pe != 0) {
    if (pe->max) {
      if (pe->next)
        snprintf(&buf[0], sizeof(buf), "%s=%u,", pe->key, pe->max);
      else
        snprintf(&buf[0], sizeof(buf), "%s=%u", pe->key, pe->max);

      body += &buf[0];
    }
    pe = pe->next;
  }

  // Create the payload with debug data for validating sensor stability
  // ------------------------------------------------------------------------------------------
  snprintf(&buf[0], sizeof(buf), "\ndebug,host=%s,device=%s ",
           myConfig.getMDNS(), myConfig.getID());
  body += &buf[0];
#if defined(ESP8266)
  snprintf(&buf[0], sizeof(buf),
           "angle=%.4f,gyro-ax=%d,gyro-ay=%d,gyro-az=%d,gyro-temp=%.2f,ds-temp="
           "%.2f,heap=%d,heap-frag=%d,heap-max=%d,stack=%d",
           myGyro.getAngle(), myGyro.getLastGyroData().ax,
           myGyro.getLastGyroData().ay, myGyro.getLastGyroData().az,
           myGyro.getSensorTempC(),
           myTempSensor.getTempC(myConfig.isGyroTemp()), ESP.getFreeHeap(),
           ESP.getHeapFragmentation(), ESP.getMaxFreeBlockSize(),
           ESP.getFreeContStack());
#else  // defined (ESP32)
  snprintf(&buf[0], sizeof(buf),
           "angle=%.4f,gyro-ax=%d,gyro-ay=%d,gyro-az=%d,gyro-temp=%.2f,ds-temp="
           "%.2f,heap=%d,heap-frag=%d,heap-max=%d",
           myGyro.getAngle(), myGyro.getLastGyroData().ax,
           myGyro.getLastGyroData().ay, myGyro.getLastGyroData().az,
           myGyro.getSensorTempC(), myTempSensor.getTempC(), ESP.getFreeHeap(),
           0, ESP.getMaxAllocHeap());
#endif

  body += &buf[0];

#if LOG_LEVEL == 6 && !defined(HELPER_DISABLE_LOGGING)
  Log.verbose(F("PERF: url %s." CR), serverPath.c_str());
  Log.verbose(F("PERF: data %s." CR), body.c_str());
#endif

  // Send HTTP POST request
  String auth = "Token " + String(myConfig.getInfluxDb2PushToken());
  http.addHeader(F("Authorization"), auth.c_str());
  http.setTimeout(myConfig.getPushTimeout() * 1000);
  int httpResponseCode = http.POST(body);

  if (httpResponseCode == 204) {
#if !defined(HELPER_DISABLE_LOGGING)
    Log.notice(
        F("PERF: InfluxDB2 push performance data successful, response=%d" CR),
        httpResponseCode);
#endif
  } else {
    Log.error(F("PERF: InfluxDB2 push performance data failed, response=%d" CR),
              httpResponseCode);
  }

  http.end();
  wifi.stop();
  tcp_cleanup();
}

#endif  // src/helper.hpp src/helper.cpp

// EOF
