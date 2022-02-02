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
#include <calc.hpp>
#include <config.hpp>
#include <gyro.hpp>
#include <helper.hpp>
#include <main.hpp>
#include <pushtarget.hpp>
#include <tempsensor.hpp>
#include <webserver.hpp>
#include <wifi.hpp>

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

enum RunMode { gravityMode = 0, configurationMode = 1, wifiSetupMode = 2 };

RunMode runMode = RunMode::gravityMode;

//
// Check if we should be in sleep mode
//
void checkSleepMode(float angle, float volt) {
#if defined(SKIP_SLEEPMODE)
  runMode = RunMode::configurationMode;
  Log.verbose(F("MAIN: Skipping sleep mode (SKIP_SLEEPMODE is defined)." CR));
  return;
#endif

  const RawGyroData &g = myConfig.getGyroCalibration();

  if (!g.ax && !g.ay && !g.az && !g.gx && !g.gy && !g.gz) {
    // Will not enter sleep mode if: no calibration data
#if !defined(MAIN_DISABLE_LOGGING)
    Log.notice(
        F("MAIN: Missing calibration data, so forcing webserver to be "
          "active." CR));
#endif
    runMode = RunMode::configurationMode;
  } else if (sleepModeAlwaysSkip) {
    // Check if the flag from the UI has been set, the we force configuration
    // mode.
#if !defined(MAIN_DISABLE_LOGGING)
    Log.notice(F("MAIN: Sleep mode disabled from web interface." CR));
#endif
    runMode = RunMode::configurationMode;
  } else if ((volt < 4.15 && (angle > 85 && angle < 95)) || (volt > 4.15)) {
    runMode = RunMode::configurationMode;
  } else {
    runMode = RunMode::gravityMode;
  }

  switch (runMode) {
    case RunMode::configurationMode:
#if !defined(MAIN_DISABLE_LOGGING)
      Log.notice(F("MAIN: run mode CONFIG (angle=%F volt=%F)." CR), angle,
                 volt);
#endif
      break;
    case RunMode::wifiSetupMode:
      break;
    case RunMode::gravityMode:
#if !defined(MAIN_DISABLE_LOGGING)
      Log.notice(F("MAIN: run mode GRAVITY (angle=%F volt=%F)." CR), angle,
                 volt);
#endif
      break;
  }
}

//
// Setup
//
void setup() {
  LOG_PERF_START("run-time");
  LOG_PERF_START("main-setup");
  runtimeMillis = millis();

#if LOG_LEVEL == 6 && !defined(MAIN_DISABLE_LOGGING)
  // Add a delay so that serial is started.
  // delay(3000);
#if defined(ESP8266)
  Log.verbose(F("Main: Reset reason %s." CR), ESP.getResetInfo().c_str());
#else  // defined (ESP32)
#endif
#endif
  // Main startup
#if defined(ESP8266)
  Log.notice(F("Main: Started setup for %s." CR),
             String(ESP.getChipId(), HEX).c_str());
#else  // defined (ESP32)
  char buf[20];
  uint32_t chipId = 0;
  for (int i = 0; i < 17; i = i + 8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  snprintf(&buf[0], sizeof(buf), "%6x", chipId);
  Log.notice(F("Main: Started setup for %s." CR), &buf[0]);
#endif
  printBuildOptions();

  LOG_PERF_START("main-config-load");
  myConfig.checkFileSystem();
  myConfig.loadFile();
  myWifi.init();
  myHardwareConfig.loadFile();
  LOG_PERF_STOP("main-config-load");

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

  // Do this setup for all modes exect wifi setup
  switch (runMode) {
    case RunMode::wifiSetupMode:
      myWifi.startPortal();
      break;

    default:
      LOG_PERF_START("main-wifi-connect");
      myWifi.connect();
      LOG_PERF_STOP("main-wifi-connect");

      LOG_PERF_START("main-temp-setup");
      myTempSensor.setup();
      LOG_PERF_STOP("main-temp-setup");

      if (!myGyro.setup()) {
        ErrorFileLog errLog;
        errLog.addEntry(
            F("MAIN: Failed to initialize the gyro, is it connected?"));
      } else {
        LOG_PERF_START("main-gyro-read");
        myGyro.read();
        LOG_PERF_STOP("main-gyro-read");
      }

      myBatteryVoltage.read();
      checkSleepMode(myGyro.getAngle(), myBatteryVoltage.getVoltage());
      break;
  }

  // Do this setup for configuration mode
  switch (runMode) {
    case RunMode::configurationMode:
      if (myWifi.isConnected()) {
#if defined(ACTIVATE_OTA)
        LOG_PERF_START("main-wifi-ota");
        if (myWifi.checkFirmwareVersion()) myWifi.updateFirmware();
        LOG_PERF_STOP("main-wifi-ota");
#endif
        myWebServerHandler
            .setupWebServer();  // Takes less than 4ms, so skip this measurement
      }

      interval = 1000;  // Change interval from 200ms to 1s
      break;

    default:
      break;
  }

  LOG_PERF_STOP("main-setup");
  Log.notice(F("Main: Setup completed." CR));
  pushMillis = stableGyroMillis =
      millis();  // Dont include time for wifi connection
}

//
// Main loop that does gravity readings and push data to targets
//
// Return true if gravity reading was successful
//
bool loopReadGravity() {
  float angle = 0;

#if LOG_LEVEL == 6 && !defined(MAIN_DISABLE_LOGGING)
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

    LOG_PERF_START("loop-temp-read");
    float tempC = myTempSensor.getTempC(myConfig.isGyroTemp());
    LOG_PERF_STOP("loop-temp-read");

    float gravitySG = calculateGravity(angle, tempC);
    float corrGravitySG = gravityTemperatureCorrectionC(gravitySG, tempC);

#if LOG_LEVEL == 6 && !defined(MAIN_DISABLE_LOGGING)
    Log.verbose(F("Main: Sensor values gyro angle=%F, temp=%FC, gravity=%F, "
                  "corr_gravity=%F." CR),
                angle, tempC, gravity, corrGravity);
#endif

    LOG_PERF_START("loop-push");
    bool pushExpired = (abs((int32_t)(millis() - pushMillis)) >
                        (myConfig.getSleepInterval() * 1000));

    if (pushExpired || runMode == RunMode::gravityMode) {
      pushMillis = millis();
      PushTarget push;
      push.send(angle, gravitySG, corrGravitySG, tempC,
                (millis() - runtimeMillis) / 1000);
    }
    LOG_PERF_STOP("loop-push");
    return true;
  } else {
    Log.error(F("MAIN: No gyro value found, the device might be moving."));
  }
  return false;
}

//
// Wrapper for loopGravity that only calls every 200ms so that we dont overload
// this.
//
void loopGravityOnInterval() {
  if (abs((int32_t)(millis() - loopMillis)) > interval) {
    loopReadGravity();
    loopMillis = millis();
    // printHeap("MAIN");
    LOG_PERF_START("loop-gyro-read");
    myGyro.read();
    LOG_PERF_STOP("loop-gyro-read");
    myBatteryVoltage.read();
    checkSleepMode(myGyro.getAngle(), myBatteryVoltage.getVoltage());
  }
}

bool skipRunTimeLog = false;

//
// Main loop that determines if device should go to sleep
//
void goToSleep(int sleepInterval) {
  float volt = myBatteryVoltage.getVoltage();
  float runtime = (millis() - runtimeMillis);

  if (!skipRunTimeLog) {
    FloatHistoryLog runLog(RUNTIME_FILENAME);
    runLog.addEntry(runtime);
  }

  Log.notice(F("MAIN: Entering deep sleep for %ds, run time %Fs, "
               "battery=%FV." CR),
             sleepInterval, reduceFloatPrecision(runtime / 1000, 2), volt);
  LittleFS.end();
  myGyro.enterSleep();
  LOG_PERF_STOP("run-time");
  LOG_PERF_PUSH();
  delay(100);
  deepSleep(sleepInterval);
}

//
// Main loops
//
void loop() {
  switch (runMode) {
    case RunMode::configurationMode:
      myWebServerHandler.loop();
      myWifi.loop();
      loopGravityOnInterval();

      // If we switched mode, dont include this in the log.
      if (runMode != RunMode::configurationMode) skipRunTimeLog = true;
      break;

    case RunMode::gravityMode:
      // If we didnt get a wifi connection, we enter sleep for a short time to
      // conserve battery.
      if (!myWifi.isConnected()) {  // no connection to wifi
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

      LOG_PERF_START("loop-gyro-read");
      myGyro.read();
      LOG_PERF_STOP("loop-gyro-read");
      myWifi.loop();
      break;

    case RunMode::wifiSetupMode:
      myWifi.loop();
      break;
  }
}

// EOF
