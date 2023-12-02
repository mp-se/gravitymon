/*
MIT License

Copyright (c) 2021-2023 Magnus

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the = "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED = "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */
#ifndef SRC_RESOURCES_HPP_
#define SRC_RESOURCES_HPP_

// Common strings used in json formats.
constexpr auto PARAM_ID = "id";
constexpr auto PARAM_MDNS = "mdns";
constexpr auto PARAM_CONFIG_VER = "config-version";
constexpr auto PARAM_OTA = "ota-url";
constexpr auto PARAM_SSID = "wifi-ssid";
constexpr auto PARAM_PASS = "wifi-pass";
constexpr auto PARAM_SSID2 = "wifi-ssid2";
constexpr auto PARAM_PASS2 = "wifi-pass2";
constexpr auto PARAM_RUNTIME_AVERAGE = "runtime-average";
constexpr auto PARAM_TOKEN = "token";
constexpr auto PARAM_TOKEN2 = "token2";
constexpr auto PARAM_PUSH_HTTP = "http-push";
constexpr auto PARAM_PUSH_HTTP_H1 = "http-push-h1";
constexpr auto PARAM_PUSH_HTTP_H2 = "http-push-h2";
constexpr auto PARAM_PUSH_HTTP2 = "http-push2";
constexpr auto PARAM_PUSH_HTTP2_H1 = "http-push2-h1";
constexpr auto PARAM_PUSH_HTTP2_H2 = "http-push2-h2";
constexpr auto PARAM_PUSH_HTTP3 = "http-push3";
constexpr auto PARAM_PUSH_INFLUXDB2 = "influxdb2-push";
constexpr auto PARAM_PUSH_INFLUXDB2_ORG = "influxdb2-org";
constexpr auto PARAM_PUSH_INFLUXDB2_BUCKET = "influxdb2-bucket";
constexpr auto PARAM_PUSH_INFLUXDB2_AUTH = "influxdb2-auth";
constexpr auto PARAM_PUSH_MQTT = "mqtt-push";
constexpr auto PARAM_PUSH_MQTT_USER = "mqtt-user";
constexpr auto PARAM_PUSH_MQTT_PASS = "mqtt-pass";
constexpr auto PARAM_PUSH_MQTT_PORT = "mqtt-port";
constexpr auto PARAM_SLEEP_INTERVAL = "sleep-interval";
constexpr auto PARAM_TEMPFORMAT = "temp-format";
constexpr auto PARAM_VOLTAGE_FACTOR = "voltage-factor";
constexpr auto PARAM_VOLTAGE_CONFIG = "voltage-config";
constexpr auto PARAM_GRAVITY_FORMULA = "gravity-formula";
constexpr auto PARAM_GRAVITY_FORMAT = "gravity-format";
constexpr auto PARAM_GRAVITY_TEMP_ADJ = "gravity-temp-adjustment";
constexpr auto PARAM_TEMP_ADJ = "temp-adjustment-value";
constexpr auto PARAM_GYRO_CALIBRATION = "gyro-calibration-data";
constexpr auto PARAM_GYRO_TEMP = "gyro-temp";
constexpr auto PARAM_STORAGE_SLEEP = "storage-sleep";
constexpr auto PARAM_GRAVITYMON_BLE = "gravitymon-ble";
constexpr auto PARAM_FORMULA_DATA = "formula-calculation-data";
constexpr auto PARAM_FILES = "files";
constexpr auto PARAM_FILE_NAME = "file-name";
constexpr auto PARAM_FILE_SIZE = "file-size";
constexpr auto PARAM_APP_VER = "app-ver";
constexpr auto PARAM_APP_BUILD = "app-build";
constexpr auto PARAM_ANGLE = "angle";
constexpr auto PARAM_GRAVITY = "gravity";
constexpr auto PARAM_TEMP_C = "temp-c";
constexpr auto PARAM_TEMP_F = "temp-f";
constexpr auto PARAM_BATTERY = "battery";
constexpr auto PARAM_SLEEP_MODE = "sleep-mode";
constexpr auto PARAM_RSSI = "rssi";
constexpr auto PARAM_ERROR = "error";
constexpr auto PARAM_PLATFORM = "platform";
constexpr auto PARAM_BLE = "ble";
constexpr auto PARAM_ISPINDEL_CONFIG = "ispindel-config";
constexpr auto PARAM_HW_GYRO_READ_COUNT = "gyro-read-count";
// constexpr auto PARAM_HW_GYRO_READ_DELAY = "gyro-read-delay";
constexpr auto PARAM_HW_GYRO_MOVING_THREASHOLD = "gyro-moving-threashold";
constexpr auto PARAM_HW_FORMULA_DEVIATION = "formula-max-deviation";
constexpr auto PARAM_HW_FORMULA_CALIBRATION_TEMP = "formula-calibration-temp";
constexpr auto PARAM_HW_WIFI_PORTAL_TIMEOUT = "wifi-portal-timeout";
constexpr auto PARAM_HW_WIFI_CONNECT_TIMEOUT = "wifi-connect-timeout";
constexpr auto PARAM_HW_TEMPSENSOR_RESOLUTION = "tempsensor-resolution";
constexpr auto PARAM_HW_PUSH_TIMEOUT = "push-timeout";
constexpr auto PARAM_HW_PUSH_INTERVAL_HTTP1 = "int-http1";
constexpr auto PARAM_HW_PUSH_INTERVAL_HTTP2 = "int-http2";
constexpr auto PARAM_HW_PUSH_INTERVAL_HTTP3 = "int-http3";
constexpr auto PARAM_HW_PUSH_INTERVAL_INFLUX = "int-influx";
constexpr auto PARAM_HW_PUSH_INTERVAL_MQTT = "int-mqtt";
constexpr auto PARAM_HW_IGNORE_LOW_ANGLES = "ignore-low-angles";
constexpr auto PARAM_HW_BATTERY_SAVING = "battery-saving";
constexpr auto PARAM_FORMAT_HTTP1 = "http-1";
constexpr auto PARAM_FORMAT_HTTP2 = "http-2";
constexpr auto PARAM_FORMAT_HTTP3 = "http-3";
constexpr auto PARAM_FORMAT_INFLUXDB = "influxdb";
constexpr auto PARAM_FORMAT_MQTT = "mqtt";
constexpr auto PARAM_PUSH_FORMAT = "format";
constexpr auto PARAM_PUSH_SUCCESS = "success";
constexpr auto PARAM_PUSH_CODE = "code";
constexpr auto PARAM_PUSH_ENABLED = "enabled";
constexpr auto PARAM_SELF = "self-check";
constexpr auto PARAM_SELF_GYRO_CONNECTED = "gyro-connected";
constexpr auto PARAM_SELF_GYRO_CALIBRATION = "gyro-calibration";
constexpr auto PARAM_SELF_TEMP_CONNECTED = "temp-connected";
constexpr auto PARAM_SELF_GRAVITY_FORMULA = "gravity-formula";
constexpr auto PARAM_SELF_BATTERY_LEVEL = "battery-level";
constexpr auto PARAM_SELF_PUSH_TARGET = "push-targets";
constexpr auto PARAM_STATUS = "status";

#endif  // SRC_RESOURCES_HPP_
