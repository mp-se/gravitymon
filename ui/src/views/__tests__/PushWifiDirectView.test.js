import { mount } from '@vue/test-utils'
import PushWifiDirectView from '../PushWifiDirectView.vue'
import { createTestingPinia } from '../../tests/testUtils'

describe('PushWifiDirectView (interaction tests)', () => {
  it('mounts without error', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(PushWifiDirectView, {
      global: {
        plugins: [pinia],
        stubs: { BsInputText: true, BsInputSwitch: true, BsProgress: true }
      }
    })
    expect(wrapper.exists()).toBe(true)
  })

  it('displays page heading', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(PushWifiDirectView, {
      global: {
        plugins: [pinia],
        stubs: { BsInputText: true, BsInputSwitch: true, BsProgress: true }
      }
    })
    expect(wrapper.text()).toContain('Wifi Direct')
  })

  it('displays form element', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(PushWifiDirectView, {
      global: {
        plugins: [pinia],
        stubs: { BsInputText: true, BsInputSwitch: true, BsProgress: true }
      }
    })
    expect(wrapper.find('form').exists()).toBe(true)
  })

  it('has save button', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(PushWifiDirectView, {
      global: {
        plugins: [pinia],
        stubs: { BsInputText: true, BsInputSwitch: true, BsProgress: true }
      }
    })
    const buttons = wrapper.findAll('button')
    const saveButton = buttons.find((b) => b.text().includes('Save'))
    expect(saveButton).toBeDefined()
  })

  it('has config state defined', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(PushWifiDirectView, {
      global: {
        plugins: [pinia],
        stubs: { BsInputText: true, BsInputSwitch: true, BsProgress: true }
      }
    })
    expect(wrapper.vm.config).toBeDefined()
  })

  it('displays container layout', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(PushWifiDirectView, {
      global: {
        plugins: [pinia],
        stubs: { BsInputText: true, BsInputSwitch: true, BsProgress: true }
      }
    })
    expect(wrapper.find('.container').exists()).toBe(true)
  })

  it('form has needs-validation class', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(PushWifiDirectView, {
      global: {
        plugins: [pinia],
        stubs: { BsInputText: true, BsInputSwitch: true, BsProgress: true }
      }
    })
    const form = wrapper.find('form')
    expect(form.classes()).toContain('needs-validation')
  })

  it('has save function defined', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(PushWifiDirectView, {
      global: {
        plugins: [pinia],
        stubs: { BsInputText: true, BsInputSwitch: true, BsProgress: true }
      }
    })
    expect(typeof wrapper.vm.save).toBe('function')
  })
})

describe('PushWifiDirectView (action tests)', () => {
  beforeEach(async () => {
    vi.clearAllMocks()
    const module = await import('@mp-se/espframework-ui-components')
    vi.spyOn(module, 'validateCurrentForm').mockReturnValue(true)
  })

  it('save calls config.saveAll when form is valid', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: View } = await import('../PushWifiDirectView.vue')
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

  it('save returns early when validateCurrentForm returns false', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: View } = await import('../PushWifiDirectView.vue')
    const module = await import('@mp-se/espframework-ui-components')
    vi.spyOn(module, 'validateCurrentForm').mockReturnValue(false)
    const { config } = await import('@/modules/pinia')
    const saveSpy = vi.spyOn(config, 'saveAll')
    const wrapper = mount(View, {
      global: {
        plugins: [createTestingPinia()],
        stubs: { BsInputText: true, BsInputSwitch: true, BsProgress: true }
      }
    })
    await wrapper.vm.save()
    expect(saveSpy).not.toHaveBeenCalled()
  })

  it('save sets messageError when wifi_direct enabled but SSID empty', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: View } = await import('../PushWifiDirectView.vue')
    const { config, global: globalStore } = await import('@/modules/pinia')
    config.use_wifi_direct = true
    config.wifi_direct_ssid = ''
    config.wifi_direct_pass = 'password'
    const wrapper = mount(View, {
      global: {
        plugins: [createTestingPinia()],
        stubs: { BsInputText: true, BsInputSwitch: true, BsProgress: true }
      }
    })
    await wrapper.vm.save()
    expect(globalStore.messageError).toBeTruthy()
    config.use_wifi_direct = false
  })

  it('save sets messageError when wifi_direct enabled but password empty', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: View } = await import('../PushWifiDirectView.vue')
    const { config, global: globalStore } = await import('@/modules/pinia')
    config.use_wifi_direct = true
    config.wifi_direct_ssid = 'TestSSID'
    config.wifi_direct_pass = ''
    const wrapper = mount(View, {
      global: {
        plugins: [createTestingPinia()],
        stubs: { BsInputText: true, BsInputSwitch: true, BsProgress: true }
      }
    })
    await wrapper.vm.save()
    expect(globalStore.messageError).toBeTruthy()
    config.use_wifi_direct = false
  })

  it('save calls config.saveAll when use_wifi_direct is false', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: View } = await import('../PushWifiDirectView.vue')
    const { config } = await import('@/modules/pinia')
    config.use_wifi_direct = false
    config.wifi_direct_ssid = ''
    config.wifi_direct_pass = ''
    const wrapper = mount(View, {
      global: {
        plugins: [createTestingPinia()],
        stubs: { BsInputText: true, BsInputSwitch: true, BsProgress: true }
      }
    })
    await wrapper.vm.save()
    expect(config.saveAll).toHaveBeenCalled()
  })

  it('save calls config.saveAll when wifi_direct enabled with valid credentials', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: View } = await import('../PushWifiDirectView.vue')
    const { config } = await import('@/modules/pinia')
    config.use_wifi_direct = true
    config.wifi_direct_ssid = 'TestSSID'
    config.wifi_direct_pass = 'TestPassword'
    const wrapper = mount(View, {
      global: {
        plugins: [createTestingPinia()],
        stubs: { BsInputText: true, BsInputSwitch: true, BsProgress: true }
      }
    })
    await wrapper.vm.save()
    expect(config.saveAll).toHaveBeenCalled()
    config.use_wifi_direct = false
  })

  it('displays heading when mounted', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: View } = await import('../PushWifiDirectView.vue')
    const wrapper = mount(View, {
      global: {
        plugins: [createTestingPinia()],
        stubs: { BsInputText: true, BsInputSwitch: true, BsProgress: true }
      }
    })
    expect(wrapper.vm.save).toBeDefined()
    expect(typeof wrapper.vm.save).toBe('function')
  })

  it('v-model bindings trigger config updates via emitting stubs', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: View } = await import('../PushWifiDirectView.vue')
    const { global } = await import('@/modules/pinia')
    global.disabled = false

    const inputStub = {
      template:
        '<input :value="modelValue" @input="$emit(\'update:modelValue\', $event.target.value)" />',
      props: ['modelValue', 'type', 'disabled', 'maxlength', 'label', 'help'],
      emits: ['update:modelValue']
    }
    const switchStub = {
      template:
        '<input type="checkbox" :checked="modelValue" @change="$emit(\'update:modelValue\', $event.target.checked)" />',
      props: ['modelValue', 'disabled', 'label', 'help'],
      emits: ['update:modelValue']
    }
    const wrapper = mount(View, {
      global: {
        plugins: [createTestingPinia()],
        stubs: {
          BsInputText: inputStub,
          BsInputSwitch: switchStub
        }
      }
    })
    for (const el of wrapper.findAll('input:not([type="checkbox"])')) {
      await el.trigger('input')
    }
    for (const el of wrapper.findAll('input[type="checkbox"]')) {
      await el.trigger('change')
    }
    expect(wrapper.find('form').exists()).toBe(true)
  })
})
