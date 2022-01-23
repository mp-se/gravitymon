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
#if defined (ESP8266)
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#else // defined (ESP32)
#include <WiFi.h>
#include <HTTPClient.h>
#endif

#include <config.hpp>
#include <gyro.hpp>
#include <helper.hpp>
#include <main.hpp>
#include <tempsensor.hpp>
#include <wifi.hpp>

SerialDebug mySerial;
BatteryVoltage myBatteryVoltage;

//
// Convert sg to plato
//
double convertToPlato(double sg) { return 259 - (259 / sg); }

//
// Convert plato to sg
//
double convertToSG(double plato) { return 259 / (259 - plato); }

//
// Conversion to F
//
float convertCtoF(float c) { return (c * 1.8) + 32.0; }

//
// Conversion to C
//
float convertFtoC(float f) { return (f - 32.0) / 1.8; }

//
// Print the heap information.
//
void printHeap() {
#if LOG_LEVEL == 6 && !defined(HELPER_DISABLE_LOGGING)
#if defined (ESP8266)
  Log.verbose(F("HELP: Heap %d kb, HeapFrag %d %%, FreeSketch %d kb." CR),
              ESP.getFreeHeap() / 1024, ESP.getHeapFragmentation(),
              ESP.getFreeSketchSpace() / 1024);
#else // defined (ESP32)
  Log.verbose(F("HELP: Heap %d kb, FreeSketch %d kb." CR),
              ESP.getFreeHeap() / 1024, ESP.getFreeSketchSpace() / 1024);
#endif
#endif
}

//
// Enter deep sleep for the defined duration (Argument is seconds)
//
void deepSleep(int t) {
#if LOG_LEVEL == 6 && !defined(HELPER_DISABLE_LOGGING)
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

  // An ESP8266 has a ADC range of 0-1023 and a maximum voltage of 3.3V
  // An ESP32 has an ADC range of 0-4095 and a maximum voltage of 3.3V

#if defined (ESP8266)
  _batteryLevel = ((3.3 / 1023) * v) * factor;
#else // defined (ESP32)
  _batteryLevel = ((3.3 / 4095) * v) * factor;
#endif 
#if LOG_LEVEL == 6 && !defined(HELPER_DISABLE_LOGGING)
  Log.verbose(
      F("BATT: Reading voltage level. Factor=%F Value=%d, Voltage=%F." CR),
      factor, v, _batteryLevel);
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

  HTTPClient http;
  String serverPath =
      String(myConfig.getInfluxDb2PushUrl()) +
      "/api/v2/write?org=" + String(myConfig.getInfluxDb2PushOrg()) +
      "&bucket=" + String(myConfig.getInfluxDb2PushBucket());

  http.begin(myWifi.getWifiClient(), serverPath);

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
      myGyro.getSensorTempC(), myTempSensor.getTempC(myConfig.isGyroTemp()));
  body += &buf[0];

  // Log.notice(F("PERF: data %s." CR), body.c_str() );

#if LOG_LEVEL == 6 && !defined(HELPER_DISABLE_LOGGING)
  Log.verbose(F("PERF: url %s." CR), serverPath.c_str());
  Log.verbose(F("PERF: data %s." CR), body.c_str());
#endif

  // Send HTTP POST request
  String auth = "Token " + String(myConfig.getInfluxDb2PushToken());
  http.addHeader(F("Authorization"), auth.c_str());
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
  myWifi.closeWifiClient();
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

//
// urlencode
//
// https://circuits4you.com/2019/03/21/esp8266-url-encode-decode-example/
//
String urlencode(String str) {
  String encodedString = "";
  char c;
  char code0;
  char code1;
  for (int i =0; i < static_cast<int>(str.length()); i++) {
    c = str.charAt(i);
    if (isalnum(c)){
      encodedString += c;
    } else {
      code1 = (c & 0xf) + '0';
      if ((c & 0xf) >9) {
          code1 = (c & 0xf) - 10 + 'A';
      }
      c = (c>>4) & 0xf;
      code0 = c + '0';
      if (c > 9) {
          code0 = c - 10 + 'A';
      }
      encodedString += '%';
      encodedString += code0;
      encodedString += code1;
    }
  }
  //Log.verbose(F("HELP: encode=%s" CR), encodedString.c_str());
  return encodedString; 
}

unsigned char h2int(char c) {
  if (c >= '0' && c <='9') {
    return((unsigned char)c - '0');
  }
  if (c >= 'a' && c <='f') {
    return((unsigned char)c - 'a' + 10);
  }
  if (c >= 'A' && c <='F') {
    return((unsigned char)c - 'A' + 10);
  }
  return(0);
}

String urldecode(String str) {
  String encodedString = "";
  char c;
  char code0;
  char code1;
  for (int i = 0; i < static_cast<int>(str.length()); i++){
    c = str.charAt(i);
    if (c == '%') {
      i++;
      code0 = str.charAt(i);
      i++;
      code1 = str.charAt(i);
      c = (h2int(code0) << 4) | h2int(code1);
      encodedString += c;
    } else {
      encodedString += c;
    }
  }   

  //Log.verbose(F("HELP: decode=%s" CR), encodedString.c_str());
  return encodedString;
}


// EOF
