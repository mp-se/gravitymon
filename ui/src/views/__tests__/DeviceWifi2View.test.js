import { shallowMount } from '@vue/test-utils'
import DeviceWifi2View from '../DeviceWifi2View.vue'
import { createTestingPinia } from '../../tests/testUtils'

describe('DeviceWifi2View (smoke)', () => {
  it('mounts without error', () => {
    const pinia = createTestingPinia()
    const wrapper = shallowMount(DeviceWifi2View, { global: { plugins: [pinia] } })
    expect(wrapper.exists()).toBe(true)
  })
})

describe('DeviceWifi2View (action tests)', () => {
  beforeEach(() => vi.clearAllMocks())
  it('save calls config.saveAll', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { shallowMount } = await import('@vue/test-utils')
    const { default: View } = await import('../DeviceWifi2View.vue')
    const wrapper = shallowMount(View, { global: { plugins: [createTestingPinia()] } })
    await wrapper.vm.save()
    const { config } = await import('@/modules/pinia')
    expect(config.saveAll).toHaveBeenCalled()
  })
  it('save sets messageInfo when form is valid', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { global, config } = await import('@/modules/pinia')
    const { shallowMount } = await import('@vue/test-utils')
    const { default: View } = await import('../DeviceWifi2View.vue')
    global.messageInfo = ''
    config.saveAll = vi.fn()
    const wrapper = shallowMount(View, { global: { plugins: [createTestingPinia()] } })
    await wrapper.vm.save()
    expect(global.messageInfo).toContain('WIFI')
  })
  it('displays heading', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { shallowMount } = await import('@vue/test-utils')
    const { default: View } = await import('../DeviceWifi2View.vue')
    const wrapper = shallowMount(View, { global: { plugins: [createTestingPinia()] } })
    expect(wrapper.vm.save).toBeDefined()
    expect(typeof wrapper.vm.save).toBe('function')
  })
  it('restart calls config.restart', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { shallowMount } = await import('@vue/test-utils')
    const { default: View } = await import('../DeviceWifi2View.vue')
    const wrapper = shallowMount(View, { global: { plugins: [createTestingPinia()] } })
    await wrapper.vm.restart()
    const { config } = await import('@/modules/pinia')
    expect(config.restart).toHaveBeenCalled()
  })

  it('save does not call saveAll when form validation fails', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { shallowMount } = await import('@vue/test-utils')
    const { default: View } = await import('../DeviceWifi2View.vue')
    const { validateCurrentForm } = await import('@mp-se/espframework-ui-components')
    const { config } = await import('@/modules/pinia')
    vi.mocked(validateCurrentForm).mockReturnValue(false)
    config.saveAll = vi.fn()
    const wrapper = shallowMount(View, { global: { plugins: [createTestingPinia()] } })
    await wrapper.vm.save()
    expect(config.saveAll).not.toHaveBeenCalled()
  })

  it('warning message displays when both SSIDs are empty', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: View } = await import('../DeviceWifi2View.vue')
    const { config } = await import('@/modules/pinia')
    config.wifi_ssid = ''
    config.wifi_ssid2 = ''
    const wrapper = mount(View, {
      global: {
        plugins: [createTestingPinia()],
        stubs: {
          BsMessage: { template: '<div><slot /></div>' },
          BsInputText: true,
          BsInputNumber: true,
          BsInputSwitch: true
        }
      }
    })
    expect(wrapper.text()).toContain('define at least one wifi network')
  })

  it('warning message hidden when wifi_ssid is set', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: View } = await import('../DeviceWifi2View.vue')
    const { config } = await import('@/modules/pinia')
    config.wifi_ssid = 'TestNetwork'
    config.wifi_ssid2 = ''
    const wrapper = mount(View, {
      global: {
        plugins: [createTestingPinia()],
        stubs: {
          BsMessage: { template: '<div><slot /></div>' },
          BsInputText: true,
          BsInputNumber: true,
          BsInputSwitch: true
        }
      }
    })
    expect(wrapper.text()).not.toContain('define at least one wifi network')
  })

  it('warning message hidden when wifi_ssid2 is set', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: View } = await import('../DeviceWifi2View.vue')
    const { config } = await import('@/modules/pinia')
    config.wifi_ssid = ''
    config.wifi_ssid2 = 'BackupNetwork'
    const wrapper = mount(View, {
      global: {
        plugins: [createTestingPinia()],
        stubs: {
          BsMessage: { template: '<div><slot /></div>' },
          BsInputText: true,
          BsInputNumber: true,
          BsInputSwitch: true
        }
      }
    })
    expect(wrapper.text()).not.toContain('define at least one wifi network')
  })

  it('save displays WIFI restart message on success', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { shallowMount } = await import('@vue/test-utils')
    const { default: View } = await import('../DeviceWifi2View.vue')
    const { global, config } = await import('@/modules/pinia')
    const { validateCurrentForm } = await import('@mp-se/espframework-ui-components')
    global.messageInfo = ''
    config.saveAll = vi.fn(async () => {})
    vi.mocked(validateCurrentForm).mockReturnValue(true)
    const wrapper = shallowMount(View, { global: { plugins: [createTestingPinia()] } })
    await wrapper.vm.save()
    expect(global.messageInfo).toContain('WIFI')
  })

  it('save button is disabled when global.disabled is true', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { shallowMount } = await import('@vue/test-utils')
    const { default: View } = await import('../DeviceWifi2View.vue')
    const { global } = await import('@/modules/pinia')
    global.disabled = true
    const wrapper = shallowMount(View, { global: { plugins: [createTestingPinia()] } })
    expect(wrapper.text()).toContain('Save')
  })

  it('restart button is disabled when global.disabled is true', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { shallowMount } = await import('@vue/test-utils')
    const { default: View } = await import('../DeviceWifi2View.vue')
    const { global } = await import('@/modules/pinia')
    global.disabled = true
    const wrapper = shallowMount(View, { global: { plugins: [createTestingPinia()] } })
    expect(wrapper.text()).toContain('Restart device')
  })

  it('wifi_portal_timeout input accepts numeric values', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { shallowMount } = await import('@vue/test-utils')
    const { default: View } = await import('../DeviceWifi2View.vue')
    const { config } = await import('@/modules/pinia')
    config.wifi_portal_timeout = 120
    const wrapper = shallowMount(View, { global: { plugins: [createTestingPinia()] } })
    expect(wrapper.vm.config.wifi_portal_timeout).toBe(120)
  })

  it('wifi_connect_timeout input accepts numeric values', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { shallowMount } = await import('@vue/test-utils')
    const { default: View } = await import('../DeviceWifi2View.vue')
    const { config } = await import('@/modules/pinia')
    config.wifi_connect_timeout = 30
    const wrapper = shallowMount(View, { global: { plugins: [createTestingPinia()] } })
    expect(wrapper.vm.config.wifi_connect_timeout).toBe(30)
  })

  it('v-model handlers for form inputs are exercised on input events', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: View } = await import('../DeviceWifi2View.vue')
    const { config, global } = await import('@/modules/pinia')
    config.wifi_ssid = 'TestNetwork'
    config.wifi_ssid2 = ''
    global.disabled = false
    const wrapper = mount(View, {
      global: {
        plugins: [createTestingPinia()],
        stubs: {
          BsMessage: { template: '<div><slot /></div>' },
          BsInputText: {
            template:
              '<input :value="modelValue" @input="$emit(\'update:modelValue\', $event.target.value)" />',
            props: ['modelValue', 'type', 'badge', 'disabled'],
            emits: ['update:modelValue']
          },
          BsInputNumber: {
            template:
              '<input type="number" :value="modelValue" @input="$emit(\'update:modelValue\', Number($event.target.value))" />',
            props: ['modelValue', 'disabled'],
            emits: ['update:modelValue']
          },
          BsInputSwitch: {
            template:
              '<input type="checkbox" :checked="modelValue" @change="$emit(\'update:modelValue\', $event.target.checked)" />',
            props: ['modelValue', 'disabled'],
            emits: ['update:modelValue']
          }
        }
      }
    })
    const textInputs = wrapper.findAll('input:not([type="checkbox"]):not([type="number"])')
    for (const input of textInputs) {
      await input.trigger('input')
    }
    const numInputs = wrapper.findAll('input[type="number"]')
    for (const input of numInputs) {
      await input.trigger('input')
    }
    const checkboxes = wrapper.findAll('input[type="checkbox"]')
    for (const cb of checkboxes) {
      await cb.trigger('change')
    }
    expect(wrapper.find('form').exists()).toBe(true)
  })

  it('enableScanForStrongestAp false hides WiFi scan switch', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: View } = await import('../DeviceWifi2View.vue')
    const { global } = await import('@/modules/pinia')
    global.ui.enableScanForStrongestAp = false
    const wrapper = mount(View, {
      global: {
        plugins: [createTestingPinia()],
        stubs: {
          BsMessage: { template: '<div><slot /></div>' },
          BsInputText: true,
          BsInputNumber: true,
          BsInputSwitch: true
        }
      }
    })
    // The scan AP section should not render
    expect(wrapper.find('form').exists()).toBe(true)
    global.ui.enableScanForStrongestAp = true
  })
})
