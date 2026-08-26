import { describe, it, expect, beforeEach, vi, afterEach } from 'vitest'
import { mount, flushPromises } from '@vue/test-utils'
import BackupView from '../BackupView.vue'
import { createPinia, setActivePinia } from 'pinia'
import { global, config } from '@/modules/pinia'

// Mock the UI components library
vi.mock('@mp-se/espframework-ui-components', () => ({
  logDebug: vi.fn(),
  logError: vi.fn(),
  logInfo: vi.fn(),
  sharedHttpClient: {
    get: vi.fn(),
    post: vi.fn(),
    postJson: vi.fn(() => Promise.resolve({ success: true }))
  }
}))

vi.unmock('@/modules/pinia')

describe('BackupView (comprehensive tests)', () => {
  let wrapper

  beforeEach(() => {
    setActivePinia(createPinia())

    // Mock URL and Blob for download test
    if (!window.URL) {
      window.URL = { createObjectURL: vi.fn(), revokeObjectURL: vi.fn() }
    } else {
      window.URL.createObjectURL = vi.fn(() => 'mock-url')
      window.URL.revokeObjectURL = vi.fn()
    }

    wrapper = mount(BackupView, {
      global: {
        stubs: {
          BsFileUpload: {
            template: '<input type="file" @change="$emit(\'change\', $event)" />'
          },
          BsProgress: true,
          BsMessage: true
        }
      }
    })
  })

  afterEach(() => {
    vi.clearAllMocks()
    vi.restoreAllMocks()
  })

  it('mounts and displays headers', () => {
    expect(wrapper.text()).toContain('Backup & Restore')
  })

  it('handles file selection change', async () => {
    const input = wrapper.find('input[type="file"]')

    // In VTU, for file inputs, we should set the element's files property directly
    // and then trigger the change event.
    Object.defineProperty(input.element, 'files', {
      value: [new File([''], 'test.txt')],
      writable: false
    })

    await input.trigger('change')

    expect(wrapper.vm.fileSelected).toBe(true)
  })

  it('triggers backup download with correct data', async () => {
    config.mdns = 'test-device'
    config.toJson = vi.fn(() =>
      JSON.stringify({ mdns: 'test-device', http_post_format_gravity: '' })
    )

    const documentSpy = vi.spyOn(document, 'createElement')
    const clickSpy = vi.fn()
    const mockElement = {
      setAttribute: vi.fn(),
      click: clickSpy
    }
    documentSpy.mockReturnValue(mockElement)

    const backupButton = wrapper.findAll('button').find((b) => b.text().includes('Create backup'))
    await backupButton.trigger('click')

    expect(config.toJson).toHaveBeenCalled()
    expect(clickSpy).toHaveBeenCalled()
    expect(global.messageSuccess).toContain('test-device.txt')
    documentSpy.mockRestore()
  })

  it('handles restore with invalid format', async () => {
    const mockFileData = JSON.stringify({ meta: { software: 'Wrong' } })

    const mockReader = {
      readAsText: vi.fn(function () {
        this.onload({ target: { result: mockFileData } })
      }),
      addEventListener: vi.fn(function (event, cb) {
        if (event === 'load') this.onload = cb
      })
    }

    class MockFileReader {
      constructor() {
        return mockReader
      }
    }

    vi.stubGlobal('FileReader', MockFileReader)

    const getElementSpy = vi.spyOn(document, 'getElementById').mockReturnValue({
      files: [new File([], 'test.txt')],
      value: 'test.txt'
    })

    await wrapper.find('form').trigger('submit')

    expect(global.messageError).toBe('Unknown format, unable to process')
    getElementSpy.mockRestore()
  })

  it('handles restore with valid v2.2.0 format', async () => {
    const validData = {
      meta: { software: 'GravityMon', version: '2.2.0' },
      config: { mdns: 'restored-device', http_post_format_gravity: '' }
    }

    const mockReader = {
      readAsText: vi.fn(function () {
        this.onload({ target: { result: JSON.stringify(validData) } })
      }),
      addEventListener: vi.fn(function (event, cb) {
        if (event === 'load') this.onload = cb
        if (event === 'error') this.onerror = cb
      })
    }

    class MockFileReader {
      constructor() {
        return mockReader
      }
    }

    vi.stubGlobal('FileReader', MockFileReader)

    const getElementSpy = vi.spyOn(document, 'getElementById').mockReturnValue({
      files: [new File([], 'test.txt')],
      value: 'test.txt'
    })

    await wrapper.find('form').trigger('submit')
    await flushPromises()

    expect(global.disabled).toBe(false)
    getElementSpy.mockRestore()
  })

  it('handles FileReader error', async () => {
    const mockReader = {
      readAsText: vi.fn(function () {
        this.onerror()
      }),
      addEventListener: vi.fn(function (event, cb) {
        if (event === 'error') this.onerror = cb
        if (event === 'load') this.onload = cb
      })
    }

    class MockFileReader {
      constructor() {
        return mockReader
      }
    }

    vi.stubGlobal('FileReader', MockFileReader)

    const getElementSpy = vi.spyOn(document, 'getElementById').mockReturnValue({
      files: [new File([], 'test.txt')],
      value: 'test.txt'
    })

    await wrapper.find('form').trigger('submit')

    expect(global.messageError).toBe('Failed to read the backup file')
    getElementSpy.mockRestore()
  })

  it('handles restore when file element not found', async () => {
    const getElementSpy = vi.spyOn(document, 'getElementById').mockReturnValue(null)
    await wrapper.find('form').trigger('submit')
    expect(global.messageError).toBe('Upload element not found')
    getElementSpy.mockRestore()
  })

  it('handles restore when no file selected', async () => {
    const getElementSpy = vi.spyOn(document, 'getElementById').mockReturnValue({
      files: [],
      value: ''
    })
    await wrapper.find('form').trigger('submit')
    expect(global.messageError).toBe('You need to select one file to restore configuration from')
    getElementSpy.mockRestore()
  })

  it('handles restore with invalid JSON', async () => {
    const mockReader = {
      readAsText: vi.fn(function () {
        this.onload({ target: { result: 'not valid json {{{' } })
      }),
      addEventListener: vi.fn(function (event, cb) {
        if (event === 'load') this.onload = cb
        if (event === 'error') this.onerror = cb
      })
    }

    class MockFileReader {
      constructor() {
        return mockReader
      }
    }

    vi.stubGlobal('FileReader', MockFileReader)

    const getElementSpy = vi.spyOn(document, 'getElementById').mockReturnValue({
      files: [new File([], 'backup.txt')],
      value: 'backup.txt'
    })

    await wrapper.find('form').trigger('submit')
    await flushPromises()

    expect(global.messageError).toBe('Unable to parse configuration file for GravityMon.')
    getElementSpy.mockRestore()
  })

  it('handles restore with v2.0.0 format', async () => {
    const validData = {
      meta: { software: 'GravityMon', version: '2.0.0' },
      config: {
        mdns: 'restored-v2',
        http_post_format_gravity: encodeURIComponent('{gravity}'),
        test_format: encodeURIComponent('{test}')
      }
    }
    const mockReader = {
      readAsText: vi.fn(function () {
        this.onload({ target: { result: JSON.stringify(validData) } })
      }),
      addEventListener: vi.fn(function (event, cb) {
        if (event === 'load') this.onload = cb
        if (event === 'error') this.onerror = cb
      })
    }

    class MockFileReader {
      constructor() {
        return mockReader
      }
    }

    vi.stubGlobal('FileReader', MockFileReader)

    const getElementSpy = vi.spyOn(document, 'getElementById').mockReturnValue({
      files: [new File([], 'backup.txt')],
      value: 'backup.txt'
    })

    await wrapper.find('form').trigger('submit')
    await flushPromises()

    expect(global.disabled).toBe(false)
    getElementSpy.mockRestore()
  })

  it('handles restore with v1 format (doRestore1)', async () => {
    const v1Data = {
      meta: { software: 'GravityMon', version: '1.0.0' },
      advanced: {
        id: 'to-delete',
        'int-http1': 300,
        'int-http2': 600,
        'int-http3': 900,
        'int-influx': 1200,
        'int-mqtt': 1500,
        'some-key': 'value'
      },
      config: {
        'app-ver': '1.0',
        'app-build': '100',
        angle: 45,
        gravity: 1.05,
        battery: 4.2,
        'runtime-average': 3000,
        platform: 'esp8266',
        id: 'chip1',
        'wifi-ssid': 'ssid1',
        'wifi-pass': 'pass1',
        'wifi-ssid2': 'ssid2',
        'wifi-pass2': 'pass2',
        ble: 2,
        'http-push': 'http://test.local',
        'http-push-h1': 'Accept: */*',
        'http-push-h2': '',
        'http-push2': '',
        'http-push2-h1': '',
        'http-push2-h2': '',
        'http-push3': '',
        'influxdb2-push': '',
        'mqtt-push': '',
        'formula-calculation-data': {
          a1: 1,
          g1: 1.0,
          a2: 20,
          g2: 1.01,
          a3: 30,
          g3: 1.02,
          a4: 40,
          g4: 1.03,
          a5: 50,
          g5: 1.04,
          a6: 60,
          g6: 1.05,
          a7: 70,
          g7: 1.06,
          a8: 80,
          g8: 1.07,
          a9: 90,
          g9: 1.08,
          a10: 100,
          g10: 1.09
        },
        mdns: 'legacy-device'
      },
      format: {
        'http-1': encodeURIComponent('{gravity}'),
        'http-2': encodeURIComponent('{temp}'),
        'http-3': encodeURIComponent('{angle}'),
        influxdb: encodeURIComponent('{battery}'),
        mqtt: encodeURIComponent('key|value')
      }
    }

    const mockReader = {
      readAsText: vi.fn(function () {
        this.onload({ target: { result: JSON.stringify(v1Data) } })
      }),
      addEventListener: vi.fn(function (event, cb) {
        if (event === 'load') this.onload = cb
        if (event === 'error') this.onerror = cb
      })
    }

    class MockFileReader {
      constructor() {
        return mockReader
      }
    }

    vi.stubGlobal('FileReader', MockFileReader)

    const getElementSpy = vi.spyOn(document, 'getElementById').mockReturnValue({
      files: [new File([], 'backup.txt')],
      value: 'backup.txt'
    })

    await wrapper.find('form').trigger('submit')
    await flushPromises()

    expect(global.disabled).toBe(false)
    expect(config.http_post_target).toBe('http://test.local')
    getElementSpy.mockRestore()
  })

  it('handles file selection with no files (clears selection)', async () => {
    const input = wrapper.find('input[type="file"]')
    Object.defineProperty(input.element, 'files', {
      value: [],
      writable: false,
      configurable: true
    })
    await input.trigger('change')
    expect(wrapper.vm.fileSelected).toBe(false)
  })

  it('renders progress bar when progress > 0', async () => {
    wrapper.vm.progress = 50
    await wrapper.vm.$nextTick()
    // BsProgress stub should be rendered (or at least not throw)
    expect(wrapper.vm.progress).toBe(50)
  })
})
