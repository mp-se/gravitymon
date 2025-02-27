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
#if defined(GATEWAY)

#include <display.hpp>
#include <fonts.hpp>
#include <log.hpp>

#if defined(ENABLE_TFT)
Display::Display() { _tft = new TFT_eSPI(); }

void Display::setup() {
  if (!_tft) return;

  _tft->init();
  _tft->setSwapBytes(true);
  _tft->setRotation(1);  // TODO: Allow rotation to be set in configuration
  clear();
  setFont(FontSize::FONT_9);
}

void Display::setFont(FontSize f) {
  if (!_tft) return;

  switch (f) {
    default:
    case FontSize::FONT_9:
      _tft->setFreeFont(FF17);
      break;
    case FontSize::FONT_12:
      _tft->setFreeFont(FF18);
      break;
    case FontSize::FONT_18:
      _tft->setFreeFont(FF19);
      break;
    case FontSize::FONT_24:
      _tft->setFreeFont(FF20);
      break;
  }
}

void Display::printLine(int l, const String& text) {
  if (!_tft) return;

  uint16_t h = _tft->fontHeight();
  _tft->fillRect(0, l * h, _tft->width(), h, TFT_BLACK);
  _tft->drawString(text.c_str(), 0, l * h, GFXFF);
}

void Display::printLineCentered(int l, const String& text) {
  if (!_tft) return;

  uint16_t h = _tft->fontHeight();
  uint16_t w = _tft->textWidth(text);
  _tft->fillRect(0, l * h, _tft->width(), h, TFT_BLACK);
  _tft->drawString(text.c_str(), (_tft->width() - w) / 2, l * h, GFXFF);
}

void Display::clear() {
  if (!_tft) return;

  _tft->fillScreen(TFT_BLACK);
  delay(1);
}

#else

Display::Display() {}

void Display::setup() {}

void Display::setFont(FontSize f) {}

void Display::printLine(int l, const String& text) {}

void Display::printLineCentered(int l, const String& text) {}

void Display::clear() {}

#endif

#endif  // GATEWAY

// EOF
