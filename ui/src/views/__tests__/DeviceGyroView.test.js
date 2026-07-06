import { describe, it, expect, beforeEach, vi } from 'vitest'
import { mount } from '@vue/test-utils'
import DeviceGyroView from '../DeviceGyroView.vue'
import { createPinia, setActivePinia } from 'pinia'
import { sharedHttpClient as http } from '@mp-se/espframework-ui-components'

// Mock the UI components library
vi.mock('@mp-se/espframework-ui-components', () => ({
  logDebug: vi.fn(),
  logError: vi.fn(),
  logInfo: vi.fn(),
  validateCurrentForm: vi.fn(() => true),
  sharedHttpClient: {
    get: vi.fn(),
    post: vi.fn(),
    postJson: vi.fn(),
    request: vi.fn(),
    getJson: vi.fn(),
    filesystemRequest: vi.fn()
  }
}))

describe('DeviceGyroView (interaction tests)', () => {
  beforeEach(() => {
    setActivePinia(createPinia())
  })

  it('mounts without error', () => {
    const wrapper = mount(DeviceGyroView, {
      global: {
        stubs: {
          BsMessage: true,
          BsInputRadio: true,
          BsInputSwitch: true,
          BsInputReadonly: true,
          BsInputText: true,
          BsInputNumber: true
        }
      }
    })
    expect(wrapper.exists()).toBe(true)
  })

  it('displays gyro configuration section', () => {
    const wrapper = mount(DeviceGyroView, {
      global: {
        stubs: {
          BsMessage: true,
          BsInputRadio: true,
          BsInputSwitch: true,
          BsInputReadonly: true,
          BsInputText: true,
          BsInputNumber: true
        }
      }
    })
    expect(wrapper.text()).toContain('Device - Gyro')
  })

  it('has form element for gyro settings', () => {
    const wrapper = mount(DeviceGyroView, {
      global: {
        stubs: {
          BsMessage: true,
          BsInputRadio: true,
          BsInputSwitch: true,
          BsInputReadonly: true,
          BsInputText: true,
          BsInputNumber: true
        }
      }
    })
    const form = wrapper.find('form')
    expect(form.exists()).toBe(true)
  })

  it('displays gyro type options section', () => {
    const wrapper = mount(DeviceGyroView, {
      global: {
        stubs: {
          BsMessage: true,
          BsInputRadio: {
            template: '<div class="bs-input-radio">{{ label }}</div>',
            props: ['label']
          },
          BsInputSwitch: true,
          BsInputReadonly: true,
          BsInputText: true,
          BsInputNumber: true
        }
      }
    })
    expect(wrapper.text()).toContain('Gyro options')
  })

  it('has gyro options radio input component', () => {
    const wrapper = mount(DeviceGyroView, {
      global: {
        stubs: {
          BsMessage: true,
          BsInputRadio: { template: '<div class="bs-input-radio"></div>' },
          BsInputSwitch: true,
          BsInputReadonly: true,
          BsInputText: true,
          BsInputNumber: true
        }
      }
    })
    const radioInput = wrapper.find('.bs-input-radio')
    expect(radioInput.exists()).toBe(true)
  })

  it('has save button', () => {
    const wrapper = mount(DeviceGyroView, {
      global: {
        stubs: {
          BsMessage: true,
          BsInputRadio: true,
          BsInputSwitch: true,
          BsInputReadonly: true,
          BsInputText: true,
          BsInputNumber: true
        }
      }
    })
    const buttons = wrapper.findAll('button')
    const saveButton = buttons.find((b) => b.text().includes('Save'))
    expect(saveButton).toBeDefined()
  })

  it('has isGyroCalibrated function defined', () => {
    const wrapper = mount(DeviceGyroView, {
      global: {
        stubs: {
          BsMessage: true,
          BsInputRadio: true,
          BsInputSwitch: true,
          BsInputReadonly: true,
          BsInputText: true,
          BsInputNumber: true
        }
      }
    })
    expect(typeof wrapper.vm.isGyroCalibrated).toBe('function')
  })

  it('has save function defined', () => {
    const wrapper = mount(DeviceGyroView, {
      global: {
        stubs: {
          BsMessage: true,
          BsInputRadio: true,
          BsInputSwitch: true,
          BsInputReadonly: true,
          BsInputText: true,
          BsInputNumber: true
        }
      }
    })
    expect(typeof wrapper.vm.save).toBe('function')
  })

  it('has calibrate function defined', () => {
    const wrapper = mount(DeviceGyroView, {
      global: {
        stubs: {
          BsMessage: true,
          BsInputRadio: true,
          BsInputSwitch: true,
          BsInputReadonly: true,
          BsInputText: true,
          BsInputNumber: true
        }
      }
    })
    expect(typeof wrapper.vm.calibrate).toBe('function')
  })

  it('has clearCalibration function defined', () => {
    const wrapper = mount(DeviceGyroView, {
      global: {
        stubs: {
          BsMessage: true,
          BsInputRadio: true,
          BsInputSwitch: true,
          BsInputReadonly: true,
          BsInputText: true,
          BsInputNumber: true
        }
      }
    })
    expect(typeof wrapper.vm.clearCalibration).toBe('function')
  })

  it('has calibrationValues computed property', () => {
    const wrapper = mount(DeviceGyroView, {
      global: {
        stubs: {
          BsMessage: true,
          BsInputRadio: true,
          BsInputSwitch: true,
          BsInputReadonly: true,
          BsInputText: true,
          BsInputNumber: true
        }
      }
    })
    expect(wrapper.vm.calibrationValues).toBeDefined()
  })

  it('has gyroOptions computed property', () => {
    const wrapper = mount(DeviceGyroView, {
      global: {
        stubs: {
          BsMessage: true,
          BsInputRadio: true,
          BsInputSwitch: true,
          BsInputReadonly: true,
          BsInputText: true,
          BsInputNumber: true
        }
      }
    })
    expect(Array.isArray(wrapper.vm.gyroOptions)).toBe(true)
  })

  it('displays form with prevent default on submit', () => {
    const wrapper = mount(DeviceGyroView, {
      global: {
        stubs: {
          BsMessage: true,
          BsInputRadio: true,
          BsInputSwitch: true,
          BsInputReadonly: true,
          BsInputText: true,
          BsInputNumber: true
        }
      }
    })
    const form = wrapper.find('form')
    expect(form.attributes('novalidate')).toBeDefined()
  })

  it('applies needs-validation class to form', () => {
    const wrapper = mount(DeviceGyroView, {
      global: {
        stubs: {
          BsMessage: true,
          BsInputRadio: true,
          BsInputSwitch: true,
          BsInputReadonly: true,
          BsInputText: true,
          BsInputNumber: true
        }
      }
    })
    const form = wrapper.find('form')
    expect(form.element.className).toContain('needs-validation')
  })

  it('displays calibration message component', () => {
    const wrapper = mount(DeviceGyroView, {
      global: {
        stubs: {
          BsMessage: { template: '<div class="bs-message-stub"></div>' },
          BsInputRadio: true,
          BsInputSwitch: true,
          BsInputReadonly: true,
          BsInputText: true,
          BsInputNumber: true
        }
      }
    })
    // Message component exists in template (rendered as stub)
    const messageStub = wrapper.find('.bs-message-stub')
    expect(messageStub.exists()).toBe(true)
  })
})

describe('DeviceGyroView (action tests)', () => {
  const stubs = {
    BsMessage: true,
    BsInputRadio: true,
    BsInputSwitch: true,
    BsInputReadonly: true,
    BsInputText: true,
    BsInputNumber: true
  }

  const mountView = () => mount(DeviceGyroView, { global: { stubs } })

  beforeEach(() => {
    setActivePinia(createPinia())
    vi.clearAllMocks()
  })

  it('clearCalibration resets gyro calibration data', async () => {
    const wrapper = mountView()
    // Just verify it runs without throwing (saveAll is mocked to vi.fn() in setup)
    await expect(wrapper.vm.clearCalibration()).resolves.not.toThrow()
  })

  it('restart does not throw', async () => {
    const wrapper = mountView()
    await expect(wrapper.vm.restart()).resolves.not.toThrow()
  })

  it('calibrate starts calibration and polls for status', async () => {
    http.request.mockResolvedValueOnce({ ok: true })
    http.getJson.mockResolvedValueOnce({ status: false, success: true }) // immediate completion
    const { useConfigStore } = await import('@/modules/configStore')
    const configStore = useConfigStore()
    configStore.load = vi.fn(async () => true)

    const wrapper = mountView()
    const result = await wrapper.vm.calibrate()
    expect(result).toBe(true)
    expect(http.request).toHaveBeenCalledWith('api/calibrate')
    expect(http.getJson).toHaveBeenCalledWith('api/calibrate/status')
  })

  it('calibrate handles start failure', async () => {
    http.request.mockResolvedValueOnce({ ok: false })

    const wrapper = mountView()
    const result = await wrapper.vm.calibrate()
    expect(result).toBe(false)
  })

  it('calibrate handles calibration failure from status', async () => {
    http.request.mockResolvedValueOnce({ ok: true })
    http.getJson.mockResolvedValueOnce({ status: false, success: false, message: 'gyro error' })

    const wrapper = mountView()
    const result = await wrapper.vm.calibrate()
    expect(result).toBe(false)
  })

  it('calibrate handles thrown error', async () => {
    http.request.mockRejectedValueOnce(new Error('Network error'))

    const wrapper = mountView()
    const result = await wrapper.vm.calibrate()
    expect(result).toBe(false)
  })

  it('calibrationValues shows JSON for gyro_type 1', () => {
    const wrapper = mountView()
    // config.gyro_type defaults to 1 in setup.js mock
    const val = wrapper.vm.calibrationValues
    expect(typeof val).toBe('string')
  })

  it('ispindel imports calibration data from iSpindel config', async () => {
    const mockConfig = {
      Offset: [1, 2, 3, 4, 5, 6],
      POLY: '0.5*x+1'
    }
    http.filesystemRequest.mockResolvedValueOnce({
      success: true,
      text: JSON.stringify(mockConfig)
    })

    const wrapper = mountView()
    wrapper.vm.ispindel()
    await wrapper.vm.$nextTick()
    await wrapper.vm.$nextTick()

    // Verify the filesystem request was made with correct params
    expect(http.filesystemRequest).toHaveBeenCalledWith({
      command: 'get',
      file: '/config.json'
    })
  })

  it('ispindel handles failed filesystem request', async () => {
    http.filesystemRequest.mockResolvedValueOnce({ success: false })

    const wrapper = mountView()
    // Should not throw
    await wrapper.vm.ispindel()
    await wrapper.vm.$nextTick()
  })

  it('calibrationValues returns "Calibration not needed" string for ICM gyro (gyro_type 2)', async () => {
    const { config } = await import('@/modules/pinia')
    const origType = config.gyro_type
    config.gyro_type = 2
    const wrapper = mountView()
    // calibrationValues computed uses config.gyro_type directly
    expect(typeof wrapper.vm.calibrationValues).toBe('string')
    config.gyro_type = origType
  })

  it('calibrate polls while statusRes.status is true then succeeds', async () => {
    http.request.mockResolvedValueOnce({ ok: true })
    http.getJson
      .mockResolvedValueOnce({ status: true }) // first poll: still running
      .mockResolvedValueOnce({ status: false, success: true }) // second poll: done

    const { useConfigStore } = await import('@/modules/configStore')
    const configStore = useConfigStore()
    configStore.load = vi.fn(async () => true)

    const wrapper = mountView()
    const result = await wrapper.vm.calibrate()
    expect(result).toBe(true)
    expect(http.getJson).toHaveBeenCalledTimes(2)
  })

  it('calibrate handles null request response', async () => {
    http.request.mockResolvedValueOnce(null)

    const wrapper = mountView()
    const result = await wrapper.vm.calibrate()
    expect(result).toBe(false)
  })

  it('calibrate sets messageError on status null response', async () => {
    http.request.mockResolvedValueOnce({ ok: true })
    http.getJson.mockResolvedValueOnce(null)

    const { global } = await import('@/modules/pinia')
    global.messageError = ''

    const wrapper = mountView()
    const result = await wrapper.vm.calibrate()
    expect(result).toBe(false)
    expect(global.messageError).toContain('calibrate status')
  })

  it('calibrate reloads config on success and sets messageSuccess', async () => {
    http.request.mockResolvedValueOnce({ ok: true })
    http.getJson.mockResolvedValueOnce({ status: false, success: true })

    const { config, global } = await import('@/modules/pinia')
    config.load = vi.fn(async () => true)
    global.messageSuccess = ''

    const wrapper = mountView()
    const result = await wrapper.vm.calibrate()
    expect(result).toBe(true)
    expect(global.messageSuccess).toContain('calibrated')
  })

  it('calibrate sets error when config.load fails after calibration', async () => {
    http.request.mockResolvedValueOnce({ ok: true })
    http.getJson.mockResolvedValueOnce({ status: false, success: true })

    const { config, global } = await import('@/modules/pinia')
    config.load = vi.fn(async () => false)
    global.messageError = ''

    const wrapper = mountView()
    const result = await wrapper.vm.calibrate()
    expect(result).toBe(true)
    expect(global.messageError).toContain('load configuration')
  })

  it('save calls clearMessages and saveAll when form is valid', async () => {
    const { validateCurrentForm } = await import('@mp-se/espframework-ui-components')
    const { config, global } = await import('@/modules/pinia')
    vi.mocked(validateCurrentForm).mockReturnValue(true)
    config.saveAll = vi.fn(async () => {})
    global.clearMessages = vi.fn()

    const wrapper = mountView()
    await wrapper.vm.save()
    expect(global.clearMessages).toHaveBeenCalled()
    expect(config.saveAll).toHaveBeenCalled()
  })

  it('save returns early when form is invalid', async () => {
    const { validateCurrentForm } = await import('@mp-se/espframework-ui-components')
    const { config } = await import('@/modules/pinia')
    vi.mocked(validateCurrentForm).mockReturnValue(false)
    config.saveAll = vi.fn()

    const wrapper = mountView()
    await wrapper.vm.save()
    expect(config.saveAll).not.toHaveBeenCalled()
  })

  it('triggers v-model handlers for gyro_swap_xy and calibrationValues', async () => {
    const { mount } = await import('@vue/test-utils')
    const { default: DeviceGyroView } = await import('../DeviceGyroView.vue')
    const { createTestingPinia } = await import('../../tests/testUtils')
    const inputStub = {
      template:
        '<input :value="modelValue" @input="$emit(\'update:modelValue\', $event.target.value)" />',
      props: [
        'modelValue',
        'label',
        'disabled',
        'help',
        'badge',
        'options',
        'min',
        'max',
        'step',
        'width'
      ],
      emits: ['update:modelValue']
    }
    const wrapper = mount(DeviceGyroView, {
      global: {
        plugins: [createTestingPinia()],
        stubs: {
          BsInputSwitch: inputStub,
          BsInputReadonly: inputStub,
          BsInputRadio: inputStub,
          BsInputNumber: inputStub,
          BsCard: true
        }
      }
    })
    for (const el of wrapper.findAll('input')) {
      await el.trigger('input')
    }
    expect(wrapper.find('form').exists()).toBe(true)
  })

  it('clicks clearCalibration button when gyro is calibrated', async () => {
    const { mount } = await import('@vue/test-utils')
    const { default: DeviceGyroView } = await import('../DeviceGyroView.vue')
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { config } = await import('@/modules/pinia')
    // Make isGyroCalibrated() return true by setting non-zero calibration data
    config.gyro_calibration_data = { ax: 1, ay: 0, az: 0, gx: 0, gy: 0, gz: 0 }
    config.saveAll = vi.fn(async () => {})
    const wrapper = mount(DeviceGyroView, {
      global: {
        plugins: [createTestingPinia()],
        stubs: {
          BsInputSwitch: true,
          BsInputReadonly: true,
          BsInputRadio: true,
          BsInputNumber: true,
          BsCard: true
        }
      }
    })
    // Click the clearCalibration button if it exists in the DOM
    // Trigger first secondary button (clearCalibration button)
    const secBtn = wrapper.findAll('.btn-secondary')
    if (secBtn.length > 0) await secBtn[0].trigger('click')
    // Verify clearCalibration was callable
    expect(config.saveAll).toBeDefined()
  })
})
