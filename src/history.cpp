/*
MIT License

Copyright (c) 2021-2025 Magnus

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
#if defined(GRAVITYMON)

#include <LittleFS.h>

#include <cstdio>
#include <history.hpp>
#include <log.hpp>

HistoryLog::HistoryLog(String fName) {
  _fName = fName;

  File runFile = LittleFS.open(_fName, "r");
  if (runFile) {
    for (int i = 0; i < 10; i++) {
      String s = runFile.readStringUntil('\n');

      sscanf(s.c_str(), "%f;%f;%d", &_log[i]._runTime, &_log[i]._gravity,
             &_log[i]._sleepTime);

      if (_log[i]._runTime) {
        _average._runTime += _log[i]._runTime;
        _average._gravity += _log[i]._gravity;
        _average._sleepTime += _log[i]._sleepTime;
        _count++;
      }
    }
    runFile.close();
    _average._runTime = _average._runTime / _count;
    _average._gravity = _average._gravity / _count;
    _average._sleepTime = _average._sleepTime / _count;
  }
}

void HistoryLog::addLog(float runTime, float gravity, int sleepTime) {
  for (int i = (10 - 1); i > 0; i--) {
    _log[i] = _log[i - 1];
  }
  _log[0]._runTime = runTime;
  _log[0]._gravity = gravity;
  _log[0]._sleepTime = sleepTime;
  save();
}

void HistoryLog::save() {
  File runFile = LittleFS.open(_fName, "w");
  if (runFile) {
    for (int i = 0; i < 10; i++) {
      runFile.printf("%.4f;%.5f;%d\n", _log[i]._runTime, _log[i]._gravity,
                     _log[i]._sleepTime);
    }
    runFile.close();
  }
}

#endif  // GRAVITYMON

// EOF
