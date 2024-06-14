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
#include <espframework.hpp>
#include <log.hpp>
#include <utils.hpp>
#include <led.hpp>

#if !defined(ESP8266)
#include <esp_int_wdt.h>
#include <esp_task_wdt.h>
#else
#include <user_interface.h>
#endif

float convertCtoF(float c) { return (c * 1.8) + 32.0; }
float convertFtoC(float f) { return (f - 32.0) / 1.8; }

float convertKGtoLBS(float kg) { return kg * 2.20462262185; }
float convertLBStoKG(float lbs) {
  return lbs == 0.0 ? 0.0 : lbs / 2.20462262185;
}

float convertCLtoUSOZ(float cl) { return cl * 0.34; }
float convertCLtoUKOZ(float cl) { return cl == 0.0 ? 0.0 : cl / 2.84; }
float convertUSOZtoCL(float usoz) { return usoz == 0.0 ? 0.0 : usoz / 0.34; }
float convertUKOZtoCL(float ukoz) { return ukoz * 2.84; }

double convertToPlato(double sg) {
  if (sg) return 259 - (259 / sg);
  return 0;
}

double convertToSG(double plato) { return 259 / (259 - plato); }

float reduceFloatPrecision(float f, int dec) {
  char buffer[10];
  dtostrf(f, 6, dec, &buffer[0]);
  return atof(&buffer[0]);
}

char* convertFloatToString(float f, char* buffer, int dec) {
  dtostrf(f, 6, dec, buffer);
  return buffer;
}

struct tcp_pcb;
extern struct tcp_pcb* tcp_tw_pcbs;
extern "C" void tcp_abort(struct tcp_pcb* pcb);
void tcp_cleanup() {  // tcp cleanup, to avoid memory crash.
  while (tcp_tw_pcbs) tcp_abort(tcp_tw_pcbs);
}

void deepSleep(int t) {
#if LOG_LEVEL == 6
  Log.verbose(F("HELP: Entering sleep mode for %d seconds." CR), t);
#endif
  ledOff();
  uint32_t wake = t * 1000000;
  ESP.deepSleep(wake);
}

void printHeap(String prefix) {
#if defined(ESP8266)
  Log.notice(
      F("%s: Free-heap %d kb, Heap-rag %d %%, Max-block %d kb Stack=%d b." CR),
      prefix.c_str(), ESP.getFreeHeap() / 1024, ESP.getHeapFragmentation(),
      ESP.getMaxFreeBlockSize() / 1024, ESP.getFreeContStack());
#else  // defined (ESP32)
  Log.notice(F("HELP: Heap %d kb, FreeSketch %d kb." CR),
             ESP.getFreeHeap() / 1024, ESP.getFreeSketchSpace() / 1024);
#endif
}

void forcedReset() {
#if !defined(ESP8266)
  ledOff();
  LittleFS.end();
  delay(100);
  esp_task_wdt_init(1, true);
  esp_task_wdt_add(NULL);
  while (true) {
    // wait for watchdog timer to be triggered
  }
#endif
}

void espReset() {
#if defined(ESP8266)
  ledOff();
  ESP.reset();
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

  Log.notice(F("UTIL: Chip=%s, Rev=%d, Feat=%X" CR), chipModel.c_str(),
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

  Log.notice(F("UTIL: Last reset cause '%s' (%d)" CR), rStr.c_str(), r);
  // Havent found a good way to get exception cause from an ESP32
#endif
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
