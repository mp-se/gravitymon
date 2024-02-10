/*
MIT License

Copyright (c) 2023-2024 Magnus

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
#include <Ticker.h>

#include <espframework.hpp>
#include <led.hpp>
#include <log.hpp>

#if defined(ESP32C3) || defined(ESP32S3)
void ledOn(LedColor l) {
  uint8_t r, g, b, pin;

  r = (l & 0xff0000) >> 16;
  g = (l & 0x00ff00) >> 8;
  b = (l & 0x0000ff);
  pin = LED_BUILTIN;

  Log.info(F("LED : Setting led %d to RGB %d-%d-%d" CR), pin, r, g, b);
  neopixelWrite(pin, r, g, b);
}
#else
bool ledInit = false;
Ticker ledTicker;

void ledToggle() { digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); }

void ledOn(LedColor l) {
  if (!ledInit) {
    pinMode(LED_BUILTIN, OUTPUT);
    ledInit = true;
  }

  if (l == LedColor::BLUE) {
    ledTicker.attach(1, ledToggle);
  } else if (l == LedColor::RED) {
    ledTicker.attach(0.2, ledToggle);
  } else {
    ledTicker.detach();
    digitalWrite(LED_BUILTIN, l);
  }
}
#endif

void ledOff() { ledOn(LedColor::OFF); }

// EOF
