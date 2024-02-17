/*
MIT License

Copyright (c) 2021-2023 Magnus

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
#if !defined(ESP8266)
#include <esp_int_wdt.h>
#include <esp_task_wdt.h>
#else
#include <user_interface.h>
#endif

#include <Ticker.h>

#include <helper.hpp>

// tcp cleanup, to avoid memory crash.
struct tcp_pcb;
extern struct tcp_pcb* tcp_tw_pcbs;
extern "C" void tcp_abort(struct tcp_pcb* pcb);
void tcp_cleanup() {
  while (tcp_tw_pcbs) tcp_abort(tcp_tw_pcbs);
}

extern "C" {
void write_bytes(int fd, char* buf, int n) { EspSerial.print(*buf); }
}

void forcedReset() {
#if !defined(ESP8266)
  LittleFS.end();
  delay(100);
  esp_task_wdt_init(1, true);
  esp_task_wdt_add(NULL);
  while (true) {
    // wait for watchdog timer to be triggered
  }
#endif
}

void runGpioHardwareTests() {
#if defined(RUN_HARDWARE_TEST)
  int max = 10;

  Log.notice(F("HELP: Configuring GPIO for output." CR));
  pinMode(PIN_SDA, OUTPUT);
  pinMode(PIN_SCL, OUTPUT);
  pinMode(PIN_CFG1, OUTPUT);
  pinMode(PIN_CFG2, OUTPUT);
  pinMode(PIN_DS, OUTPUT);
  pinMode(PIN_VOLT, OUTPUT);
  delay(100);
  digitalWrite(PIN_SDA, LOW);
  digitalWrite(PIN_SCL, LOW);
  digitalWrite(PIN_CFG1, LOW);
  digitalWrite(PIN_CFG2, LOW);
  digitalWrite(PIN_DS, LOW);
  digitalWrite(PIN_VOLT, LOW);
  delay(100);

  int sleep = 700;

  Log.notice(F("HELP: Testing SDA." CR));
  for (int i = 0; i < max; i++) {
    digitalWrite(PIN_SDA, i % 2);
    delay(sleep);
  }
  digitalWrite(PIN_SDA, LOW);

  Log.notice(F("HELP: Testing SCL." CR));
  for (int i = 0; i < max; i++) {
    digitalWrite(PIN_SCL, i % 2);
    delay(sleep);
  }
  digitalWrite(PIN_SCL, LOW);

  Log.notice(F("HELP: Testing CFG1." CR));
  for (int i = 0; i < max; i++) {
    digitalWrite(PIN_CFG1, i % 2);
    delay(sleep);
  }
  digitalWrite(PIN_CFG1, LOW);

  Log.notice(F("HELP: Testing CFG2." CR));
  for (int i = 0; i < max; i++) {
    digitalWrite(PIN_CFG2, i % 2);
    delay(sleep);
  }
  digitalWrite(PIN_CFG2, LOW);

  Log.notice(F("HELP: Testing DS." CR));
  for (int i = 0; i < max; i++) {
    digitalWrite(PIN_DS, i % 2);
    delay(sleep);
  }
  digitalWrite(PIN_DS, LOW);

  Log.notice(F("HELP: Testing VOLT." CR));
  for (int i = 0; i < max; i++) {
    digitalWrite(PIN_VOLT, i % 2);
    delay(sleep);
  }
  digitalWrite(PIN_VOLT, LOW);

  Log.notice(F("HELP: Tests finished, enter waiting for reset." CR));
  while (true) delay(sleep);
#endif
}

#if defined(ESP8266)
void detectChipRevision() {}
bool isEsp32c3() { return false; }
#else
esp_chip_info_t chip_info;

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
    case CHIP_ESP32S3:
      chipModel = "ESP32S3";
      break;
    case CHIP_ESP32C3:
      chipModel = "ESP32C3";
      break;
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
