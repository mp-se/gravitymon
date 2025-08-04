/*
MIT License

Copyright (c) 2025 Magnus

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
#ifndef SRC_SPI_MANAGER_HPP_
#define SRC_SPI_MANAGER_HPP_

#include <Arduino.h>

#if defined(ESP32)
class SPIBusManager {
 private:
  SemaphoreHandle_t mutex;

 public:
  SPIBusManager() { mutex = xSemaphoreCreateMutex(); }

  bool acquire() {
    return xSemaphoreTake(mutex, pdMS_TO_TICKS(5)) == pdTRUE;
  }
  void release() { xSemaphoreGive(mutex); }
};
#else
class SPIBusManager {
 public:
  SPIBusManager() {}

  bool acquire() { return true; }
  void release() {}
};
#endif

extern SPIBusManager spiBus;

#endif  // SRC_SPI_MANAGER_HPP_

// EOF
