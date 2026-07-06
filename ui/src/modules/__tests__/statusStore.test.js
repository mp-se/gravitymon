import { describe, it, expect, beforeEach, vi } from 'vitest'
import { createPinia, setActivePinia } from 'pinia'
import { sharedHttpClient as http } from '@mp-se/espframework-ui-components'

// Ensure statusStore is not auto-mocked due to circular imports
vi.doUnmock('../statusStore')

// Mock the UI components library
vi.mock('@mp-se/espframework-ui-components', () => ({
  logDebug: vi.fn(),
  logError: vi.fn(),
  logInfo: vi.fn(),
  sharedHttpClient: {
    getJson: vi.fn()
  }
}))

// Mock useConfigStore to avoid dependencies
vi.mock('@/modules/configStore', () => ({
  useConfigStore: vi.fn().mockImplementation(() => ({
    gyro_type: 1
  }))
}))

// Import after mocks
// import { useStatusStore } from '../statusStore'

describe('statusStore', () => {
  let store
  let pinia
  let useStatusStore

  beforeEach(async () => {
    pinia = createPinia()
    setActivePinia(pinia)
    // Import dynamically to avoid auto-mocking
    const { useStatusStore: imported } = await import('../statusStore')
    useStatusStore = imported
    store = useStatusStore(pinia)
    vi.clearAllMocks()
  })

  it('can create store instance', () => {
    expect(store).toBeDefined()
    expect(store.$id).toBe('status')
  })

  it('updateFromJson updates state correctly', () => {
    const mockJson = {
      id: 'test-id',
      angle: 12.345,
      temp_unit: 'C',
      gravity: 1.05567,
      gravity_unit: 'G',
      temp: 20.123,
      sleep_mode: true,
      battery: 4.123,
      rssi: -60,
      mdns: 'dev',
      wifi_ssid: 'wifi',
      ip: '1.2.3.4',
      ispindel_config: true,
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
      gyro_family: 'mpu6050',
      total_heap: 102400,
      free_heap: 51200,
      wifi_setup: true
    }

    store.updateFromJson(mockJson)

    expect(store.id).toBe('test-id')
    expect(store.angle).toBe('12.35')
    expect(store.gravity).toBe('1.0557')
    expect(store.temp).toBe('20.12')
    expect(store.battery).toBe('4.12')
    expect(store.total_heap).toBe('100')
    expect(store.free_heap).toBe('50')
    expect(store.gyro_family).toBe('MPU6050')
    expect(store.sleep_mode).toBe(true)
    expect(store.ble_supported).toBe(true)
    expect(store.wifi_setup).toBe(true)
  })

  it('updateFromJson handles gravity in P units', () => {
    const mockJson = {
      gravity: 1.05567,
      gravity_unit: 'P',
      temp: 20.123,
      battery: 4.123,
      angle: 12.345,
      self_check: {
        gyro_connected: true,
        gyro_moving: false,
        gyro_calibration: true,
        temp_connected: true,
        gravity_formula: true,
        battery_level: true,
        push_targets: true
      }
    }

    store.updateFromJson(mockJson)

    expect(store.gravity).toBe('1.06') // Rounded to 2 decimals for P
  })

  it('load succeeds and updates state', async () => {
    const mockData = {
      id: 'loaded-id',
      angle: 10.5,
      gravity: 1.05,
      temp: 25.0,
      self_check: {
        gyro_connected: true,
        gyro_moving: false,
        gyro_calibration: true,
        temp_connected: true,
        gravity_formula: true,
        battery_level: true,
        push_targets: true
      }
    }
    http.getJson.mockResolvedValueOnce(mockData)
    const result = await store.load()
    expect(result).toBe(true)
    expect(store.id).toBe('loaded-id')
    expect(store.angle).toBe('10.50')
  })

  it('load fails on http error', async () => {
    http.getJson.mockRejectedValueOnce(new Error('Fail'))
    const result = await store.load()
    expect(result).toBe(false)
  })

  it('getGyro succeeds', async () => {
    const mockGyro = { angle: 10.5, temp: 25.0 }
    http.getJson.mockResolvedValueOnce(mockGyro)
    const result = await store.getGyro()
    expect(result.success).toBe(true)
    expect(result.data).toEqual(mockGyro)
  })

  it('getGyro fails on http error', async () => {
    http.getJson.mockRejectedValueOnce(new Error('Gyro fail'))
    const result = await store.getGyro()
    expect(result.success).toBe(false)
    expect(result.error.message).toBe('Gyro fail')
  })

  it('needsCalibration getter returns true for gyro_type 1', () => {
    expect(store.needsCalibration).toBe(true)
  })

  it('initializes with default state', () => {
    expect(store.id).toBe('')
    expect(store.angle).toBe(0)
    expect(store.gravity).toBe(0)
    expect(store.temp).toBe(0)
    expect(store.battery).toBe(0)
    expect(store.sleep_mode).toBe(false)
    expect(store.connected).toBe(true)
    expect(store.ble_supported).toBe(true)
  })

  it('load skips rounding when battery/angle/temp/gravity are undefined', async () => {
    // Provide a response that omits battery, angle, temp, gravity so they remain undefined
    const mockData = {
      id: 'partial-id',
      self_check: {
        gyro_connected: false,
        gyro_moving: false,
        gyro_calibration: false,
        temp_connected: false,
        gravity_formula: false,
        battery_level: false,
        push_targets: false
      }
    }
    http.getJson.mockResolvedValueOnce(mockData)
    const result = await store.load()
    expect(result).toBe(true)
    expect(store.id).toBe('partial-id')
    // Rounding branches for undefined values should be skipped without error
    expect(store.battery).toBeUndefined()
    expect(store.angle).toBeUndefined()
    expect(store.temp).toBeUndefined()
    expect(store.gravity).toBeUndefined()
  })
})
