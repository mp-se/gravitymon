/*
 * GravityMon
 * Copyright (c) 2021-2026 Magnus
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Alternatively, this software may be used under the terms of a
 * commercial license. See LICENSE_COMMERCIAL for details.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
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
