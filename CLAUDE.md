# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What This Project Is

GravityMon is an Arduino/PlatformIO firmware for an iSpindel-compatible gravity monitor used in beer brewing. It runs on ESP8266 and ESP32 (C3, S2, S3 variants) microcontrollers. The device floats in fermenting beer, uses a gyro (MPU6050 or ICM42670P) to measure tilt angle, converts that angle to specific gravity via a polynomial formula, and periodically pushes data to brewing endpoints (HTTP, MQTT, Bluetooth, etc.).

## Build System

Uses PlatformIO. All targets are defined in `platformio.ini`.

```bash
# Build for a specific target
pio run -e gravity-8266
pio run -e gravity-32c3_mini

# Upload to device
pio run -e gravity-32c3_mini -t upload

# Serial monitor (115200 baud)
pio device monitor

# Build unit test target (runs on device via AUnit)
pio run -e gravity-unit32
pio run -e gravity-unit32 -t upload
```

**Active targets:** `gravity-8266`, `gravity-32c3_mini`, `gravity-32c3_zero`, `gravity-32c3_supermini`, `gravity-32c3_cuckoo`, `gravity-32s2_mini`, `gravity-32s3_mini`, `gravity-olimex_esp32c3_devkit_lipo`

**Unit test target:** `gravity-unit32` — links `test/tests*.cpp` instead of `main*.cpp`, uses AUnit framework on-device.

## Running Tests

**On-device unit tests** (AUnit): Build and upload `gravity-unit32`, then read serial output.

**Python API tests** (requires a live device on network):
```bash
python -m venv .env
pip install -r test/scripts/requirements.txt
cd test/scripts
python3 -m unittest -v apitests.py          # all tests
python3 -m unittest -v apitests.API.test_status  # single test
```
Update the device IP in the test script before running.

## Architecture

### Device Operation Flow
`main_gravitymon.cpp` drives the main loop. The device wakes from deep sleep, reads the gyro angle and temperature, calculates gravity via the stored polynomial formula, then pushes data to configured endpoints before sleeping again. RTC memory (ESP32 only) persists gyro state across sleep cycles.

### Key Subsystems

| File(s) | Role |
|---------|------|
| `src/gyro.hpp/cpp`, `src/MPU6050_gyro.*`, `src/ICM42670P_gyro.*` | Gyro abstraction layer. `GyroSensor` owns a `GyroSensorInterface` impl detected at runtime. |
| `src/tempsensor.*` | DS18B20 one-wire temperature sensor. `GyroSensor` also implements `SecondayTempSensorInterface` for chip temperature. |
| `src/calc.*` | Converts gyro angle → gravity using a polynomial formula stored in config. |
| `src/config_gravitymon.*` | Main config class (`GravitymonConfig`). Persisted to LittleFS as JSON. |
| `src/config_brewing.*` | Brewing-specific config (targets, formula data). |
| `src/push_gravitymon.*`, `src/pushtarget.*`, `src/push_templates.*` | Push data to HTTP/MQTT/BLE endpoints using templated payloads. |
| `src/web_gravitymon.*`, `src/web_brewing.*` | Async web server endpoints (configuration UI, API). |
| `src/ble_gravitymon.*` | BLE advertising (ESP32 only, `ENABLE_BLE` flag). |
| `src/battery.*` | Battery voltage reading and percentage calculation. |
| `src/helper.*` | Shared utilities. |
| `lib/ds18b20_checker/` | Detects counterfeit DS18B20 sensors (clone classification). |
| `lib/tinyexpr/` | Expression evaluator used for gravity formula. |
| `lib/mpu6050/` | Local copy of MPU6050 I2C driver. |

### Configuration System
`GravitymonConfig` (extends a framework base class) holds all settings, serializes to/from JSON on LittleFS. Config is accessed through interface classes (`GyroConfigInterface`, `TempSensorConfigInterface`) so subsystems remain decoupled from the concrete config type.

### Platform Conditionals
- `ESP32C3`, `ESP32S2`, `ESP32S3` — chip-specific GPIO and feature flags
- `ENABLE_BLE` — BLE advertising support (most ESP32 targets)
- `ENABLE_RTCMEM` — persist gyro state across deep sleep via RTC memory (ESP32 only)
- `RUN_HARDWARE_TEST` — diagnostic GPIO validation mode (uncomment in `platformio.ini`)
- `SKIP_SLEEPMODE` / `FORCE_GRAVITY_MODE` — debug flags

### HTML UI
The web UI is a pre-built SPA. Compressed assets live in `html/` and are embedded into the firmware binary via `board_build.embed_txtfiles`. To update the UI, run `copy_ui.sh`.

### Build Scripts (`script/`)
Python scripts run as PlatformIO extra_scripts: `board.py` sets board-specific flags, `copy_firmware.py` copies built binaries to `bin/`, `create_versionjson.py` writes `bin/version.json`, `git_rev.py` injects the git revision as a build flag.

## Libraries
Key external dependencies (pinned versions in `platformio.ini`):
- `ArduinoJson v7` — JSON serialization
- `espframework` — Magnus's own framework (WiFi, web server base, OTA, file system helpers)
- `ESPAsyncWebServer` — async HTTP server
- `OneWireNg` + `Arduino-Temperature-Control-Library` — DS18B20 sensor
- `NimBLE-Arduino` — BLE (ESP32 targets with `ENABLE_BLE`)
- `AUnit` — unit test framework (unit test target only)

