/*
MIT License

Copyright (c) 2022-2025 Magnus

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
#if defined(PRESSUREMON)

#include <main.hpp>
#include <main_pressuremon.hpp>

// EspFramework
#include <wificonnection.hpp>
#include <serialws.hpp>
#include <led.hpp>
#include <log.hpp>
#include <ota.hpp>
#include <perf.hpp>
#include <looptimer.hpp>

// Common
#include <config.hpp>
#include <battery.hpp>
#include <webserver.hpp>
#include <helper.hpp>
#include <utils.hpp>
#include <history.hpp>
#include <pushtarget.hpp>

// Pressuremon specific
#include <ble.hpp>

const char* CFG_APPNAME = "pressuremon";
const char* CFG_FILENAME = "/pressuremon2.json";
const char* CFG_AP_SSID = "PressureMon";
const char* CFG_AP_PASS = "password";

#if !defined(USER_SSID)
#define USER_SSID ""
#define USER_PASS ""
#endif

#if defined(ESP8266)
#error "This target is not supported for Pressuremon"
#endif

SerialDebug mySerial;
PressuremonConfig myConfig(CFG_APPNAME, CFG_FILENAME);
WifiConnection myWifi(&myConfig, CFG_AP_SSID, CFG_AP_PASS, CFG_APPNAME,
                      USER_SSID, USER_PASS);
OtaUpdate myOta(&myConfig, CFG_APPVER);
BatteryVoltage myBatteryVoltage;
BrewingWebServer myWebServer(&myConfig);
SerialWebSocket mySerialWebSocket;
#if defined(ENABLE_BLE)
BleSender myBleSender;
#endif

// Define constats for this program
LoopTimer timerLoop(200);
bool sleepModeAlwaysSkip =
    false;  // Flag set in web interface to override normal behaviour
uint32_t runtimeMillis;   // Used to calculate the total time since start/wakeup
bool skipRunTimeLog = false;
RunMode runMode = RunMode::measurementMode;

void checkSleepModePressure(float volt);

void setup() {
  PERF_BEGIN("run-time");
  PERF_BEGIN("main-setup");
  runtimeMillis = millis();

  // Main startup
  Log.notice(F("Main: Started setup for %s." CR), myConfig.getID());
  printBuildOptions();
  detectChipRevision();

#if defined(RUN_HARDWARE_TEST)
  Log.notice(
      F("Main: Entering harware pin test, ensure that LED are connected to all "
        "relevant pints." CR));
  delay(2000);
  runGpioHardwareTests();
#endif

  PERF_BEGIN("main-config-load");
  myConfig.checkFileSystem();
  myWifi.init();  // double reset check
  checkResetReason();
  myConfig.loadFile();
  PERF_END("main-config-load");

  sleepModeAlwaysSkip = checkPinConnected();
  if (sleepModeAlwaysSkip) {
    Log.notice(F("Main: Forcing config mode since D7/D8 are connected." CR));
  }

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

  bool needWifi = true;  // Under ESP32 we dont need wifi if only BLE is active
                         // in measurementMode

  // Do this setup for all modes exect wifi setup
  switch (runMode) {
    case RunMode::wifiSetupMode:
      ledOn(LedColor::RED);  // Red or fast flashing to indicate connection
                             // error
      myWifi.startAP();
      break;

    default:
      /*
      if (!myConfig.isGyroDisabled()) {
        if (myGyro.setup()) {
          PERF_BEGIN("main-gyro-read");
          myGyro.read();
          PERF_END("main-gyro-read");
        } else {
          Log.notice(F(
              "Main: Failed to connect to the gyro, software will not be able "
              "to detect angles." CR));
        }
      } else {
        Log.notice(F("Main: Gyro is disabled in configuration." CR));
      }
      myBatteryVoltage.read();
      checkSleepModeGravity(myGyro.getAngle(), myBatteryVoltage.getVoltage());
      Log.notice(F("Main: Battery %F V, Gyro=%F, Run-mode=%d." CR),
                 myBatteryVoltage.getVoltage(), myGyro.getAngle(), runMode);*/

      // TODO: Setup pressure sensor

      myBatteryVoltage.read();
      checkSleepModePressure(myBatteryVoltage.getVoltage());
      Log.notice(F("Main: Battery %F V, Run-mode=%d." CR),
                 myBatteryVoltage.getVoltage(), runMode);

      if (!myConfig.isWifiPushActive() && runMode == RunMode::measurementMode) {
        Log.notice(
            F("Main: Wifi is not needed in gravity mode, skipping "
              "connection." CR));
        needWifi = false;
      }

      if (needWifi) {
        PERF_BEGIN("main-wifi-connect");
        if (myConfig.isWifiDirect() && runMode == RunMode::measurementMode) {
          myWifi.connect(true);
        } else {
          myWifi.connect();
        }
        PERF_END("main-wifi-connect");
      }

      /*
      PERF_BEGIN("main-temp-setup");
      myTempSensor.setup();
      PERF_END("main-temp-setup");*/

      // TODO Setup pressure sensor
      break;
  }

  // Do this setup for configuration mode
  switch (runMode) {
    case RunMode::configurationMode:
      if (myWifi.isConnected()) {
        Log.notice(F("Main: Activating web server." CR));
        // We cant use LED on ESP32C3 since that pin is connected to GYRO
        ledOn(LedColor::BLUE);  // Blue or slow flashing to indicate config mode
        PERF_BEGIN("main-wifi-ota");
        if (myOta.checkFirmwareVersion()) myOta.updateFirmware();
        PERF_END("main-wifi-ota");
        case RunMode::wifiSetupMode:
          myWebServer.setupWebServer("pressuremon");  // Takes less than 4ms, so
                                                      // skip this measurement
          mySerialWebSocket.begin(myWebServer.getWebServer(), &Serial);
          mySerial.begin(&mySerialWebSocket);
      } else {
        // We cant use LED on ESP32C3 since that pin is connected to GYRO
        ledOn(LedColor::RED);  // Red or fast flashing to indicate connection
                               // error
      }

      // interval = 1000;  // Change interval from 200ms to 1s
      break;

    default:
      // We cant use LED on ESP32C3 since that pin is connected to GYRO
      ledOn(
          LedColor::GREEN);  // Green or fast flashing to indicate gravity mode
      break;
  }

  PERF_END("main-setup");
  Log.notice(F("Main: Setup completed." CR));
}

bool loopReadPressure() { return false; }

void loopPressureOnInterval() {
  if (timerLoop.hasExipred()) {
    loopReadPressure();
    timerLoop.reset();

    /* TODO read pressure sensor
    if (!myConfig.isGyroDisabled()) {
      PERF_BEGIN("loop-gyro-read");
      myGyro.read();
      PERF_END("loop-gyro-read");
    }*/

    myBatteryVoltage.read();
    if (runMode != RunMode::wifiSetupMode)
      checkSleepModePressure(myBatteryVoltage.getVoltage());
  }
}

void goToSleep(int sleepInterval) {
  float volt = myBatteryVoltage.getVoltage();
  float runtime = (millis() - runtimeMillis);

  Log.notice(F("MAIN: Entering deep sleep for %ds, run time %Fs, "
               "battery=%FV." CR),
             sleepInterval,
             reduceFloatPrecision(runtime / 1000, DECIMALS_RUNTIME), volt);
  LittleFS.end();
  PERF_END("run-time");
  PERF_PUSH();

  if (myConfig.isBatterySaving() && (volt < 3.73 && volt > 2.0)) {
    sleepInterval = 3600;
  }

  delay(100);
  deepSleep(sleepInterval);
}

void loop() {
  switch (runMode) {
    case RunMode::wifiSetupMode:
    case RunMode::configurationMode:
      myWebServer.loop();
      myWifi.loop();
      loopPressureOnInterval();
      delay(1);

      // If we switched mode, dont include this in the log.
      if (runMode != RunMode::configurationMode) skipRunTimeLog = true;
      break;

    case RunMode::measurementMode:
      // If we didnt get a wifi connection, we enter sleep for a short time to
      // conserve battery.
      if (!myWifi.isConnected() &&
          myConfig.isWifiPushActive()) {  // no connection to wifi and we have
                                          // defined push targets.
        Log.notice(
            F("MAIN: No connection to wifi established, sleeping for 60s." CR));
        myWifi.stopDoubleReset();
        goToSleep(60);
      }

      if (loopReadPressure()) {
        myWifi.stopDoubleReset();
        goToSleep(myConfig.getSleepInterval());
      }

      myWifi.loop();
      break;
  }
}

void checkSleepModePressure(float volt) {
#if defined(SKIP_SLEEPMODE)
  runMode = RunMode::configurationMode;
  Log.verbose(F("MAIN: Skipping sleep mode (SKIP_SLEEPMODE is defined)." CR));
  return;
#endif

#if defined(FORCE_PRESSIRE_MODE)
  Log.notice(F("MAIN: Forcing device into pressure mode for debugging" CR));
  runMode = RunMode::measurementMode;
  return;
#endif

  /*  TODO: Will need this once we have selected pressure sensor

    if (!myConfig.hasGyroCalibration() && !myConfig.isGyroDisabled()) {
      // Will not enter sleep mode if: no calibration data
  #if LOG_LEVEL == 6
      Log.notice(
          F("MAIN: Missing calibration data, so forcing webserver to be "
            "active." CR));
  #endif
      runMode = RunMode::configurationMode;
    } else*/
  if (sleepModeAlwaysSkip) {
    // Check if the flag from the UI has been set, the we force configuration
    // mode.
#if LOG_LEVEL == 6
    Log.notice(F("MAIN: Sleep mode disabled from web interface." CR));
#endif
    runMode = RunMode::configurationMode;
  } else if (volt < myConfig.getVoltageConfig() ||
             (volt > myConfig.getVoltageConfig())) {
    runMode = RunMode::configurationMode;
  } else {
    runMode = RunMode::measurementMode;
  }

  switch (runMode) {
    case RunMode::configurationMode:
#if LOG_LEVEL == 6
      Log.notice(F("MAIN: run mode CONFIG (volt=%F)." CR), volt);
#endif
      break;
    case RunMode::wifiSetupMode:
      break;
    case RunMode::measurementMode:
#if LOG_LEVEL == 6
      Log.notice(F("MAIN: run mode PRESSURE (volt=%F)." CR), volt);
#endif
      break;
  }
}

#endif  // PRESSUREMON

// EOF
