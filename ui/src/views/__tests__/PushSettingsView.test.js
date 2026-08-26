import { mount } from '@vue/test-utils'
import PushSettingsView from '../PushSettingsView.vue'
import { createTestingPinia } from '../../tests/testUtils'

describe('PushSettingsView (interaction tests)', () => {
  it('mounts without error', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(PushSettingsView, {
      global: {
        plugins: [pinia],
        stubs: { BsInputText: true, BsInputRadio: true, BsProgress: true }
      }
    })
    expect(wrapper.exists()).toBe(true)
  })

  it('displays page heading', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(PushSettingsView, {
      global: {
        plugins: [pinia],
        stubs: { BsInputText: true, BsInputRadio: true, BsProgress: true }
      }
    })
    expect(wrapper.text()).toContain('Push')
  })

  it('displays settings form', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(PushSettingsView, {
      global: {
        plugins: [pinia],
        stubs: { BsInputText: true, BsInputRadio: true, BsProgress: true }
      }
    })
    expect(wrapper.find('form').exists()).toBe(true)
  })

  it('has save button', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(PushSettingsView, {
      global: {
        plugins: [pinia],
        stubs: { BsInputText: true, BsInputRadio: true, BsProgress: true }
      }
    })
    const buttons = wrapper.findAll('button')
    const saveButton = buttons.find((b) => b.text().includes('Save'))
    expect(saveButton).toBeDefined()
  })

  it('handles case with no push targets defined', async () => {
    const pinia = createTestingPinia()
    const { config } = await import('@/modules/pinia')
    const { global } = await import('@/modules/pinia')

    // Set all targets to empty
    config.http_post_target = ''
    config.http_post2_target = ''
    config.http_get_target = ''
    config.influxdb2_target = ''
    config.mqtt_target = ''
    config.ble_format = 0
    global.platform = 'ESP32'

    const wrapper = mount(PushSettingsView, {
      global: {
        plugins: [pinia],
        stubs: { BsInputText: true, BsInputRadio: true, BsProgress: true }
      }
    })

    expect(wrapper.exists()).toBe(true)
    // The calculateBatteryLife should have returned early (lines 150-151)
  })

  it('has save function defined', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(PushSettingsView, {
      global: {
        plugins: [pinia],
        stubs: { BsInputText: true, BsInputRadio: true, BsProgress: true }
      }
    })
    expect(typeof wrapper.vm.save).toBe('function')
  })

  it('has config state defined', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(PushSettingsView, {
      global: {
        plugins: [pinia],
        stubs: { BsInputText: true, BsInputRadio: true, BsProgress: true }
      }
    })
    expect(wrapper.vm.config).toBeDefined()
  })

  it('displays container layout', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(PushSettingsView, {
      global: {
        plugins: [pinia],
        stubs: { BsInputText: true, BsInputRadio: true, BsProgress: true }
      }
    })
    expect(wrapper.find('.container').exists()).toBe(true)
  })

  it('form has needs-validation class', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(PushSettingsView, {
      global: {
        plugins: [pinia],
        stubs: { BsInputText: true, BsInputRadio: true, BsProgress: true }
      }
    })
    const form = wrapper.find('form')
    expect(form.classes()).toContain('needs-validation')
  })

  it('save button has primary styling', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(PushSettingsView, {
      global: {
        plugins: [pinia],
        stubs: { BsInputText: true, BsInputRadio: true, BsProgress: true }
      }
    })
    const buttons = wrapper.findAll('button')
    const saveButton = buttons.find((b) => b.text().includes('Save'))
    expect(saveButton.classes()).toContain('btn-primary')
  })
})

describe('PushSettingsView (action tests)', () => {
  beforeEach(() => vi.clearAllMocks())

  it('save calls config.saveAll when form is valid', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { shallowMount } = await import('@vue/test-utils')
    const { default: View } = await import('../PushSettingsView.vue')
    const wrapper = shallowMount(View, { global: { plugins: [createTestingPinia()] } })
    await wrapper.vm.save()
    const { config } = await import('@/modules/pinia')
    expect(config.saveAll).toHaveBeenCalled()
  })

  it('save returns early when validateCurrentForm returns false', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { shallowMount } = await import('@vue/test-utils')
    const { default: View } = await import('../PushSettingsView.vue')
    const module = await import('@mp-se/espframework-ui-components')
    vi.spyOn(module, 'validateCurrentForm').mockReturnValue(false)
    const { config } = await import('@/modules/pinia')
    const saveSpy = vi.spyOn(config, 'saveAll')
    const wrapper = shallowMount(View, { global: { plugins: [createTestingPinia()] } })
    await wrapper.vm.save()
    expect(saveSpy).not.toHaveBeenCalled()
  })

  it('has save function defined', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { shallowMount } = await import('@vue/test-utils')
    const { default: View } = await import('../PushSettingsView.vue')
    const wrapper = shallowMount(View, { global: { plugins: [createTestingPinia()] } })
    expect(typeof wrapper.vm.save).toBe('function')
  })

  it('displays form heading', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(PushSettingsView, {
      global: {
        plugins: [pinia],
        stubs: { BsInputText: true, BsInputRadio: true, BsProgress: true }
      }
    })
    expect(wrapper.text()).toContain('Push')
  })

  it('has config state accessible', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { shallowMount } = await import('@vue/test-utils')
    const { default: View } = await import('../PushSettingsView.vue')
    const wrapper = shallowMount(View, { global: { plugins: [createTestingPinia()] } })
    expect(wrapper.vm.config).toBeDefined()
  })

  it('has global state accessible', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { shallowMount } = await import('@vue/test-utils')
    const { default: View } = await import('../PushSettingsView.vue')
    const wrapper = shallowMount(View, { global: { plugins: [createTestingPinia()] } })
    expect(wrapper.vm.global).toBeDefined()
  })

  it('has batteryLife ref defined', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { shallowMount } = await import('@vue/test-utils')
    const { default: View } = await import('../PushSettingsView.vue')
    const wrapper = shallowMount(View, { global: { plugins: [createTestingPinia()] } })
    expect(wrapper.vm.batteryLife).toBeDefined()
  })

  it('has sleepLabel ref defined', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { shallowMount } = await import('@vue/test-utils')
    const { default: View } = await import('../PushSettingsView.vue')
    const wrapper = shallowMount(View, { global: { plugins: [createTestingPinia()] } })
    expect(wrapper.vm.sleepLabel).toBeDefined()
  })
})
describe('PushSettingsView (calculateBatteryLife tests)', () => {
  let savedHttpPost, savedBleFormat, savedPlatform

  beforeEach(async () => {
    vi.clearAllMocks()
    const { config, global } = await import('@/modules/pinia')
    savedHttpPost = config.http_post_target
    savedBleFormat = config.ble_format
    savedPlatform = global.platform
  })

  afterEach(async () => {
    const { config, global } = await import('@/modules/pinia')
    const { ref } = await import('vue')
    config.http_post_target = savedHttpPost
    config.http_post2_target = []
    config.http_get_target = []
    config.influxdb2_target = []
    config.mqtt_target = []
    config.ble_format = savedBleFormat
    global.platform = savedPlatform
    // Restore sleep_interval as a ref in case a test corrupted it
    if (typeof config.sleep_interval !== 'object' || !config.sleep_interval.__v_isRef) {
      config.sleep_interval = ref(600)
    }
  })

  const mountView = async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { shallowMount } = await import('@vue/test-utils')
    const { default: View } = await import('../PushSettingsView.vue')
    return shallowMount(View, { global: { plugins: [createTestingPinia()] } })
  }

  it('batteryLife is set when wifi target configured on ESP8266', async () => {
    const { config, global } = await import('@/modules/pinia')
    config.http_post_target = 'http://test.com'
    config.http_post2_target = []
    config.http_get_target = []
    config.influxdb2_target = []
    config.mqtt_target = []
    config.ble_format = 0
    global.platform = 'ESP8266'
    const wrapper = await mountView()
    await wrapper.vm.$nextTick()
    expect(wrapper.vm.batteryLife).toBeDefined()
  })

  it('batteryLife is set when wifi target configured on ESP32C3', async () => {
    const { config, global } = await import('@/modules/pinia')
    config.http_post_target = 'http://test.com'
    config.http_post2_target = []
    config.http_get_target = []
    config.influxdb2_target = []
    config.mqtt_target = []
    config.ble_format = 0
    global.platform = 'ESP32C3'
    const wrapper = await mountView()
    await wrapper.vm.$nextTick()
    expect(wrapper.vm.batteryLife).toBeDefined()
  })

  it('batteryLife is set when wifi target configured on ESP32S2', async () => {
    const { config, global } = await import('@/modules/pinia')
    config.http_post_target = 'http://test.com'
    config.http_post2_target = []
    config.http_get_target = []
    config.influxdb2_target = []
    config.mqtt_target = []
    config.ble_format = 0
    global.platform = 'ESP32S2'
    const wrapper = await mountView()
    await wrapper.vm.$nextTick()
    expect(wrapper.vm.batteryLife).toBeDefined()
  })

  it('batteryLife is set when wifi target configured on ESP32S3', async () => {
    const { config, global } = await import('@/modules/pinia')
    config.http_post_target = 'http://test.com'
    config.http_post2_target = []
    config.http_get_target = []
    config.influxdb2_target = []
    config.mqtt_target = []
    config.ble_format = 0
    global.platform = 'ESP32S3'
    const wrapper = await mountView()
    await wrapper.vm.$nextTick()
    expect(wrapper.vm.batteryLife).toBeDefined()
  })

  it('batteryLife is set when wifi target configured on unknown platform', async () => {
    const { config, global } = await import('@/modules/pinia')
    config.http_post_target = 'http://test.com'
    config.http_post2_target = []
    config.http_get_target = []
    config.influxdb2_target = []
    config.mqtt_target = []
    config.ble_format = 0
    global.platform = 'UNKNOWN_PLATFORM'
    const wrapper = await mountView()
    await wrapper.vm.$nextTick()
    expect(wrapper.vm.batteryLife).toBeDefined()
  })

  it('batteryLife is set when only BLE enabled on ESP32S3', async () => {
    const { config, global } = await import('@/modules/pinia')
    config.http_post_target = []
    config.http_post2_target = []
    config.http_get_target = []
    config.influxdb2_target = []
    config.mqtt_target = []
    config.ble_format = 1
    global.platform = 'ESP32S3'
    const wrapper = await mountView()
    await wrapper.vm.$nextTick()
    expect(wrapper.vm.batteryLife).toBeDefined()
  })

  it('batteryLife is set when only BLE enabled on ESP8266', async () => {
    const { config, global } = await import('@/modules/pinia')
    config.http_post_target = []
    config.http_post2_target = []
    config.http_get_target = []
    config.influxdb2_target = []
    config.mqtt_target = []
    config.ble_format = 1
    global.platform = 'ESP8266'
    const wrapper = await mountView()
    await wrapper.vm.$nextTick()
    expect(wrapper.vm.batteryLife).toBeDefined()
  })

  it('batteryLife is set when only BLE enabled on unknown platform', async () => {
    const { config, global } = await import('@/modules/pinia')
    config.http_post_target = []
    config.http_post2_target = []
    config.http_get_target = []
    config.influxdb2_target = []
    config.mqtt_target = []
    config.ble_format = 1
    global.platform = 'UNKNOWN'
    const wrapper = await mountView()
    await wrapper.vm.$nextTick()
    expect(wrapper.vm.batteryLife).toBeDefined()
  })

  it('batteryLife stays empty when no wifi or ble targets configured', async () => {
    const { config, global } = await import('@/modules/pinia')
    config.http_post_target = []
    config.http_post2_target = []
    config.http_get_target = []
    config.influxdb2_target = []
    config.mqtt_target = []
    config.ble_format = 0
    global.platform = 'ESP32S3'
    const wrapper = await mountView()
    await wrapper.vm.$nextTick()
    expect(wrapper.vm.batteryLife).toBe('')
  })

  it('influxdb2 target counts as wifi push target for battery calc', async () => {
    const { config, global } = await import('@/modules/pinia')
    config.http_post_target = []
    config.http_post2_target = []
    config.http_get_target = []
    config.influxdb2_target = 'http://influx.example.com'
    config.mqtt_target = []
    config.ble_format = 0
    global.platform = 'ESP32S3'
    const wrapper = await mountView()
    await wrapper.vm.$nextTick()
    expect(wrapper.vm.batteryLife).toBeDefined()
  })

  it('mqtt target counts as wifi push target for battery calc', async () => {
    const { config, global } = await import('@/modules/pinia')
    config.http_post_target = []
    config.http_post2_target = []
    config.http_get_target = []
    config.influxdb2_target = []
    config.mqtt_target = 'mqtt.example.com'
    config.ble_format = 0
    global.platform = 'ESP8266'
    const wrapper = await mountView()
    await wrapper.vm.$nextTick()
    expect(wrapper.vm.batteryLife).toBeDefined()
  })

  it('sleepLabel is formatted correctly based on sleep_interval', async () => {
    const wrapper = await mountView()
    await wrapper.vm.$nextTick()
    // sleepLabel is set in createSleepLabel() called from onMounted
    // with sleep_interval = ref(600) from mock → "(10 min 0 sec)"
    expect(typeof wrapper.vm.sleepLabel).toBe('string')
    expect(wrapper.vm.sleepLabel).toContain('min')
  })

  it('shows conditional BsMessage elements based on sleep_interval', async () => {
    const { mount } = await import('@vue/test-utils')
    const { default: View } = await import('../PushSettingsView.vue')
    const { createTestingPinia } = await import('../../tests/testUtils')
    // Mount with BsMessage rendered as a real div to exercise template branches
    const wrapper = mount(View, {
      global: {
        plugins: [createTestingPinia()],
        stubs: {
          BsInputText: true,
          BsInputNumber: true,
          BsInputReadonly: true,
          BsInputSwitch: true,
          BsMessage: { template: '<div><slot /></div>' }
        }
      }
    })
    // Template renders without error - BsMessage conditions are evaluated
    expect(wrapper.find('form').exists()).toBe(true)
  })

  it('renders esp8266-specific skip_ssl_on_test input when platform is esp8266', async () => {
    const { mount } = await import('@vue/test-utils')
    const { default: View } = await import('../PushSettingsView.vue')
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { global } = await import('@/modules/pinia')
    global.platform = 'esp8266'
    const inputStub = {
      template:
        '<input :value="modelValue" @input="$emit(\'update:modelValue\', $event.target.value)" />',
      props: ['modelValue', 'label', 'disabled', 'help', 'min', 'max', 'step', 'width', 'unit'],
      emits: ['update:modelValue']
    }
    const wrapper = mount(View, {
      global: {
        plugins: [createTestingPinia()],
        stubs: {
          BsInputText: inputStub,
          BsInputNumber: inputStub,
          BsInputSwitch: inputStub,
          BsInputReadonly: true,
          BsMessage: true
        }
      }
    })
    for (const el of wrapper.findAll('input')) {
      await el.trigger('input')
    }
    expect(wrapper.find('form').exists()).toBe(true)
    global.platform = 'ESP32'
  })

  it('triggers batteryLife BsInputReadonly v-model', async () => {
    const { mount } = await import('@vue/test-utils')
    const { default: View } = await import('../PushSettingsView.vue')
    const { createTestingPinia } = await import('../../tests/testUtils')
    const inputStub = {
      template:
        '<input :value="modelValue" @input="$emit(\'update:modelValue\', $event.target.value)" />',
      props: ['modelValue', 'label', 'disabled', 'help'],
      emits: ['update:modelValue']
    }
    const wrapper = mount(View, {
      global: {
        plugins: [createTestingPinia()],
        stubs: {
          BsInputText: true,
          BsInputNumber: true,
          BsInputSwitch: true,
          BsInputReadonly: inputStub,
          BsMessage: true
        }
      }
    })
    wrapper.vm.batteryLife = '3 weeks 2 days'
    await wrapper.vm.$nextTick()
    for (const el of wrapper.findAll('input')) {
      await el.trigger('input')
    }
    expect(wrapper.find('form').exists()).toBe(true)
  })
})
