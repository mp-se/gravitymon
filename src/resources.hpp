/*
MIT License

Copyright (c) 2021-2024 Magnus

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
constexpr auto PARAM_CONFIG_VER = "config_version";
constexpr auto PARAM_HARDWARE = "hardware";
constexpr auto PARAM_RUNTIME_AVERAGE = "runtime_average";
// constexpr auto PARAM_TOKEN = "token";
constexpr auto PARAM_TOKEN2 = "token2";
constexpr auto PARAM_SLEEP_INTERVAL = "sleep_interval";
constexpr auto PARAM_VOLTAGE_FACTOR = "voltage_factor";
constexpr auto PARAM_VOLTAGE_CONFIG = "voltage_config";
constexpr auto PARAM_GRAVITY_FORMULA = "gravity_formula";
constexpr auto PARAM_GRAVITY_FORMAT = "gravity_format";
constexpr auto PARAM_GRAVITY_TEMP_ADJ = "gravity_temp_adjustment";
constexpr auto PARAM_TEMP_ADJ = "temp_adjustment_value";
constexpr auto PARAM_GYRO_CALIBRATION = "gyro_calibration_data";
constexpr auto PARAM_GYRO_TEMP = "gyro_temp";
constexpr auto PARAM_STORAGE_SLEEP = "storage_sleep";
constexpr auto PARAM_FORMULA_DATA = "formula_calculation_data";
// constexpr auto PARAM_FILES = "files";
// constexpr auto PARAM_FILE_NAME = "file_name";
// constexpr auto PARAM_FILE_SIZE = "file_size";
constexpr auto PARAM_APP_VER = "app_ver";
constexpr auto PARAM_APP_BUILD = "app_build";
constexpr auto PARAM_ANGLE = "angle";
constexpr auto PARAM_GRAVITY = "gravity";
constexpr auto PARAM_TEMP = "temp";
constexpr auto PARAM_BATTERY = "battery";
constexpr auto PARAM_SLEEP_MODE = "sleep_mode";
// constexpr auto PARAM_RSSI = "rssi";
constexpr auto PARAM_PLATFORM = "platform";
constexpr auto PARAM_BLE_TILT_COLOR = "ble_tilt_color";
constexpr auto PARAM_BLE_FORMAT = "ble_format";
constexpr auto PARAM_ISPINDEL_CONFIG = "ispindel_config";
constexpr auto PARAM_GRAVITYMON1_CONFIG = "gravitymon1_config";
constexpr auto PARAM_GYRO_READ_COUNT = "gyro_read_count";
// constexpr auto PARAM_GYRO_READ_DELAY = "gyro_read_delay";
constexpr auto PARAM_GYRO_MOVING_THREASHOLD = "gyro_moving_threashold";
constexpr auto PARAM_FORMULA_DEVIATION = "formula_max_deviation";
constexpr auto PARAM_FORMULA_CALIBRATION_TEMP = "formula_calibration_temp";
constexpr auto PARAM_TEMPSENSOR_RESOLUTION = "tempsensor_resolution";
// constexpr auto PARAM_PUSH_INTERVAL_HTTP1 = "http_int";
// constexpr auto PARAM_PUSH_INTERVAL_HTTP2 = "http_int2";
// constexpr auto PARAM_PUSH_INTERVAL_HTTP3 = "http_int3";
constexpr auto PARAM_PUSH_INTERVAL_POST = "http_post_int";
constexpr auto PARAM_PUSH_INTERVAL_POST2 = "http_post2_int";
constexpr auto PARAM_PUSH_INTERVAL_GET = "http_get_int";
constexpr auto PARAM_PUSH_INTERVAL_INFLUX = "influxdb2_int";
constexpr auto PARAM_PUSH_INTERVAL_MQTT = "mqtt_int";
constexpr auto PARAM_IGNORE_LOW_ANGLES = "ignore_low_angles";
constexpr auto PARAM_BATTERY_SAVING = "battery_saving";
// constexpr auto PARAM_FORMAT_HTTP1 = "http_format";
// constexpr auto PARAM_FORMAT_HTTP2 = "http_format2";
// constexpr auto PARAM_FORMAT_HTTP3 = "http_format3";
constexpr auto PARAM_FORMAT_POST = "http_post_format";
constexpr auto PARAM_FORMAT_POST2 = "http_post2_format";
constexpr auto PARAM_FORMAT_GET = "http_get_format";
constexpr auto PARAM_FORMAT_INFLUXDB = "influxdb2_format";
constexpr auto PARAM_FORMAT_MQTT = "mqtt_format";
constexpr auto PARAM_PUSH_FORMAT = "push_format";
constexpr auto PARAM_PUSH_RETURN_CODE = "push_return_code";
constexpr auto PARAM_PUSH_ENABLED = "push_enabled";
constexpr auto PARAM_SELF = "self_check";
constexpr auto PARAM_SELF_GYRO_CONNECTED = "gyro_connected";
constexpr auto PARAM_SELF_GYRO_CALIBRATION = "gyro_calibration";
constexpr auto PARAM_SELF_TEMP_CONNECTED = "temp_connected";
constexpr auto PARAM_SELF_GRAVITY_FORMULA = "gravity_formula";
constexpr auto PARAM_SELF_BATTERY_LEVEL = "battery_level";
constexpr auto PARAM_SELF_PUSH_TARGET = "push_targets";
// constexpr auto PARAM_STATUS = "status";
// constexpr auto PARAM_SUCCESS = "success";
// constexpr auto PARAM_MESSAGE = "message";
constexpr auto PARAM_TOTAL_HEAP = "total_heap";
constexpr auto PARAM_FREE_HEAP = "free_heap";
constexpr auto PARAM_IP = "ip";
constexpr auto PARAM_SKIP_SSL_ON_TEST = "skip_ssl_on_test";
// constexpr auto PARAM_ENCRYPTION = "encryption";
// constexpr auto PARAM_CHANNEL = "channel";
// constexpr auto PARAM_NETWORKS = "networks";
constexpr auto PARAM_WIFI_SETUP = "wifi_setup";
// constexpr auto PARAM_FS_COMMAND = "command";
// constexpr auto PARAM_FS_FILE = "file";
// constexpr auto PARAM_FS_FILES = "files";

#endif  // SRC_RESOURCES_HPP_
