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
#include <main.hpp>
#include <serialdbg.hpp>

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
#elif defined(USE_SERIAL_PINS) && defined(ESP32S3)
  EspSerial.begin(115200L, SERIAL_8N1, 37, 39);
#elif defined(ESP32S3)
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

// EOF
