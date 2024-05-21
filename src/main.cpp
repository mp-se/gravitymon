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
#include <ble.hpp>
#undef LOG_LEVEL_ERROR
#undef LOG_LEVEL_INFO
#include <battery.hpp>
#include <calc.hpp>
#include <config.hpp>
#include <gyro.hpp>
#include <helper.hpp>
#include <history.hpp>
#include <led.hpp>
#include <log.hpp>
#include <main.hpp>
#include <ota.hpp>
#include <perf.hpp>
#include <pushtarget.hpp>
#include <serialws.hpp>
#include <tempsensor.hpp>
#include <utils.hpp>
#include <webserver.hpp>
#include <wificonnection.hpp>

#if defined(ACTIVATE_GCOV)
extern "C" {
#include <gcov_public.h>
}
#endif

constexpr auto CFG_APPNAME = "gravitymon";
constexpr auto CFG_FILENAME = "/gravitymon2.json";
constexpr auto CFG_AP_SSID = "gravitymon";
constexpr auto CFG_AP_PASS = "password";

#if !defined(USER_SSID)
#define USER_SSID ""
#define USER_PASS ""
#endif

// #define FORCE_GRAVITY_MODE
SerialDebug mySerial;
GravmonConfig myConfig(CFG_APPNAME, CFG_FILENAME);
WifiConnection myWifi(&myConfig, CFG_AP_SSID, CFG_AP_PASS, CFG_APPNAME,
                      USER_SSID, USER_PASS);
OtaUpdate myOta(&myConfig, CFG_APPVER);
BatteryVoltage myBatteryVoltage;
GravmonWebServer myWebServer(&myConfig);
SerialWebSocket mySerialWebSocket;
#if defined(ENABLE_BLE)
BleSender myBleSender;
#endif

// Define constats for this program
#ifdef DEACTIVATE_SLEEPMODE
const int interval = 1000;  // ms, time to wait between changes to output
#else
int interval = 200;  // ms, time to wait between changes to output
#endif
bool sleepModeAlwaysSkip =
    false;  // Flag set in web interface to override normal behaviour
uint32_t loopMillis = 0;  // Used for main loop to run the code every _interval_
uint32_t pushMillis = 0;  // Used to control how often we will send push data
uint32_t runtimeMillis;   // Used to calculate the total time since start/wakeup
uint32_t stableGyroMillis;  // Used to calculate the total time since last
                            // stable gyro reading

RunMode runMode = RunMode::gravityMode;

void checkSleepMode(float angle, float volt);

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
  myConfig.migrateSettings();
  myConfig.migrateHwSettings();
  myConfig.loadFile();
  PERF_END("main-config-load");

  // For restoring ispindel backup to test migration
  // LittleFS.rename("/ispindel.json", "/config.json");

  sleepModeAlwaysSkip = checkPinConnected();
  if (sleepModeAlwaysSkip) {
    Log.notice(F("Main: Forcing config mode since D7/D8 are connected." CR));
  }

  // Setup watchdog
#if defined(ESP8266)
  ESP.wdtDisable();
  ESP.wdtEnable(5000);  // 5 seconds
#else                   // defined (ESP32)
#endif

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
                         // in gravityMode

  // Do this setup for all modes exect wifi setup
  switch (runMode) {
    case RunMode::wifiSetupMode:
      myWifi.startWifiAP();
      break;

    default:
      if (myGyro.setup()) {
        PERF_BEGIN("main-gyro-read");
        myGyro.read();
        PERF_END("main-gyro-read");
      } else {
        Log.notice(
            F("Main: Failed to connect to the gyro, software will not be able "
              "to detect angles." CR));
      }

      myBatteryVoltage.read();
      checkSleepMode(myGyro.getAngle(), myBatteryVoltage.getVoltage());
      Log.notice(F("Main: Battery %F V, Gyro=%F, Run-mode=%d." CR),
                 myBatteryVoltage.getVoltage(), myGyro.getAngle(), runMode);

#if defined(ESP32)
      if (!myConfig.isWifiPushActive() && runMode == RunMode::gravityMode) {
        Log.notice(
            F("Main: Wifi is not needed in gravity mode, skipping "
              "connection." CR));
        needWifi = false;
      }
#endif

      if (needWifi) {
        PERF_BEGIN("main-wifi-connect");
        myWifi.connect();
        PERF_END("main-wifi-connect");
      }

      PERF_BEGIN("main-temp-setup");
      myTempSensor.setup();
      PERF_END("main-temp-setup");
      break;
  }

  // Do this setup for configuration mode
  switch (runMode) {
    case RunMode::configurationMode:
      if (myWifi.isConnected()) {
        Log.notice(F("Main: Activating web server." CR));
        ledOn(LedColor::BLUE);  // Blue or slow flashing to indicate config mode
                                // myWifi.timeSync();
        PERF_BEGIN("main-wifi-ota");
        if (myOta.checkFirmwareVersion()) myOta.updateFirmware();
        PERF_END("main-wifi-ota");
        case RunMode::wifiSetupMode:
          myWebServer.setupWebServer();  // Takes less than 4ms, so skip
                                         // this measurement
          mySerialWebSocket.begin(myWebServer.getWebServer(), &Serial);
          mySerial.begin(&mySerialWebSocket);
      } else {
        ledOn(LedColor::RED);  // Red or fast flashing to indicate connection
                               // error
      }

      interval = 1000;  // Change interval from 200ms to 1s
      break;

    default:
      break;
  }

  PERF_END("main-setup");
  Log.notice(F("Main: Setup completed." CR));
  pushMillis = stableGyroMillis =
      millis();  // Dont include time for wifi connection
}

// Main loop that does gravity readings and push data to targets
// Return true if gravity reading was successful
bool loopReadGravity() {
  float angle = 0;

#if LOG_LEVEL == 6
  Log.verbose(F("Main: Entering main loopGravity." CR));
#endif

  // Process the sensor values and push data to targets.
  // ------------------------------------------------------------------------------------------------
  // If we dont get any readings we just skip this and try again the next
  // interval.
  //
  if (myGyro.hasValue()) {
    angle = myGyro.getAngle();    // Gyro angle
    stableGyroMillis = millis();  // Reset timer

    PERF_BEGIN("loop-temp-read");
    myTempSensor.readSensor(myConfig.isGyroTemp());
    float tempC = myTempSensor.getTempC();
    PERF_END("loop-temp-read");

    float gravitySG = calculateGravity(angle, tempC);
    float corrGravitySG = gravityTemperatureCorrectionC(
        gravitySG, tempC, myConfig.getDefaultCalibrationTemp());

    if (myConfig.isGravityTempAdj()) {
      gravitySG = corrGravitySG;
    }
#if LOG_LEVEL == 6
    Log.verbose(F("Main: Sensor values gyro angle=%F, temp=%FC, gravity=%F, "
                  "corr_gravity=%F." CR),
                angle, tempC, gravitySG, corrGravitySG);
#endif

    bool pushExpired = (abs((int32_t)(millis() - pushMillis)) >
                        (myConfig.getSleepInterval() * 1000));

    if (myConfig.isIgnoreLowAnges() &&
        (angle < myConfig.getFormulaData().a[0])) {
      Log.warning(
          F("Main: Angle is lower than water, so we regard this as faulty and "
            "dont send any data." CR));
      pushExpired = false;
    }

    if (pushExpired || runMode == RunMode::gravityMode) {
      pushMillis = millis();
      PERF_BEGIN("loop-push");

#if defined(ESP32) && !defined(ESP32S2)
      if (myConfig.isBleActive()) {
        myBleSender.init();

        switch (myConfig.getBleFormat()) {
          case BleFormat::BLE_TILT: {
            String color = myConfig.getBleTiltColor();
            myBleSender.sendTiltData(color, convertCtoF(tempC), gravitySG,
                                     false);
          } break;
          case BleFormat::BLE_TILT_PRO: {
            String color = myConfig.getBleTiltColor();
            myBleSender.sendTiltData(color, convertCtoF(tempC), gravitySG,
                                     true);
          } break;
          case BleFormat::BLE_GRAVITYMON_IBEACON: {
            myBleSender.sendCustomBeaconData(myBatteryVoltage.getVoltage(),
                                             tempC, gravitySG, angle);
          } break;

          case BleFormat::BLE_GRAVITYMON_EDDYSTONE: {
            myBleSender.sendEddystone(myBatteryVoltage.getVoltage(), tempC,
                                      gravitySG, angle);
          } break;
          case BleFormat::BLE_GRAVITYMON_SERVICE: {
            TemplatingEngine engine;
            GravmonPush push(&myConfig);
            push.setupTemplateEngine(engine, angle, gravitySG, corrGravitySG,
                                     tempC, (millis() - runtimeMillis) / 1000,
                                     myBatteryVoltage.getVoltage());
            String tpl = push.getTemplate(GravmonPush::TEMPLATE_BLE);
            String payload = engine.create(tpl.c_str());
            myBleSender.sendGravitymonData(payload);
          } break;
        }
      }
#endif  // ESP32 && !ESP32S2

      if (myWifi.isConnected()) {  // no need to try if there is no wifi
                                   // connection.
        GravmonPush push(&myConfig);
        push.sendAll(angle, gravitySG, corrGravitySG, tempC,
                     (millis() - runtimeMillis) / 1000);
      }

#if defined(ESP32) && !defined(ESP32S2)
      if (myConfig.getBleFormat() == BleFormat::BLE_GRAVITYMON_SERVICE) {
        Log.notice(F("Main: Waiting for ble service to be read." CR));
        int i = 0;
        while (!myBleSender.isGravitymonDataSent() && i < 10) {
          delay(500);
          EspSerial.print(".");
          i++;
        }
        EspSerial.print(CR);
        if (myBleSender.isGravitymonDataSent())
          Log.notice(F("Main: Ble service was read by client." CR));
      }
#endif  // ESP32 && !ESP32S2

      PERF_END("loop-push");

      // Send stats to influx after each push run.
      if (runMode == RunMode::configurationMode) {
        PERF_PUSH();
      }
    }
    return true;
  } else {
    // Log.error(F("MAIN: No gyro value found, the device might be moving."
    // CR));
  }
  return false;
}

// Wrapper for loopGravity that only calls every 200ms so that we dont overload
// this.
void loopGravityOnInterval() {
  if (abs((int32_t)(millis() - loopMillis)) > interval) {
    loopReadGravity();
    loopMillis = millis();
    // printHeap("MAIN");
    PERF_BEGIN("loop-gyro-read");
    myGyro.read();
    PERF_END("loop-gyro-read");
    myBatteryVoltage.read();

    if (runMode != RunMode::wifiSetupMode)
      checkSleepMode(myGyro.getAngle(), myBatteryVoltage.getVoltage());
  }
}

bool skipRunTimeLog = false;

void goToSleep(int sleepInterval) {
  float volt = myBatteryVoltage.getVoltage();
  float runtime = (millis() - runtimeMillis);

  if (!skipRunTimeLog) {
    FloatHistoryLog runLog(RUNTIME_FILENAME);
    runLog.addEntry(runtime);
  }

  Log.notice(F("MAIN: Entering deep sleep for %ds, run time %Fs, "
               "battery=%FV." CR),
             sleepInterval,
             reduceFloatPrecision(runtime / 1000, DECIMALS_RUNTIME), volt);
  LittleFS.end();
  myGyro.enterSleep();
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
    case RunMode::storageMode:
      // This point is never reached, just here to remove warning.
      break;

    case RunMode::wifiSetupMode:
    case RunMode::configurationMode:
      myWebServer.loop();
      myWifi.loop();
      loopGravityOnInterval();
      delay(1);

      // If we switched mode, dont include this in the log.
      if (runMode != RunMode::configurationMode) skipRunTimeLog = true;
      break;

    case RunMode::gravityMode:
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

      if (loopReadGravity()) {
        myWifi.stopDoubleReset();
        goToSleep(myConfig.getSleepInterval());
      }

      // If the sensor is moving and we are not getting a clear reading, we
      // enter sleep for a short time to conserve battery.
      if (((millis() - stableGyroMillis) >
           10000L)) {  // 10s since last stable gyro reading
        Log.notice(
            F("MAIN: Unable to get a stable reading for 10s, sleeping for "
              "60s." CR));
        myWifi.stopDoubleReset();
        goToSleep(60);
      }

      PERF_BEGIN("loop-gyro-read");
      myGyro.read();
      PERF_END("loop-gyro-read");
      myWifi.loop();
      break;
  }
}

void checkSleepMode(float angle, float volt) {
#if defined(SKIP_SLEEPMODE)
  runMode = RunMode::configurationMode;
  Log.verbose(F("MAIN: Skipping sleep mode (SKIP_SLEEPMODE is defined)." CR));
  return;
#endif

#if defined(FORCE_GRAVITY_MODE)
  Log.notice(F("MAIN: Forcing device into gravity mode for debugging" CR));
  runMode = RunMode::gravityMode;
  return;
#endif

  if (!myConfig.hasGyroCalibration()) {
    // Will not enter sleep mode if: no calibration data
#if LOG_LEVEL == 6
    Log.notice(
        F("MAIN: Missing calibration data, so forcing webserver to be "
          "active." CR));
#endif
    runMode = RunMode::configurationMode;
  } else if (!myGyro.hasValue() || !myGyro.isConnected()) {
    runMode = RunMode::configurationMode;
  } else if (sleepModeAlwaysSkip) {
    // Check if the flag from the UI has been set, the we force configuration
    // mode.
#if LOG_LEVEL == 6
    Log.notice(F("MAIN: Sleep mode disabled from web interface." CR));
#endif
    runMode = RunMode::configurationMode;
  } else if ((volt < myConfig.getVoltageConfig() &&
              (angle > 85 && angle < 95)) ||
             (volt > myConfig.getVoltageConfig())) {
    runMode = RunMode::configurationMode;
  } else if (angle < 5 && myConfig.isStorageSleep()) {
    runMode = RunMode::storageMode;
  } else {
    runMode = RunMode::gravityMode;
  }

  switch (runMode) {
    case RunMode::configurationMode:
#if LOG_LEVEL == 6
      Log.notice(F("MAIN: run mode CONFIG (angle=%F volt=%F)." CR), angle,
                 volt);
#endif
      break;
    case RunMode::wifiSetupMode:
      break;
    case RunMode::gravityMode:
#if LOG_LEVEL == 6
      Log.notice(F("MAIN: run mode GRAVITY (angle=%F volt=%F)." CR), angle,
                 volt);
#endif
      break;
    case RunMode::storageMode:
#if LOG_LEVEL == 6
      Log.notice(F("MAIN: run mode STORAGE (angle=%F)." CR), angle);
#endif
      break;
  }

  // If we are in storage mode, just go back to sleep
  if (runMode == RunMode::storageMode) {
    Log.notice(
        F("Main: Storage mode entered, going to sleep for maximum time." CR));
#if defined(ESP8266)
    ESP.deepSleep(0);  // indefinite sleep
#else
    ESP.deepSleep(0);  // indefinite sleep
#endif
  }
}

// EOF
