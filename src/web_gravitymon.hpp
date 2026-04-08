/*
  Copyright (c) 2021-2026 Magnus
  https://github.com/mp-se/gravitymon

  Licensed under the GNU General Public License v3.0 (GPL v3) for open source use,
  or a Commercial License for proprietary use. See LICENSE and LICENSE_COMMERCIAL.
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
  bool _gyroCalibrationSuccess = false;

  void doTaskSensorCalibration();
  void doTaskPushTestSetup(TemplatingEngine &engine, BrewingPush &push);
  void doTaskHardwareScanning(JsonObject &obj);

  void doWebStatus(JsonObject &obj);
  void doWebFeature(JsonObject &obj);
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
