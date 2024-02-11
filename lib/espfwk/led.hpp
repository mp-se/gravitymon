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
#ifndef SRC_LED_HPP_
#define SRC_LED_HPP_

enum LedColor {
#if defined(ESP32C3) || defined(ESP32S3)
  OFF = 0x000000,
  BLACK = 0x000000,
  RED = 0xff0000,
  GREEN = 0x00ff00,
  BLUE = 0x0000ff,
  CYAN = 0x00ffff,
  PURPLE = 0xff00ff,
  YELLOW = 0xffff00,
  WHITE = 0xffffff
#else
  OFF = HIGH,
  BLACK = HIGH,
  RED = 3,  // TIcker at fast pace
  GREEN = LOW,
  BLUE = 2,  // Ticker at slow pace
  PURPLE = LOW,
  CYAN = LOW,
  YELLOW = LOW,
  WHITE = LOW
#endif
};

void ledOn(LedColor l = LedColor::WHITE);
void ledOff();

#endif  // SRC_LED_HPP_

// EOF
