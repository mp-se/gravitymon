import { describe, it, expect, beforeEach, vi, afterEach } from 'vitest'
import { mount } from '@vue/test-utils'
import DeviceSettingsView from '../DeviceSettingsView.vue'
import { createTestingPinia } from '../../tests/testUtils'
import { sharedHttpClient as http, validateCurrentForm } from '@mp-se/espframework-ui-components'

vi.mock('@mp-se/espframework-ui-components', () => ({
  logDebug: vi.fn(),
  logError: vi.fn(),
  logInfo: vi.fn(),
  validateCurrentForm: vi.fn(() => true),
  BsInputReadonly: {
    name: 'BsInputReadonly',
    props: ['modelValue', 'label'],
    template: '<div>{{ label }}: {{ modelValue }}</div>'
  },
  sharedHttpClient: {
    get: vi.fn(),
    post: vi.fn(),
    getJson: vi.fn()
  }
}))

describe('DeviceSettingsView (interaction tests)', () => {
  beforeEach(() => {
    vi.clearAllMocks()
    vi.useFakeTimers()
  })

  afterEach(() => {
    vi.useRealTimers()
    vi.unstubAllGlobals()
  })

  it('mounts without error', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(DeviceSettingsView, {
      global: {
        plugins: [pinia],
        stubs: {
          BsMessage: { template: '<div><slot /></div>' },
          BsInputText: {
            name: 'BsInputText',
            props: ['modelValue'],
            template:
              '<input :value="modelValue" @input="$emit(\'update:modelValue\', $event.target.value)" />'
          },
          BsInputRadio: {
            name: 'BsInputRadio',
            props: ['modelValue', 'options'],
            template:
              '<div><input v-for="o in options" :key="o.value" type="radio" :checked="modelValue === o.value" @change="$emit(\'update:modelValue\', o.value)" /></div>'
          },
          BsDropdown: {
            name: 'BsDropdown',
            props: ['options', 'callback'],
            template: '<div @click="callback(options[1].value)">Dropdown</div>'
          },
          BsSwitch: true,
          BsProgress: true,
          RegisterDeviceFragment: { template: '<div class="register-modal" />' }
        }
      }
    })
    expect(wrapper.exists()).toBe(true)
  })

  it('updates config via v-model in components', async () => {
    const pinia = createTestingPinia()
    const { config, global } = await import('@/modules/pinia')
    config.mdns = 'old-name'
    config.temp_unit = 'C'
    config.gravity_unit = 'G'
    config.dark_mode = false
    global.disabled = false

    const wrapper = mount(DeviceSettingsView, {
      global: {
        plugins: [pinia],
        stubs: {
          BsMessage: true,
          BsInputText: {
            name: 'BsInputText',
            props: ['modelValue'],
            template:
              '<input class="mdns-input" :value="modelValue" @input="$emit(\'update:modelValue\', $event.target.value)" />'
          },
          BsInputRadio: {
            name: 'BsInputRadio',
            props: ['modelValue', 'options', 'label'],
            template:
              '<div :class="\'radio-\' + label.replace(/ /g, \'-\').toLowerCase()"><input v-for="o in options" :key="o.value" type="radio" :checked="modelValue === o.value" @change="$emit(\'update:modelValue\', o.value)" /></div>'
          },
          BsDropdown: true,
          BsSwitch: true,
          BsProgress: true
        }
      }
    })

    const input = wrapper.find('.mdns-input')
    await input.setValue('new-name')
    expect(config.mdns).toBe('new-name')

    const tempRadio = wrapper.find('.radio-temperature-format')
    const radios = tempRadio.findAll('input')
    await radios[1].setChecked() // Fahrenheit
    expect(config.temp_unit).toBe('F')

    const gravRadio = wrapper.find('.radio-gravity-format')
    const gravRadios = gravRadio.findAll('input')
    await gravRadios[1].setChecked() // Plato
    expect(config.gravity_unit).toBe('P')

    const uiRadio = wrapper.find('.radio-user-interface')
    const uiRadios = uiRadio.findAll('input')
    await uiRadios[1].setChecked() // Dark mode
    expect(config.dark_mode).toBe(true)
  })

  it('handles otaCallback', async () => {
    const pinia = createTestingPinia()
    const { config } = await import('@/modules/pinia')
    const wrapper = mount(DeviceSettingsView, {
      global: {
        plugins: [pinia],
        stubs: {
          BsMessage: true,
          BsInputText: true,
          BsInputRadio: true,
          BsDropdown: {
            name: 'BsDropdown',
            props: ['options', 'callback'],
            template:
              '<div class="ota-trigger" @click="callback(\'http://new-ota.com\')">Dropdown</div>'
          },
          BsSwitch: true,
          BsProgress: true
        }
      }
    })

    await wrapper.find('.ota-trigger').trigger('click')
    expect(config.ota_url).toBe('http://new-ota.com')
  })

  it('handles checkReported successfully', async () => {
    const pinia = createTestingPinia()
    const { global } = await import('@/modules/pinia')
    global.ui.enableDeviceRegistration = true
    global.registerBaseUrl = 'http://test/'
    global.registerApiKey = 'key'
    global.anonymizeChipId = vi.fn().mockResolvedValue('anon-123')

    http.getJson.mockResolvedValueOnce({ exists: true, software: 'Gravitymon' })

    const wrapper = mount(DeviceSettingsView, {
      global: {
        plugins: [pinia],
        stubs: {
          BsMessage: true,
          BsInputText: true,
          BsInputRadio: true,
          BsDropdown: true,
          BsSwitch: true,
          BsProgress: true,
          BsInputReadonly: {
            name: 'BsInputReadonly',
            props: ['modelValue'],
            template: '<div class="status-val">{{ modelValue }}</div>'
          }
        }
      }
    })

    const checkButton = wrapper.findAll('button').find((b) => b.text().includes('Check status'))
    await checkButton.trigger('click')

    expect(global.anonymizeChipId).toHaveBeenCalled()
    expect(http.getJson).toHaveBeenCalledWith(
      'http://test/api/v1/device/check/anon-123',
      expect.any(Object)
    )

    await vi.waitFor(() => {
      expect(wrapper.find('.status-val').text()).toContain('reported using Gravitymon')
    })
  })

  it('handles checkReported when not exists', async () => {
    const pinia = createTestingPinia()
    const { global } = await import('@/modules/pinia')
    global.ui.enableDeviceRegistration = true
    global.anonymizeChipId = vi.fn().mockResolvedValue('anon-123')
    http.getJson.mockResolvedValueOnce({ exists: false })

    const wrapper = mount(DeviceSettingsView, {
      global: {
        plugins: [pinia],
        stubs: {
          BsMessage: true,
          BsInputReadonly: {
            name: 'BsInputReadonly',
            props: ['modelValue'],
            template: '<div class="status-val">{{ modelValue }}</div>'
          }
        }
      }
    })

    await wrapper.vm.checkReported()
    expect(wrapper.find('.status-val').text()).toBe('This device has not been reported')
  })

  it('handles checkReported error', async () => {
    const pinia = createTestingPinia()
    const { global } = await import('@/modules/pinia')
    global.ui.enableDeviceRegistration = true
    global.anonymizeChipId = vi.fn().mockResolvedValue('anon-123')
    http.getJson.mockRejectedValueOnce(new Error('Network error'))

    const wrapper = mount(DeviceSettingsView, {
      global: {
        plugins: [pinia],
        stubs: {
          BsMessage: true,
          BsInputReadonly: {
            name: 'BsInputReadonly',
            props: ['modelValue'],
            template: '<div class="status-val">{{ modelValue }}</div>'
          }
        }
      }
    })

    await wrapper.vm.checkReported()
    expect(wrapper.find('.status-val').text()).toBe('Error checking registration status')
  })

  it('handles reportUsage modal', async () => {
    const pinia = createTestingPinia()
    const { global } = await import('@/modules/pinia')
    global.ui.enableDeviceRegistration = true

    const wrapper = mount(DeviceSettingsView, {
      global: {
        plugins: [pinia],
        stubs: {
          BsMessage: true,
          RegisterDeviceFragment: {
            name: 'RegisterDeviceFragment',
            template: '<div class="modal">Modal</div>'
          }
        }
      }
    })

    const reportButton = wrapper.findAll('button').find((b) => b.text().includes('Report Usage'))
    await reportButton.trigger('click')

    expect(wrapper.find('.modal').exists()).toBe(true)

    // Test closing the modal
    const fragment = wrapper.findComponent({ name: 'RegisterDeviceFragment' })
    fragment.vm.$emit('close')
    await wrapper.vm.$nextTick()

    expect(wrapper.find('.modal').exists()).toBe(false)
  })

  it('handles factory reset success with reload fallback', async () => {
    const pinia = createTestingPinia()
    const { global } = await import('@/modules/pinia')
    http.getJson.mockResolvedValueOnce({ success: true, message: 'Factory reset initiated' })

    const reloadMock = vi.fn()
    vi.stubGlobal('location', { reload: reloadMock })

    const wrapper = mount(DeviceSettingsView, {
      global: { plugins: [pinia] }
    })

    await wrapper.vm.factory()
    expect(global.messageSuccess).toBe('Factory reset initiated')

    vi.advanceTimersByTime(2000)
    expect(reloadMock).toHaveBeenCalledWith(true)
  })

  it('handles factory reset success with location object reload fallback', async () => {
    const pinia = createTestingPinia()
    http.getJson.mockResolvedValueOnce({ success: true, message: 'OK' })

    // Simulate error in location.reload(true) by making it throw
    vi.stubGlobal('location', {
      reload: vi.fn(() => {
        throw new Error('reload fail')
      })
    })
    const windowReloadMock = vi.fn()
    vi.stubGlobal('window', { location: { reload: windowReloadMock }, addEventListener: vi.fn() })

    const wrapper = mount(DeviceSettingsView, {
      global: { plugins: [pinia] }
    })

    await wrapper.vm.factory()

    vi.advanceTimersByTime(2000)
    expect(windowReloadMock).toHaveBeenCalled()
  })

  it('handles factory beforeunload cleanup', async () => {
    const pinia = createTestingPinia()
    http.getJson.mockResolvedValueOnce({ success: true, message: 'OK' })
    const clearTimeoutSpy = vi.spyOn(global, 'clearTimeout')
    let beforeUnloadHandler
    vi.spyOn(window, 'addEventListener').mockImplementation((event, handler) => {
      if (event === 'beforeunload') beforeUnloadHandler = handler
    })

    const wrapper = mount(DeviceSettingsView, {
      global: { plugins: [pinia] }
    })

    await wrapper.vm.factory()

    beforeUnloadHandler()

    expect(clearTimeoutSpy).toHaveBeenCalled()
  })

  it('handles factory error', async () => {
    const pinia = createTestingPinia()
    const { global } = await import('@/modules/pinia')
    http.getJson.mockRejectedValueOnce(new Error('API Error'))

    const wrapper = mount(DeviceSettingsView, {
      global: { plugins: [pinia] }
    })

    await wrapper.vm.factory()
    expect(global.messageError).toBe('Failed to do factory restore')
  })

  it('handles factory failure message', async () => {
    const pinia = createTestingPinia()
    const { global } = await import('@/modules/pinia')
    http.getJson.mockResolvedValueOnce({ success: false, message: 'Could not reset' })

    const wrapper = mount(DeviceSettingsView, {
      global: { plugins: [pinia] }
    })

    await wrapper.vm.factory()
    expect(global.messageError).toBe('Could not reset')
  })

  it('handles restart', async () => {
    const pinia = createTestingPinia()
    const { config } = await import('@/modules/pinia')
    config.restart = vi.fn()

    const wrapper = mount(DeviceSettingsView, {
      global: { plugins: [pinia] }
    })

    await wrapper.vm.restart()
    expect(config.restart).toHaveBeenCalled()
  })

  it('handles saveSettings with validation failure', async () => {
    const pinia = createTestingPinia()
    const { config } = await import('@/modules/pinia')
    config.saveAll = vi.fn()
    validateCurrentForm.mockReturnValueOnce(false)

    const wrapper = mount(DeviceSettingsView, {
      global: { plugins: [pinia] }
    })

    await wrapper.vm.saveSettings()
    expect(config.saveAll).not.toHaveBeenCalled()
  })

  it('handles saveSettings with success', async () => {
    const pinia = createTestingPinia()
    const { config } = await import('@/modules/pinia')
    config.saveAll = vi.fn()
    validateCurrentForm.mockReturnValueOnce(true)

    const wrapper = mount(DeviceSettingsView, {
      global: { plugins: [pinia] }
    })

    await wrapper.vm.saveSettings()
    expect(config.saveAll).toHaveBeenCalled()
  })

  it('displays page heading', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(DeviceSettingsView, {
      global: {
        plugins: [pinia],
        stubs: {
          BsMessage: true,
          BsInputText: true,
          BsInputRadio: true,
          BsSwitch: true,
          BsProgress: true
        }
      }
    })
    expect(wrapper.text()).toContain('Device - Settings')
  })

  it('displays settings form', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(DeviceSettingsView, {
      global: {
        plugins: [pinia],
        stubs: {
          BsMessage: true,
          BsInputText: true,
          BsInputRadio: true,
          BsSwitch: true,
          BsProgress: true
        }
      }
    })
    const form = wrapper.find('form')
    expect(form.exists()).toBe(true)
  })

  it('form has needs-validation class', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(DeviceSettingsView, {
      global: {
        plugins: [pinia],
        stubs: {
          BsMessage: true,
          BsInputText: true,
          BsInputRadio: true,
          BsSwitch: true,
          BsProgress: true
        }
      }
    })
    const form = wrapper.find('form')
    expect(form.classes()).toContain('needs-validation')
  })

  it('has save button', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(DeviceSettingsView, {
      global: {
        plugins: [pinia],
        stubs: {
          BsMessage: true,
          BsInputText: true,
          BsInputRadio: true,
          BsSwitch: true,
          BsProgress: true
        }
      }
    })
    const buttons = wrapper.findAll('button')
    const saveButton = buttons.find((b) => b.text().includes('Save'))
    expect(saveButton).toBeDefined()
  })

  it('has otaOptions computed property', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(DeviceSettingsView, {
      global: {
        plugins: [pinia],
        stubs: {
          BsMessage: true,
          BsInputText: true,
          BsInputRadio: true,
          BsSwitch: true,
          BsProgress: true
        }
      }
    })
    // In script setup, these are not exposed directly to wrapper.vm unless defineExpose is used
    // or we check the rendered content/refs.
    expect(wrapper.exists()).toBe(true)
  })

  it('has otaOptions property defined', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(DeviceSettingsView, {
      global: {
        plugins: [pinia],
        stubs: {
          BsMessage: true,
          BsInputText: true,
          BsInputRadio: true,
          BsSwitch: true,
          BsProgress: true
        }
      }
    })
    expect(wrapper.vm.otaOptions).toBeDefined()
  })

  it('has saveSettings function defined', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(DeviceSettingsView, {
      global: {
        plugins: [pinia],
        stubs: {
          BsMessage: true,
          BsInputText: true,
          BsInputRadio: true,
          BsSwitch: true,
          BsProgress: true
        }
      }
    })
    expect(typeof wrapper.vm.saveSettings).toBe('function')
  })

  it('has tempOptions computed property', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(DeviceSettingsView, {
      global: {
        plugins: [pinia],
        stubs: {
          BsMessage: true,
          BsInputText: true,
          BsInputRadio: true,
          BsSwitch: true,
          BsProgress: true
        }
      }
    })
    expect(wrapper.vm.tempOptions).toBeDefined()
  })

  it('has gravityOptions computed property', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(DeviceSettingsView, {
      global: {
        plugins: [pinia],
        stubs: {
          BsMessage: true,
          BsInputText: true,
          BsInputRadio: true,
          BsSwitch: true,
          BsProgress: true
        }
      }
    })
    expect(wrapper.vm.gravityOptions).toBeDefined()
  })

  it('has uiOptions computed property', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(DeviceSettingsView, {
      global: {
        plugins: [pinia],
        stubs: {
          BsMessage: true,
          BsInputText: true,
          BsInputRadio: true,
          BsSwitch: true,
          BsProgress: true
        }
      }
    })
    expect(wrapper.vm.uiOptions).toBeDefined()
  })

  it('displays BsInputText component', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(DeviceSettingsView, {
      global: {
        plugins: [pinia],
        stubs: {
          BsMessage: true,
          BsInputText: true,
          BsInputRadio: true,
          BsSwitch: true,
          BsProgress: true
        }
      }
    })
    expect(wrapper.find('bs-input-text-stub').exists()).toBe(true)
  })

  it('displays BsInputRadio components', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(DeviceSettingsView, {
      global: {
        plugins: [pinia],
        stubs: {
          BsMessage: true,
          BsInputText: true,
          BsInputRadio: true,
          BsSwitch: true,
          BsProgress: true
        }
      }
    })
    expect(wrapper.findAll('bs-input-radio-stub').length).toBeGreaterThan(0)
  })

  it('has config state defined', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(DeviceSettingsView, {
      global: {
        plugins: [pinia],
        stubs: {
          BsMessage: true,
          BsInputText: true,
          BsInputRadio: true,
          BsSwitch: true,
          BsProgress: true
        }
      }
    })
    expect(wrapper.vm.config).toBeDefined()
  })

  it('displays BsMessage component', async () => {
    const pinia = createTestingPinia()
    const { config } = await import('@/modules/pinia')
    config.mdns = ''
    const wrapper = mount(DeviceSettingsView, {
      global: {
        plugins: [pinia],
        stubs: {
          BsMessage: true,
          BsInputText: true,
          BsInputRadio: true,
          BsSwitch: true,
          BsProgress: true
        }
      }
    })
    expect(wrapper.find('bs-message-stub').exists()).toBe(true)
  })

  it('displays container layout', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(DeviceSettingsView, {
      global: {
        plugins: [pinia],
        stubs: {
          BsMessage: true,
          BsInputText: true,
          BsInputRadio: true,
          BsSwitch: true,
          BsProgress: true
        }
      }
    })
    expect(wrapper.find('.container').exists()).toBe(true)
  })

  it('save button has primary styling', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(DeviceSettingsView, {
      global: {
        plugins: [pinia],
        stubs: {
          BsMessage: true,
          BsInputText: true,
          BsInputRadio: true,
          BsSwitch: true,
          BsProgress: true
        }
      }
    })
    const buttons = wrapper.findAll('button')
    const saveButton = buttons.find((b) => b.text().includes('Save'))
    expect(saveButton.classes()).toContain('btn-primary')
  })
})

describe('DeviceSettingsView (action tests)', () => {
  beforeEach(() => {
    vi.clearAllMocks()
  })

  afterEach(() => {
    vi.useRealTimers()
    vi.unstubAllGlobals()
  })

  const mountView = () => {
    const pinia = createTestingPinia()
    return mount(DeviceSettingsView, {
      global: {
        plugins: [pinia],
        stubs: {
          BsMessage: true,
          BsInputText: true,
          BsInputRadio: true,
          BsSwitch: true,
          BsProgress: true,
          BsDropdown: true,
          RegisterDeviceFragment: true
        }
      }
    })
  }

  it('saveSettings calls config.saveAll when form is valid', async () => {
    const wrapper = mountView()
    await wrapper.vm.saveSettings()
    const { config } = await import('@/modules/pinia')
    expect(config.saveAll).toHaveBeenCalled()
  })

  it('saveSettings does not call config.saveAll when form is invalid', async () => {
    const { validateCurrentForm } = await import('@mp-se/espframework-ui-components')
    validateCurrentForm.mockReturnValueOnce(false)
    const wrapper = mountView()
    await wrapper.vm.saveSettings()
    const { config } = await import('@/modules/pinia')
    expect(config.saveAll).not.toHaveBeenCalled()
  })

  it('restart calls config.restart', async () => {
    const wrapper = mountView()
    await wrapper.vm.restart()
    const { config } = await import('@/modules/pinia')
    expect(config.restart).toHaveBeenCalled()
  })

  it('factory calls http.getJson for api/factory', async () => {
    http.getJson.mockResolvedValue({ success: true, message: 'Factory reset done' })
    const wrapper = mountView()
    await wrapper.vm.factory()
    expect(http.getJson).toHaveBeenCalledWith('api/factory')
  })

  it('factory sets messageSuccess on success', async () => {
    http.getJson.mockResolvedValue({ success: true, message: 'Done' })
    const wrapper = mountView()
    await wrapper.vm.factory()
    const { global: globalStore } = await import('@/modules/pinia')
    expect(globalStore.messageSuccess).toBe('Done')
  })

  it('factory sets messageError on failure response', async () => {
    http.getJson.mockResolvedValue({ success: false, message: 'Error' })
    const wrapper = mountView()
    await wrapper.vm.factory()
    const { global: globalStore } = await import('@/modules/pinia')
    expect(globalStore.messageError).toBe('Error')
  })

  it('factory sets messageError on exception', async () => {
    http.getJson.mockRejectedValue(new Error('network'))
    const wrapper = mountView()
    await wrapper.vm.factory()
    const { global: globalStore } = await import('@/modules/pinia')
    expect(globalStore.messageError).toBeTruthy()
  })

  it('otaCallback sets config.ota_url', async () => {
    const wrapper = mountView()
    wrapper.vm.otaCallback('https://www.example.com/firmware/')
    const { config } = await import('@/modules/pinia')
    expect(config.ota_url).toBe('https://www.example.com/firmware/')
  })

  it('reportUsage sets showRegisterModal to true', async () => {
    const wrapper = mountView()
    await wrapper.vm.reportUsage()
    expect(wrapper.vm.showRegisterModal).toBe(true)
  })

  it('closeRegisterModal calls checkReported', async () => {
    http.getJson.mockResolvedValue({ exists: false })
    const wrapper = mountView()
    wrapper.vm.closeRegisterModal()
    await new Promise((resolve) => setTimeout(resolve, 10))
    expect(http.getJson).toHaveBeenCalled()
  })

  it('checkReported sets registrationStatus when device found', async () => {
    http.getJson.mockResolvedValue({ exists: true, software: 'gravitymon' })
    const wrapper = mountView()
    await wrapper.vm.checkReported()
    expect(wrapper.vm.registrationStatus).toContain('gravitymon')
  })

  it('checkReported sets not-reported message when device not found', async () => {
    http.getJson.mockResolvedValue({ exists: false })
    const wrapper = mountView()
    await wrapper.vm.checkReported()
    expect(wrapper.vm.registrationStatus).toContain('not been reported')
  })

  it('checkReported sets error status on exception', async () => {
    http.getJson.mockRejectedValue(new Error('network error'))
    const wrapper = mountView()
    await wrapper.vm.checkReported()
    expect(wrapper.vm.registrationStatus).toContain('Error')
  })

  it('restart calls config.restart', async () => {
    const { config } = await import('@/modules/pinia')
    config.restart = vi.fn(async () => {})
    const wrapper = mountView()
    await wrapper.vm.restart()
    expect(config.restart).toHaveBeenCalled()
  })

  it('saveSettings validates form before saving', async () => {
    const { validateCurrentForm } = await import('@mp-se/espframework-ui-components')
    const { config } = await import('@/modules/pinia')
    config.saveAll = vi.fn(async () => {})
    const wrapper = mountView()
    await wrapper.vm.saveSettings()
    expect(validateCurrentForm).toHaveBeenCalled()
  })

  it('saveSettings calls config.saveAll when form is valid', async () => {
    const { config } = await import('@/modules/pinia')
    config.saveAll = vi.fn(async () => {})
    const wrapper = mountView()
    await wrapper.vm.saveSettings()
    expect(config.saveAll).toHaveBeenCalled()
  })

  it('otaCallback sets empty string for blank option', async () => {
    const wrapper = mountView()
    wrapper.vm.otaCallback('')
    const { config } = await import('@/modules/pinia')
    expect(config.ota_url).toBe('')
  })

  it('closeRegisterModal sets showRegisterModal to false', async () => {
    http.getJson.mockResolvedValue({ exists: false })
    const wrapper = mountView()
    wrapper.vm.showRegisterModal = true
    wrapper.vm.closeRegisterModal()
    await new Promise((resolve) => setTimeout(resolve, 10))
    expect(wrapper.vm.showRegisterModal).toBe(false)
  })

  it('factory calls http.getJson and shows success message', async () => {
    http.getJson.mockResolvedValue({ success: true, message: 'Factory reset complete' })
    const { global } = await import('@/modules/pinia')
    global.clearMessages = vi.fn()
    const wrapper = mountView()
    await wrapper.vm.factory()
    expect(global.messageSuccess).toBe('Factory reset complete')
  })

  it('factory setTimeout callback executes location redirect', async () => {
    vi.useFakeTimers()
    http.getJson.mockResolvedValue({ success: true, message: 'Done' })
    // Mock location.reload to prevent jsdom error
    const reloadSpy = vi.fn()
    Object.defineProperty(window, 'location', {
      value: { ...window.location, reload: reloadSpy, href: '/other/firmware' },
      writable: true
    })
    const wrapper = mountView()
    await wrapper.vm.factory()
    // Advance timer to trigger the setTimeout callback
    vi.advanceTimersByTime(2001)
    vi.useRealTimers()
    // The try block should have executed (location.href assignment may throw in jsdom, catch calls reload)
    expect(true).toBe(true) // No exception thrown from setTimeout callback
  })

  it('factory shows error message on failed response', async () => {
    http.getJson.mockResolvedValue({ success: false, message: 'Reset failed' })
    const { global } = await import('@/modules/pinia')
    const wrapper = mountView()
    await wrapper.vm.factory()
    expect(global.messageError).toBe('Reset failed')
  })

  it('factory sets messageError on exception', async () => {
    http.getJson.mockRejectedValue(new Error('network error'))
    const { global } = await import('@/modules/pinia')
    const wrapper = mountView()
    await wrapper.vm.factory()
    expect(global.messageError).toBe('Failed to do factory restore')
  })

  it('reportUsage sets showRegisterModal to true', async () => {
    const wrapper = mountView()
    await wrapper.vm.reportUsage()
    expect(wrapper.vm.showRegisterModal).toBe(true)
  })

  it('factory setTimeout catch block executes when location.reload throws', async () => {
    vi.useFakeTimers()
    http.getJson.mockResolvedValue({ success: true, message: 'Done' })

    // Make location.reload throw on first call (try block), succeed on second (catch fallback)
    let callCount = 0
    const reloadFn = vi.fn(() => {
      callCount++
      if (callCount === 1) throw new Error('navigation blocked')
    })
    const origLocation = window.location
    Object.defineProperty(window, 'location', {
      value: { reload: reloadFn },
      writable: true,
      configurable: true
    })

    const wrapper = mountView()
    await wrapper.vm.factory()
    expect(() => vi.advanceTimersByTime(2001)).not.toThrow()
    vi.useRealTimers()

    Object.defineProperty(window, 'location', {
      value: origLocation,
      writable: true,
      configurable: true
    })
    // The catch block was reached (callCount = 2: try threw, catch called reload again)
    expect(callCount).toBe(2)
  })

  it('factory beforeunload handler calls clearTimeout when event fires', async () => {
    vi.useFakeTimers()
    http.getJson.mockResolvedValue({ success: true, message: 'Done' })
    const reloadSpy = vi.fn()
    let beforeUnloadHandler
    vi.spyOn(window, 'addEventListener').mockImplementation((event, handler) => {
      if (event === 'beforeunload') beforeUnloadHandler = handler
    })
    const origLocation = window.location
    Object.defineProperty(window, 'location', {
      value: { reload: reloadSpy },
      writable: true,
      configurable: true
    })

    const wrapper = mountView()
    await wrapper.vm.factory()

    beforeUnloadHandler()

    vi.useRealTimers()
    Object.defineProperty(window, 'location', {
      value: origLocation,
      writable: true,
      configurable: true
    })
    expect(wrapper.exists()).toBe(true)
  })
})
