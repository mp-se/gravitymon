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
#ifndef SRC_PERF_HPP_
#define SRC_PERF_HPP_

#if defined(PERF_ENABLE)

#include <baseconfig.hpp>

class PerfLogging {
 private:
  PushConfig* _config = 0;

  struct PerfEntry {
    uint32_t start;   // millis()
    uint32_t end;     // millis()
    uint32_t max;     // max time in ms
    const char* key;  // measurement

    PerfEntry* next;

    float mA;  // used when a power meeter is attached
    float V;
  };

  PerfEntry* first = 0;
  bool measurePower = false;

  PerfEntry* find(const char* k) {
    if (first == 0) return 0;

    PerfEntry* pe = first;

    while (strcmp(k, pe->key) != 0) {
      if (pe->next == 0) return 0;
      pe = pe->next;
    }
    return pe;
  }

  PerfEntry* add(const char* k) {
    if (first == 0) {
      first = new PerfEntry();
      first->key = k;
      first->next = 0;
      first->max = 0;
      return first;
    }

    PerfEntry* pe = first;

    while (strcmp(k, pe->key) != 0) {
      if (pe->next == 0) {
        pe->next = new PerfEntry();
        pe->next->key = k;
        pe->next->max = 0;
        pe->next->next = 0;
        return pe->next;
      }

      pe = pe->next;
    }

    return pe;
  }

  PerfLogging(PerfLogging const&) = delete;
  void operator=(PerfLogging const&) = delete;

 public:
  PerfLogging() {}

  static PerfLogging& getInstance() {
    static PerfLogging _instance;
    return _instance;
  }

  void setBaseConfig(PushConfig* config) { _config = config; }

  void clear();
  void start(const char* key);
  void stop(const char* key);
  void print();
  void pushInflux();
};

extern PerfLogging gblPerfLogging;

// Use these to collect performance data from various parts of the code
#define PERF_BEGIN(s) gblPerfLogging.getInstance().start(s)
#define PERF_END(s) gblPerfLogging.getInstance().stop(s)
#define PERF_CLEAR() gblPerfLogging.getInstance().clear()
#define PERF_PUSH() gblPerfLogging.getInstance().pushInflux()

#else

// These will disable the performance collection function
#define PERF_BEGIN(s)
#define PERF_END(s)
#define PERF_CLEAR()
#define PERF_PUSH()

#endif  // PERF_ENABLE

#endif  // SRC_PERF_HPP_

// EOF
