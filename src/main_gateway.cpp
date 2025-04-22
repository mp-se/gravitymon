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
#if defined(GATEWAY)

#include <ble.hpp>
#include <config.hpp>
#include <display.hpp>
#include <helper.hpp>
#include <led.hpp>
#include <log.hpp>
#include <main.hpp>
#include <pushtarget.hpp>
#include <serialws.hpp>
#include <utils.hpp>
#include <web_gateway.hpp>
#include <wificonnection.hpp>
#if defined(ENABLE_SD_CARD)
#include <sd.h>
#endif
#include <battery.hpp>
#include <cstdio>
#include <deque>
#include <looptimer.hpp>
#include <measurement.hpp>
#include <uptime.hpp>

constexpr auto CFG_FILENAME = "/gravitymon-gw.json";
constexpr auto CFG_AP_SSID = "Gateway";
constexpr auto CFG_AP_PASS = "password";

#if !defined(USER_SSID)
#define USER_SSID ""
#define USER_PASS ""
#endif

void controller();
// void renderDisplayHeader();
// void renderDisplayLogs();
void updateDisplayStatus();
void updateDisplayLogs();
void checkSleepMode(float angle, float volt);

SerialDebug mySerial;
GravmonGatewayConfig myConfig(CFG_APPNAME, CFG_FILENAME);
WifiConnection myWifi(&myConfig, CFG_AP_SSID, CFG_AP_PASS, CFG_APPNAME,
                      USER_SSID, USER_PASS);
GatewayWebServer myWebServer(&myConfig);
SerialWebSocket mySerialWebSocket;
Display myDisplay;
BatteryVoltage myBatteryVoltage;  // Needs to be defined but not used in gateway
MeasurementList myMeasurementList;  // Data recevied from http or bluetooth
LoopTimer controllerTimer(5000), displayTimer(2000);

bool sleepModeAlwaysSkip =
    false;  // Needs to be defined but not used in gateway
RunMode runMode = RunMode::measurementMode;

const auto MAX_LOG_ENTRIES =
    5;  // Max number of events to save (what can be displayed)
std::deque<String> logEntryList;  // Last number of events
bool logUpdated = true;           // If the history log should be updated
int displayMeasurementIndex =
    0;  // What entry is shown on the top of the display

void setup() {
  // Main startup
  Log.notice(F("Main: Started setup for %s." CR), myConfig.getID());
  printBuildOptions();
  detectChipRevision();

  delay(2000);

  Log.notice(F("Main: TOUCH_CS %d." CR), TOUCH_CS);
  Log.notice(F("Main: TFT_BL %d." CR), TFT_BL);
  Log.notice(F("Main: TFT_DC %d." CR), TFT_DC);
  Log.notice(F("Main: TFT_MISO %d." CR), TFT_MISO);
  Log.notice(F("Main: TFT_MOSI %d." CR), TFT_MOSI);
  Log.notice(F("Main: TFT_SCLK %d." CR), TFT_SCLK);
  Log.notice(F("Main: TFT_RST %d." CR), TFT_RST);
  Log.notice(F("Main: TFT_CS %d." CR), TFT_CS);

  Log.notice(F("Main: Initialize display." CR));
  myDisplay.setup();
  myDisplay.setFont(FontSize::FONT_12);
  myDisplay.printLineCentered(1, "GravityMon Gateway");
  myDisplay.printLineCentered(3, "Starting");

  myConfig.checkFileSystem();
  myWifi.init();  // double reset check
  checkResetReason();
  myConfig.loadFile();

  // No stored config, move to portal
  if (!myWifi.hasConfig()) {
    Log.notice(
        F("Main: No wifi configuration detected, entering wifi setup." CR));
    runMode = RunMode::wifiSetupMode;
  }

  // Double reset, go to portal.
  if (myWifi.isDoubleResetDetected()) {
    Log.notice(F("Main: Double reset detected, entering wifi setup." CR));
    runMode = RunMode::wifiSetupMode;
  }

  // Do this setup for all modes exect wifi setup
  switch (runMode) {
    case RunMode::wifiSetupMode:
      Log.notice(F("Main: Entering WIFI Setup." CR));
      myDisplay.printLineCentered(3, "Entering WIFI Setup");
      myWifi.enableImprov(true);
      myWifi.startAP();
      break;

    case RunMode::measurementMode:
      Log.notice(F("Main: Connecting to WIFI." CR));
      myDisplay.printLineCentered(3, "Connecting to WIFI");

      if (strlen(myConfig.getWifiDirectSSID())) {
        myDisplay.printLineCentered(4, "Creating AP");
        Log.notice(F("Main: Connecting to WIFI and creating AP." CR));
        myWifi.connect(false, WIFI_AP_STA);
        myWifi.setAP(myConfig.getWifiDirectSSID(),
                     myConfig.getWifiDirectPass());
        myWifi.startAP(WIFI_AP_STA);
      } else {
        myWifi.connect(false, WIFI_AP);
      }
      break;
  }

  // Do this setup for configuration mode
  switch (runMode) {
    case RunMode::measurementMode:
      if (myWifi.isConnected()) {
        Log.notice(F("Main: Activating web server." CR));
        ledOn(LedColor::BLUE);  // Blue or slow flashing to indicate config mode
        Log.notice(F("Main: Synchronizing time." CR));
        myDisplay.printLineCentered(3, "Synchronizing time");
        myDisplay.printLineCentered(4, "");
        myWifi.timeSync(myConfig.getTimezone());

        case RunMode::wifiSetupMode:
          Log.notice(F("Main: Initializing the web server." CR));
          myWebServer.setupWebServer(
              "gravitymon-gateway");  // Takes less than 4ms, so skip
                                      // this measurement
          mySerialWebSocket.begin(myWebServer.getWebServer(), &Serial);
          mySerial.begin(&mySerialWebSocket);
      } else {
        Log.error(F("Main: Failed to connect with WIFI." CR));
        ledOn(LedColor::RED);  // Red or fast flashing to indicate connection
                               // error
        myDisplay.printLineCentered(5, "Failed to connect with WIFI");
      }
      break;

    default:
      break;
  }

    // Testing some SD access
#if defined(ENABLE_SD_CARD)
  if (!SD.begin(5)) {
    Log.error(F("Main: Failed to mount SD card." CR));
  } else {
    uint8_t cardType = SD.cardType();
    String type("Unknown");

    switch (cardType) {
      case CARD_NONE:
        type = "No memory";
        break;

      case CARD_MMC:
        type = "MMC";
        break;

      case CARD_SD:
        type = "SD";
        break;

      case CARD_SDHC:
        type = "SDCH";
        break;
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Log.info(F("Main: %s with %d MB attached." CR), type.c_str(), cardSize);
  }
#endif

  if (runMode == RunMode::measurementMode && myConfig.isBleEnable()) {
    Log.notice(F("Main: Initialize ble scanner." CR));
    bleScanner.setScanTime(myConfig.getBleScanTime());
    bleScanner.setAllowActiveScan(myConfig.getBleActiveScan());
    bleScanner.init();
  }

  Log.notice(F("Main: Startup completed." CR));
#if defined(ENABLE_TFT)
  myDisplay.printLineCentered(3, "Startup completed");
  myDisplay.setFont(FontSize::FONT_9);
  delay(1000);
  myDisplay.createUI();
#endif
  updateDisplayStatus();
}

void loop() {
  myUptime.calculate();
  myWebServer.loop();
  myWifi.loop();

  switch (runMode) {
    case RunMode::measurementMode:
      if (!myWifi.isConnected()) {
        Log.warning(F("Loop: Wifi was disconnected, trying to reconnect." CR));
        myWifi.connect();
        updateDisplayStatus();
      }
      controller();
      break;

    case RunMode::wifiSetupMode:
      break;
  }

  if (logUpdated) {
    updateDisplayLogs();
    logUpdated = false;
  }

  if (displayTimer.hasExipred()) {
    displayTimer.reset();

    if (myMeasurementList.size() == 0) {  // No data to display
      myDisplay.updateDevice("No data received", "", "", "", "", 0, 0);
    } else {
      // If the index is out of bounds, start over
      if (displayMeasurementIndex >= myMeasurementList.size())
        displayMeasurementIndex = 0;

      MeasurementEntry* entry =
          myMeasurementList.getMeasurementEntry(displayMeasurementIndex);

      char t[20], v1[20], v2[20], v3[20], s[20];
      strftime(t, sizeof(t), "%Y-%m-%d %H:%M:%S", entry->getTimeinfoUpdated());

      switch (entry->getType()) {
        case MeasurementType::Gravitymon: {
          const GravityData* gd = entry->getGravityData();

          float temp = myConfig.isTempFormatF() ? convertCtoF(gd->getTempC())
                                                : gd->getTempC();
          float gravity = myConfig.isGravityPlato()
                              ? convertToPlato(gd->getGravity())
                              : gd->getGravity();

          snprintf(v1, sizeof(v1), "%.3F%s", gravity,
                   myConfig.isGravitySG() ? "SG" : "P");
          snprintf(v2, sizeof(v2), "%.1F%s", temp,
                   myConfig.isTempUnitC() ? "°C" : "°F");
          snprintf(v3, sizeof(v3), "%.2FV", gd->getBattery());
          snprintf(s, sizeof(s), "Gravmon (%s)", gd->getId());

          myDisplay.updateDevice(strlen(gd->getName()) ? gd->getName() : s, v1,
                                 v2, v3, t, displayMeasurementIndex,
                                 myMeasurementList.size());
        } break;
        case MeasurementType::Pressuremon: {
          const PressureData* pd = entry->getPressureData();

          float temp = myConfig.isTempFormatF() ? convertCtoF(pd->getTempC())
                                                : pd->getTempC();
          float pressure = myConfig.isPressureBar()
                               ? convertPsiPressureToBar(pd->getPressure())
                           : myConfig.isPressureKpa()
                               ? convertPsiPressureToKPa(pd->getPressure())
                               : pd->getPressure();
          // float pressure1 =
          //     myConfig.isPressureBar()   ?
          //     convertPsiPressureToBar(pd->getPressure1()) :
          //     myConfig.isPressureKpa() ?
          //     convertPsiPressureToKPa(pd->getPressure1())
          //                                : pd->getPressure1();

          snprintf(v1, sizeof(v1), "%.3F%s", pressure,
                   myConfig.getPressureUnit());
          snprintf(v2, sizeof(v2), "%.1F%s", temp,
                   myConfig.isTempUnitC() ? "°C" : "°F");
          snprintf(v3, sizeof(v3), "%.2FV", pd->getBattery());
          snprintf(s, sizeof(s), "Pressmon (%s)", pd->getId());

          myDisplay.updateDevice(strlen(pd->getName()) ? pd->getName() : s, v1,
                                 v2, v3, t, displayMeasurementIndex,
                                 myMeasurementList.size());
        } break;
        case MeasurementType::Chamber: {
          const ChamberData* cd = entry->getChamberData();

          float chamberTemp = myConfig.isTempFormatF()
                                  ? convertCtoF(cd->getChamberTempC())
                                  : cd->getChamberTempC();
          float beerTemp = myConfig.isTempFormatF()
                               ? convertCtoF(cd->getBeerTempC())
                               : cd->getBeerTempC();

          snprintf(v1, sizeof(v1), "C: %.1F%s", chamberTemp,
                   myConfig.isTempUnitC() ? "°C" : "°F");
          snprintf(v2, sizeof(v2), "B: %.1F%s", beerTemp,
                   myConfig.isTempUnitC() ? "°C" : "°F");
          snprintf(v3, sizeof(v3), "");
          snprintf(s, sizeof(s), "Chamber (%s)", cd->getId());

          myDisplay.updateDevice(s, v1, v2, v3, t, displayMeasurementIndex,
                                 myMeasurementList.size());
        } break;
        case MeasurementType::TiltPro:
        case MeasurementType::Tilt: {
          const TiltData* td = entry->getTiltData();

          float temp = myConfig.isTempFormatF() ? convertCtoF(td->getTempC())
                                                : td->getTempC();
          float gravity = myConfig.isGravityPlato()
                              ? convertToPlato(td->getGravity())
                              : td->getGravity();

          snprintf(v1, sizeof(v1), "%.3F%s", gravity,
                   myConfig.isGravitySG() ? "SG" : "P");
          snprintf(v2, sizeof(v2), "%.1F%s", temp,
                   myConfig.isTempUnitC() ? "°C" : "°F");
          snprintf(v3, sizeof(v3), "");
          snprintf(s, sizeof(s), "Tilt: %s", td->getId());

          myDisplay.updateDevice(s, v1, v2, v3, t, displayMeasurementIndex,
                                 myMeasurementList.size());
        } break;
      }

      displayMeasurementIndex++;
    }
  }
}

void addGravityLogEntry(const char* id, const tm* timeinfo, float gravitySG,
                        float tempC) {
  float temp = myConfig.isTempFormatF() ? convertCtoF(tempC) : tempC;
  float gravity =
      myConfig.isGravityPlato() ? convertToPlato(gravitySG) : gravitySG;

  char s[60];
  snprintf(s, sizeof(s), "%02d:%02d ID:%s Gravity:%.3F%s Temp: %.1F%s",
           timeinfo->tm_hour, timeinfo->tm_min, id, gravity,
           myConfig.isGravitySG() ? "SG" : "P", temp,
           myConfig.isTempFormatC() ? "C" : "F");

  logEntryList.push_back(s);
  logUpdated = true;
}

void addPressureLogEntry(const char* id, const tm* timeinfo, float pressurePSI,
                         float pressure1PSI, float tempC) {
  float temp = myConfig.isTempFormatF() ? convertCtoF(tempC) : tempC;
  float pressure =
      myConfig.isPressureBar()   ? convertPsiPressureToBar(pressurePSI)
      : myConfig.isPressureKpa() ? convertPsiPressureToKPa(pressurePSI)
                                 : pressurePSI;
  float pressure1 =
      myConfig.isPressureBar()   ? convertPsiPressureToBar(pressure1PSI)
      : myConfig.isPressureKpa() ? convertPsiPressureToKPa(pressure1PSI)
                                 : pressure1PSI;

  char s[60];
  snprintf(s, sizeof(s), "%02d:%02d ID:%s Pressure:%.3F%s Temp:%.1F%s",
           timeinfo->tm_hour, timeinfo->tm_min, id, pressure,
           myConfig.getPressureUnit(), temp,
           myConfig.isTempFormatC() ? "C" : "F");

  logEntryList.push_back(s);
  logUpdated = true;
}

void addChamberLogEntry(const char* id, const tm* timeinfo, float chamberTempC,
                        float beerTempC) {
  float chamberTemp =
      myConfig.isTempFormatF() ? convertCtoF(chamberTempC) : chamberTempC;
  float beerTemp =
      myConfig.isTempFormatF() ? convertCtoF(beerTempC) : beerTempC;

  char s[60];
  snprintf(s, sizeof(s), "%02d:%02d ID:%s Chamber:%.1F%s Beer:%.1F%s",
           timeinfo->tm_hour, timeinfo->tm_min, id, chamberTemp,
           myConfig.isTempFormatC() ? "C" : "F", beerTemp,
           myConfig.isTempFormatC() ? "C" : "F");

  logEntryList.push_back(s);
  logUpdated = true;
}

void controller() {
  // Scan for ble beacons
  if (myConfig.isBleEnable()) {
    bleScanner.setScanTime(myConfig.getBleScanTime());
    bleScanner.setAllowActiveScan(myConfig.getBleActiveScan());
    bleScanner.scan();
  }

  if (controllerTimer.hasExipred()) {
    controllerTimer.reset();

    BrewingPush push(&myConfig);

    for (int i = 0; i < myMeasurementList.size(); i++) {
      MeasurementEntry* entry = myMeasurementList.getMeasurementEntry(i);

      switch (entry->getType()) {
        case MeasurementType::Gravitymon: {
          Log.notice("Loop: Processing Gravitymon data %d." CR, i);

          if (entry->isUpdated() &&
              (entry->getPushAge() > myConfig.getPushResendTime())) {
            const GravityData* gd = entry->getGravityData();

            addGravityLogEntry(gd->getId(), entry->getTimeinfoUpdated(),
                               gd->getGravity(), gd->getTempC());

            TemplatingEngine engine;

            setupTemplateEngineGravityGateway(
                engine, gd->getAngle(), gd->getGravity(), gd->getTempC(),
                gd->getBattery(), gd->getInterval(), gd->getId(),
                gd->getToken(), gd->getName());
            push.sendAll(engine, BrewingPush::MeasurementType::GRAVITY,
                         myConfig.isHttpPostGravityEnable(),
                         myConfig.isHttpPost2GravityEnable(),
                         myConfig.isHttpGetGravityEnable(),
                         myConfig.isInfluxdb2GravityEnable(),
                         myConfig.isMqttGravityEnable());

            entry->setPushed();
          }
        } break;

        case MeasurementType::Pressuremon: {
          Log.notice("Loop: Processing Pressuremon data %d." CR, i);

          if (entry->isUpdated() &&
              (entry->getPushAge() > myConfig.getPushResendTime())) {
            const PressureData* pd = entry->getPressureData();

            addPressureLogEntry(pd->getId(), entry->getTimeinfoUpdated(),
                                pd->getPressure(), pd->getPressure1(),
                                pd->getTempC());

            TemplatingEngine engine;

            setupTemplateEnginePressureGateway(
                engine, pd->getPressure(), pd->getPressure1(), pd->getTempC(),
                pd->getBattery(), pd->getInterval(), pd->getId(),
                pd->getToken(), pd->getName());
            push.sendAll(engine, BrewingPush::MeasurementType::PRESSURE,
                         myConfig.isHttpPostPressureEnable(),
                         myConfig.isHttpPost2PressureEnable(),
                         myConfig.isHttpGetPressureEnable(),
                         myConfig.isInfluxdb2PressureEnable(),
                         myConfig.isMqttPressureEnable());

            entry->setPushed();
          }
        } break;

        case MeasurementType::Tilt:
        case MeasurementType::TiltPro: {
          Log.notice("Loop: Processing Tilt data %d." CR, i);

#define ENABLE_TILT_SCANNING

#if defined(ENABLE_TILT_SCANNING)
          //  This part is for testing / debugging only, use Tiltbridge if you
          //  use Tilt
          //  as BLE transmission, will show detected tilt devices but dont
          //  send data.

          if (entry->isUpdated() &&
              (entry->getPushAge() > myConfig.getPushResendTime())) {
            const TiltData* pd = entry->getTiltData();

            addGravityLogEntry(pd->getId(), entry->getTimeinfoUpdated(),
                               pd->getGravity(), pd->getTempC());

            // TODO: Add push support

            entry->setPushed();
          }
#endif
        } break;

        case MeasurementType::Chamber: {
          Log.notice("Loop: Processing Chamber data %d." CR, i);
        } break;
      }
    }
  }
}

// void renderDisplayHeader() {
//   myDisplay.printLineCentered(0, "GravityMon Gateway");
// }

void updateDisplayStatus() {
  char info[80];

  switch (runMode) {
    case RunMode::measurementMode:
      if (strlen(myConfig.getWifiDirectSSID())) {
        snprintf(&info[0], sizeof(info), "%s - %s",
                 WiFi.localIP().toString().c_str(),
                 myConfig.getWifiDirectSSID());
      } else {
        snprintf(&info[0], sizeof(info), "%s",
                 WiFi.localIP().toString().c_str());
      }
      break;

    case RunMode::wifiSetupMode:
      snprintf(&info[0], sizeof(info), "Wifi Setup - 192.168.4.1");
      break;
  }

  myDisplay.updateStatus(info);
}

void updateDisplayLogs() {
  int idx = 0;

  while (logEntryList.size() > MAX_LOG_ENTRIES) logEntryList.pop_front();

  for (const auto& entry : logEntryList) {
    if (idx >= MAX_LOG_ENTRIES) break;
    Log.notice("Loop: Updating log entry %s (%d)." CR, entry.c_str(), idx);
    myDisplay.updateHistory(entry.c_str(), idx++);
  }
}

#endif  // GATEWAY

// EOF
