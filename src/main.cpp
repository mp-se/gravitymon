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

#if defined(GRAVITYMON)
const char* CFG_APPNAME = "gravitymon";
const char* CFG_FILENAME = "/gravitymon2.json";
const char* CFG_AP_SSID = "GravityMon";
const char* CFG_AP_PASS = "password";
#elif defined(PRESSUREMON)
const char* CFG_APPNAME = "pressuremon";
const char* CFG_FILENAME = "/pressuremon2.json";
const char* CFG_AP_SSID = "PressureMon";
const char* CFG_AP_PASS = "password";
#undef ENABLE_BLE  // Temporary
#endif             // GRAVITYMON

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
bool skipRunTimeLog = false;
RunMode runMode = RunMode::measurementMode;

#if defined(GRAVITYMON)
uint32_t stableGyroMillis;  // Used to calculate the total time since last
                            // stable gyro reading
#endif // GRAVITYMON

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

  // TODO: Remove the file from the old wifi manager if that exist.
  /*if (LittleFS.exists("/drd.dat")) {
    LittleFS.remove("/drd.dat");
  }*/

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
                         // in measurementMode

  // Do this setup for all modes exect wifi setup
  switch (runMode) {
    case RunMode::wifiSetupMode:
      // We cant use LED on ESP32C3 since that pin is connected to GYRO
      ledOn(LedColor::RED);  // Red or fast flashing to indicate connection
                             // error
      myWifi.startAP();
      break;

    default:
#if defined(GRAVITYMON)
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
                 myBatteryVoltage.getVoltage(), myGyro.getAngle(), runMode);
#elif defined(PRESSUREMON)
      // TODO: Setup pressure sensor 

      myBatteryVoltage.read();
      checkSleepModePressure(myBatteryVoltage.getVoltage());
      Log.notice(F("Main: Battery %F V, Run-mode=%d." CR),
                 myBatteryVoltage.getVoltage(), runMode);
#endif  // GRAVITYMON

#if defined(ESP32)
      if (!myConfig.isWifiPushActive() && runMode == RunMode::measurementMode) {
        Log.notice(
            F("Main: Wifi is not needed in gravity mode, skipping "
              "connection." CR));
        needWifi = false;
      }
#endif

      if (needWifi) {
        PERF_BEGIN("main-wifi-connect");
        if (myConfig.isWifiDirect() && runMode == RunMode::measurementMode) {
          myWifi.connect(true);
        } else {
          myWifi.connect();
        }
        PERF_END("main-wifi-connect");
      }

#if defined(GRAVITYMON)
      PERF_BEGIN("main-temp-setup");
      myTempSensor.setup();
      PERF_END("main-temp-setup");
#elif defined(PRESSUREMON)
#endif 
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
          myWebServer.setupWebServer();  // Takes less than 4ms, so skip
                                         // this measurement
          mySerialWebSocket.begin(myWebServer.getWebServer(), &Serial);
          mySerial.begin(&mySerialWebSocket);
      } else {
        // We cant use LED on ESP32C3 since that pin is connected to GYRO
        ledOn(LedColor::RED);  // Red or fast flashing to indicate connection
                               // error
      }

      interval = 1000;  // Change interval from 200ms to 1s
      break;

    default:
      // We cant use LED on ESP32C3 since that pin is connected to GYRO
      ledOn(
          LedColor::GREEN);  // Green or fast flashing to indicate gravity mode
      break;
  }

  PERF_END("main-setup");
  Log.notice(F("Main: Setup completed." CR));
#if defined(GRAVITYMON)
  pushMillis = stableGyroMillis =
      millis();  // Dont include time for wifi connection
#endif // GRAVITYMON
}

#if defined(GRAVITYMON)
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

    bool pushExpired = (abs(static_cast<int32_t>((millis() - pushMillis))) >
                        (myConfig.getSleepInterval() * 1000));

    if (myConfig.isIgnoreLowAnges() &&
        (angle < myConfig.getFormulaData().a[0])) {
      Log.warning(
          F("Main: Angle is lower than water, so we regard this as faulty and "
            "dont send any data." CR));
      pushExpired = false;
    }

    if (pushExpired || runMode == RunMode::measurementMode) {
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
        }
      }
#endif  // ESP32 && !ESP32S2

      if (myWifi.isConnected()) {  // no need to try if there is no wifi
                                   // connection.
        if (myConfig.isWifiDirect() && runMode == RunMode::measurementMode) {
          Log.notice(F(
              "Main: Sending data via Wifi Direct to Gravitymon Gateway." CR));

          TemplatingEngine engine;
          GravmonPush push(&myConfig);
          push.setupTemplateEngine(engine, angle, gravitySG, corrGravitySG,
                                   tempC, (millis() - runtimeMillis) / 1000,
                                   myBatteryVoltage.getVoltage());
          String tpl = push.getTemplate(GravmonPush::TEMPLATE_HTTP1,
                                        true);  // Use default post template
          String payload = engine.create(tpl.c_str());
          myConfig.setTargetHttpPost(
              "http://192.168.4.1/post");  // Default URL for Gravitymon Gateway
                                           // v0.3+
          myConfig.setHeader1HttpPost("Content-Type: application/json");
          myConfig.setHeader2HttpPost("");
          push.sendHttpPost(payload);
        } else {
          Log.notice(F("Main: Sending data to all defined push targets." CR));

          GravmonPush push(&myConfig);
          push.sendAll(angle, gravitySG, corrGravitySG, tempC,
                       (millis() - runtimeMillis) / 1000);

          // Only log when in gravity mode
          if (!skipRunTimeLog && runMode == RunMode::measurementMode) {
            Log.notice(
                F("Main: Updating history log with, runtime, gravity and "
                  "interval." CR));
            float runtime = (millis() - runtimeMillis);
            HistoryLog runLog(RUNTIME_FILENAME);
            runLog.addLog(runtime, gravitySG, myConfig.getSleepInterval());
          }
        }
      }
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
  if (abs(static_cast<int32_t>((millis() - loopMillis))) > interval) {
    loopReadGravity();
    loopMillis = millis();
    if (!myConfig.isGyroDisabled()) {
      PERF_BEGIN("loop-gyro-read");
      myGyro.read();
      PERF_END("loop-gyro-read");
    }
    myBatteryVoltage.read();

    if (runMode != RunMode::wifiSetupMode)
      checkSleepModeGravity(myGyro.getAngle(), myBatteryVoltage.getVoltage());
  }
}
#elif defined(PRESSUREMON)
bool loopReadPressure() {
}
void loopPressureOnInterval() {
  if (abs(static_cast<int32_t>((millis() - loopMillis))) > interval) {
    loopReadPressure();
    loopMillis = millis();

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
#endif

void goToSleep(int sleepInterval) {
  float volt = myBatteryVoltage.getVoltage();
  float runtime = (millis() - runtimeMillis);

  Log.notice(F("MAIN: Entering deep sleep for %ds, run time %Fs, "
               "battery=%FV." CR),
             sleepInterval,
             reduceFloatPrecision(runtime / 1000, DECIMALS_RUNTIME), volt);
  LittleFS.end();
#if defined(GRAVITYMON)
  myGyro.enterSleep();
#endif  // GRAVITYMON
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
#if defined(GRAVITYMON)
    case RunMode::storageMode:
      // This point is never reached, just here to remove warning.
      break;
#endif // GRAVITYMON

    case RunMode::wifiSetupMode:
    case RunMode::configurationMode:
      myWebServer.loop();
      myWifi.loop();
#if defined(GRAVITYMON)
      loopGravityOnInterval();
      delay(1);
#elif defined(PRESSUREMON)
      loopPressureOnInterval();
      delay(1);
#endif 

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

#if defined(GRAVITYMON)
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

      if (!myConfig.isGyroDisabled()) {
        PERF_BEGIN("loop-gyro-read");
        myGyro.read();
        PERF_END("loop-gyro-read");
      }
#elif defined(PRESSUREMON)
      if (loopReadPressure()) {
        myWifi.stopDoubleReset();
        goToSleep(myConfig.getSleepInterval());
      }
#endif 
      myWifi.loop();
      break;
  }
}

#if defined(GRAVITYMON)
void checkSleepModeGravity(float angle, float volt) {
#if defined(SKIP_SLEEPMODE)
  runMode = RunMode::configurationMode;
  Log.verbose(F("MAIN: Skipping sleep mode (SKIP_SLEEPMODE is defined)." CR));
  return;
#endif

#if defined(FORCE_GRAVITY_MODE)
  Log.notice(F("MAIN: Forcing device into gravity mode for debugging" CR));
  runMode = RunMode::measurementMode;
  return;
#endif

  if (!myConfig.hasGyroCalibration() && !myConfig.isGyroDisabled()) {
    // Will not enter sleep mode if: no calibration data
#if LOG_LEVEL == 6
    Log.notice(
        F("MAIN: Missing calibration data, so forcing webserver to be "
          "active." CR));
#endif
    runMode = RunMode::configurationMode;
  } else if (!myConfig.isGyroDisabled() &&
             (!myGyro.hasValue() || !myGyro.isConnected())) {
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
    runMode = RunMode::measurementMode;
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
    case RunMode::measurementMode:
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
#elif defined(PRESSUREMON)
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
  } else*/ if (sleepModeAlwaysSkip) {
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
#endif 


// EOF
