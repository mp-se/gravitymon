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
#ifndef SRC_TEMPLATING_HPP_
#define SRC_TEMPLATING_HPP_

#include <algorithm>
#include <espframework.hpp>
#include <log.hpp>
#include <utils.hpp>

constexpr auto MAX_KEY_VAL = 30;

class TemplatingEngine {
 private:
  struct KeyVal {
    String key;
    String val;
  };

  KeyVal _items[MAX_KEY_VAL];

  char _buffer[20] = "";
  char *_output = NULL;

  void transform(const char *format) {
    int len = strlen(format);
    int size = len;

    // Lets check how much memory will be needed to transform the template
    for (int j = 0; j < len - 2; j++) {
      if (*(format + j) == '$' && *(format + j + 1) == '{') {
        // Start of format tag found
        int max = sizeof(_items) / sizeof(KeyVal);
        for (int i = 0; i < max; i++) {
          if (strncmp(format + j, _items[i].key.c_str(),
                      _items[i].key.length()) == 0) {
            // Found key
            size = size - _items[i].key.length() + _items[i].val.length();
          }
        }
      }
    }

    Log.notice(F("TPL : Buffer needed %d." CR), size);

    freeMemory();  // In case this is reused
    _output = static_cast<char *>(malloc(size + 20));

    if (!_output) {
      Log.error(F("TPL : Unable to allocate memory for transforming template, "
                  "needed %d." CR),
                size);
      return;
    }

    memset(_output, 0, size + 20);

    // Lets do the transformation
    int k = 0;
    for (int j = 0; j < len - 2; j++) {
      if (*(format + j) == '$' && *(format + j + 1) == '{') {
        // Start of format tag found
        int max = sizeof(_items) / sizeof(KeyVal);
        for (int i = 0; i < max; i++) {
          if (!_items[i].key.equals("")) {
            if (strncmp(format + j, _items[i].key.c_str(),
                        _items[i].key.length()) == 0) {
              // Found key
              strncat(_output, format + k, j - k);
              strncat(_output, _items[i].val.c_str(), _items[i].val.length());
              k = j + _items[i].key.length();
            }
          }
        }
      }
    }
    // strncat(_output, format + k, size - k);
    strncat(_output, format + k, strlen(format + k));
    Log.notice(F("TPL : Transformed template %d chars to %d chars" CR),
               strlen(format), strlen(_output));
  }

 public:
  TemplatingEngine() {}
  ~TemplatingEngine() { freeMemory(); }

  void setVal(String key, float val, int dec = 2) {
    String s = convertFloatToString(val, &_buffer[0], dec);
    s.trim();
    setVal(key, s);
  }
  void setVal(String key, int val) { setVal(key, String(val)); }
  void setVal(String key, char val) { setVal(key, String(val)); }
  void setVal(String key, String val) {
    int max = sizeof(_items) / sizeof(KeyVal);
    for (int i = 0; i < max; i++) {
      if (_items[i].key.equals("")) {
        _items[i].key = key;
        _items[i].val = val;
        return;
      } else if (_items[i].key.equals(key)) {
        _items[i].val = val;
        return;
      }
    }

    Log.warning(F("TPL : Too many keys, max is %d." CR), MAX_KEY_VAL);
  }

  void dumpAll() {
    int max = sizeof(_items) / sizeof(KeyVal);
    for (int i = 0; i < max; i++) {
      if (!_items[i].key.equals("")) {
        EspSerial.print("Key=\'");
        EspSerial.print(_items[i].key.c_str());
        EspSerial.print("\', Val=\'");
        EspSerial.print(_items[i].val.c_str());
        EspSerial.println("\'");
      }
    }
  }

  void freeMemory() {
    if (_output) free(_output);
    _output = NULL;
  }

  const char *create(const char *base);
};

#endif  // SRC_TEMPLATING_HPP_

// EOF
