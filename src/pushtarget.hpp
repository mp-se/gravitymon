/*
MIT License

Copyright (c) 2021-22 Magnus

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
#ifndef SRC_PUSHTARGET_HPP_
#define SRC_PUSHTARGET_HPP_

// Includes
#include <Arduino.h>
#include <ArduinoJson.h>

#include <helper.hpp>

// Classes
class PushTarget {
 private:
  uint32_t _ms;  // Used to check that we do not post to often

  void sendBrewfather(float angle, float gravity, float corrGravity,
                      float tempC);
  void sendHttp(String serverPath, float angle, float gravity,
                float corrGravity, float tempC, float runTime);
  void sendInfluxDb2(float angle, float gravity, float corrGravity, float tempC,
                     float runTime);
  void sendMqtt(float angle, float gravity, float corrGravity, float tempC,
                float runTime);
  void createIspindleFormat(DynamicJsonDocument &doc, float angle,
                            float gravity, float corrGravity, float tempC,
                            float runTime);

 public:
  PushTarget() { _ms = millis(); }
  void send(float angle, float gravity, float corrGravity, float temp,
            float runTime, bool force = false);
};

extern PushTarget myPushTarget;

#endif  // SRC_PUSHTARGET_HPP_

// EOF
