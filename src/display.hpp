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
#include <lvgl.h>

#include "TFT_eSPI.h"
#endif

struct LVGL_Data {
  lv_obj_t* _txtDeviceName;
  lv_obj_t* _txtDeviceIndex;
  lv_obj_t* _txtDeviceValue1;
  lv_obj_t* _txtDeviceValue2;
  lv_obj_t* _txtDeviceValue3;
  lv_obj_t* _txtDeviceTimeStamp;
  lv_obj_t* _txtHistory[5];
  lv_obj_t* _txtStatusbar;

  lv_style_t _font12;
  lv_style_t _font12c;
  lv_style_t _font16c;
  lv_style_t _font20c;

  lv_display_t* _display;

  String _dataDeviceName;
  String _dataDeviceIndex;
  String _dataDeviceValue1;
  String _dataDeviceValue2;
  String _dataDeviceValue3;
  String _dataDeviceTimeStamp;
  String _dataStatusbar;
  String _dataHistory[5];
};

enum FontSize { FONT_9 = 9, FONT_12 = 12, FONT_18 = 18, FONT_24 = 24 };

class Display {
 public:
  enum Rotation {
    // ROTATION_0 = 0, // Not supported
    ROTATION_90 = 1,
    // ROTATION_180 = 2,  // Not supported
    ROTATION_270 = 3
  };

 private:
#if defined(ENABLE_TFT)
  TFT_eSPI* _tft = NULL;
#endif
  FontSize _fontSize = FontSize::FONT_9;
  Rotation _rotation = ROTATION_90;
  uint32_t _backgroundColor = TFT_BLACK;

 public:
  Display();
  void setup();
  void createUI();

  void clear();
  void setFont(FontSize f);
  void printLine(int l, const String& text);
  void printLineCentered(int l, const String& text);
  Rotation getRotation() { return _rotation; }
  void setRotation(Rotation rotation);

  void updateDevice(const char* name, const char* value1, const char* value2,
                    const char* value3, const char* timestamp, int index,
                    int maxIndex);
  void updateHistory(const char* history, int idx);
  void updateStatus(const char* status);
};

// Wrappers to simplify interaction with LVGL
lv_obj_t* createLabel(const char* label, int32_t x, int32_t y, int32_t w,
                      int32_t h, lv_style_t* style);
void updateLabel(lv_obj_t* obj, const char* label);
void setStyle(lv_obj_t* obj, lv_style_t* style);
void log_print(lv_log_level_t level, const char* buf);
void lvgl_loop_handler(void* parameter);

extern Display myDisplay;
extern struct LVGL_Data lvglData;

#endif  // GATEWAY

#endif  // SRC_DISPLAY_HPP_

// EOF
