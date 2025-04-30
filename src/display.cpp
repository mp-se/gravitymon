/*
MIT License

Copyright (c) 2024-2025 Magnus

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
#include <looptimer.hpp>

#if defined(ENABLE_TFT)
TaskHandle_t lvglTaskHandler;
struct LVGL_Data lvglData;

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

void Display::printLine(int l, const String &text) {
  if (!_tft) return;

  uint16_t h = _tft->fontHeight();
  _tft->fillRect(0, l * h, _tft->width(), h, TFT_BLACK);
  _tft->drawString(text.c_str(), 0, l * h, GFXFF);
}

void Display::printLineCentered(int l, const String &text) {
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

void Display::createUI() {
  if (!_tft) return;

  Log.notice(F("DISP: Using LVL v%d.%d.%d." CR), lv_version_major(),
             lv_version_minor(), lv_version_patch());

  lv_init();
  lv_log_register_print_cb(log_print);

#define DRAW_BUF_SIZE (TFT_WIDTH * TFT_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))

  void *draw_buf = ps_malloc(DRAW_BUF_SIZE);

  if (!draw_buf) {
    Log.error(
        F("DISP: Failed to allocate ps ram for display buffer, size=%d" CR),
        DRAW_BUF_SIZE);
  }

  lvglData._display =
      lv_tft_espi_create(TFT_WIDTH, TFT_HEIGHT, draw_buf, DRAW_BUF_SIZE);

  if (_rotation == Rotation::ROTATION_90) {
    lv_display_set_rotation(lvglData._display, LV_DISPLAY_ROTATION_90);
  } else {  // Rotation::ROTATION_270
    lv_display_set_rotation(lvglData._display, LV_DISPLAY_ROTATION_270);
  }

  // Create components
  lv_style_init(&lvglData._font12);
  lv_style_init(&lvglData._font12c);
  lv_style_init(&lvglData._font16c);
  lv_style_init(&lvglData._font20c);
  lv_style_set_text_font(&lvglData._font12, &lv_font_montserrat_12);
  lv_style_set_text_font(&lvglData._font12c, &lv_font_montserrat_12);
  lv_style_set_text_font(&lvglData._font16c, &lv_font_montserrat_16);
  lv_style_set_text_font(&lvglData._font20c, &lv_font_montserrat_20);
  lv_style_set_text_align(&lvglData._font12, LV_TEXT_ALIGN_LEFT);
  lv_style_set_text_align(&lvglData._font12c, LV_TEXT_ALIGN_CENTER);
  lv_style_set_text_align(&lvglData._font16c, LV_TEXT_ALIGN_CENTER);
  lv_style_set_text_align(&lvglData._font20c, LV_TEXT_ALIGN_CENTER);

  // For showing the layout
  // lv_style_set_outline_width(&lvglData._font12, 1);
  // lv_style_set_outline_color(&lvglData._font12,
  // lv_palette_main(LV_PALETTE_BLUE));
  // lv_style_set_outline_width(&lvglData._font12c, 1);
  // lv_style_set_outline_color(&lvglData._font12c,
  // lv_palette_main(LV_PALETTE_BLUE));
  // lv_style_set_outline_width(&lvglData._font16c, 1);
  // lv_style_set_outline_color(&lvglData._font16c,
  // lv_palette_main(LV_PALETTE_BLUE));
  // lv_style_set_outline_width(&lvglData._font20c, 1);
  // lv_style_set_outline_color(&lvglData._font20c,
  // lv_palette_main(LV_PALETTE_BLUE));

  Log.notice(F("DISP: Creating UI components." CR));

  lvglData._txtDeviceName = createLabel("", 5, 5, 250, 36, &lvglData._font20c);
  lvglData._txtDeviceIndex =
      createLabel("", 260, 5, 54, 36, &lvglData._font16c);
  lvglData._txtDeviceValue1 =
      createLabel("", 5, 45, 100, 36, &lvglData._font20c);
  lvglData._txtDeviceValue2 =
      createLabel("", 110, 45, 100, 36, &lvglData._font20c);
  lvglData._txtDeviceValue3 =
      createLabel("", 215, 45, 100, 36, &lvglData._font20c);
  lvglData._txtDeviceTimeStamp =
      createLabel("", 5, 85, 310, 26, &lvglData._font16c);
  lvglData._txtStatusbar = createLabel("", 5, 219, 310, 18, &lvglData._font12c);

  for (int i = 0; i < 5; i++)
    lvglData._txtHistory[i] =
        createLabel("", 5, 114 + (i * 21), 310, 18, &lvglData._font12);

  xTaskCreatePinnedToCore(lvgl_loop_handler,  // Function to implement the task
                          "LVGL_Handler",     // Name of the task
                          10000,              // Stack size in words
                          NULL,               // Task input parameter
                          0,                  // Priority of the task
                          &lvglTaskHandler,   // Task handle.
                          0);                 // Core where the task should run
}

void Display::updateDevice(const char *name, const char *value1,
                           const char *value2, const char *value3,
                           const char *timestamp, int index, int maxIndex) {
  lvglData._dataDeviceName = name;
  lvglData._dataDeviceValue1 = value1;
  lvglData._dataDeviceValue2 = value2;
  lvglData._dataDeviceValue3 = value3;
  lvglData._dataDeviceTimeStamp = timestamp;

  char buf[10] = "";

  if (maxIndex) snprintf(buf, sizeof(buf), "%d/%d", index + 1, maxIndex);

  lvglData._dataDeviceIndex = buf;
}

void Display::updateHistory(const char *history, int idx) {
  lvglData._dataHistory[idx] = history;
}

void Display::updateStatus(const char *status) {
  lvglData._dataStatusbar = status;
}

// LVGL Wrappers and Handlers
// **************************************************************************************************

void log_print(lv_log_level_t level, const char *buf) {
  LV_UNUSED(level);
  Log.notice(F("LVGL: %s." CR), buf);
}

lv_obj_t *createLabel(const char *label, int32_t x, int32_t y, int32_t w,
                      int32_t h, lv_style_t *style) {
  lv_obj_t *lbl = lv_label_create(lv_screen_active());
  lv_label_set_text(lbl, label);
  lv_obj_set_size(lbl, w, h);
  lv_obj_set_pos(lbl, x, y);
  lv_obj_add_style(lbl, style, 0);
  return lbl;
}

void updateLabel(lv_obj_t *obj, const char *label) {
  lv_label_set_text(obj, label);
}

void lvgl_loop_handler(void *parameter) {
  LoopTimer taskLoop(500);

  for (;;) {
    if (taskLoop.hasExpired()) {
      taskLoop.reset();

      updateLabel(lvglData._txtDeviceName, lvglData._dataDeviceName.c_str());
      updateLabel(lvglData._txtDeviceIndex, lvglData._dataDeviceIndex.c_str());
      updateLabel(lvglData._txtDeviceValue1,
                  lvglData._dataDeviceValue1.c_str());
      updateLabel(lvglData._txtDeviceValue2,
                  lvglData._dataDeviceValue2.c_str());
      updateLabel(lvglData._txtDeviceValue3,
                  lvglData._dataDeviceValue3.c_str());
      updateLabel(lvglData._txtDeviceTimeStamp,
                  lvglData._dataDeviceTimeStamp.c_str());
      updateLabel(lvglData._txtStatusbar, lvglData._dataStatusbar.c_str());

      for (int i = 0; i < 5; i++)
        updateLabel(lvglData._txtHistory[i], lvglData._dataHistory[i].c_str());
    }

    lv_task_handler();
    lv_tick_inc(5);
    delay(5);
  }
}

#else

Display::Display() {}

void Display::setup() {}

void Display::createUI() {}

void Display::setFont(FontSize f) {}

void Display::printLine(int l, const String& text) {}

void Display::printLineCentered(int l, const String& text) {}

void Display::clear() {}

void Display::updateDevice(const char* name, const char* value1,
                           const char* value2, const char* value3,
                           const char* timestamp, int index, int maxIndex) {}
void Display::updateHistory(const char* history, int idx) {}
void Display::updateStatus(const char* status) {}

#endif

#endif  // GATEWAY

// EOF
