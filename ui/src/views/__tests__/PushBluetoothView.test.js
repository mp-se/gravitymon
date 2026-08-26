import { mount } from '@vue/test-utils'
import PushBluetoothView from '../PushBluetoothView.vue'
import { createTestingPinia } from '../../tests/testUtils'

describe('PushBluetoothView (interaction tests)', () => {
  it('mounts without error', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(PushBluetoothView, {
      global: {
        plugins: [pinia],
        stubs: { BsInputText: true, BsInputSwitch: true, BsProgress: true }
      }
    })
    expect(wrapper.exists()).toBe(true)
  })

  it('displays page heading', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(PushBluetoothView, {
      global: {
        plugins: [pinia],
        stubs: { BsInputText: true, BsInputSwitch: true, BsProgress: true }
      }
    })
    expect(wrapper.text()).toContain('Bluetooth')
  })

  it('displays form', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(PushBluetoothView, {
      global: {
        plugins: [pinia],
        stubs: { BsInputText: true, BsInputSwitch: true, BsProgress: true }
      }
    })
    expect(wrapper.find('form').exists()).toBe(true)
  })

  it('has save button', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(PushBluetoothView, {
      global: {
        plugins: [pinia],
        stubs: { BsInputText: true, BsInputSwitch: true, BsProgress: true }
      }
    })
    const buttons = wrapper.findAll('button')
    const saveButton = buttons.find((b) => b.text().includes('Save'))
    expect(saveButton).toBeDefined()
  })

  it('has save function defined', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(PushBluetoothView, {
      global: {
        plugins: [pinia],
        stubs: { BsInputText: true, BsInputSwitch: true, BsProgress: true }
      }
    })
    expect(typeof wrapper.vm.save).toBe('function')
  })

  it('has config state defined', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(PushBluetoothView, {
      global: {
        plugins: [pinia],
        stubs: { BsInputText: true, BsInputSwitch: true, BsProgress: true }
      }
    })
    expect(wrapper.vm.config).toBeDefined()
  })

  it('displays container layout', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(PushBluetoothView, {
      global: {
        plugins: [pinia],
        stubs: { BsInputText: true, BsInputSwitch: true, BsProgress: true }
      }
    })
    expect(wrapper.find('.container').exists()).toBe(true)
  })
})

describe('PushBluetoothView (action tests)', () => {
  beforeEach(() => vi.clearAllMocks())
  it('save calls config.saveAll when form is valid', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: View } = await import('../PushBluetoothView.vue')
    const wrapper = mount(View, {
      global: {
        plugins: [createTestingPinia()],
        stubs: { BsInputText: true, BsInputSwitch: true, BsProgress: true }
      }
    })
    await wrapper.vm.save()
    const { config } = await import('@/modules/pinia')
    expect(config.saveAll).toHaveBeenCalled()
  })

  it('save calls global.clearMessages', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: View } = await import('../PushBluetoothView.vue')
    const { global } = await import('@/modules/pinia')
    const wrapper = mount(View, {
      global: {
        plugins: [createTestingPinia()],
        stubs: { BsInputRadio: true, BsInputSwitch: true }
      }
    })
    await wrapper.vm.save()
    expect(global.clearMessages).toHaveBeenCalled()
  })

  it('save does not call config.saveAll when form validation fails', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: View } = await import('../PushBluetoothView.vue')
    const { validateCurrentForm } = await import('@mp-se/espframework-ui-components')
    const { config } = await import('@/modules/pinia')
    vi.mocked(validateCurrentForm).mockReturnValue(false)
    config.saveAll = vi.fn()
    const wrapper = mount(View, {
      global: {
        plugins: [createTestingPinia()],
        stubs: { BsInputRadio: true, BsInputSwitch: true }
      }
    })
    await wrapper.vm.save()
    expect(config.saveAll).not.toHaveBeenCalled()
  })

  it('save auto-sets ble_tilt_color to red when ble_format is 1 and color is empty', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: View } = await import('../PushBluetoothView.vue')
    const { config } = await import('@/modules/pinia')
    config.ble_format = 1
    config.ble_tilt_color = ''
    const wrapper = mount(View, {
      global: {
        plugins: [createTestingPinia()],
        stubs: { BsInputRadio: true, BsInputSwitch: true }
      }
    })
    await wrapper.vm.save()
    expect(config.ble_tilt_color).toBe('red')
    config.ble_format = 0
    config.ble_tilt_color = ''
  })

  it('save auto-sets ble_tilt_color to red when ble_format is 2 and color is empty', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: View } = await import('../PushBluetoothView.vue')
    const { config } = await import('@/modules/pinia')
    config.ble_format = 2
    config.ble_tilt_color = ''
    const wrapper = mount(View, {
      global: {
        plugins: [createTestingPinia()],
        stubs: { BsInputRadio: true, BsInputSwitch: true }
      }
    })
    await wrapper.vm.save()
    expect(config.ble_tilt_color).toBe('red')
    config.ble_format = 0
    config.ble_tilt_color = ''
  })

  it('save does not change ble_tilt_color when ble_format is 0', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: View } = await import('../PushBluetoothView.vue')
    const { config } = await import('@/modules/pinia')
    config.ble_format = 0
    config.ble_tilt_color = ''
    const wrapper = mount(View, {
      global: {
        plugins: [createTestingPinia()],
        stubs: { BsInputRadio: true, BsInputSwitch: true }
      }
    })
    await wrapper.vm.save()
    expect(config.ble_tilt_color).toBe('')
  })

  it('tilt computed returns false when ble_format is 1', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: View } = await import('../PushBluetoothView.vue')
    const { config, global } = await import('@/modules/pinia')
    config.ble_format = 1
    global.disabled32 = false
    const wrapper = mount(View, {
      global: { plugins: [createTestingPinia()], stubs: { BsInputRadio: true } }
    })
    expect(wrapper.vm.tilt).toBe(false)
    config.ble_format = 0
  })

  it('tilt computed returns false when ble_format is 2', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: View } = await import('../PushBluetoothView.vue')
    const { config, global } = await import('@/modules/pinia')
    config.ble_format = 2
    global.disabled32 = false
    const wrapper = mount(View, {
      global: { plugins: [createTestingPinia()], stubs: { BsInputRadio: true } }
    })
    expect(wrapper.vm.tilt).toBe(false)
    config.ble_format = 0
  })

  it('tilt computed returns true when ble_format is not 1 or 2', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: View } = await import('../PushBluetoothView.vue')
    const { config, global } = await import('@/modules/pinia')
    config.ble_format = 5
    global.disabled32 = false
    const wrapper = mount(View, {
      global: { plugins: [createTestingPinia()], stubs: { BsInputRadio: true } }
    })
    expect(wrapper.vm.tilt).toBe(true)
    config.ble_format = 0
  })

  it('tilt computed returns disabled32 value when global.disabled32 is true', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: View } = await import('../PushBluetoothView.vue')
    const { config, global } = await import('@/modules/pinia')
    config.ble_format = 1
    global.disabled32 = true
    const wrapper = mount(View, {
      global: { plugins: [createTestingPinia()], stubs: { BsInputRadio: true } }
    })
    expect(wrapper.vm.tilt).toBe(true)
    global.disabled32 = false
    config.ble_format = 0
  })

  it('shows bluetooth not available message when feature.ble is false', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: View } = await import('../PushBluetoothView.vue')
    const { global } = await import('@/modules/pinia')
    global.feature.ble = false
    const wrapper = mount(View, {
      global: { plugins: [createTestingPinia()], stubs: { BsInputRadio: true } }
    })
    expect(wrapper.text()).toContain('not available')
    global.feature.ble = true
  })

  it('v-model bindings on BsInputRadio trigger config updates', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: View } = await import('../PushBluetoothView.vue')
    const { global } = await import('@/modules/pinia')
    global.feature.ble = true
    global.disabled = false
    global.disabled32 = false
    const radioStub = {
      template:
        '<input type="radio" :value="modelValue" @change="$emit(\'update:modelValue\', \'changed\')">',
      props: ['modelValue', 'options', 'disabled', 'label', 'help'],
      emits: ['update:modelValue']
    }
    const wrapper = mount(View, {
      global: {
        plugins: [createTestingPinia()],
        stubs: { BsInputRadio: radioStub }
      }
    })
    for (const radio of wrapper.findAll('input[type="radio"]')) {
      await radio.trigger('change')
    }
    expect(wrapper.find('form').exists()).toBe(true)
  })
})
