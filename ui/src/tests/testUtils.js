import { createPinia, setActivePinia } from 'pinia'
import { vi } from 'vitest'

/**
 * Creates a fresh Pinia instance for testing
 * @returns {object} pinia instance
 */
export function createTestingPinia() {
  const pinia = createPinia()
  setActivePinia(pinia)
  return pinia
}

/**
 * Resets all mocks to initial state
 */
export function resetAllMocks() {
  vi.clearAllMocks()
}

/**
 * Mock config object for testing
 */
export const mockConfigState = {
  // Device
  id: 'test-id',
  mdns: 'gravitymon-test',
  temp_unit: 'C',
  gravity_unit: 'G',
  // Hardware
  ota_url: '',
  storage_sleep: false,
  voltage_factor: 0,
  voltage_config: 0,
  gyro_type: 1,
  gyro_temp: false,
  gyro_swap_xy: false,
  gyro_filter: false,
  battery_saving: false,
  tempsensor_resolution: 0,
  temp_adjustment_value: 0,
  charging_pin_enabled: false,
  // Wifi
  wifi_portal_timeout: 0,
  wifi_connect_timeout: 0,
  wifi_ssid: 'test-ssid',
  wifi_ssid2: '',
  wifi_pass: 'test-pass',
  wifi_pass2: '',
  wifi_direct_ssid: '',
  wifi_direct_pass: '',
  use_wifi_direct: false,
  wifi_scan_ap: false,
  // Push - Generic
  token: 'token123',
  token2: '',
  sleep_interval: 60,
  push_timeout: 5000,
  skip_ssl_on_test: false,
  // Push - Http Post 1
  http_post_target: '',
  http_post_header1: '',
  http_post_header2: '',
  http_post_tcp: false,
  http_post_int: 0,
  http_post_format_gravity: '',
  // Push - Http Post 2
  http_post2_target: '',
  http_post2_header1: '',
  http_post2_header2: '',
  http_post2_int: 0,
  http_post2_format_gravity: '',
  // Push - Http Get
  http_get_target: '',
  http_get_header1: '',
  http_get_header2: '',
  http_get_int: 0,
  http_get_format_gravity: '',
  // Push - Influx
  influxdb2_target: '',
  influxdb2_org: '',
  influxdb2_bucket: '',
  influxdb2_token: '',
  influxdb2_int: 0,
  influxdb2_format_gravity: '',
  // Push - MQTT
  mqtt_target: '',
  mqtt_port: '',
  mqtt_user: '',
  mqtt_pass: '',
  mqtt_int: 0,
  mqtt_format_gravity: '',
  // Push BLE
  ble_tilt_color: '',
  ble_format: 0,
  // Gravity formula
  gravity_formula: '1.05',
  gravity_temp_adjustment: false,
  formula_calculation_data: [],
  gyro_read_count: 0,
  gyro_moving_threashold: 0,
  formula_max_deviation: 0.1,
  formula_calibration_temp: 20,
  ignore_low_angles: false,
  gyro_calibration_data: { ax: 0, ay: 0, az: 0, gx: 0, gy: 0, gz: 0 },
  dark_mode: false
}

/**
 * Mock status object for testing
 */
export const mockStatusState = {
  id: 'test-id',
  angle: 25.5,
  gravity: 1.05,
  gravity_unit: 'G',
  temp: 20,
  temp_unit: 'C',
  sleep_interval: 60,
  battery: 3.7,
  sleep_mode: false,
  rssi: -50,
  mdns: 'gravitymon-test',
  wifi_ssid: 'test-ssid',
  ip: '192.168.1.100',
  total_heap: 64,
  free_heap: 32,
  ispindel_config: false,
  self_check: {
    gyro_connected: true,
    gyro_moving: true,
    gyro_calibration: true,
    temp_connected: true,
    gravity_formula: true,
    battery_level: true,
    push_targets: true
  },
  ble_supported: true,
  gyro_family: 'MPU6050',
  wifi_setup: true,
  connected: true
}

/**
 * Mock global app state
 */
export const mockGlobalState = {
  platform: 'ESP32',
  chip_id: 'ABC12345',
  board: 'ESP32-DevKit',
  app_ver: '3.0.0',
  app_build: '42',
  hardware: 'GravityMon v4',
  firmware_file: '',
  registered: true,
  ui: {
    enableVoltageFragment: true,
    enableManualWifiEntry: true,
    enableScanForStrongestAp: true,
    enableDeviceRegistration: true,
    enableCalibrationRegistration: true
  },
  feature: {
    ble: true,
    velocity: true,
    filter: true,
    charging: true
  },
  initialized: true,
  disabled: false,
  configChanged: false,
  messageError: '',
  messageWarning: '',
  messageSuccess: '',
  messageInfo: ''
}
