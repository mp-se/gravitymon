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
#include <ESP8266WiFi.h>

#include <config.hpp>
#include <gyro.hpp>
#include <helper.hpp>
#include <tempsensor.hpp>

SerialDebug mySerial;
BatteryVoltage myBatteryVoltage;

//
// Print the heap information.
//
void printHeap() {
#if LOG_LEVEL == 6
  Log.verbose(F("HELP: Heap %d kb, HeapFrag %d %%, FreeSketch %d kb." CR),
              ESP.getFreeHeap() / 1024, ESP.getHeapFragmentation(),
              ESP.getFreeSketchSpace() / 1024);
#endif
}

//
// Enter deep sleep for the defined duration (Argument is seconds)
//
void deepSleep(int t) {
#if LOG_LEVEL == 6
  Log.verbose(F("HELP: Entering sleep mode for %ds." CR), t);
#endif
  uint32_t wake = t * 1000000;
  ESP.deepSleep(wake);
}

//
// Print the build options used
//
void printBuildOptions() {
  Log.notice(F("Build options: %s LOGLEVEL %d "
#ifdef SKIP_SLEEPMODE
               "SKIP_SLEEP "
#endif
#ifdef EMBED_HTML
               "EMBED_HTML "
#endif
#ifdef COLLECT_PERFDATA
               "PERFDATA "
#endif
#ifdef ACTIVATE_OTA
               "OTA "
#endif
               CR),
             CFG_APPVER, LOG_LEVEL);
}

//
// Configure serial debug output
//
SerialDebug::SerialDebug(const uint32_t serialSpeed) {
  // Start serial with auto-detected rate (default to defined BAUD)
  Serial.flush();
  Serial.begin(serialSpeed);

  getLog()->begin(LOG_LEVEL, &Serial, true);
  getLog()->setPrefix(printTimestamp);
  getLog()->notice(F("SDBG: Serial logging started at %u." CR), serialSpeed);
}

//
// Print the timestamp (ms since start of device)
//
void printTimestamp(Print* _logOutput, int _logLevel) {
  char c[12];
  snprintf(c, sizeof(c), "%10lu ", millis());
  _logOutput->print(c);
}

//
// Read and calculate the battery voltage
//
void BatteryVoltage::read() {
  // The analog pin can only handle 3.3V maximum voltage so we need to reduce
  // the voltage (from max 5V)
  float factor = myConfig.getVoltageFactor();  // Default value is 1.63
  int v = analogRead(A0);
  batteryLevel = ((3.3 / 1023) * v) * factor;
#if LOG_LEVEL == 6
  Log.verbose(
      F("BATT: Reading voltage level. Factor=%F Value=%d, Voltage=%F." CR),
      factor, v, batteryLevel);
#endif
}

#if defined(COLLECT_PERFDATA)

PerfLogging myPerfLogging;

//
// Clear the current cache
//
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

//
// Start measuring this performance point
//
void PerfLogging::start(const char* key) {
  PerfEntry* pe = add(key);
  pe->start = millis();
}

//
// Finalize measuring of this performance point
//
void PerfLogging::stop(const char* key) {
  PerfEntry* pe = find(key);

  if (pe != 0) {
    pe->end = millis();

    uint32_t t = pe->end - pe->start;

    if (t > pe->max) pe->max = t;
  }
}

//
// Print the collected performance data
//
void PerfLogging::print() {
  PerfEntry* pe = first;

  while (pe != 0) {
    Log.notice(F("PERF: %s %ums" CR), pe->key, pe->max);
    pe = pe->next;
  }
}

//
// Push collected performance data to influx (use influx configuration)
//
void PerfLogging::pushInflux() {
  if (!myConfig.isInfluxDb2Active()) return;

  WiFiClient client;
  HTTPClient http;
  String serverPath =
      String(myConfig.getInfluxDb2PushUrl()) +
      "/api/v2/write?org=" + String(myConfig.getInfluxDb2PushOrg()) +
      "&bucket=" + String(myConfig.getInfluxDb2PushBucket());

  http.begin(client, serverPath);

  // Create body for influxdb2, format used
  // key,host=mdns value=0.0
  String body;

  // Create the payload with performance data.
  // ------------------------------------------------------------------------------------------
  PerfEntry* pe = first;
  char buf[100];
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
  snprintf(
      &buf[0], sizeof(buf),
      "angle=%.4f,gyro-ax=%d,gyro-ay=%d,gyro-az=%d,gyro-temp=%.2f,ds-temp=%.2f",
      myGyro.getAngle(), myGyro.getLastGyroData().ax,
      myGyro.getLastGyroData().ay, myGyro.getLastGyroData().az,
      myGyro.getSensorTempC(), myTempSensor.getTempC());
  body += &buf[0];

  // Log.notice(F("PERF: data %s." CR), body.c_str() );

#if LOG_LEVEL == 6
  Log.verbose(F("PERF: url %s." CR), serverPath.c_str());
  Log.verbose(F("PERF: data %s." CR), body.c_str());
#endif

  // Send HTTP POST request
  String auth = "Token " + String(myConfig.getInfluxDb2PushToken());
  http.addHeader(F("Authorization"), auth.c_str());
  int httpResponseCode = http.POST(body);

  if (httpResponseCode == 204) {
    Log.notice(
        F("PERF: InfluxDB2 push performance data successful, response=%d" CR),
        httpResponseCode);
  } else {
    Log.error(F("PERF: InfluxDB2 push performance data failed, response=%d" CR),
              httpResponseCode);
  }

  http.end();
}

#endif  // COLLECT_PERFDATA

//
// Convert float to formatted string with n decimals. Buffer should be at least
// 10 chars.
//
char* convertFloatToString(float f, char* buffer, int dec) {
  dtostrf(f, 6, dec, buffer);
  return buffer;
}

//
// Reduce precision to n decimals
//
float reduceFloatPrecision(float f, int dec) {
  char buffer[5];
  dtostrf(f, 6, dec, &buffer[0]);
  return atof(&buffer[0]);
}

// EOF
