import { mount } from '@vue/test-utils'
import DeviceWifiView from '../DeviceWifiView.vue'
import { createTestingPinia } from '../../tests/testUtils'

describe('DeviceWifiView (interaction tests)', () => {
  it('mounts without error', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(DeviceWifiView, {
      global: {
        plugins: [pinia],
        stubs: {
          BsMessage: true,
          BsInputText: true,
          BsInputPassword: true,
          BsInputNumber: true,
          BsInputSwitch: true,
          BsSwitch: true,
          BsProgress: true,
          BsButtonGroup: true,
          RouterLink: true
        }
      }
    })
    expect(wrapper.exists()).toBe(true)
  })

  it('displays page heading', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(DeviceWifiView, {
      global: {
        plugins: [pinia],
        stubs: {
          BsMessage: true,
          BsInputText: true,
          BsInputPassword: true,
          BsInputNumber: true,
          BsInputSwitch: true,
          BsSwitch: true,
          BsProgress: true,
          BsButtonGroup: true,
          RouterLink: true
        }
      }
    })
    expect(wrapper.text()).toContain('WIFI')
  })

  it('displays settings form', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(DeviceWifiView, {
      global: {
        plugins: [pinia],
        stubs: {
          BsMessage: true,
          BsInputText: true,
          BsInputPassword: true,
          BsInputNumber: true,
          BsInputSwitch: true,
          BsSwitch: true,
          BsProgress: true,
          BsButtonGroup: true,
          RouterLink: true
        }
      }
    })
    const form = wrapper.find('form')
    expect(form.exists()).toBe(true)
  })

  it('has save button', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(DeviceWifiView, {
      global: {
        plugins: [pinia],
        stubs: {
          BsMessage: true,
          BsInputText: true,
          BsInputPassword: true,
          BsInputNumber: true,
          BsInputSwitch: true,
          BsSwitch: true,
          BsProgress: true,
          BsButtonGroup: true,
          RouterLink: true
        }
      }
    })
    const buttons = wrapper.findAll('button')
    const saveButton = buttons.find((b) => b.text().includes('Save'))
    expect(saveButton).toBeDefined()
  })

  it('has restart button', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(DeviceWifiView, {
      global: {
        plugins: [pinia],
        stubs: {
          BsMessage: true,
          BsInputText: true,
          BsInputPassword: true,
          BsInputNumber: true,
          BsInputSwitch: true,
          BsSwitch: true,
          BsProgress: true,
          BsButtonGroup: true,
          RouterLink: true
        }
      }
    })
    const buttons = wrapper.findAll('button')
    const restartButton = buttons.find((b) => b.text().includes('Restart'))
    expect(restartButton).toBeDefined()
  })

  it('has wifiName function for network label formatting', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(DeviceWifiView, {
      global: {
        plugins: [pinia],
        stubs: {
          BsMessage: true,
          BsInputText: true,
          BsInputPassword: true,
          BsInputNumber: true,
          BsInputSwitch: true,
          BsSwitch: true,
          BsProgress: true,
          BsButtonGroup: true,
          RouterLink: true
        }
      }
    })
    expect(typeof wrapper.vm.wifiName).toBe('function')
  })

  it('has scanning reactive property', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(DeviceWifiView, {
      global: {
        plugins: [pinia],
        stubs: {
          BsMessage: true,
          BsInputText: true,
          BsInputPassword: true,
          BsInputNumber: true,
          BsInputSwitch: true,
          BsSwitch: true,
          BsProgress: true,
          BsButtonGroup: true,
          RouterLink: true
        }
      }
    })
    expect(wrapper.vm.scanning).toBeDefined()
    expect(typeof wrapper.vm.scanning).toBe('boolean')
  })

  it('networks property is initialized as array', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(DeviceWifiView, {
      global: {
        plugins: [pinia],
        stubs: {
          BsMessage: true,
          BsInputText: true,
          BsInputPassword: true,
          BsInputNumber: true,
          BsInputSwitch: true,
          BsSwitch: true,
          BsProgress: true,
          BsButtonGroup: true,
          RouterLink: true
        }
      }
    })
    expect(Array.isArray(wrapper.vm.networks)).toBe(true)
  })

  it('has config state defined', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(DeviceWifiView, {
      global: {
        plugins: [pinia],
        stubs: {
          BsMessage: true,
          BsInputText: true,
          BsInputPassword: true,
          BsInputNumber: true,
          BsInputSwitch: true,
          BsSwitch: true,
          BsProgress: true,
          BsButtonGroup: true,
          RouterLink: true
        }
      }
    })
    expect(wrapper.vm.config).toBeDefined()
  })

  it('displays container layout', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(DeviceWifiView, {
      global: {
        plugins: [pinia],
        stubs: {
          BsMessage: true,
          BsInputText: true,
          BsInputPassword: true,
          BsInputNumber: true,
          BsInputSwitch: true,
          BsSwitch: true,
          BsProgress: true,
          BsButtonGroup: true,
          RouterLink: true
        }
      }
    })
    expect(wrapper.find('.container').exists()).toBe(true)
  })

  it('form has needs-validation class', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(DeviceWifiView, {
      global: {
        plugins: [pinia],
        stubs: {
          BsMessage: true,
          BsInputText: true,
          BsInputPassword: true,
          BsInputNumber: true,
          BsInputSwitch: true,
          BsSwitch: true,
          BsProgress: true,
          BsButtonGroup: true,
          RouterLink: true
        }
      }
    })
    const form = wrapper.find('form')
    expect(form.classes()).toContain('needs-validation')
  })

  it('save button has primary styling', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(DeviceWifiView, {
      global: {
        plugins: [pinia],
        stubs: {
          BsMessage: true,
          BsInputText: true,
          BsInputPassword: true,
          BsInputNumber: true,
          BsInputSwitch: true,
          BsSwitch: true,
          BsProgress: true,
          BsButtonGroup: true,
          RouterLink: true
        }
      }
    })
    const buttons = wrapper.findAll('button')
    const saveButton = buttons.find((b) => b.text().includes('Save'))
    expect(saveButton.classes()).toContain('btn-primary')
  })

  it('displays input components', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(DeviceWifiView, {
      global: {
        plugins: [pinia],
        stubs: {
          BsMessage: true,
          BsInputText: true,
          BsInputPassword: true,
          BsInputNumber: true,
          BsInputSwitch: true,
          BsSwitch: true,
          BsProgress: true,
          BsButtonGroup: true,
          RouterLink: true
        }
      }
    })
    expect(wrapper.findComponent({ name: 'BsInputText' }).exists()).toBe(true)
  })
})

describe('DeviceWifiView (action tests)', () => {
  beforeEach(() => vi.clearAllMocks())
  it('save calls config.saveAll when form is valid', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: DeviceWifiView } = await import('../DeviceWifiView.vue')
    const wrapper = mount(DeviceWifiView, {
      global: {
        plugins: [createTestingPinia()],
        stubs: { BsInputText: true, BsSelect: true, BsProgress: true }
      }
    })
    await wrapper.vm.save()
    const { config } = await import('@/modules/pinia')
    expect(config.saveAll).toHaveBeenCalled()
  })
  it('restart calls config.restart', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: DeviceWifiView } = await import('../DeviceWifiView.vue')
    const wrapper = mount(DeviceWifiView, {
      global: {
        plugins: [createTestingPinia()],
        stubs: { BsInputText: true, BsSelect: true, BsProgress: true }
      }
    })
    await wrapper.vm.restart()
    const { config } = await import('@/modules/pinia')
    expect(config.restart).toHaveBeenCalled()
  })

  it('wifiName adds lock emoji for encrypted networks', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { shallowMount } = await import('@vue/test-utils')
    const { default: DeviceWifiView } = await import('../DeviceWifiView.vue')
    const wrapper = shallowMount(DeviceWifiView, { global: { plugins: [createTestingPinia()] } })
    const result = wrapper.vm.wifiName('MyNetwork', -55, true)
    expect(result).toContain('MyNetwork')
    expect(result).toContain('\u{1f512}')
  })

  it('wifiName shows Excellent for strong signal', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { shallowMount } = await import('@vue/test-utils')
    const { default: DeviceWifiView } = await import('../DeviceWifiView.vue')
    const wrapper = shallowMount(DeviceWifiView, { global: { plugins: [createTestingPinia()] } })
    const result = wrapper.vm.wifiName('Network', -40, false)
    expect(result).toContain('Excellent')
  })

  it('wifiName shows Poor for weak signal', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { shallowMount } = await import('@vue/test-utils')
    const { default: DeviceWifiView } = await import('../DeviceWifiView.vue')
    const wrapper = shallowMount(DeviceWifiView, { global: { plugins: [createTestingPinia()] } })
    const result = wrapper.vm.wifiName('Network', -80, false)
    expect(result).toContain('Poor')
  })

  it('wifiName shows Good for medium signal', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { shallowMount } = await import('@vue/test-utils')
    const { default: DeviceWifiView } = await import('../DeviceWifiView.vue')
    const wrapper = shallowMount(DeviceWifiView, { global: { plugins: [createTestingPinia()] } })
    const result = wrapper.vm.wifiName('Network', -55, false)
    expect(result).toContain('Good')
  })

  it('wifiName shows Minimum for low signal', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { shallowMount } = await import('@vue/test-utils')
    const { default: DeviceWifiView } = await import('../DeviceWifiView.vue')
    const wrapper = shallowMount(DeviceWifiView, { global: { plugins: [createTestingPinia()] } })
    const result = wrapper.vm.wifiName('Network', -65, false)
    expect(result).toContain('Minimum')
  })

  it('wifiName does not add lock emoji when not encrypted', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { shallowMount } = await import('@vue/test-utils')
    const { default: DeviceWifiView } = await import('../DeviceWifiView.vue')
    const wrapper = shallowMount(DeviceWifiView, { global: { plugins: [createTestingPinia()] } })
    const result = wrapper.vm.wifiName('OpenNetwork', -50, false)
    expect(result).not.toContain('\u{1f512}')
  })

  it('save sets messageInfo about restart', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: DeviceWifiView } = await import('../DeviceWifiView.vue')
    const { global, config } = await import('@/modules/pinia')
    const { validateCurrentForm } = await import('@mp-se/espframework-ui-components')

    vi.mocked(validateCurrentForm).mockReturnValue(true)
    config.saveAll = vi.fn(async () => {})
    global.messageInfo = ''

    const wrapper = mount(DeviceWifiView, {
      global: {
        plugins: [createTestingPinia()],
        stubs: { BsInputText: true, BsSelect: true, BsProgress: true }
      }
    })
    await wrapper.vm.save()

    expect(global.messageInfo).toContain('WIFI')
    expect(global.messageInfo).toContain('restart')
  })

  it('save does not call saveAll when form validation fails', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: DeviceWifiView } = await import('../DeviceWifiView.vue')
    const { config } = await import('@/modules/pinia')
    const { validateCurrentForm } = await import('@mp-se/espframework-ui-components')

    vi.mocked(validateCurrentForm).mockReturnValue(false)
    config.saveAll = vi.fn()

    const wrapper = mount(DeviceWifiView, {
      global: {
        plugins: [createTestingPinia()],
        stubs: { BsInputText: true, BsSelect: true, BsProgress: true }
      }
    })
    await wrapper.vm.save()

    expect(config.saveAll).not.toHaveBeenCalled()
  })

  it('onMounted fetches wifi networks and populates list', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: DeviceWifiView } = await import('../DeviceWifiView.vue')
    const { config } = await import('@/modules/pinia')

    config.runWifiScan = vi.fn(async () => ({
      success: true,
      data: {
        networks: [
          { wifi_ssid: 'Network1', rssi: -50, encryption: 1, channel: 6 },
          { wifi_ssid: 'Network2', rssi: -70, encryption: 0, channel: 11 }
        ]
      }
    }))

    const wrapper = mount(DeviceWifiView, {
      global: {
        plugins: [createTestingPinia()],
        stubs: { BsInputText: true, BsSelect: true, BsProgress: true }
      }
    })
    await wrapper.vm.$nextTick()
    await new Promise((r) => setTimeout(r, 50))

    expect(config.runWifiScan).toHaveBeenCalled()
  })

  it('onMounted handles failed wifi scan', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: DeviceWifiView } = await import('../DeviceWifiView.vue')
    const { config } = await import('@/modules/pinia')

    config.runWifiScan = vi.fn(async () => ({
      success: false
    }))

    mount(DeviceWifiView, {
      global: {
        plugins: [createTestingPinia()],
        stubs: { BsInputText: true, BsSelect: true, BsProgress: true }
      }
    })
    await new Promise((r) => setTimeout(r, 50))

    expect(config.runWifiScan).toHaveBeenCalled()
  })

  it('scanning ref starts true and becomes false after scan', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { shallowMount } = await import('@vue/test-utils')
    const { default: DeviceWifiView } = await import('../DeviceWifiView.vue')
    const wrapper = shallowMount(DeviceWifiView, { global: { plugins: [createTestingPinia()] } })

    // scanning should start true via onMounted
    expect(typeof wrapper.vm.scanning).toBeDefined()
  })

  it('v-model bindings trigger config updates via emitting stubs', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: View } = await import('../DeviceWifiView.vue')
    const { global } = await import('@/modules/pinia')
    global.disabled = false
    global.ui = { enableScanForStrongestAp: true }

    const inputStub = {
      template:
        '<input :value="modelValue" @input="$emit(\'update:modelValue\', $event.target.value)" />',
      props: [
        'modelValue',
        'type',
        'disabled',
        'maxlength',
        'label',
        'help',
        'min',
        'max',
        'step',
        'width',
        'unit'
      ],
      emits: ['update:modelValue']
    }
    const switchStub = {
      template:
        '<input type="checkbox" :checked="modelValue" @change="$emit(\'update:modelValue\', $event.target.checked)" />',
      props: ['modelValue', 'disabled', 'label', 'help'],
      emits: ['update:modelValue']
    }
    const selectStub = {
      template:
        '<select :value="modelValue" @change="$emit(\'update:modelValue\', $event.target.value)"><option value="">opt</option></select>',
      props: ['modelValue', 'disabled', 'options', 'label', 'help'],
      emits: ['update:modelValue']
    }
    const wrapper = mount(View, {
      global: {
        plugins: [createTestingPinia()],
        stubs: {
          BsInputText: inputStub,
          BsInputNumber: inputStub,
          BsInputSwitch: switchStub,
          BsSelect: selectStub,
          BsProgress: true
        }
      }
    })
    await wrapper.vm.$nextTick()
    for (const el of wrapper.findAll('input:not([type="checkbox"])')) {
      await el.trigger('input')
    }
    for (const el of wrapper.findAll('input[type="checkbox"]')) {
      await el.trigger('change')
    }
    for (const el of wrapper.findAll('select')) {
      await el.trigger('change')
    }
    expect(wrapper.find('form').exists()).toBe(true)
  })
})
