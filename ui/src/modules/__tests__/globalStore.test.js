// Unmock the real globalStore so coverage runs against actual code
// (setup.js mocks @/modules/globalStore globally for component tests)
vi.doUnmock('../globalStore')

import { describe, it, expect, beforeEach, afterEach, vi } from 'vitest'
import { createPinia, setActivePinia } from 'pinia'
import { sharedHttpClient as http } from '@mp-se/espframework-ui-components'

describe('globalStore', () => {
  let store
  let useGlobalStore

  beforeEach(async () => {
    // Define build-time global used by registerApiKey getter
    // Note: vite define replaces __REGISTER_API_KEY__ with a bare base64 string
    vi.stubGlobal('__REGISTER_API_KEY__', btoa('test-api-key'))
    setActivePinia(createPinia())
    // Dynamic import ensures we get the real module, not the auto-mock
    const module = await import('../globalStore')
    useGlobalStore = module.useGlobalStore
    store = useGlobalStore()
    vi.clearAllMocks()
  })

  afterEach(() => {
    vi.unstubAllGlobals()
  })

  // --- Store creation ---
  it('can create store instance', () => {
    expect(store).toBeDefined()
    expect(store.$id).toBe('global')
  })

  // --- Default state ---
  it('initializes with empty platform', () => {
    expect(store.platform).toBe('')
  })

  it('initializes with empty messages', () => {
    expect(store.messageError).toBe('')
    expect(store.messageWarning).toBe('')
    expect(store.messageSuccess).toBe('')
    expect(store.messageInfo).toBe('')
  })

  it('initializes with ui feature flags', () => {
    expect(store.ui.enableVoltageFragment).toBe(true)
    expect(store.ui.enableManualWifiEntry).toBe(true)
    expect(store.ui.enableDeviceRegistration).toBe(true)
  })

  it('initializes with feature flags false', () => {
    expect(store.feature.ble).toBe(false)
    expect(store.feature.velocity).toBe(false)
    expect(store.feature.filter).toBe(false)
    expect(store.feature.charging).toBe(false)
  })

  it('initializes with initialized false', () => {
    expect(store.initialized).toBe(false)
  })

  // --- clearMessages action ---
  it('clearMessages clears all messages', () => {
    store.messageError = 'some error'
    store.messageWarning = 'some warning'
    store.messageSuccess = 'some success'
    store.messageInfo = 'some info'
    store.clearMessages()
    expect(store.messageError).toBe('')
    expect(store.messageWarning).toBe('')
    expect(store.messageSuccess).toBe('')
    expect(store.messageInfo).toBe('')
  })

  // --- Getters: isError / isWarning / isSuccess / isInfo ---
  it('isError returns true when messageError is set', () => {
    store.messageError = 'error!'
    expect(store.isError).toBe(true)
  })

  it('isError returns false when messageError is empty', () => {
    expect(store.isError).toBe(false)
  })

  it('isWarning returns true when messageWarning is set', () => {
    store.messageWarning = 'warn!'
    expect(store.isWarning).toBe(true)
  })

  it('isWarning returns false when empty', () => {
    expect(store.isWarning).toBe(false)
  })

  it('isSuccess returns true when messageSuccess is set', () => {
    store.messageSuccess = 'ok!'
    expect(store.isSuccess).toBe(true)
  })

  it('isSuccess returns false when empty', () => {
    expect(store.isSuccess).toBe(false)
  })

  it('isInfo returns true when messageInfo is set', () => {
    store.messageInfo = 'info!'
    expect(store.isInfo).toBe(true)
  })

  it('isInfo returns false when empty', () => {
    expect(store.isInfo).toBe(false)
  })

  // --- Getters: isEsp8266 / disabled32 ---
  it('isEsp8266 returns true when platform is ESP8266', () => {
    store.platform = 'ESP8266'
    expect(store.isEsp8266).toBe(true)
  })

  it('isEsp8266 returns false for other platforms', () => {
    store.platform = 'ESP32'
    expect(store.isEsp8266).toBe(false)
  })

  it('disabled32 returns true when disabled is true', () => {
    store.disabled = true
    store.platform = 'ESP32'
    expect(store.disabled32).toBe(true)
  })

  it('disabled32 returns true for ESP8266 even when not disabled', () => {
    store.disabled = false
    store.platform = 'ESP8266'
    expect(store.disabled32).toBe(true)
  })

  it('disabled32 returns false for ESP32 when not disabled', () => {
    store.disabled = false
    store.platform = 'ESP32'
    expect(store.disabled32).toBe(false)
  })

  // --- Getter: uiVersion / uiBuild ---
  it('uiVersion returns VITE_APP_VERSION from import.meta.env', () => {
    const ver = store.uiVersion
    // import.meta.env is defined by Vite; in tests it may be undefined or set via vitest config
    expect(ver === undefined || typeof ver === 'string').toBe(true)
  })

  it('uiBuild returns VITE_APP_BUILD from import.meta.env', () => {
    const build = store.uiBuild
    expect(build === undefined || typeof build === 'string').toBe(true)
  })

  // --- Getter: registerBaseUrl ---
  it('registerBaseUrl has a default fallback', () => {
    expect(typeof store.registerBaseUrl).toBe('string')
    expect(store.registerBaseUrl.length).toBeGreaterThan(0)
  })

  // --- Getter: registerApiKey ---
  it('registerApiKey decodes the base64 key when defined', () => {
    const key = store.registerApiKey
    expect(key).toBe('test-api-key')
  })

  it('registerApiKey returns null when key is empty', () => {
    vi.stubGlobal('__REGISTER_API_KEY__', '')
    const key = store.registerApiKey
    expect(key).toBeNull()
  })

  // --- anonymizeChipId action ---
  it('anonymizeChipId returns a hash string for a valid chip_id', async () => {
    store.chip_id = 'deadbeef'
    const result = await store.anonymizeChipId()
    expect(typeof result).toBe('string')
    expect(result).not.toBe('unknown')
  })

  it('anonymizeChipId returns unknown when chip_id is empty', async () => {
    store.chip_id = ''
    const result = await store.anonymizeChipId()
    expect(result).toBe('unknown')
  })

  it('anonymizeChipId returns unknown when chip_id is the string unknown', async () => {
    store.chip_id = 'unknown'
    const result = await store.anonymizeChipId()
    expect(result).toBe('unknown')
  })

  // --- load action ---
  it('load succeeds and updates all state fields', async () => {
    const mockData = {
      chip_id: 'ABC123',
      board: 'nodemcu',
      app_ver: '2.5.0',
      app_build: '200',
      platform: 'esp32',
      hardware: 'rev3',
      firmware_file: 'Gravitymon.BIN',
      registered: true,
      ble: true,
      velocity: true,
      filter: false,
      charging: true
    }
    http.getJson.mockResolvedValueOnce(mockData)
    const result = await store.load()
    expect(result).toBe(true)
    expect(store.chip_id).toBe('abc123')
    expect(store.board).toBe('NODEMCU')
    expect(store.app_ver).toBe('2.5.0')
    expect(store.platform).toBe('ESP32')
    expect(store.hardware).toBe('REV3')
    expect(store.firmware_file).toBe('gravitymon.bin')
    expect(store.registered).toBe(true)
    expect(store.feature.ble).toBe(true)
    expect(store.feature.velocity).toBe(true)
    expect(store.feature.filter).toBe(false)
    expect(store.feature.charging).toBe(true)
  })

  it('load returns false when http call fails', async () => {
    http.getJson.mockRejectedValueOnce(new Error('Network error'))
    const result = await store.load()
    expect(result).toBe(false)
  })
})
