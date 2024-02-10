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
#include <LittleFS.h>

#include <log.hpp>

void writeErrorLog(const char *format, ...) {
  File f = LittleFS.open(ERR_FILENAME, "a");

  if (f && f.size() > ERR_FILEMAXSIZE) {
    f.close();
    LittleFS.remove(ERR_FILENAME2);
    LittleFS.rename(ERR_FILENAME, ERR_FILENAME2);
    f = LittleFS.open(ERR_FILENAME, "a");
  }

  if (f) {
    va_list arg;
    va_start(arg, format);
    char buf[80];
    vsnprintf(&buf[0], sizeof(buf), format, arg);
    f.write(reinterpret_cast<unsigned char *>(&buf[0]), strlen(&buf[0]));
    va_end(arg);
    f.println();
    f.close();
  }
}

void dumpErrorLog(const char *fname) {
  File f = LittleFS.open(fname, "r");

  if (f) {
    String s;
    do {
      s = f.readString();
      Serial.print(s.c_str());
    } while (s.length());
    f.close();
  }
  LittleFS.remove(fname);
}

void dumpErrorLog1() { dumpErrorLog(ERR_FILENAME); }

void dumpErrorLog2() { dumpErrorLog(ERR_FILENAME2); }

SerialDebug::SerialDebug(const uint32_t serialSpeed) {
#if defined(USE_SERIAL_PINS) && defined(ESP8266)
  // EspSerial.begin(serialSpeed, SERIAL_8N1, 3, 1);
  EspSerial.begin(serialSpeed);
#warning "SerialPins is not implemented on ESP8266"
#elif defined(ESP8266)
  EspSerial.begin(serialSpeed);
#elif defined(USE_SERIAL_PINS) && defined(ESP32C3)
  // EspSerial.begin(115200L, SERIAL_8N1, 20, 21);
  EspSerial.begin(115200L);
#elif defined(ESP32C3)
  EspSerial.begin(115200L);
#elif defined(USE_SERIAL_PINS) && defined(ESP32S2)
  EspSerial.begin(115200L, SERIAL_8N1, 37, 39);
#elif defined(ESP32S2)
  EspSerial.begin(115200L);
#elif defined(USE_SERIAL_PINS) && defined(ESP32S3)
  EspSerial.begin(115200L, SERIAL_8N1, 44, 43);
#elif defined(ESP32S3)
  EspSerial.begin(115200L);
#elif defined(USE_SERIAL_PINS) && defined(ESP32)
  EspSerial.begin(serialSpeed, SERIAL_8N1, 3, 1);
#elif defined(ESP32)
  EspSerial.begin(115200L);
#endif
  EspSerial.println("Serial console activated.");

  begin(&EspSerial);
  getLog()->setPrefix(printTimestamp);
  getLog()->notice(F("SDBG: Serial logging started at %l." CR), serialSpeed);
}

void printTimestamp(Print *_logOutput, int _logLevel) {
  // TODO: Add real time if we are using NTP

  char c[12];
  snprintf(c, sizeof(c), "%10lu ", millis());
  _logOutput->print(c);
}

// EOF
