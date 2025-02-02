/*
MIT License

Copyright (c) 2024 Magnus

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
#ifndef SRC_DISPLAY_HPP_
#define SRC_DISPLAY_HPP_

#if defined(GATEWAY)

#include <config.hpp>
#include <main.hpp>

#if defined(ENABLE_TFT)
#include "TFT_eSPI.h"
#endif

enum FontSize { FONT_9 = 9, FONT_12 = 12, FONT_18 = 18, FONT_24 = 24 };

class Display {
 private:
#if defined(ENABLE_TFT)
  TFT_eSPI* _tft = NULL;
#endif
  FontSize _fontSize = FontSize::FONT_9;

 public:
  Display();
  void setup();
  // void loop();

  void clear();
  void setFont(FontSize f);
  void printLine(int l, const String& text);
  void printLineCentered(int l, const String& text);
};

extern Display myDisplay;

#endif  // GATEWAY

#endif  // SRC_DISPLAY_HPP_

// EOF
