import { vi } from 'vitest'
import { config as vtConfig } from '@vue/test-utils'
import { ref } from 'vue'

// Mock localStorage before anything else
class LocalStorageMock {
  constructor() {
    this.store = {}
  }

  clear() {
    this.store = {}
  }

  getItem(key) {
    return this.store[key] || null
  }
  setItem(key, value) {
    this.store[key] = String(value)
  }

  removeItem(key) {
    delete this.store[key]
  }

  get length() {
    return Object.keys(this.store).length
  }

  key(index) {
    const keys = Object.keys(this.store)
    return keys[index] || null
  }
}

Object.defineProperty(window, 'localStorage', {
  value: new LocalStorageMock()
})

// Mock pinia stores BEFORE importing anything that uses them
vi.mock('@/modules/configStore', () => ({
  useConfigStore: vi.fn(() => ({
    id: '',
    mdns: '',
    temp_unit: 'C',
    gravity_unit: 'G',
    gravity_formula: 'tilt',
    gyro_calibration_data: { ax: 0, ay: 0, az: 0, gx: 0, gy: 0, gz: 0 },
    gyro_type: 1,
    wifi_ssid: '',
    formula_calculation_data: [],
    formula_max_deviation: 0.1,
    toJson: vi.fn(),
    convertTemp: vi.fn(),
    convertTempToC: vi.fn(),
    convertTempToF: vi.fn(),
    load: vi.fn(),
    loadFormat: vi.fn(),
    sendConfig: vi.fn(),
    // sleep interval used by PushSettingsView (in seconds)
    sleep_interval: ref(600),
    // push target arrays expected by PushSettingsView
    http_post_target: [],
    http_post2_target: [],
    http_get_target: [],
    influxdb2_target: [],
    mqtt_target: [],
    // ble format flag expected by PushSettingsView
    ble_format: 0,
    // convenience method used by DeviceWifiView during mount
    runWifiScan: vi.fn(async () => ({ success: true, data: [] })),
    saveAll: vi.fn(async () => true),
    restart: vi.fn(async () => {}),
    runPushTest: vi.fn(async () => {}),
    $state: {},
    $subscribe: vi.fn()
  }))
}))

vi.mock('@/modules/globalStore', () => ({
  useGlobalStore: vi.fn(() => ({
    initialized: true,
    disabled: false,
    configChanged: false,
    messageError: '',
    messageWarning: '',
    messageSuccess: '',
    messageInfo: '',
    platform: 'ESP32',
    app_ver: '1.0.0',
    app_build: '1',
    registerBaseUrl: 'https://api.test.gravitymon.com/',
    registerApiKey: 'test-key',
    clearMessages: vi.fn(),
    anonymizeChipId: vi.fn(async () => 'anon-chip-123'),
    // UI toggles and feature flags used by views
    ui: {
      enableVoltageFragment: true,
      enableManualWifiEntry: true,
      enableScanForStrongestAp: true,
      enableDeviceRegistration: true,
      enableCalibrationRegistration: true,
      enableGravity: true,
      enablePressure: false,
      enableHttpPostTcpMode: false
    },
    feature: {
      ble: true,
      velocity: true,
      filter: true,
      charging: true
    },
    isError: false,
    isWarning: false,
    isSuccess: false,
    isInfo: false,
    $state: {}
  }))
}))

vi.mock('@/modules/statusStore', () => ({
  useStatusStore: vi.fn(() => ({
    id: '',
    angle: 0,
    gravity: 0,
    temp: 0,
    battery: 0,
    rssi: 0,
    needsCalibration: false,
    updateFromJson: vi.fn(),
    wifi_setup: false,
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
    // lifecycle helpers used by views
    load: vi.fn(async () => true),
    getGyro: vi.fn(async () => ({ success: true, data: { angle: 0 } })),
    $state: {}
  }))
}))

// Mock external dependencies (provide commonly used exports)
vi.mock('@mp-se/espframework-ui-components', () => ({
  // logging
  logError: vi.fn(),
  logDebug: vi.fn(),
  logInfo: vi.fn(),

  // util conversions
  tempToF: (c) => (c * 9) / 5 + 32,
  tempToC: (f) => ((f - 32) * 5) / 9,
  gravityToPlato: (g) => g,
  gravityToSG: (g) => g,
  roundVal: (val, precision) => parseFloat(val.toFixed(precision)),

  // form util
  validateCurrentForm: vi.fn(() => true),

  // timer helpers
  useTimers: () => ({
    createInterval: () => ({
      start: () => {},
      stop: () => {}
    }),
    createTimeout: vi.fn((cb, t) => setTimeout(cb, t))
  }),
  useFetch: () => ({
    managedFetch: () => ({})
  }),

  // HTTP helpers used across the app
  sharedHttpClient: {
    getJson: vi.fn(),
    postJson: vi.fn(),
    putJson: vi.fn(),
    deleteJson: vi.fn(),
    request: vi.fn(async () => ({ ok: true })),
    filesystemRequest: vi.fn(async () => ({ success: false })),
    uploadFile: vi.fn(async () => ({ success: true })),
    // websocket helper used by SerialView — returns object with close and socketGetter()
    createWebSocket: vi.fn((path, handlers = {}) => {
      const ws = {
        close: () => {
          if (handlers.onClose) handlers.onClose()
        },
        socketGetter: () => ({
          close: () => {
            if (handlers.onClose) handlers.onClose()
          },
          send: (data) => {
            if (handlers.onMessage) handlers.onMessage({ data })
          }
        })
      }
      // call onOpen immediately to simulate an open socket
      if (handlers.onOpen) handlers.onOpen()
      return ws
    })
  },

  // UI component placeholders (exported names used in main.js and views)
  BsMessage: {},
  BsCard: {},
  BsFileUpload: {},
  BsProgress: {},
  BsInputBase: {},
  BsInputText: {},
  BsInputReadonly: {},
  BsSelect: {},
  BsInputTextArea: {},
  BsInputNumber: {},
  BsInputSwitch: {},
  BsInputRadio: {},
  BsDropdown: {},
  BsModal: {},
  BsModalConfirm: {},
  BsInputTextAreaFormat: {},
  BsMenuBar: {},
  BsFooter: {},
  BsInput: {},
  BsButton: {},

  // Icons (placeholders)
  IconHome: {},
  IconTools: {},
  IconGraphUpArrow: {},
  IconCloudUpArrow: {},
  IconUpArrow: {},
  IconCpu: {},
  IconWifi: {},
  IconEye: {},
  IconEyeSlash: {},
  IconCheckCircle: {},
  IconXCircle: {},
  IconExclamationTriangle: {},
  IconInfoCircle: {},
  // export a version string used by App.onMounted()
  version: 'test-1.0.0',
  // convenience helpers used by views
  createTimeout: vi.fn((cb, t) => setTimeout(cb, t)),
  createInterval: vi.fn(() => ({ start: () => {}, stop: () => {} }))
}))

// Mock crypto-js
vi.mock('crypto-js/sha256', () => ({
  default: vi.fn((val) => val)
}))

// Set up global variables for testing
global.__REGISTER_API_KEY__ = btoa('test-key')
global.import = {
  meta: {
    env: {
      VITE_APP_VERSION: '1.0.0-test',
      VITE_APP_BUILD: '1',
      VITE_APP_REGISTER_BASEURL: 'https://api.test.gravitymon.com/'
    }
  }
}

// Mock anonymize function used by some fragments
global.anonymizeChipId = vi.fn(async () => 'anon-chip-123')

// NOTE: Avoid mocking '@/modules/formula' globally here because module's
// unit tests rely on the real implementation. Fragment tests may mock
// `@/modules/formula` locally when needed.

// Stub UI components from the espframework UI library used across fragments
vtConfig.global = vtConfig.global || {}
vtConfig.global.components = vtConfig.global.components || {}
const uiStubs = [
  'BsInputReadonly',
  'BsInputNumber',
  'BsProgress',
  'BsFileUpload',
  'BsModalConfirm',
  'BsInput',
  'BsButton',
  'BsMessage',
  'BsCard',
  'BsInputText',
  'BsInputSwitch',
  'BsInputRadio',
  'BsDropdown',
  'BsModal',
  'BsInputTextAreaFormat',
  'BsSelect',
  'BsMenuBar',
  'BsFooter',
  'router-link',
  'router-view',
  'RegisterDeviceFragment',
  'RegisterCalibrationFragment',
  'VoltageFragment',
  'ListFilesFragment',
  'AdvancedFilesFragment',
  'EnableCorsFragment'
]
uiStubs.forEach((name) => {
  vtConfig.global.components[name] = {
    template: '<div />'
  }
})

// Mock Chart.js to avoid canvas/context issues in JSDOM
vi.mock('chart.js', () => ({
  Chart: class {
    constructor(_ctx, _cfg) {
      // no-op: do not attempt to access canvas context in tests
      this.ctx = null
      this.config = _cfg
    }
    update() {}
    destroy() {}
  }
}))

vi.mock('chart.js/auto', () => ({}))
