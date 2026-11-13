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
#if defined(GRAVITYMON) && defined(RUN_HARDWARE_TEST)

#include <LittleFS.h>

#include <battery.hpp>
#include <cmath>
#include <config_gravitymon.hpp>
#include <gyro.hpp>
#include <hardware_scan.hpp>
#include <hwtest.hpp>
#include <led.hpp>
#include <log.hpp>
#include <main_gravitymon.hpp>
#include <tempsensor.hpp>
#include <vector>

extern GravitymonConfig myConfig;  // Defined in main_gravitymon.cpp

namespace {

enum class Level { PASS, FAIL, WARN, INFO };

struct Check {
  Level level;
  String name;
  String text;
};

std::vector<Check> checks;

const char* levelName(Level l) {
  switch (l) {
    case Level::PASS:
      return "PASS";
    case Level::FAIL:
      return "FAIL";
    case Level::WARN:
      return "WARN";
    default:
      return "INFO";
  }
}

void addCheck(Level level, const char* name, const char* fmt, ...) {
  char buf[128];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);
  checks.push_back({level, name, buf});
}

#if defined(ESP8266)
constexpr const char* EXPECTED_FAMILY = "ESP8266";
#elif defined(ESP32C3)
constexpr const char* EXPECTED_FAMILY = "ESP32C3";
#elif defined(ESP32S2)
constexpr const char* EXPECTED_FAMILY = "ESP32S2";
#elif defined(ESP32S3)
constexpr const char* EXPECTED_FAMILY = "ESP32S3";
#else
constexpr const char* EXPECTED_FAMILY = "ESP32";
#endif

// Accepted range for the supply voltage measured when powered over USB-C
// (nominal 5 V, the ADC and the voltage divider are not precise).
constexpr float SUPPLY_MIN_V = 4.5f;
constexpr float SUPPLY_MAX_V = 5.5f;

#if defined(HWTEST_CLASSIFY_SENSOR)
constexpr bool CLASSIFY_SENSOR = true;
#else
constexpr bool CLASSIFY_SENSOR = false;
#endif

#if defined(BOARD)
constexpr const char* BOARD_NAME = BOARD;
#else
constexpr const char* BOARD_NAME = "UNDEFINED";
#endif

String deviceId() {
#if defined(ESP8266)
  String id = String(ESP.getChipId(), HEX);
#else
  String id = String(ESP.getEfuseMac(), HEX);
#endif
  id.toUpperCase();
  return id;
}

void checkGyro() {
  GyroType type = myGyro.detectGyro();

  if (type == GyroType::GYRO_NONE) {
    addCheck(Level::FAIL, "Gyro/accelerometer", "No gyro found on I2C (SDA=%d, SCL=%d)",
             PIN_SDA, PIN_SCL);
    return;
  }

  myConfig.setGyroType(type);  // Not saved, hardware test keeps no settings

  // A read is only valid if the sensor is not moving, that is based on gyro
  // offsets from the calibration. The hardware test has no calibration and the
  // board can be held in a hand, so disable the movement check. We only verify
  // that the sensor delivers values.
  myConfig.setGyroSensorMovingThreashold(INT16_MAX);

  if (!myGyro.setup(GyroMode::GYRO_RUN, false) || !myGyro.read() ||
      !myGyro.hasValue()) {
    addCheck(Level::FAIL, "Gyro/accelerometer", "%s detected but could not be read",
             myGyro.getGyroFamily());
    return;
  }

  addCheck(Level::PASS, "Gyro/accelerometer", "%s, angle %.2f, chip temp %.1f C",
           myGyro.getGyroFamily(), myGyro.getAngle(), myGyro.getSensorTempC());
}

void checkTempSensor() {
  myTempSensor.setup(PIN_DS, PIN_DS2);
  myTempSensor.readSensor(false);

  if (myTempSensor.getSensorCount() < 1) {
    addCheck(Level::FAIL, "Temperature sensor", "No DS18B20 found on pin %d", PIN_DS);
    return;
  }

  float t = myTempSensor.getTempC();

  // 85 C is the power-on value and -127 C a read error of the DS18B20
  if (isnan(t) || t <= -126.0f || fabsf(t - 85.0f) < 0.01f) {
    addCheck(Level::FAIL, "Temperature sensor", "DS18B20 found but invalid reading %.2f C",
             t);
    return;
  }

  addCheck(Level::PASS, "Temperature sensor", "DS18B20 found, %.2f C", t);
}

void checkSensorGenuine(JsonObject &scan) {
  int deviations = scan["ds18b20_counterfeit"] | -1;

  if (deviations > 0) {
    addCheck(Level::WARN, "Sensor authenticity",
             "%d deviation(s) from a genuine DS18B20, probably a clone",
             deviations);
  } else if (deviations == 0) {
    addCheck(Level::INFO, "Sensor authenticity", "No deviations from a genuine DS18B20");
  }

  if (CLASSIFY_SENSOR) {
    addCheck(Level::INFO, "Sensor class", "%s",
             scan["ds18b20_type"] | "unknown");
  }
}

void checkChip(JsonObject &scan) {
  const char *family = scan["chip"]["family"] | "unknown";
  const char *rev = "";
  char revBuf[16] = "";

  if (!scan["chip"]["revision"].isNull()) {
    snprintf(revBuf, sizeof(revBuf), " rev %d",
             scan["chip"]["revision"].as<int>());
    rev = revBuf;
  }

  if (strcmp(family, EXPECTED_FAMILY) == 0) {
    addCheck(Level::PASS, "Processor", "%s%s, flash %u KB", family, rev,
             static_cast<unsigned>(ESP.getFlashChipSize() / 1024));
  } else {
    addCheck(Level::FAIL, "Processor", "Found %s but firmware is built for %s",
             family, EXPECTED_FAMILY);
  }
}

void checkI2c(JsonObject &scan) {
  JsonArray i2c = scan[PARAM_I2C];
  String list;

  for (JsonObject d : i2c) {
    if (list.length()) list += " ";
    list += d[PARAM_ADRESS].as<const char *>();
  }

  addCheck(Level::INFO, "I2C devices", "%d device(s): %s",
           static_cast<int>(i2c.size()), list.length() ? list.c_str() : "none");
}

void checkBattery() {
  myBatteryVoltage.read();

  // The test is run over USB with no battery connected (it would be charged),
  // so this is the USB supply voltage. The estimate is based on the voltage
  // factor of the target, the raw value is reported for judging the hardware.
  const float volt = myBatteryVoltage.getVoltage();
  const bool ok = volt >= SUPPLY_MIN_V && volt <= SUPPLY_MAX_V;

  addCheck(ok ? Level::PASS : Level::FAIL, "Supply voltage",
           "%.2f V (raw ADC %d, factor %.2f), expected %.1f-%.1f V on USB",
           volt, myBatteryVoltage.getRawValue(), myConfig.getVoltageFactor(),
           SUPPLY_MIN_V, SUPPLY_MAX_V);
}

void checkFilesystem() {
  const char *file = "/hwtest.tmp";
  const char *data = "gravitymon-hwtest";

  myConfig.checkFileSystem();

  File w = LittleFS.open(file, "w");
  if (!w) {
    addCheck(Level::FAIL, "Flash file system", "Unable to create test file");
    return;
  }
  w.print(data);
  w.close();

  File r = LittleFS.open(file, "r");
  if (!r) {
    addCheck(Level::FAIL, "Flash file system", "Unable to open test file");
    return;
  }
  String content = r.readString();
  r.close();
  LittleFS.remove(file);

  if (content != data) {
    addCheck(Level::FAIL, "Flash file system", "Test file content mismatch");
    return;
  }

  addCheck(Level::PASS, "Flash file system", "Write/read/delete ok");
}

void cycleLed() {
  ledOn(LedColor::RED);
  delay(300);
  ledOn(LedColor::GREEN);
  delay(300);
  ledOn(LedColor::BLUE);
  delay(300);
  ledOn(LedColor::WHITE);
  delay(300);

  addCheck(Level::INFO, "LED", "Cycled red/green/blue/white (check visually)");
}

String buildReport(bool passed, int passCount, int total, int warnings) {
  const char *line =
      "======================================================================"
      "\r\n";
  String r;

  r += "\r\n";
  r += line;
  r += "GRAVITYMON HARDWARE TEST\r\n";
  r += line;
  r += String("Firmware : ") + CFG_APPNAME + " " + CFG_APPVER + " (" +
       CFG_GITREV + ")\r\n";
  r += String("Board    : ") + BOARD_NAME + "\r\n";
  r += String("Device ID: ") + deviceId() + "\r\n";
  r += "----------------------------------------------------------------------"
       "\r\n";

  char buf[200];
  snprintf(buf, sizeof(buf), "%-23s %-7s %s\r\n", "TEST", "RESULT", "DETAILS");
  r += buf;
  r += "----------------------------------------------------------------------"
       "\r\n";

  String failed;
  int info = 0;

  for (const auto &c : checks) {
    snprintf(buf, sizeof(buf), "%-23s %-7s %s\r\n", c.name.c_str(),
             levelName(c.level), c.text.c_str());
    r += buf;

    if (c.level == Level::FAIL) {
      if (failed.length()) failed += ", ";
      failed += c.name;
    } else if (c.level == Level::INFO) {
      info++;
    }
  }

  r += "----------------------------------------------------------------------"
       "\r\n";
  r += "SUMMARY\r\n";
  snprintf(buf, sizeof(buf), "  Tests passed   : %d of %d\r\n", passCount,
           total);
  r += buf;
  snprintf(buf, sizeof(buf), "  Tests failed   : %d\r\n", total - passCount);
  r += buf;
  if (failed.length()) {
    r += "  Failed tests   : " + failed + "\r\n";
  }
  snprintf(buf, sizeof(buf), "  Warnings       : %d (does not affect result)\r\n",
           warnings);
  r += buf;
  snprintf(buf, sizeof(buf),
           "  Information    : %d (values for the record, not pass/fail)\r\n",
           info);
  r += buf;
  r += "\r\n";
  r += passed ? "  OVERALL RESULT : PASS\r\n" : "  OVERALL RESULT : FAIL\r\n";

  r += line;
  return r;
}

// The USB serial port can accept fewer bytes than requested when the host does
// not read fast enough, and print() then silently drops the rest. Write again
// until everything has been accepted.
void writeAll(const char *data, size_t len) {
  int retries = 0;

  while (len > 0 && retries < 500) {
    size_t n = EspSerial.write(reinterpret_cast<const uint8_t *>(data), len);
    data += n;
    len -= n;

    if (n == 0) {
      retries++;
      delay(2);
    }
  }
}

// Write the report in small pieces to give the host time to read it.
void printReport(const String &report) {
  constexpr int CHUNK = 32;
  const int len = report.length();

  for (int start = 0; start < len; start += CHUNK) {
    writeAll(report.c_str() + start, min(CHUNK, len - start));
    EspSerial.flush();
    delay(8);
  }
}

}  // namespace

void runHardwareTest() {
  EspSerial.println();
  EspSerial.println("Hardware test started, running checks...");

  Log.setLevel(0);  // Silent, log lines would be mixed into the report

  checks.clear();

  checkGyro();
  checkTempSensor();

  // The scan result is only used internally to evaluate the checks
  JsonDocument doc;
  JsonObject scan = doc.to<JsonObject>();
  scanHardwareCommon(scan);
  scanHardwareGravitymon(scan, CLASSIFY_SENSOR);

  checkChip(scan);
  checkI2c(scan);
  checkSensorGenuine(scan);
  checkBattery();
  checkFilesystem();
  cycleLed();

  int passCount = 0, total = 0, warnings = 0;
  bool passed = true;

  for (const auto &c : checks) {
    if (c.level == Level::PASS) {
      passCount++;
      total++;
    } else if (c.level == Level::FAIL) {
      passed = false;
      total++;
    } else if (c.level == Level::WARN) {
      warnings++;
    }
  }

  String report = buildReport(passed, passCount, total, warnings);

  ledOn(passed ? LedColor::GREEN : LedColor::RED);

  // Repeat the report so it is visible whenever a serial console is opened
  while (true) {
    printReport(report);
    for (int i = 0; i < 10; i++) delay(1000);
  }
}

#endif  // GRAVITYMON && RUN_HARDWARE_TEST

// EOF
