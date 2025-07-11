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
#ifndef SRC_WEB_GRAVITYMON_HPP_
#define SRC_WEB_GRAVITYMON_HPP_

#if defined(GRAVITYMON)

#include <battery.hpp>
#include <config_gravitymon.hpp>
#include <pushtarget.hpp>
#include <templating.hpp>
#include <web_brewing.hpp>

class GravitymonWebServer : public BrewingWebServer {
 private:
  GravitymonConfig *_gravConfig = nullptr;

  void doTaskSensorCalibration();
  void doTaskPushTestSetup(TemplatingEngine &engine, BrewingPush &push);
  void doTaskHardwareScanning(JsonObject &obj);

  void doWebStatus(JsonObject &obj);
  void doWebConfigWrite();
  void doWebCalibrateStatus(JsonObject &obj);

 public:
  explicit GravitymonWebServer(GravitymonConfig *gravConfig)
      : BrewingWebServer(gravConfig), _gravConfig(gravConfig) {}

  virtual bool setupWebServer(const char *serviceName);

  void webHandleGyro(AsyncWebServerRequest *request);
};

// Global instance created
extern GravitymonWebServer myWebServer;

#endif  // GRAVITYMON

#endif  // SRC_WEB_GRAVITYMON_HPP_

// EOF
