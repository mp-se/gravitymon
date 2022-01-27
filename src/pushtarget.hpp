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

#include <templating.hpp>

#if defined(ESP8266)
#include <ESP8266HTTPClient.h>
#else  // defined (ESP32)
#include <HTTPClient.h>
#endif

class PushTarget {
 private:
  uint32_t _ms;  // Used to check that we do not post to often
  bool _memErrorReported =
      false;  // Avoid filling the error log with memory errors.

  void sendBrewfather(TemplatingEngine& engine);
  void sendHttp(TemplatingEngine& engine, int index);
  void sendInfluxDb2(TemplatingEngine& engine);
  void sendMqtt(TemplatingEngine& engine);
  void addHttpHeader(HTTPClient& http, String header);

 public:
  PushTarget() { _ms = millis(); }
  void send(float angle, float gravitySG, float corrGravitySG, float tempC,
            float runTime, bool force = false);
};

extern PushTarget myPushTarget;

#endif  // SRC_PUSHTARGET_HPP_

// EOF
