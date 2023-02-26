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
#include <ESP8266WiFi.h>
#else  // defined (ESP32)
#include <HTTPClient.h>
#include <WiFi.h>
#endif

#include <config.hpp>
#include <gyro.hpp>
#include <helper.hpp>
#include <main.hpp>
#include <tempsensor.hpp>
#include <wifi.hpp>

// tcp cleanup, to avoid memory crash.
struct tcp_pcb;
extern struct tcp_pcb* tcp_tw_pcbs;
extern "C" void tcp_abort(struct tcp_pcb* pcb);
void tcp_cleanup() {
  while (tcp_tw_pcbs) tcp_abort(tcp_tw_pcbs);
}

#if defined(ESP8266)
void detectChipRevision() {}
bool isEsp32c3() { return false; }
#else
esp_chip_info_t chip_info;

bool isEsp32c3() { return (chip_info.model == CHIP_ESP32C3); }

void detectChipRevision() {
  esp_chip_info(&chip_info);
  String chipModel;

  /*
    ESP32D1 = ESP32 rev 1
    ESP32S2 = ESP32S2 rev 0
    ESP32C3 = ESP32C3 rev 3
  */

  switch (chip_info.model) {
    case CHIP_ESP32:
      chipModel = "ESP32";
      break;
    case CHIP_ESP32S2:
      chipModel = "ESP32S2";
      break;
    case CHIP_ESP32C3:
      chipModel = "ESP32C3";
      break;
    case CHIP_ESP32S3:
    case CHIP_ESP32H2:
    default:
      chipModel = "Unsupported";
      break;
  }

  Log.notice(F("HELP: Chip=%s, Rev=%d, Feat=%X" CR), chipModel.c_str(),
             chip_info.revision, chip_info.features);
}
#endif

void checkResetReason() {
#if defined(ESP8266)
  rst_info* _rinfo;
  _rinfo = ESP.getResetInfoPtr();

  Log.notice(F("HELP: Last reset cause %d" CR), _rinfo->exccause);

  if (_rinfo->exccause > 0) {
    char s[120];
    snprintf(&s[0], sizeof(s),
             "HELP: Exception (%d) reason=%d epc1=0x%08x epc2=0x%08x "
             "epc3=0x%08x execvaddr=0x%08x depc=0x%08x",
             _rinfo->exccause, _rinfo->reason, _rinfo->epc1, _rinfo->epc2,
             _rinfo->epc3, _rinfo->excvaddr, _rinfo->depc);
    writeErrorLog(&s[0]);
  }
#else  // defined (ESP32)
  RESET_REASON r = rtc_get_reset_reason(
      0);  // We only check cpu0 since we dont use cpu1 on the esp32
  String rStr;

  switch (r) {
    case 0:
      rStr = F("None");
      break;
    case 1:
      rStr = F("vbat power on reset");
      break;
    case 3:
      rStr = F("software reset digital core");
      break;
    case 4:
      rStr = F("legacy watch dog reset digital core");
      break;
    case 5:
      rStr = F("deep Sleep reset digital core");
      break;
    case 6:
      rStr = F("reset by SLC module, reset digital core");
      break;
    case 7:
      rStr = F("timer Group0 Watch dog reset digital core");
      break;
    case 8:
      rStr = F("timer Group1 Watch dog reset digital core");
      break;
    case 9:
      rStr = F("RTC Watch dog Reset digital core");
      break;
    case 10:
      rStr = F("instrusion tested to reset CPU");
      break;
    case 11:
      rStr = F("time Group reset CPU");
      break;
    case 12:
      rStr = F("software reset CPU");
      break;
    case 13:
      rStr = F("RTC Watch dog Reset CPU");
      break;
    case 14:
      rStr = F("for APP CPU, reseted by PRO CPU");
      break;
    case 15:
      rStr = F("reset when the vdd voltage is not stable");
      break;
    case 16:
      rStr = F("RTC Watch dog reset digital core and rtc module");
      break;
    default:
      rStr = F("unknown reset reason");
      break;
  }

  Log.notice(F("HELP: Last reset cause '%s' (%d)" CR), rStr.c_str(), r);
  // Havent found a good way to get exception cause from an ESP32
#endif
}

void writeErrorLog(const char* format, ...) {
  File f = LittleFS.open(ERR_FILENAME, "a");

  if (f && f.size() > ERR_FILEMAXSIZE) {
    f.close();
    LittleFS.remove(ERR_FILENAME2);
    LittleFS.rename(ERR_FILENAME, ERR_FILENAME2);
    f = LittleFS.open(ERR_FILENAME, "a");
  }

  va_list arg;
  va_start(arg, format);
  char buf[120];
  vsnprintf(&buf[0], sizeof(buf), format, arg);
  va_end(arg);
  Log.errorln(&buf[0]);

  if (f) {
#if defined(ESP8266)
    f.write(&buf[0], strlen(&buf[0]));
#else  // ESP32
    f.write((unsigned char*)&buf[0], strlen(&buf[0]));
#endif
    f.println();
    f.close();
  } else {
    Log.warning(F("HELP: Failed to open error log." CR));
  }
}

double convertToPlato(double sg) {
  if (sg) return 259 - (259 / sg);
  return 0;
}

double convertToSG(double plato) { return 259 / (259 - plato); }

float convertCtoF(float c) { return (c * 1.8) + 32.0; }

float convertFtoC(float f) { return (f - 32.0) / 1.8; }

FloatHistoryLog::FloatHistoryLog(String fName) {
  _fName = fName;

  File runFile = LittleFS.open(_fName, "r");
  if (runFile) {
    for (int i = 0; i < 10; i++) {
      _runTime[i] = runFile.readStringUntil('\n').toFloat();
      if (_runTime[i]) {
        _average += _runTime[i];
        _count++;
      }
    }
    runFile.close();
    _average = _average / _count;
  }
}

void FloatHistoryLog::addEntry(float time) {
  for (int i = (10 - 1); i > 0; i--) {
    _runTime[i] = _runTime[i - 1];
  }
  _runTime[0] = time;
  save();
}

void FloatHistoryLog::save() {
  File runFile = LittleFS.open(_fName, "w");
  if (runFile) {
    for (int i = 0; i < 10; i++) {
      runFile.println(_runTime[i], 2);
    }
    runFile.close();
  }
}

void printHeap(String prefix) {
#if defined(ESP8266)
  Log.notice(
      F("%s: Free-heap %d kb, Heap-rag %d %%, Max-block %d kb Stack=%d b." CR),
      prefix.c_str(), ESP.getFreeHeap() / 1024, ESP.getHeapFragmentation(),
      ESP.getMaxFreeBlockSize() / 1024, ESP.getFreeContStack());
  // Log.notice(F("%s: Heap %d kb, HeapFrag %d %%, FreeSketch %d kb." CR),
  //            prefix.c_str(), ESP.getFreeHeap() / 1024,
  //            ESP.getHeapFragmentation(), ESP.getFreeSketchSpace() / 1024);
#else  // defined (ESP32)
  Log.verbose(F("HELP: Heap %d kb, FreeSketch %d kb." CR),
              ESP.getFreeHeap() / 1024, ESP.getFreeSketchSpace() / 1024);
#endif
}

void deepSleep(int t) {
#if LOG_LEVEL == 6 && !defined(HELPER_DISABLE_LOGGING)
  Log.verbose(F("HELP: Entering sleep mode for %ds." CR), t);
#endif
  uint32_t wake = t * 1000000;
  ESP.deepSleep(wake);
}

void printBuildOptions() {
  Log.notice(F("Build options: %s (%s) LOGLEVEL %d "
#if defined(ESP8266)
               "ESP8266 "
#elif defined(ESP32LITE)
               "FLOATY "
#endif
#ifdef SKIP_SLEEPMODE
               "SKIP_SLEEP "
#endif
#ifdef COLLECT_PERFDATA
               "PERFDATA "
#endif
               CR),
             CFG_APPVER, CFG_GITREV, LOG_LEVEL);
}

SerialDebug::SerialDebug(const uint32_t serialSpeed) {
  // Start serial with auto-detected rate (default to defined BAUD)
#if defined(USE_SERIAL_PINS) && defined(ESP8266)
  uint8_t txPin = 3;
  EspSerial.begin(serialSpeed, SERIAL_8N1, SERIAL_TX_ONLY, txPin);
#elif defined(ESP8266)
  EspSerial.begin(serialSpeed);
#elif defined(USE_SERIAL_PINS) && defined(ESP32C3)
  EspSerial.begin(115200L, SERIAL_8N1, 20, 21);
#elif defined(ESP32C3)
  EspSerial.begin(115200L);
#elif defined(USE_SERIAL_PINS) && defined(ESP32S2)
  EspSerial.begin(115200L, SERIAL_8N1, 37, 39);
#elif defined(ESP32S2)
  EspSerial.begin(115200L);
#elif defined(USE_SERIAL_PINS) && defined(ESP32LITE)
  EspSerial.begin(serialSpeed, SERIAL_8N1, 16, 17);
#elif defined(USE_SERIAL_PINS) && defined(ESP32)
  EspSerial.begin(serialSpeed, SERIAL_8N1, 1, 3);
#elif defined(ESP32)
  EspSerial.begin(115200L);
#endif

  EspSerial.println("Serial connection established");
  EspSerial.setDebugOutput(true);
  getLog()->begin(LOG_LEVEL, &EspSerial, true);
  getLog()->setPrefix(printTimestamp);
  getLog()->notice(F("SDBG: Serial logging started at %u." CR), serialSpeed);
}

void printTimestamp(Print* _logOutput, int _logLevel) {
  char c[12];
  snprintf(c, sizeof(c), "%10lu ", millis());
  _logOutput->print(c);
}

bool checkPinConnected() {
#if defined(ESP8266)
  pinMode(PIN_CFG1, INPUT);
#else
  pinMode(PIN_CFG1, INPUT_PULLDOWN);
#endif
  pinMode(PIN_CFG2, OUTPUT);
  delay(5);
  digitalWrite(PIN_CFG2, 1);
  delay(5);
  int i = digitalRead(PIN_CFG1);
  digitalWrite(PIN_CFG2, 0);
  return i == LOW ? false : true;
}

BatteryVoltage::BatteryVoltage() {
#if defined(ESP8266)
  pinMode(PIN_VOLT, INPUT);
#else
  pinMode(PIN_VOLT, INPUT_PULLDOWN);
#endif
}

void BatteryVoltage::read() {
  // The analog pin can only handle 3.3V maximum voltage so we need to reduce
  // the voltage (from max 5V)
  float factor = myConfig.getVoltageFactor();  // Default value is 1.63
  int v = analogRead(PIN_VOLT);

  // An ESP8266 has a ADC range of 0-1023 and a maximum voltage of 3.3V
  // An ESP32 has an ADC range of 0-4095 and a maximum voltage of 3.3V

#if defined(ESP8266)
  _batteryLevel = ((3.3 / 1023) * v) * factor;
#else  // defined (ESP32)
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
           myGyro.getSensorTempC(),
           myTempSensor.getTempC(myConfig.isGyroTemp()), ESP.getFreeHeap(), 0,
           ESP.getMaxAllocHeap());
#endif

  body += &buf[0];

#if LOG_LEVEL == 6 && !defined(HELPER_DISABLE_LOGGING)
  Log.verbose(F("PERF: url %s." CR), serverPath.c_str());
  Log.verbose(F("PERF: data %s." CR), body.c_str());
#endif

  // Send HTTP POST request
  String auth = "Token " + String(myConfig.getInfluxDb2PushToken());
  http.addHeader(F("Authorization"), auth.c_str());
  http.setTimeout(myAdvancedConfig.getPushTimeout() * 1000);
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

#endif  // COLLECT_PERFDATA

char* convertFloatToString(float f, char* buffer, int dec) {
  dtostrf(f, 6, dec, buffer);
  return buffer;
}

float reduceFloatPrecision(float f, int dec) {
  char buffer[10];
  dtostrf(f, 6, dec, &buffer[0]);
  // Log.notice(F("HELP: %s, %F" CR), &buffer[0], atof(&buffer[0]));
  return atof(&buffer[0]);
}

// urlencode
// https://circuits4you.com/2019/03/21/esp8266-url-encode-decode-example/
String urlencode(String str) {
  String encodedString;
  encodedString.reserve(str.length() * 2);
  encodedString = "";
  char c;
  char code0;
  char code1;
  for (int i = 0; i < static_cast<int>(str.length()); i++) {
    c = str.charAt(i);
    if (isalnum(c)) {
      encodedString += c;
    } else {
      code1 = (c & 0xf) + '0';
      if ((c & 0xf) > 9) {
        code1 = (c & 0xf) - 10 + 'A';
      }
      c = (c >> 4) & 0xf;
      code0 = c + '0';
      if (c > 9) {
        code0 = c - 10 + 'A';
      }
      encodedString += '%';
      encodedString += code0;
      encodedString += code1;
    }
  }
  // Log.verbose(F("HELP: encode=%s" CR), encodedString.c_str());
  return encodedString;
}

unsigned char h2int(char c) {
  if (c >= '0' && c <= '9') {
    return ((unsigned char)c - '0');
  }
  if (c >= 'a' && c <= 'f') {
    return ((unsigned char)c - 'a' + 10);
  }
  if (c >= 'A' && c <= 'F') {
    return ((unsigned char)c - 'A' + 10);
  }
  return (0);
}

String urldecode(String str) {
  String encodedString;
  encodedString.reserve(str.length());
  encodedString = "";
  char c;
  char code0;
  char code1;
  for (int i = 0; i < static_cast<int>(str.length()); i++) {
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

  // Log.verbose(F("HELP: decode=%s" CR), encodedString.c_str());
  return encodedString;
}

// EOF
