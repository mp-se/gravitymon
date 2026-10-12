// Mock device for the CuckooTilt UI.
//
// The standard mock server (../../ui/mock-server) with the values of the
// CuckooTilt firmware seeded first, so the custom pages behave like they do
// against a real device. The standard mock itself is not changed.
//
//   cd custom/ui && npm run mock
import { configData, featureData, statusData } from '../../ui/mock-server/data.js'

// Settings that only exist in the CuckooTilt firmware (CuckooConfig), with the
// same defaults as the firmware
Object.assign(configData, {
  http_post_sleep_interval: 900,
  ble_sleep_interval: 120,
  use_http_post: true,
  use_ble: false,

  // Other CuckooConfig defaults
  ota_url: 'http://api.ckbrew.com/rest/public/',
  http_post_target: 'http://api.ckbrew.com/rest/public/deviceData',
  http_post_header1: 'Content-Type: application/json',
  http_post2_target: '',
  storage_sleep: true,
  battery_saving: false,
  voltage_config: 4.3,
  tempsensor_resolution: 11,
  gyro_type: 2,
  mdns: 'cuckootilt19a54c',
  locale: 'en'
})

// A CuckooTilt device in wifi mode (no sensors running)
Object.assign(statusData, { wifi_setup: true, gyro_family: 'ICM42670-p' })
Object.assign(featureData, {
  board: 'CUCKOO_C3',
  platform: 'esp32c3',
  hardware: 'cuckoo',
  firmware_file: 'firmware32c3.bin'
})

// Start the standard mock server (it reads the data above)
await import('../../ui/mock-server/index.js')
