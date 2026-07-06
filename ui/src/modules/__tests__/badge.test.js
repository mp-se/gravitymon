import { describe, it, expect, beforeEach, vi } from 'vitest'
import { createPinia, setActivePinia } from 'pinia'
import { config, status } from '@/modules/pinia'
import * as badge from '../badge'
import * as utils from '../utils'

// Mock the pinia module to control config and status
vi.mock('@/modules/pinia', () => ({
  config: {},
  status: {}
}))

// Mock utils
vi.mock('../utils', () => ({
  isGyroCalibrated: vi.fn()
}))

describe('badge.js', () => {
  beforeEach(() => {
    setActivePinia(createPinia())
    // Reset mocks
    config.mdns = 'test'
    config.wifi_ssid = 'ssid'
    config.wifi_ssid2 = ''
    config.gravity_formula = 'formula'
    config.http_post_target = ''
    config.http_post2_target = ''
    config.http_get_target = ''
    config.influxdb2_target = ''
    config.mqtt_target = ''
    config.ble_format = 0
    config.use_wifi_direct = false

    status.ispindel_config = false
    status.needsCalibration = false
  })

  describe('deviceMdnsBadge', () => {
    it('returns 1 when mdns is empty', () => {
      config.mdns = ''
      expect(badge.deviceMdnsBadge()).toBe(1)
    })
    it('returns 0 when mdns is set', () => {
      config.mdns = 'something'
      expect(badge.deviceMdnsBadge()).toBe(0)
    })
  })

  describe('deviceMigrateIspindelBadge', () => {
    it('returns 1 when ispindel_config is true', () => {
      status.ispindel_config = true
      expect(badge.deviceMigrateIspindelBadge()).toBe(1)
    })
  })

  describe('deviceGyroCalibratedBadge', () => {
    it('returns 0 when calibration not needed', () => {
      status.needsCalibration = false
      expect(badge.deviceGyroCalibratedBadge()).toBe(0)
    })
    it('returns 1 when calibration needed and not calibrated', () => {
      status.needsCalibration = true
      utils.isGyroCalibrated.mockReturnValue(false)
      expect(badge.deviceGyroCalibratedBadge()).toBe(1)
    })
    it('returns 0 when calibration needed and already calibrated', () => {
      status.needsCalibration = true
      utils.isGyroCalibrated.mockReturnValue(true)
      expect(badge.deviceGyroCalibratedBadge()).toBe(0)
    })
  })

  describe('deviceWifiBadge', () => {
    it('returns 1 when primary ssid is empty', () => {
      config.wifi_ssid = ''
      expect(badge.deviceWifiBadge()).toBe(1)
    })
    it('returns 0 when ssid is set', () => {
      config.wifi_ssid = 'ok'
      expect(badge.deviceWifiBadge()).toBe(0)
    })
  })

  describe('gravityFormulaBadge', () => {
    it('returns 1 when formula is empty', () => {
      config.gravity_formula = ''
      expect(badge.gravityFormulaBadge()).toBe(1)
    })
  })

  describe('push badges', () => {
    it('returns 1 for pushHttpPost1Badge when no targets are configured', () => {
      expect(badge.pushHttpPost1Badge()).toBe(1)
    })
    it('returns 0 for pushHttpPost1Badge when at least one target is configured', () => {
      config.mqtt_target = 'broker.local'
      expect(badge.pushHttpPost1Badge()).toBe(0)
    })

    it('pushWifiDirectBadge returns 1 when no targets configured', () => {
      expect(badge.pushWifiDirectBadge()).toBe(1)
    })

    it('pushWifiDirectBadge returns 0 when a target is configured', () => {
      config.http_post_target = 'http://test.local'
      expect(badge.pushWifiDirectBadge()).toBe(0)
    })

    it('pushHttpPost2Badge returns 0 when target count is non-zero', () => {
      config.mqtt_target = 'broker.local'
      expect(badge.pushHttpPost2Badge()).toBe(0)
    })

    it('pushHttpGetBadge returns 0 when target count is non-zero', () => {
      config.mqtt_target = 'broker.local'
      expect(badge.pushHttpGetBadge()).toBe(0)
    })

    it('pushInfluxdb2Badge returns 0 when target count is non-zero', () => {
      config.mqtt_target = 'broker.local'
      expect(badge.pushInfluxdb2Badge()).toBe(0)
    })

    it('pushMqttBadge returns 0 when target count is non-zero', () => {
      config.mqtt_target = 'broker.local'
      expect(badge.pushMqttBadge()).toBe(0)
    })

    it('pushBluetoothBadge returns 0 when target count is non-zero', () => {
      config.mqtt_target = 'broker.local'
      expect(badge.pushBluetoothBadge()).toBe(0)
    })

    it('pushTargetCount counts ble_format when non-zero', () => {
      config.ble_format = 1
      expect(badge.pushHttpPost1Badge()).toBe(0)
    })

    it('pushTargetCount counts use_wifi_direct when true', () => {
      config.use_wifi_direct = true
      expect(badge.pushHttpPost1Badge()).toBe(0)
    })

    it('pushTargetCount counts each target field', () => {
      config.http_post_target = 'http://a.local'
      config.http_post2_target = 'http://b.local'
      config.http_get_target = 'http://c.local'
      config.influxdb2_target = 'http://d.local'
      config.mqtt_target = 'mqtt://e.local'
      config.ble_format = 1
      config.use_wifi_direct = true
      expect(badge.pushBadge()).toBe(0)
    })
  })

  describe('aggregate badges', () => {
    it('deviceBadge sums up child badges', () => {
      config.mdns = '' // +1
      status.ispindel_config = true // +1
      // +0 others
      expect(badge.deviceBadge()).toBe(2)
    })

    it('deviceMigrateIspindelBadge returns 0 when ispindel_config is false', () => {
      status.ispindel_config = false
      expect(badge.deviceMigrateIspindelBadge()).toBe(0)
    })

    it('gravityFormulaBadge returns 0 when formula is set', () => {
      config.gravity_formula = 'x^2'
      expect(badge.gravityFormulaBadge()).toBe(0)
    })

    it('gravityBadge sums up gravity child badges', () => {
      config.gravity_formula = '' // +1
      expect(badge.gravityBadge()).toBe(1)
    })

    it('pushBadge sums up push child badges', () => {
      // 7 child functions, each is 1 if targetCount is 0
      // 1. pushSettingBadge: 0
      // 2. pushHttpPost1Badge: 1
      // 3. pushHttpPost2Badge: 1
      // 4. pushHttpGetBadge: 1
      // 5. pushInfluxdb2Badge: 1
      // 6. pushMqttBadge: 1
      // 7. pushBluetoothBadge: 1
      // Total = 6
      expect(badge.pushBadge()).toBe(6)
    })
  })
})
