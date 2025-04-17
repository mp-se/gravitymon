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
void renderDisplayHeader();
void renderDisplayFooter();
void renderDisplayLogs();
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
LoopTimer controllerTimer(5000);

// Define constats for this program
bool sleepModeAlwaysSkip =
    false;            // Needs to be defined but not used in gateway
int interval = 1000;  // ms, time to wait between changes to output
RunMode runMode = RunMode::measurementMode;

struct LogEntry {
  char s[60] = "";
};

const auto maxLogEntries = 9;
LogEntry logEntryList[maxLogEntries];
int logIndex = 0;
bool logUpdated = true;

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
  myDisplay.clear();
#endif
  renderDisplayHeader();
  renderDisplayFooter();
}

void loop() {
  myUptime.calculate();
  myWebServer.loop();
  myWifi.loop();
  // myDisplay.loop();

  switch (runMode) {
    case RunMode::measurementMode:
      if (!myWifi.isConnected()) {
        Log.warning(F("Loop: Wifi was disconnected, trying to reconnect." CR));
        myWifi.connect();
        renderDisplayFooter();
      }
      controller();
      break;

    case RunMode::wifiSetupMode:
      break;
  }

  if (logUpdated) {
    renderDisplayLogs();
    logUpdated = false;
  }
}

void addGravityLogEntry(const char* id, tm timeinfo, float gravitySG,
                        float tempC) {
  float temp = myConfig.isTempFormatF() ? convertCtoF(tempC) : tempC;
  float gravity =
      myConfig.isGravityPlato() ? convertToPlato(gravitySG) : gravitySG;

  snprintf(&logEntryList[logIndex].s[0], sizeof(LogEntry::s),
           "%02d:%02d G %s %.3F%s %.1F%s", timeinfo.tm_hour, timeinfo.tm_min,
           id, gravity, myConfig.isGravitySG() ? "SG" : "P", temp,
           myConfig.isTempFormatC() ? "C" : "F");

  if (++logIndex >= maxLogEntries) logIndex = 0;

  logUpdated = true;
}

void addPressureLogEntry(const char* id, tm timeinfo, float pressurePSI,
                         float pressure1PSI, float tempC) {
  float temp = myConfig.isTempFormatF() ? convertCtoF(tempC) : tempC;

  // TODO FIX correct pressure conversion based on current setting
  float pressure = pressurePSI;
  float pressure1 = pressure1PSI;

  snprintf(&logEntryList[logIndex].s[0], sizeof(LogEntry::s),
           "%02d:%02d P %s %.3F%s %.1F%s", timeinfo.tm_hour, timeinfo.tm_min,
           id, pressure, "PSI", temp, myConfig.isTempFormatC() ? "C" : "F");

  if (++logIndex >= maxLogEntries) logIndex = 0;

  logUpdated = true;
}

void addChamberLogEntry(const char* id, tm timeinfo, float chamberTempC,
                        float beerTempC) {
  float chamberTemp =
      myConfig.isTempFormatF() ? convertCtoF(chamberTempC) : chamberTempC;
  float beerTemp =
      myConfig.isTempFormatF() ? convertCtoF(beerTempC) : beerTempC;

  snprintf(&logEntryList[logIndex].s[0], sizeof(LogEntry::s),
           "%02d:%02d C %s %.1F%s %.1F%s", timeinfo.tm_hour, timeinfo.tm_min,
           id, chamberTemp, myConfig.isTempFormatC() ? "C" : "F", beerTemp,
           myConfig.isTempFormatC() ? "C" : "F");

  if (++logIndex >= maxLogEntries) logIndex = 0;

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

void renderDisplayHeader() {
  myDisplay.printLineCentered(0, "GravityMon Gateway");
}

void renderDisplayFooter() {
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

  myDisplay.printLineCentered(10, &info[0]);
}

void renderDisplayLogs() {
  for (int i = 0, j = logIndex; i < maxLogEntries; i++) {
    j--;
    if (j < 0) j = maxLogEntries - 1;
    myDisplay.printLine(i + 1, &logEntryList[j].s[0]);
  }
}

#endif  // GATEWAY

// EOF
