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
#if defined(GRAVITYMON)

// #define FORCE_GRAVITY_MODE

#include <main_gravitymon.hpp>

// EspFramework
#include <led.hpp>
#include <log.hpp>
#include <looptimer.hpp>
#include <main.hpp>
#include <ota.hpp>
#include <perf.hpp>
#include <serialws.hpp>
#include <wificonnection.hpp>

// Common
#include <battery.hpp>
#include <helper.hpp>
#include <pushtarget.hpp>
#include <utils.hpp>

// Gravitymon specific
#include <ble_gravitymon.hpp>
#include <calc.hpp>
#include <config_gravitymon.hpp>
#include <gyro.hpp>
#include <push_gravitymon.hpp>
#include <tempsensor.hpp>
#include <velocity.hpp>
#include <web_gravitymon.hpp>

#if defined(ESP32)
#include <esp_attr.h>
#endif

const char* CFG_FILENAME = "/gravitymon2.json";
const char* CFG_AP_SSID = "GravityMon";
const char* CFG_AP_PASS = "password";

#if !defined(USER_SSID)
#define USER_SSID ""
#define USER_PASS ""
#endif

// #define FORCE_GRAVITY_MODE
SerialDebug mySerial;
GravitymonConfig myConfig(CFG_APPNAME, CFG_FILENAME);
WifiConnection myWifi(&myConfig, CFG_AP_SSID, CFG_AP_PASS, CFG_APPNAME,
                      USER_SSID, USER_PASS);
OtaUpdate myOta(&myConfig, CFG_APPVER, CFG_FILENAMEBIN);
BatteryVoltage myBatteryVoltage(&myConfig);
GravitymonWebServer myWebServer(&myConfig);
SerialWebSocket mySerialWebSocket;
#if defined(ENABLE_BLE)
BleSender myBleSender;
#endif
GyroSensor myGyro(&myConfig);
TempSensor myTempSensor(&myConfig, &myGyro);
#if defined(ESP32)
RTC_DATA_ATTR GravityVelocityData data = {0};
#endif
LoopTimer timerLoop(200);
bool sleepModeAlwaysSkip =
    false;  // Flag set in web interface to override normal behaviour
uint32_t pushMillis = 0;  // Used to control how often we will send push data
uint32_t runtimeMillis;   // Used to calculate the total time since start/wakeup
uint32_t stableGyroMillis;  // Used to calculate the total time since last
                            // stable gyro reading
RunMode runMode = RunMode::measurementMode;

void checkSleepMode(float angle, float volt);
void runGpioHardwareTests();

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

#if defined(PIN_CFG1) && defined(PIN_CFG2)
  sleepModeAlwaysSkip = checkPinConnected(PIN_CFG1, PIN_CFG2);
  if (sleepModeAlwaysSkip) {
    Log.notice(F("Main: Forcing config mode since TX/RX are connected." CR));
  }
#endif

  // TODO: Remove the file from the old wifi manager if that exist.
  if (LittleFS.exists("/drd.dat")) {
    LittleFS.remove("/drd.dat");
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
                         // in measurementMode

  // Do this setup for all modes exect wifi setup
  switch (runMode) {
    case RunMode::wifiSetupMode:
      // We cant use LED on ESP32C3 since that pin is connected to GYRO
      ledOn(LedColor::RED);  // Red or fast flashing to indicate connection
                             // error
      myWifi.enableImprov(true);
      myWifi.startAP();
      break;

    default:
      if (myGyro.setup(GyroMode::GYRO_RUN, false)) {
        PERF_BEGIN("main-gyro-read");
        myGyro.read();
        PERF_END("main-gyro-read");
      } else {
        Log.notice(F(
            "Main: Failed to connect to the gyro, software will not be able "
            "to detect angles." CR));
      }

      myBatteryVoltage.read();
      checkSleepMode(myGyro.getAngle(), myBatteryVoltage.getVoltage());
      Log.notice(F("Main: Battery %F V, Gyro=%F, Run-mode=%d." CR),
                 myBatteryVoltage.getVoltage(), myGyro.getAngle(), runMode);

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

      PERF_BEGIN("main-temp-setup");
      myTempSensor.setup(PIN_DS);
      PERF_END("main-temp-setup");
      break;
  }

  // Do this setup for configuration mode
  switch (runMode) {
    case RunMode::configurationMode:
      if (myWifi.isConnected()) {
        // We cant use LED on ESP32C3 since that pin is connected to GYRO
        ledOn(LedColor::BLUE);  // Blue or slow flashing to indicate config mode

        Log.notice(F("Main: Switching gyro to continous." CR));
        myGyro.setup(GyroMode::GYRO_CONTINUOUS, true);

        PERF_BEGIN("main-wifi-ota");
        if (myOta.checkFirmwareVersion()) myOta.updateFirmware();
        PERF_END("main-wifi-ota");

        case RunMode::wifiSetupMode:
          Log.notice(F("Main: Activating web server." CR));
          myWebServer.setupWebServer("gravitymon");  // Takes less than 4ms, so
                                                     // skip this measurement
          mySerialWebSocket.begin(myWebServer.getWebServer(), &Serial);
          mySerial.begin(&mySerialWebSocket);
      } else {
        // We cant use LED on ESP32C3 since that pin is connected to GYRO
        ledOn(LedColor::RED);  // Red or fast flashing to indicate connection
                               // error
      }
      break;

    default:
      // We cant use LED on ESP32C3 since that pin is connected to GYRO
      ledOn(
          LedColor::GREEN);  // Green or fast flashing to indicate gravity mode
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
  float angle = 0, filteredAngle = 0;

#if LOG_LEVEL == 6
  Log.verbose(F("Main: Entering main loopGravity." CR));
#endif

  // Process the sensor values and push data to targets.
  // ------------------------------------------------------------------------------------------------
  // If we dont get any readings we just skip this and try again the next
  // interval.
  //
  if (myGyro.hasValue()) {
    angle = myGyro.getAngle();  // Gyro angle
    filteredAngle = myGyro.getFilteredAngle();
    stableGyroMillis = millis();  // Reset timer

    PERF_BEGIN("loop-temp-read");
    myTempSensor.readSensor(myConfig.isGyroTemp());
    float tempC = myTempSensor.getTempC();
    PERF_END("loop-temp-read");

    float gravitySG =
        calculateGravity(myConfig.getGravityFormula(), angle, tempC);
    float filteredGravitySG =
        calculateGravity(myConfig.getGravityFormula(), filteredAngle, tempC);
    float corrGravitySG = gravityTemperatureCorrectionC(
        myConfig.isGyroFilter() ? filteredGravitySG : gravitySG, tempC,
        myConfig.getDefaultCalibrationTemp());

    // Corrected gravity can contain either temperature corrected gravity /
    // filtered gravity.
    if (myConfig.isGravityTempAdj()) {
      gravitySG = corrGravitySG;
    } else if (myConfig.isGyroFilter()) {
      // TODO: Uncomment this next line once the
      // filters has been tested correctly
      // gravitySG = filteredGravitySG;
    }

    float velocity = 0;

#if defined(ESP32)
    GravityVelocity gv(&data, myConfig.getSleepInterval());
    gv.addValue(gravitySG);
    velocity = gv.getVelocity();
#endif

    Log.warning(F("Main: Angle: %F (%F), Velocity: %F" CR), angle,
                filteredAngle, velocity);

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

#if defined(ENABLE_BLE)
      if (myConfig.isBleActive()) {
        myBleSender.init();

        switch (myConfig.getGravitymonBleFormat()) {
          case GravitymonBleFormat::BLE_TILT: {
            String color = myConfig.getBleTiltColor();
            myBleSender.sendTiltData(color, convertCtoF(tempC), gravitySG,
                                     false);
          } break;
          case GravitymonBleFormat::BLE_TILT_PRO: {
            String color = myConfig.getBleTiltColor();
            myBleSender.sendTiltData(color, convertCtoF(tempC), gravitySG,
                                     true);
          } break;
          case GravitymonBleFormat::BLE_GRAVITYMON_IBEACON: {
            myBleSender.sendCustomBeaconData(myBatteryVoltage.getVoltage(),
                                             tempC, gravitySG, angle);
          } break;

          case GravitymonBleFormat::BLE_GRAVITYMON_EDDYSTONE: {
            myBleSender.sendEddystoneData(myBatteryVoltage.getVoltage(), tempC,
                                          gravitySG, angle);
          } break;
        }
      }
#endif  // ENABLE_BLE

      if (myWifi.isConnected()) {  // no need to try if there is no wifi
                                   // connection.
        if (myConfig.isWifiDirect() && runMode == RunMode::measurementMode) {
          Log.notice(F(
              "Main: Sending data via Wifi Direct to Gravitymon Gateway." CR));

          TemplatingEngine engine;
          BrewingPush push(&myConfig);
          setupTemplateEngineGravity(&myConfig, engine, angle, velocity,
                                     gravitySG, corrGravitySG, tempC,
                                     (millis() - runtimeMillis) / 1000,
                                     myBatteryVoltage.getVoltage());
          String tpl = push.getTemplate(BrewingPush::GRAVITY_TEMPLATE_HTTP1,
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

          TemplatingEngine engine;
          BrewingPush push(&myConfig);

          setupTemplateEngineGravity(&myConfig, engine, angle, velocity,
                                     gravitySG, corrGravitySG, tempC,
                                     (millis() - runtimeMillis) / 1000,
                                     myBatteryVoltage.getVoltage());
          push.sendAll(engine, BrewingPush::MeasurementType::GRAVITY);
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
  if (timerLoop.hasExpired()) {
    loopReadGravity();
    timerLoop.reset();

    PERF_BEGIN("loop-gyro-read");
    myGyro.read();
    PERF_END("loop-gyro-read");
    myBatteryVoltage.read();

    if (runMode != RunMode::wifiSetupMode)
      checkSleepMode(myGyro.getAngle(), myBatteryVoltage.getVoltage());
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
  myGyro.enterSleep();
  PERF_END("run-time");
  PERF_PUSH();

  if (myConfig.isBatterySaving() && getBatteryPercentage(volt, BatteryType::LithiumIon) < 30) {
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
  runMode = RunMode::measurementMode;
  return;
#endif

  if ((!myConfig.hasGyroCalibration() && myGyro.needCalibration())) {
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
    myWifi.stopDoubleReset(); // Ensure we dont go into wifi mode when wakeup
    delay(100);
#if defined(ESP8266)
    ESP.deepSleep(0);  // indefinite sleep
#else
    ESP.deepSleep(0);  // indefinite sleep
#endif
  }
}

void runGpioHardwareTests() {
#if defined(RUN_HARDWARE_TEST)
  int max = 10;

  Log.notice(F("Main: Configuring GPIO for output." CR));
  pinMode(PIN_SDA, OUTPUT);
  pinMode(PIN_SCL, OUTPUT);
  pinMode(PIN_CFG1, OUTPUT);
  pinMode(PIN_CFG2, OUTPUT);
  pinMode(PIN_DS, OUTPUT);
  pinMode(PIN_VOLT, OUTPUT);
  delay(100);
  digitalWrite(PIN_SDA, LOW);
  digitalWrite(PIN_SCL, LOW);
  digitalWrite(PIN_CFG1, LOW);
  digitalWrite(PIN_CFG2, LOW);
  digitalWrite(PIN_DS, LOW);
  digitalWrite(PIN_VOLT, LOW);
  delay(100);

  int sleep = 700;

  Log.notice(F("Main: Testing SDA." CR));
  for (int i = 0; i < max; i++) {
    digitalWrite(PIN_SDA, i % 2);
    delay(sleep);
  }
  digitalWrite(PIN_SDA, LOW);

  Log.notice(F("Main: Testing SCL." CR));
  for (int i = 0; i < max; i++) {
    digitalWrite(PIN_SCL, i % 2);
    delay(sleep);
  }
  digitalWrite(PIN_SCL, LOW);

  Log.notice(F("Main: Testing CFG1." CR));
  for (int i = 0; i < max; i++) {
    digitalWrite(PIN_CFG1, i % 2);
    delay(sleep);
  }
  digitalWrite(PIN_CFG1, LOW);

  Log.notice(F("Main: Testing CFG2." CR));
  for (int i = 0; i < max; i++) {
    digitalWrite(PIN_CFG2, i % 2);
    delay(sleep);
  }
  digitalWrite(PIN_CFG2, LOW);

  Log.notice(F("Main: Testing DS." CR));
  for (int i = 0; i < max; i++) {
    digitalWrite(PIN_DS, i % 2);
    delay(sleep);
  }
  digitalWrite(PIN_DS, LOW);

  Log.notice(F("Main: Testing VOLT." CR));
  for (int i = 0; i < max; i++) {
    digitalWrite(PIN_VOLT, i % 2);
    delay(sleep);
  }
  digitalWrite(PIN_VOLT, LOW);

  Log.notice(F("Main: Tests finished, enter waiting for reset." CR));
  while (true) delay(sleep);
#endif  // RUN_HARDWARE_TEST
}

#endif  // GRAVITYMON

// EOF
