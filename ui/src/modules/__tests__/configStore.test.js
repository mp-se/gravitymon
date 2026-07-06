import { describe, it, expect, beforeEach, vi } from 'vitest'
import { createPinia, setActivePinia } from 'pinia'
import { sharedHttpClient as http } from '@mp-se/espframework-ui-components'

// Ensure configStore is not auto-mocked due to circular imports with pinia.js
vi.doUnmock('../configStore')

// Mock the UI components library
vi.mock('@mp-se/espframework-ui-components', () => ({
  logDebug: vi.fn(),
  logError: vi.fn(),
  logInfo: vi.fn(),
  tempToC: (t) => (t - 32) / 1.8,
  tempToF: (t) => t * 1.8 + 32,
  roundVal: (v, d) => Math.round(v * Math.pow(10, d)) / Math.pow(10, d),
  sharedHttpClient: {
    getJson: vi.fn(),
    postJson: vi.fn(),
    request: vi.fn(),
    restart: vi.fn(),
    getErrorString: vi.fn((c) => `Error ${c}`)
  }
}))

// Mock pinia module with minimal implementation to avoid circular dependencies
// and provide the global state we need.
vi.mock('@/modules/pinia', () => {
  return {
    global: {
      disabled: false,
      configChanged: false,
      initialized: true,
      clearMessages: vi.fn(),
      messageSuccess: '',
      messageError: '',
      messageWarning: ''
    },
    saveConfigState: vi.fn(),
    getConfigChanges: vi.fn(() => ({}))
    // We don't need the exports that use useConfigStore here
  }
})

// We need to import the mocked global to check its properties
import { global, saveConfigState, getConfigChanges } from '@/modules/pinia'

describe('configStore', () => {
  let store
  let useConfigStore

  beforeEach(async () => {
    setActivePinia(createPinia())
    // Dynamic import to avoid auto-mocking caused by circular dependency with pinia.js
    const module = await import('../configStore')
    useConfigStore = module.useConfigStore
    store = useConfigStore()
    vi.clearAllMocks()

    // Reset global mock state
    global.messageSuccess = ''
    global.messageError = ''
    global.messageWarning = ''
    global.disabled = false
    global.configChanged = false
  })

  it('can create store instance', () => {
    expect(store).toBeDefined()
    expect(store.$id).toBe('config')
  })

  it('has toJson method', () => {
    const jsonStr = store.toJson()
    expect(jsonStr).toBeDefined()
    const json = JSON.parse(jsonStr)
    expect(json).toHaveProperty('id')
  })

  it('convertTemp works correctly (C to F)', () => {
    store.internal_temp_unit = 'C'
    store.temp_unit = 'F'
    store.formula_calibration_temp = 20
    store.temp_adjustment_value = 1

    store.convertTemp()

    expect(store.internal_temp_unit).toBe('F')
    expect(store.formula_calibration_temp).toBe(68)
    expect(store.temp_adjustment_value).toBe(1.8)
  })

  it('convertTempToC works correctly', () => {
    store.internal_temp_unit = 'F'
    store.formula_calibration_temp = 68
    store.temp_adjustment_value = 1.8

    store.convertTempToC()

    expect(store.internal_temp_unit).toBe('C')
    expect(store.formula_calibration_temp).toBe(20)
    expect(store.temp_adjustment_value).toBe(1)
  })

  it('convertTempToC is a no-op when already C', () => {
    store.internal_temp_unit = 'C'
    store.formula_calibration_temp = 20
    store.convertTempToC()
    expect(store.internal_temp_unit).toBe('C')
    expect(store.formula_calibration_temp).toBe(20)
  })

  it('convertTempToF is a no-op when already F', () => {
    store.internal_temp_unit = 'F'
    store.formula_calibration_temp = 68
    store.convertTempToF()
    expect(store.internal_temp_unit).toBe('F')
    expect(store.formula_calibration_temp).toBe(68)
  })

  it('convertTemp is a no-op when temp_unit equals internal_temp_unit', () => {
    store.internal_temp_unit = 'C'
    store.temp_unit = 'C'
    store.formula_calibration_temp = 20
    store.convertTemp()
    expect(store.formula_calibration_temp).toBe(20)
  })

  it('load fails on http error', async () => {
    http.getJson.mockRejectedValueOnce(new Error('Network error'))
    const result = await store.load()
    expect(result).toBe(false)
  })

  it('load succeeds and updates state', async () => {
    const mockData = { id: 'test-id', temp_unit: 'F' }
    http.getJson.mockResolvedValueOnce(mockData)
    const result = await store.load()
    expect(result).toBe(true)
    expect(store.id).toBe('test-id')
    expect(store.temp_unit).toBe('F')
  })

  it('sendConfig sends changes', async () => {
    getConfigChanges.mockReturnValueOnce({ id: 'new-id' })
    http.postJson.mockResolvedValueOnce({ ok: true })
    const result = await store.sendConfig()
    expect(result).toBe(true)
    expect(http.postJson).toHaveBeenCalledWith('api/config', { id: 'new-id' })
  })

  it('sendFormat handles format changes', async () => {
    getConfigChanges.mockReturnValue({
      http_post_format_gravity: 'format1'
    })
    http.postJson.mockResolvedValue({ ok: true })

    const result = await store.sendFormat()
    expect(result).toBe(true)
    expect(http.postJson).toHaveBeenCalledWith(
      'api/format',
      expect.objectContaining({
        http_post_format_gravity: 'format1'
      })
    )
  })

  it('sendPushTest calls API', async () => {
    const testData = { foo: 'bar' }
    http.postJson.mockResolvedValueOnce({ ok: true })
    const result = await store.sendPushTest(testData)
    expect(result).toBe(true)
    expect(http.postJson).toHaveBeenCalledWith('api/push', testData)
  })

  it('setSleepMode calls API', async () => {
    http.request.mockResolvedValueOnce({ ok: true, json: () => Promise.resolve({}) })
    const result = await store.setSleepMode(true)
    expect(result).toBe(true)
    expect(http.request).toHaveBeenCalled()
  })

  it('setSleepMode returns false when response.ok is false', async () => {
    http.request.mockResolvedValueOnce({ ok: false, status: 403, statusText: 'Forbidden' })
    const result = await store.setSleepMode(false)
    expect(result).toBe(false)
  })

  it('setSleepMode returns false on exception', async () => {
    http.request.mockRejectedValueOnce(new Error('Network error'))
    const result = await store.setSleepMode(true)
    expect(result).toBe(false)
  })

  it('restart handles success', async () => {
    store.mdns = 'gravitymon'
    http.restart.mockResolvedValueOnce({
      success: true,
      json: { status: true, message: 'Restarting' }
    })
    await store.restart()
    expect(global.messageSuccess).toContain('Restarting')
  })

  it('restart handles failure response', async () => {
    store.mdns = 'gravitymon'
    http.restart.mockResolvedValueOnce({ success: false })
    await store.restart()
    expect(global.messageError).toBe('Failed to request restart')
  })

  it('restart handles json error message', async () => {
    store.mdns = 'gravitymon'
    http.restart.mockResolvedValueOnce({
      success: true,
      json: { status: false, message: 'Bad state' }
    })
    await store.restart()
    expect(global.messageError).toBe('Bad state')
  })

  it('restart handles thrown error', async () => {
    http.restart.mockRejectedValueOnce(new Error('Connection refused'))
    await store.restart()
    expect(global.messageError).toBe('Failed to do restart')
  })

  it('saveAll handles success', async () => {
    getConfigChanges.mockReturnValue({})
    http.postJson.mockResolvedValue({ ok: true })

    await store.saveAll()
    expect(global.messageSuccess).toBe('Configuration has been saved to device')
    expect(saveConfigState).toHaveBeenCalled()
  })

  it('saveAll handles sendConfig failure', async () => {
    getConfigChanges.mockReturnValue({ id: 'new' })
    http.postJson.mockRejectedValueOnce(new Error('Network error'))
    await store.saveAll()
    expect(global.messageError).toBe('Failed to store configuration to device')
  })

  it('saveAll sets error when sendFormat fails', async () => {
    vi.spyOn(store, 'sendConfig').mockResolvedValueOnce(true)
    vi.spyOn(store, 'sendFormat').mockResolvedValueOnce(false)
    await store.saveAll()
    expect(global.messageError).toBe('Failed to store format to device')
  })

  it('getPushTestStatus handles success', async () => {
    http.getJson.mockResolvedValueOnce({ status: 'ok' })
    const result = await store.getPushTestStatus()
    expect(result.success).toBe(true)
    expect(result.data.status).toBe('ok')
  })

  it('getPushTestStatus handles failure', async () => {
    http.getJson.mockRejectedValueOnce(new Error('Network error'))
    const result = await store.getPushTestStatus()
    expect(result.success).toBe(false)
    expect(result.data).toBeNull()
  })

  it('sendWifiScan succeeds', async () => {
    http.request.mockResolvedValueOnce({ ok: true })
    const result = await store.sendWifiScan()
    expect(result).toBe(true)
    expect(http.request).toHaveBeenCalledWith('api/wifi')
  })

  it('sendWifiScan fails on error', async () => {
    http.request.mockRejectedValueOnce(new Error('Network error'))
    const result = await store.sendWifiScan()
    expect(result).toBe(false)
  })

  it('getWifiScanStatus returns data on success', async () => {
    http.getJson.mockResolvedValueOnce({ status: false, data: ['SSID1'] })
    const result = await store.getWifiScanStatus()
    expect(result.success).toBe(true)
    expect(result.data.status).toBe(false)
  })

  it('getWifiScanStatus returns failure on error', async () => {
    http.getJson.mockRejectedValueOnce(new Error('err'))
    const result = await store.getWifiScanStatus()
    expect(result.success).toBe(false)
  })

  it('sendHardwareScan succeeds', async () => {
    http.request.mockResolvedValueOnce({ ok: true })
    const result = await store.sendHardwareScan()
    expect(result).toBe(true)
    expect(http.request).toHaveBeenCalledWith('api/hardware')
  })

  it('sendHardwareScan fails on error', async () => {
    http.request.mockRejectedValueOnce(new Error('err'))
    const result = await store.sendHardwareScan()
    expect(result).toBe(false)
  })

  it('getHardwareScanStatus returns data on success', async () => {
    http.getJson.mockResolvedValueOnce({ status: false, devices: [] })
    const result = await store.getHardwareScanStatus()
    expect(result.success).toBe(true)
  })

  it('getHardwareScanStatus returns failure on error', async () => {
    http.getJson.mockRejectedValueOnce(new Error('err'))
    const result = await store.getHardwareScanStatus()
    expect(result.success).toBe(false)
  })

  // loadFormat
  it('loadFormat succeeds and decodes format strings', async () => {
    http.getJson.mockResolvedValueOnce({
      http_post_format_gravity: encodeURIComponent('{gravity}'),
      http_post2_format_gravity: encodeURIComponent('{temp}'),
      http_get_format_gravity: encodeURIComponent('{angle}'),
      influxdb2_format_gravity: encodeURIComponent('{battery}'),
      mqtt_format_gravity: encodeURIComponent('key|value')
    })
    const result = await store.loadFormat()
    expect(result).toBe(true)
    expect(store.http_post_format_gravity).toBe('{gravity}')
    expect(store.http_post2_format_gravity).toBe('{temp}')
    expect(store.http_get_format_gravity).toBe('{angle}')
    expect(store.influxdb2_format_gravity).toBe('{battery}')
    expect(store.mqtt_format_gravity).toBe('key|\nvalue')
  })

  it('loadFormat fails on http error', async () => {
    http.getJson.mockRejectedValueOnce(new Error('Network error'))
    const result = await store.loadFormat()
    expect(result).toBe(false)
  })

  // sendConfig edge cases
  it('sendConfig skips when no changes', async () => {
    getConfigChanges.mockReturnValueOnce({})
    const result = await store.sendConfig()
    expect(result).toBe(true)
    expect(http.postJson).not.toHaveBeenCalled()
  })

  it('sendConfig handles http error', async () => {
    getConfigChanges.mockReturnValueOnce({ id: 'new' })
    http.postJson.mockRejectedValueOnce(new Error('Network error'))
    const result = await store.sendConfig()
    expect(result).toBe(false)
  })

  // sendOneFormat
  it('sendOneFormat skips empty data', async () => {
    const result = await store.sendOneFormat({})
    expect(result).toBe(true)
    expect(http.postJson).not.toHaveBeenCalled()
  })

  it('sendOneFormat sends data successfully', async () => {
    http.postJson.mockResolvedValueOnce({ ok: true })
    const result = await store.sendOneFormat({ http_post_format_gravity: 'fmt' })
    expect(result).toBe(true)
    expect(http.postJson).toHaveBeenCalledWith('api/format', { http_post_format_gravity: 'fmt' })
  })

  it('sendOneFormat handles http error', async () => {
    http.postJson.mockRejectedValueOnce(new Error('Network error'))
    const result = await store.sendOneFormat({ http_post_format_gravity: 'fmt' })
    expect(result).toBe(false)
  })

  // sendFormat with mqtt newline stripping
  it('sendFormat strips newlines from mqtt format', async () => {
    getConfigChanges.mockReturnValue({ mqtt_format_gravity: 'key|\nvalue\r\n' })
    http.postJson.mockResolvedValue({ ok: true })
    await store.sendFormat()
    expect(http.postJson).toHaveBeenCalledWith(
      'api/format',
      expect.objectContaining({ mqtt_format_gravity: encodeURIComponent('key|value') })
    )
  })

  // sendPushTest failure
  it('sendPushTest fails on http error', async () => {
    http.postJson.mockRejectedValueOnce(new Error('Network error'))
    const result = await store.sendPushTest({ target: 'http1' })
    expect(result).toBe(false)
  })

  // runPushTest
  it('runPushTest handles sendPushTest failure', async () => {
    vi.spyOn(store, 'sendPushTest').mockResolvedValueOnce(false)
    const result = await store.runPushTest({ target: 'http1' })
    expect(result).toBe(false)
    expect(global.messageError).toBe('Failed to start push test')
  })

  it('runPushTest handles sendPushTest exception', async () => {
    vi.spyOn(store, 'sendPushTest').mockRejectedValueOnce(new Error('crash'))
    const result = await store.runPushTest({ target: 'http1' })
    expect(result).toBe(false)
    expect(global.messageError).toBe('Push test failed unexpectedly')
  })

  it('runPushTest succeeds with push_enabled=true', async () => {
    vi.spyOn(store, 'sendPushTest').mockResolvedValueOnce(true)
    vi.spyOn(store, 'getPushTestStatus').mockResolvedValueOnce({
      success: true,
      data: { status: false, success: true, push_enabled: true, push_return_code: 200 }
    })
    const result = await store.runPushTest({ target: 'http1' })
    expect(result).toBe(true)
    expect(global.messageSuccess).toBe('Test was successful')
  })

  it('runPushTest shows warning when push_enabled=false', async () => {
    vi.spyOn(store, 'sendPushTest').mockResolvedValueOnce(true)
    vi.spyOn(store, 'getPushTestStatus').mockResolvedValueOnce({
      success: true,
      data: { status: false, success: true, push_enabled: false, push_return_code: 0 }
    })
    const result = await store.runPushTest({ target: 'http1' })
    expect(result).toBe(true)
    expect(global.messageWarning).toBe('No endpoint is defined for this target. Cannot run test.')
  })

  it('runPushTest shows error when success=false', async () => {
    vi.spyOn(store, 'sendPushTest').mockResolvedValueOnce(true)
    vi.spyOn(store, 'getPushTestStatus').mockResolvedValueOnce({
      success: true,
      data: { status: false, success: false, push_enabled: true, push_return_code: 42 }
    })
    const result = await store.runPushTest({ target: 'http1' })
    expect(result).toBe(true)
    expect(global.messageError).toContain('42')
  })

  it('runPushTest handles getPushTestStatus failure', async () => {
    vi.spyOn(store, 'sendPushTest').mockResolvedValueOnce(true)
    vi.spyOn(store, 'getPushTestStatus').mockResolvedValueOnce({ success: false, data: null })
    const result = await store.runPushTest({ target: 'http1' })
    expect(result).toBe(false)
    expect(global.messageError).toBe('Failed to get push test status')
  })

  it('runPushTest retries when status=true (still running)', async () => {
    vi.spyOn(store, 'sendPushTest').mockResolvedValueOnce(true)
    vi.spyOn(store, 'getPushTestStatus')
      .mockResolvedValueOnce({ success: true, data: { status: true } })
      .mockResolvedValueOnce({
        success: true,
        data: { status: false, success: true, push_enabled: true, push_return_code: 200 }
      })
    vi.useFakeTimers()
    const resultPromise = store.runPushTest({ target: 'http1' })
    await vi.runAllTimersAsync()
    const result = await resultPromise
    vi.useRealTimers()
    expect(result).toBe(true)
    expect(global.messageSuccess).toBe('Test was successful')
  })

  // runWifiScan
  it('runWifiScan succeeds', async () => {
    vi.spyOn(store, 'sendWifiScan').mockResolvedValueOnce(true)
    vi.spyOn(store, 'getWifiScanStatus').mockResolvedValueOnce({
      success: true,
      data: { status: false, success: true, networks: [] }
    })
    const result = await store.runWifiScan()
    expect(result.success).toBe(true)
  })

  it('runWifiScan handles sendWifiScan failure', async () => {
    vi.spyOn(store, 'sendWifiScan').mockResolvedValueOnce(false)
    const result = await store.runWifiScan()
    expect(result.success).toBe(false)
    expect(global.messageError).toBe('Failed to start wifi scan')
  })

  it('runWifiScan handles getWifiScanStatus failure', async () => {
    vi.spyOn(store, 'sendWifiScan').mockResolvedValueOnce(true)
    vi.spyOn(store, 'getWifiScanStatus').mockResolvedValueOnce({ success: false, data: null })
    const result = await store.runWifiScan()
    expect(result.success).toBe(false)
    expect(global.messageError).toBe('Failed to get wifi scan status')
  })

  it('runWifiScan retries when status=true', async () => {
    vi.spyOn(store, 'sendWifiScan').mockResolvedValueOnce(true)
    vi.spyOn(store, 'getWifiScanStatus')
      .mockResolvedValueOnce({ success: true, data: { status: true } })
      .mockResolvedValueOnce({
        success: true,
        data: { status: false, success: true, networks: ['X'] }
      })
    vi.useFakeTimers()
    const resultPromise = store.runWifiScan()
    await vi.runAllTimersAsync()
    const result = await resultPromise
    vi.useRealTimers()
    expect(result.success).toBe(true)
  })

  // runHardwareScan
  it('runHardwareScan succeeds', async () => {
    vi.spyOn(store, 'sendHardwareScan').mockResolvedValueOnce(true)
    vi.spyOn(store, 'getHardwareScanStatus').mockResolvedValueOnce({
      success: true,
      data: { status: false, success: true, devices: [] }
    })
    const result = await store.runHardwareScan()
    expect(result.success).toBe(true)
  })

  it('runHardwareScan handles sendHardwareScan failure', async () => {
    vi.spyOn(store, 'sendHardwareScan').mockResolvedValueOnce(false)
    const result = await store.runHardwareScan()
    expect(result.success).toBe(false)
    expect(global.messageError).toBe('Failed to start hardware scan')
  })

  it('runHardwareScan handles getHardwareScanStatus failure', async () => {
    vi.spyOn(store, 'sendHardwareScan').mockResolvedValueOnce(true)
    vi.spyOn(store, 'getHardwareScanStatus').mockResolvedValueOnce({ success: false, data: null })
    const result = await store.runHardwareScan()
    expect(result.success).toBe(false)
    expect(global.messageError).toBe('Failed to get hardware scan status')
  })

  it('runHardwareScan retries when status=true', async () => {
    vi.spyOn(store, 'sendHardwareScan').mockResolvedValueOnce(true)
    vi.spyOn(store, 'getHardwareScanStatus')
      .mockResolvedValueOnce({ success: true, data: { status: true } })
      .mockResolvedValueOnce({ success: true, data: { status: false, success: true, devices: [] } })
    vi.useFakeTimers()
    const resultPromise = store.runHardwareScan()
    await vi.runAllTimersAsync()
    const result = await resultPromise
    vi.useRealTimers()
    expect(result.success).toBe(true)
  })
})
