/*
 * Project specific data objects, should contain configData and statusData as minimum
 *
 * (c) 2023-2024 Magnus Persson
 */

export var configData = {
  // Device configuration
  id: '7376ef',
  mdns: 'gravmon3',
  temp_unit: 'C',
  // Hardware
  ota_url: 'https://www.gravitymon.com/firmware/',
  storage_sleep: true,
  voltage_factor: 1.59,
  voltage_config: 4.15,
  gyro_temp: false,
  gyro_swap_xy: false,
  gyro_filter: false,
  battery_saving: true,
  tempsensor_resolution: 9,
  temp_adjustment_value: 0,
  charging_pin_enabled: false,
  // Wifi
  wifi_scan_ap: true,
  wifi_portal_timeout: 120,
  wifi_connect_timeout: 20,
  wifi_ssid: 'network A',
  wifi_ssid2: '',
  wifi_pass: 'password',
  wifi_pass2: 'mypass',
  wifi_direct_ssid: '',
  wifi_direct_pass: '',
  use_wifi_direct: false,
  // Push - Generic
  token: 'mytoken1',
  token2: 'mytoken2',
  sleep_interval: 30,
  push_timeout: 10,
  skip_ssl_on_test: false,
  // Push - Http Post 1
  http_post_target: 'http://post.home.arpa:9090/api/v1/ZYfjlUNeiuyu9N/telemetry',
  http_post_header1: 'Auth: Basic T7IF9DD9fF3RDddE=',
  http_post_header2: '',
  http_post_int: 1,
  http_post_tcp: false,
  // Push - Http Post 2
  http_post2_target: 'http://post2.home.arpa/ispindel',
  http_post2_header1: '',
  http_post2_header2: '',
  http_post2_int: 1,
  // Push - Http Get
  http_get_target: 'http://get.home.arpa/ispindel',
  http_get_header1: '',
  http_get_header2: '',
  http_get_int: 1,
  // Push - Influx
  influxdb2_target: 'http://influx.home.arpa:8086',
  influxdb2_org: 'myorg',
  influxdb2_bucket: 'mybucket',
  influxdb2_token: 'OijkU((jhfkh=',
  influxdb2_int: 1,
  // Push - MQTT
  mqtt_target: 'mqtt.home.arpa',
  mqtt_port: 1883,
  mqtt_user: 'user',
  mqtt_pass: 'pass',
  mqtt_int: 1,
  // mqtt_retain: false,
  // Push BLE
  ble_tilt_color: 'pink',
  ble_format: 1,
  // Gravity - Generic
  gyro_type: 1,
  gravity_formula: '-0.0000047197506*tilt^2+0.0019926321*tilt+0.96079491',
  gravity_temp_adjustment: false,
  gyro_read_count: 50,
  gyro_moving_threashold: 500,
  formula_max_deviation: 0.02,
  formula_calibration_temp: 20,
  ignore_low_angles: false,
/*
  gravity_unit: 'P',
  formula_calculation_data: [
    { a: 25.6, g: 2.0515 },
    { a: 27.4, g: 2.8147 },
    { a: 31.75, g: 5.0809 },
    { a: 37.08, g: 7.5582 },
    { a: 44.9, g: 10.2348 },
    { a: 51.45, g: 12.6248 },
    { a: 59.76, g: 14.9746 },
    { a: 65.24, g: 16.8259 },
    { a: 69.62, g: 19.1052 },
    { a: 0, g: 1 },
    { a: 0, g: 1 },
    { a: 0, g: 1 },
    { a: 0, g: 1 },
    { a: 0, g: 1 },
    { a: 0, g: 1 },
    { a: 0, g: 1 },
    { a: 0, g: 1 },
    { a: 0, g: 1 },
    { a: 0, g: 1 },
  ],
*/
gravity_unit: 'G',
  formula_calculation_data: [
    { a: 25.6, g: 1.008 },
    { a: 27.4, g: 1.011 },
    { a: 31.75, g: 1.02 },
    { a: 37.08, g: 1.03 },
    { a: 44.9, g: 1.041 },
    { a: 51.45, g: 1.051 },
    { a: 59.76, g: 1.061 },
    { a: 65.24, g: 1.069 },
    { a: 69.62, g: 1.079 },
    { a: 0, g: 1 },
    { a: 0, g: 1 },
    { a: 0, g: 1 },
    { a: 0, g: 1 },
    { a: 0, g: 1 },
    { a: 0, g: 1 },
    { a: 0, g: 1 },
    { a: 0, g: 1 },
    { a: 0, g: 1 },
    { a: 0, g: 1 },
  ],
/* 
  gravity_unit: 'G',
  formula_calculation_data: [
    { a: 25, g: 1.000 },
    { a: 30, g: 1.010 },
    { a: 35, g: 1.020 },
    { a: 45, g: 1.040 },
    { a: 50, g: 1.050 },
    { a: 40, g: 1.030 },
    { a: 55, g: 1.060 },
    { a: 60, g: 1.070 },
    { a: 65, g: 1.080 },
    { a: 70, g: 1.090 },
    { a: 0, g: 1 },
    { a: 0, g: 1 },
    { a: 0, g: 1 },
    { a: 0, g: 1 },
    { a: 0, g: 1 },
    { a: 0, g: 1 },
    { a: 0, g: 1 },
    { a: 0, g: 1 },
    { a: 0, g: 1 },
    { a: 0, g: 1 },
  ],
*/
  gyro_calibration_data: {
    ax: 0,
    ay: 0,
    az: 0,
    gx: 0,
    gy: 0,
    gz: 0
  },
  dark_mode: false,
}

export var formatData = {
  http_post_format_gravity:
    '%7B%22name%22%20%3A%20%22gravmon%22%2C%20%22ID%22%3A%20%22%24%7Bid%7D%22%2C%20%22token%22%20%3A%20%22gravmon%22%2C%20%22interval%22%3A%20%24%7Bsleep%2Dinterval%7D%2C%20%22temperature%22%3A%20%24%7Btemp%7D%2C%20%22temp%2Dunits%22%3A%20%22%24%7Btemp%2Dunit%7D%22%2C%20%22gravity%22%3A%20%24%7Bgravity%7D%2C%20%22angle%22%3A%20%24%7Bangle%7D%2C%20%22battery%22%3A%20%24%7Bbattery%7D%2C%20%22rssi%22%3A%20%24%7Brssi%7D%2C%20%22corr%2Dgravity%22%3A%20%24%7Bcorr%2Dgravity%7D%2C%20%22gravity%2Dunit%22%3A%20%22%24%7Bgravity%2Dunit%7D%22%2C%20%22run%2Dtime%22%3A%20%24%7Brun%2Dtime%7D%20%7D',
  http_post2_format_gravity:
    '%7B%22name%22%20%3A%20%22gravmon%22%2C%20%22ID%22%3A%20%22%24%7Bid%7D%22%2C%20%22token%22%20%3A%20%22gravmon%22%2C%20%22interval%22%3A%20%24%7Bsleep%2Dinterval%7D%2C%20%22temperature%22%3A%20%24%7Btemp%7D%2C%20%22temp%2Dunits%22%3A%20%22%24%7Btemp%2Dunit%7D%22%2C%20%22gravity%22%3A%20%24%7Bgravity%7D%2C%20%22angle%22%3A%20%24%7Bangle%7D%2C%20%22battery%22%3A%20%24%7Bbattery%7D%2C%20%22rssi%22%3A%20%24%7Brssi%7D%2C%20%22corr%2Dgravity%22%3A%20%24%7Bcorr%2Dgravity%7D%2C%20%22gravity%2Dunit%22%3A%20%22%24%7Bgravity%2Dunit%7D%22%2C%20%22run%2Dtime%22%3A%20%24%7Brun%2Dtime%7D%20%7D',
  http_get_format_gravity:
    '%7B%22name%22%20%3A%20%22gravmon%22%2C%20%22ID%22%3A%20%22%24%7Bid%7D%22%2C%20%22token%22%20%3A%20%22gravmon%22%2C%20%22interval%22%3A%20%24%7Bsleep%2Dinterval%7D%2C%20%22temperature%22%3A%20%24%7Btemp%7D%2C%20%22temp%2Dunits%22%3A%20%22%24%7Btemp%2Dunit%7D%22%2C%20%22gravity%22%3A%20%24%7Bgravity%7D%2C%20%22angle%22%3A%20%24%7Bangle%7D%2C%20%22battery%22%3A%20%24%7Bbattery%7D%2C%20%22rssi%22%3A%20%24%7Brssi%7D%2C%20%22corr%2Dgravity%22%3A%20%24%7Bcorr%2Dgravity%7D%2C%20%22gravity%2Dunit%22%3A%20%22%24%7Bgravity%2Dunit%7D%22%2C%20%22run%2Dtime%22%3A%20%24%7Brun%2Dtime%7D%20%7D',
  influxdb2_format_gravity:
    'measurement%2Chost%3D%24%7Bmdns%7D%2Cdevice%3D%24%7Bid%7D%2Ctemp%2Dformat%3D%24%7Btemp%2Dunit%7D%2Cgravity%2Dformat%3D%24%7Bgravity%2Dunit%7D%20gravity%3D%24%7Bgravity%7D%2Ccorr%2Dgravity%3D%24%7Bcorr%2Dgravity%7D%2Cangle%3D%24%7Bangle%7D%2Ctemp%3D%24%7Btemp%7D%2Cbattery%3D%24%7Bbattery%7D%2Crssi%3D%24%7Brssi%7D%0A',
  mqtt_format_gravity:
    'ispindel%2F%24%7Bmdns%7D%2Ftilt%3A%24%7Bangle%7D%7Cispindel%2F%24%7Bmdns%7D%2Ftemperature%3A%24%7Btemp%7D%7Cispindel%2F%24%7Bmdns%7D%2Ftemp%5Funits%3A%24%7Btemp%2Dunit%7D%7Cispindel%2F%24%7Bmdns%7D%2Fbattery%3A%24%7Bbattery%7D%7Cispindel%2F%24%7Bmdns%7D%2Fgravity%3A%24%7Bgravity%7D%7Cispindel%2F%24%7Bmdns%7D%2Finterval%3A%24%7Bsleep%2Dinterval%7D%7Cispindel%2F%24%7Bmdns%7D%2FRSSI%3A%24%7Brssi%7D%7C'
}

export var statusData = {
  id: '7376ef',
  angle: 22.4,
  gravity: 1.044,
  gravity_unit: 'G',
  temp: 12,
  sleep_interval: 300,
  battery: 3.81,
  temp_unit: 'C',
  sleep_mode: false,
  rssi: -56,
  mdns: 'gravmon',
  // platform: 'esp8266',
  hardware: 'ispindel',
  wifi_ssid: 'wifi',
  ispindel_config: false,
  total_heap: 1000,
  free_heap: 500,
  ip: '192.0.0.1',
  self_check: {
    gyro_connected: true,
    gyro_moving: false,
    gyro_calibration: true,
    temp_connected: true,
    gravity_formula: true,
    battery_level: true,
    push_targets: true
  },
  ble_supported: true,
  gyro_family: 'MPU6050',
  // gyro_family: 'MPU6500',
  // gyro_family: 'ICM42670-p',
  wifi_setup: false
}

export var featureData = {
  chip_id: "68e3e17554dc",
  // chip_id: "registered", // Used for testing the already registered scenario
  // chip_id: "test_401", // Used for testing wrong api key scenario
  // chip_id: "test_429", // Used for testing too many requests scenario
  // chip_id: "test_500", // Used for testing internal server error scenario
  board: 'D1_MINI',
  platform: 'esp32c3',
  app_ver: '2.0.0',
  app_build: 'gitrev',
  hardware: 'ispindel',
  firmware_file: 'firmware.bin',
  registered: true,

  // Feature flags
  ble: true,
  filter: true,
  velocity: true,
  charging: true, 
}

// EOF
