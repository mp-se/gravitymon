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
#ifndef SRC_RESOURCES_HPP_
#define SRC_RESOURCES_HPP_

// Common strings used in json formats.
#define PARAM_ID "id"
#define PARAM_MDNS "mdns"
#define PARAM_CONFIG_VER "config-version"
#define PARAM_OTA "ota-url"
#define PARAM_SSID "wifi-ssid"
#define PARAM_PASS "wifi-pass"
#define PARAM_SSID2 "wifi-ssid2"
#define PARAM_PASS2 "wifi-pass2"
#define PARAM_RUNTIME_AVERAGE "runtime-average"
#define PARAM_TOKEN "token"
#define PARAM_TOKEN2 "token2"
#define PARAM_PUSH_HTTP "http-push"
#define PARAM_PUSH_HTTP_H1 "http-push-h1"
#define PARAM_PUSH_HTTP_H2 "http-push-h2"
#define PARAM_PUSH_HTTP2 "http-push2"
#define PARAM_PUSH_HTTP2_H1 "http-push2-h1"
#define PARAM_PUSH_HTTP2_H2 "http-push2-h2"
#define PARAM_PUSH_HTTP3 "http-push3"
#define PARAM_PUSH_INFLUXDB2 "influxdb2-push"
#define PARAM_PUSH_INFLUXDB2_ORG "influxdb2-org"
#define PARAM_PUSH_INFLUXDB2_BUCKET "influxdb2-bucket"
#define PARAM_PUSH_INFLUXDB2_AUTH "influxdb2-auth"
#define PARAM_PUSH_MQTT "mqtt-push"
#define PARAM_PUSH_MQTT_USER "mqtt-user"
#define PARAM_PUSH_MQTT_PASS "mqtt-pass"
#define PARAM_PUSH_MQTT_PORT "mqtt-port"
#define PARAM_SLEEP_INTERVAL "sleep-interval"
#define PARAM_TEMPFORMAT "temp-format"
#define PARAM_VOLTAGE_FACTOR "voltage-factor"
#define PARAM_VOLTAGE_CONFIG "voltage-config"
#define PARAM_GRAVITY_FORMULA "gravity-formula"
#define PARAM_GRAVITY_FORMAT "gravity-format"
#define PARAM_GRAVITY_TEMP_ADJ "gravity-temp-adjustment"
#define PARAM_TEMP_ADJ "temp-adjustment-value"
#define PARAM_GYRO_CALIBRATION "gyro-calibration-data"
#define PARAM_GYRO_TEMP "gyro-temp"
#define PARAM_STORAGE_SLEEP "storage-sleep"
#define PARAM_FORMULA_DATA "formula-calculation-data"
#define PARAM_FILES "files"
#define PARAM_FILE_NAME "file-name"
#define PARAM_FILE_SIZE "file-size"
#define PARAM_APP_VER "app-ver"
#define PARAM_APP_BUILD "app-build"
#define PARAM_ANGLE "angle"
#define PARAM_GRAVITY "gravity"
#define PARAM_TEMP_C "temp-c"
#define PARAM_TEMP_F "temp-f"
#define PARAM_BATTERY "battery"
#define PARAM_SLEEP_MODE "sleep-mode"
#define PARAM_RSSI "rssi"
#define PARAM_ERROR "error"
#define PARAM_PLATFORM "platform"
#define PARAM_BLE "ble"
#define PARAM_HW_GYRO_READ_COUNT "gyro-read-count"
// #define PARAM_HW_GYRO_READ_DELAY "gyro-read-delay"
#define PARAM_HW_GYRO_MOVING_THREASHOLD "gyro-moving-threashold"
#define PARAM_HW_FORMULA_DEVIATION "formula-max-deviation"
#define PARAM_HW_FORMULA_CALIBRATION_TEMP "formula-calibration-temp"
#define PARAM_HW_WIFI_PORTAL_TIMEOUT "wifi-portal-timeout"
#define PARAM_HW_WIFI_CONNECT_TIMEOUT "wifi-connect-timeout"
#define PARAM_HW_TEMPSENSOR_RESOLUTION "tempsensor-resolution"
#define PARAM_HW_PUSH_TIMEOUT "push-timeout"
#define PARAM_HW_PUSH_INTERVAL_HTTP1 "int-http1"
#define PARAM_HW_PUSH_INTERVAL_HTTP2 "int-http2"
#define PARAM_HW_PUSH_INTERVAL_HTTP3 "int-http3"
#define PARAM_HW_PUSH_INTERVAL_INFLUX "int-influx"
#define PARAM_HW_PUSH_INTERVAL_MQTT "int-mqtt"
#define PARAM_HW_IGNORE_LOW_ANGLES "ignore-low-angles"
#define PARAM_FORMAT_HTTP1 "http-1"
#define PARAM_FORMAT_HTTP2 "http-2"
#define PARAM_FORMAT_HTTP3 "http-3"
#define PARAM_FORMAT_INFLUXDB "influxdb"
#define PARAM_FORMAT_MQTT "mqtt"
#define PARAM_PUSH_FORMAT "format"
#define PARAM_PUSH_SUCCESS "success"
#define PARAM_PUSH_CODE "code"
#define PARAM_PUSH_ENABLED "enabled"

#endif  // SRC_RESOURCES_HPP_
