/*
MIT License

Copyright (c) 2025 Magnus

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
#include <led.hpp>
#include <log.hpp>
#include <looptimer.hpp>
#include <ota.hpp>
#include <perf.hpp>
#include <serialws.hpp>
#include <wificonnection.hpp>

// Common
#include <battery.hpp>
#include <helper.hpp>
#include <pushtarget.hpp>
#include <utils.hpp>

// Pressuremon specific
#include <ble_pressuremon.hpp>
#include <config_pressuremon.hpp>
#include <pressure.hpp>
#include <push_pressuremon.hpp>
#include <tempsensor.hpp>
#include <web_pressuremon.hpp>

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

// #define FORCE_PRESSURE_MODE

SerialDebug mySerial;
PressuremonConfig myConfig(CFG_APPNAME, CFG_FILENAME);
WifiConnection myWifi(&myConfig, CFG_AP_SSID, CFG_AP_PASS, CFG_APPNAME,
                      USER_SSID, USER_PASS);
OtaUpdate myOta(&myConfig, CFG_APPVER);
BatteryVoltage myBatteryVoltage(&myConfig);
PressuremonWebServer myWebServer(&myConfig);
SerialWebSocket mySerialWebSocket;
#if defined(ENABLE_BLE)
BleSender myBleSender;
#endif
PressureSensor myPressureSensor(&myConfig);
PressureSensor myPressureSensor1(&myConfig);
TempSensor myTempSensor(&myConfig, nullptr);

// Define constats for this program
LoopTimer timerLoop(1000);
bool sleepModeAlwaysSkip =
    false;  // Flag set in web interface to override normal behaviour
uint32_t pushMillis = 0;  // Used to control how often we will send push data
uint32_t runtimeMillis;   // Used to calculate the total time since start/wakeup
RunMode runMode = RunMode::measurementMode;

void checkSleepModePressure(float volt);

void setup() {
  pinMode(PIN_PWR, OUTPUT);
  delay(5);

  delay(3000);  // Wait for power to stabilize

  PERF_BEGIN("run-time");
  PERF_BEGIN("main-setup");
  runtimeMillis = millis();

#if defined(ESPFWK_USE_SERIAL_PINS)
  // Serial pints used for serial output
  mySerial.setup(115200L, TX, RX);
  Log.notice(F("Main: Using serial pins as output." CR));
#else
  // Serial pints used to force config mode
  sleepModeAlwaysSkip = checkPinConnected();
  if (sleepModeAlwaysSkip) {
    Log.notice(F("Main: Forcing config mode since TX/RX are connected." CR));
  }
#endif

  // Main startup
  Log.notice(F("Main: Started setup for %s." CR), myConfig.getID());
  printBuildOptions();
  detectChipRevision();

  ledOn(LedColor::GREEN);

  PERF_BEGIN("main-config-load");
  myConfig.checkFileSystem();
  myWifi.init();
  checkResetReason();
  myConfig.loadFile();
  PERF_END("main-config-load");

  gpio_hold_dis((gpio_num_t)PIN_PWR);
  digitalWrite(PIN_PWR, HIGH);  // Power to sensors = On
  delay(200);                   // Wait for pin to settle and power to stabilize

  int clock = 400000;

  Log.notice(F("Main: OneWire SDA=%d, SCL=%d." CR), PIN_SDA, PIN_SCL);
  Wire.setPins(PIN_SDA, PIN_SCL);
  Wire.begin();
  Wire.setClock(clock);

  // Log.notice(F("Main: OneWire1 SDA=%d, SCL=%d." CR), PIN_SDA1, PIN_SCL1);
  // Wire1.setPins(PIN_SDA1, PIN_SCL1);
  // Wire1.begin();
  // Wire1.setClock(clock);

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
      myWifi.enableImprov(true);
      myWifi.startAP();
      break;

    default:
      Log.notice(F("Main: Setting up pressure sensors." CR));
      PERF_BEGIN("main-sensor-read");
      myPressureSensor.setup(0, &Wire);
      // myPressureSensor[1].setup(1, &Wire1);
      PERF_END("main-sensor-read");

      if (!myPressureSensor.isActive() && !myPressureSensor1.isActive()) {
        Log.error(F("Main: No sensors are active, stopping." CR));
      }

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

      PERF_BEGIN("main-temp-setup");
      myTempSensor.setup(PIN_DS);
      PERF_END("main-temp-setup");
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
      ledOn(LedColor::GREEN);  // Green or fast flashing to indicate measurement
                               // mode
      break;
  }

  PERF_END("main-setup");
  Log.notice(F("Main: Setup completed." CR));
  pushMillis = millis();  // Dont include time for wifi connection
}

bool loopReadPressure() {
#if LOG_LEVEL == 6
  Log.verbose(F("Main: Entering main loopReadPressure." CR));
#endif

  // Process the sensor values and push data to targets.
  // ------------------------------------------------------------------------------------------------
  // If we dont get any readings we just skip this and try again the next
  // interval.
  //

  myPressureSensor.read();
  // myPressureSensor1.read();
  myTempSensor.readSensor(false);

  // float pressure, pressure1, temp, temp1;
  float pressurePsi, pressurePsi1, tempC;

  pressurePsi = myPressureSensor.getPressurePsi();
  // pressurePsi1 = myPressureSensor1.getPressurePsi();
  pressurePsi1 = NAN;

  tempC = myTempSensor.getTempC();
  // temp = myPressureSensor[0].getTemperatureC();
  // temp1 = myPressureSensor[1].getTemperatureC();
  // temp1 = NAN;

#if LOG_LEVEL == 6
  Log.verbose(F("Main: Sensor values pressure=%F PSI, "
                "temp=%FC." CR),
              pressurePsi, tempC);
  // Log.verbose(F("Main: Sensor values pressure=%F PSI, pressure1=%F PSI, "
  //               "temp=%FC, temp1=%FC." CR),
  //             pressure, pressure1, temp, temp1);
#endif

  if (isnan(pressurePsi) && isnan(pressurePsi1)) {
    Log.warning(F("Main: No valid pressure readings, skipping push." CR));
    return false;
  }

  bool pushExpired = (abs(static_cast<int32_t>((millis() - pushMillis))) >
                      (myConfig.getSleepInterval() * 1000));

  if (pushExpired || runMode == RunMode::measurementMode) {
    pushMillis = millis();
    PERF_BEGIN("loop-push");

#if defined(ENABLE_BLE)
    if (myConfig.isBleActive()) {
      myBleSender.init();

      switch (myConfig.getPressuremonBleFormat()) {
        case PressuremonBleFormat::BLE_PRESSUREMON_IBEACON: {
          myBleSender.sendCustomBeaconData(myBatteryVoltage.getVoltage(), tempC,
                                           pressurePsi, pressurePsi1);
        } break;

          // case PressuremonBleFormat::BLE_PRESSUREMON_EDDYSTONE: {
          //   myBleSender.sendEddystoneData(myBatteryVoltage.getVoltage(),
          //   tempC,
          //                             pressurePsi, pressurePsi1);
          // } break;
      }
    }
#endif  // ENABLE_BLE

    if (myWifi.isConnected()) {  // no need to try if there is no wifi
                                 // connection.
      if (myConfig.isWifiDirect() && runMode == RunMode::measurementMode) {
        Log.notice(
            F("Main: Sending data via Wifi Direct to Gravitymon Gateway." CR));

        TemplatingEngine engine;
        BrewingPush push(&myConfig);

        setupTemplateEnginePressure(
            &myConfig, engine, pressurePsi, pressurePsi1, tempC,
            (millis() - runtimeMillis) / 1000, myBatteryVoltage.getVoltage());
        String tpl = push.getTemplate(BrewingPush::PRESSURE_TEMPLATE_HTTP1,
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

        setupTemplateEnginePressure(
            &myConfig, engine, pressurePsi, pressurePsi1, tempC,
            (millis() - runtimeMillis) / 1000, myBatteryVoltage.getVoltage());
        push.sendAll(engine, BrewingPush::MeasurementType::PRESSURE);
      }
    }
    PERF_END("loop-push");

    // Send stats to influx after each push run.
    if (runMode == RunMode::configurationMode) {
      PERF_PUSH();
    }
  }
  return true;
}

void loopPressureOnInterval() {
  if (timerLoop.hasExpired()) {
    loopReadPressure();
    timerLoop.reset();

    myBatteryVoltage.read();
    if (runMode != RunMode::wifiSetupMode)
      checkSleepModePressure(myBatteryVoltage.getVoltage());
  }
}

void goToSleep(int sleepInterval) {
  float volt = myBatteryVoltage.getVoltage();
  float runtime = (millis() - runtimeMillis);

  digitalWrite(PIN_PWR, LOW);
  gpio_hold_en((gpio_num_t)PIN_PWR);
  gpio_deep_sleep_hold_en();

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

  ledOn(LedColor::OFF);
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

#if defined(FORCE_PRESSURE_MODE)
  Log.notice(F("MAIN: Forcing device into pressure mode for debugging" CR));
  runMode = RunMode::measurementMode;
  return;
#endif

  if (sleepModeAlwaysSkip) {
    // Check if the flag from the UI has been set, the we force configuration
    // mode.
#if LOG_LEVEL == 6
    Log.notice(F("MAIN: Sleep mode disabled from web interface." CR));
#endif
    runMode = RunMode::configurationMode;
  } else if (!myPressureSensor.isActive() && !myPressureSensor1.isActive()) {
    Log.notice(F("MAIN: No sensors active, will go into config mode." CR));
    runMode = RunMode::configurationMode;
  } else if (volt > myConfig.getVoltageConfig() || volt < 2.0) {
    Log.notice(F("MAIN: Voltage out of range, will go into config mode." CR));
    runMode = RunMode::configurationMode;
  } else {
    Log.notice(F("MAIN: Going into measurement mode." CR));
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
