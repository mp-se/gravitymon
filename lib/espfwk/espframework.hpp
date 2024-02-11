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
#ifndef SRC_ESPFRAMEWORK_HPP_
#define SRC_ESPFRAMEWORK_HPP_

#include <Arduino.h>
#include <ArduinoJson.h>

#define ESPFWK_VER "1.0.0-alfa1"

void forcedReset();

#if defined(ESP8266)
#include <LittleFS.h>
#define ESP_RESET ESP.reset
#elif defined(ESP32C3)
#include <FS.h>
#include <LittleFS.h>

#include "esp32c3/rom/rtc.h"
#define ESP_RESET forcedReset
#elif defined(ESP32S2)
#include <FS.h>
#include <LittleFS.h>

#include "esp32s2/rom/rtc.h"
#define ESP_RESET forcedReset
#elif defined(ESP32S3)
#include <FS.h>
#include <LittleFS.h>

#include "esp32s3/rom/rtc.h"
#define ESP_RESET forcedReset
#elif defined(ESP32LITE)
#include <FS.h>
#include <LittleFS.h>

#include "esp32/rom/rtc.h"
#define ESP_RESET forcedReset
#else  // defined (ESP32)
#include <FS.h>
#include <LittleFS.h>

#include "esp32/rom/rtc.h"
#define ESP_RESET forcedReset
#endif

constexpr auto JSON_BUFFER_SIZE_S = 500;
constexpr auto JSON_BUFFER_SIZE_M = 1000;
constexpr auto JSON_BUFFER_SIZE_L = 3000;
constexpr auto JSON_BUFFER_SIZE_XL = 5000;

// Config
constexpr auto PARAM_ID = "id";
constexpr auto PARAM_MDNS = "mdns";
constexpr auto PARAM_SSID = "wifi_ssid";
constexpr auto PARAM_PASS = "wifi_pass";
constexpr auto PARAM_SSID2 = "wifi_ssid2";
constexpr auto PARAM_PASS2 = "wifi_pass2";
constexpr auto PARAM_OTA_URL = "ota_url";
constexpr auto PARAM_TEMP_FORMAT = "temp_format";
constexpr auto PARAM_HTTP_POST_TARGET = "http_post_target";
constexpr auto PARAM_HTTP_POST_HEADER1 = "http_post_header1";
constexpr auto PARAM_HTTP_POST_HEADER2 = "http_post_header2";
constexpr auto PARAM_HTTP_POST2_TARGET = "http_post2_target";
constexpr auto PARAM_HTTP_POST2_HEADER1 = "http_post2_header1";
constexpr auto PARAM_HTTP_POST2_HEADER2 = "http_post2_header2";
constexpr auto PARAM_HTTP_GET_TARGET = "http_get_target";
constexpr auto PARAM_HTTP_GET_HEADER1 = "http_get_header1";
constexpr auto PARAM_HTTP_GET_HEADER2 = "http_get_header2";
constexpr auto PARAM_INFLUXDB2_TARGET = "influxdb2_target";
constexpr auto PARAM_INFLUXDB2_ORG = "influxdb2_org";
constexpr auto PARAM_INFLUXDB2_BUCKET = "influxdb2_bucket";
constexpr auto PARAM_INFLUXDB2_TOKEN = "influxdb2_token";
constexpr auto PARAM_MQTT_TARGET = "mqtt_target";
constexpr auto PARAM_MQTT_PORT = "mqtt_port";
constexpr auto PARAM_MQTT_USER = "mqtt_user";
constexpr auto PARAM_MQTT_PASS = "mqtt_pass";
constexpr auto PARAM_WIFI_PORTAL_TIMEOUT = "wifi_portal_timeout";
constexpr auto PARAM_WIFI_CONNECT_TIMEOUT = "wifi_connect_timeout";
constexpr auto PARAM_PUSH_TIMEOUT = "push_timeout";
constexpr auto PARAM_DARK_MODE = "dark_mode";

// Webserver
constexpr auto PARAM_SUCCESS = "success";
constexpr auto PARAM_STATUS = "status";
constexpr auto PARAM_MESSAGE = "message";
constexpr auto PARAM_TOKEN = "token";
constexpr auto PARAM_COMMAND = "command";
constexpr auto PARAM_FILE = "file";
constexpr auto PARAM_FILES = "files";
constexpr auto PARAM_NETWORKS = "networks";
constexpr auto PARAM_RSSI = "rssi";
constexpr auto PARAM_CHANNEL = "channel";
constexpr auto PARAM_ENCRYPTION = "encryption";

#endif  // SRC_ESPFRAMEWORK_HPP_

// EOF
