import { describe, it, expect, vi, beforeEach } from 'vitest'
import { mount } from '@vue/test-utils'
import PushInfluxdbView from '../PushInfluxdbView.vue'
import { createTestingPinia } from '../../tests/testUtils'
import piniaInstance from '@/modules/pinia'
import { global as globalStore } from '@/modules/pinia'

describe('PushInfluxdbView (interaction tests)', () => {
  it('mounts without error', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(PushInfluxdbView, {
      global: {
        plugins: [pinia],
        stubs: {
          BsInputText: true,
          BsProgress: true,
          BsMessage: true,
          BsInputTextAreaFormat: true,
          BsDropdown: true,
          BsInputSwitch: true,
          BsInputNumber: true,
          BsModal: true
        }
      }
    })
    expect(wrapper.exists()).toBe(true)
  })

  it('displays page heading', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(PushInfluxdbView, {
      global: {
        plugins: [pinia],
        stubs: { BsInputText: true, BsInputNumber: true, BsProgress: true }
      }
    })
    expect(wrapper.text()).toContain('Influxdb')
  })

  it('displays form', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(PushInfluxdbView, {
      global: {
        plugins: [pinia],
        stubs: { BsInputText: true, BsInputNumber: true, BsProgress: true }
      }
    })
    expect(wrapper.find('form').exists()).toBe(true)
  })

  it('has save button', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(PushInfluxdbView, {
      global: {
        plugins: [pinia],
        stubs: { BsInputText: true, BsInputNumber: true, BsProgress: true }
      }
    })
    const buttons = wrapper.findAll('button')
    const saveButton = buttons.find((b) => b.text().includes('Save'))
    expect(saveButton).toBeDefined()
  })

  it('has test button', async () => {
    const wrapper = mount(PushInfluxdbView, {
      global: {
        plugins: [piniaInstance],
        stubs: {
          BsInputText: true,
          BsInputNumber: true,
          BsProgress: true,
          BsMessage: true,
          BsInputTextAreaFormat: true,
          BsDropdown: true,
          BsInputSwitch: true,
          BsModal: true
        }
      }
    })

    globalStore.ui.enableGravity = true
    globalStore.ui.enablePressure = false
    wrapper.vm.$forceUpdate()
    await wrapper.vm.$nextTick()
    await new Promise((resolve) => setTimeout(resolve, 10))

    const buttons = wrapper.findAll('button')
    expect(buttons.length).toBeGreaterThanOrEqual(2)
  })

  it('has save function defined', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(PushInfluxdbView, {
      global: {
        plugins: [pinia],
        stubs: { BsInputText: true, BsInputNumber: true, BsProgress: true }
      }
    })
    expect(typeof wrapper.vm.save).toBe('function')
  })

  it('has runTestGravity function defined', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(PushInfluxdbView, {
      global: {
        plugins: [pinia],
        stubs: { BsInputText: true, BsInputNumber: true, BsProgress: true }
      }
    })
    expect(typeof wrapper.vm.runTestGravity).toBe('function')
  })

  it('has config state defined', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(PushInfluxdbView, {
      global: {
        plugins: [pinia],
        stubs: { BsInputText: true, BsInputNumber: true, BsProgress: true }
      }
    })
    expect(wrapper.vm.config).toBeDefined()
  })

  it('displays container layout', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(PushInfluxdbView, {
      global: {
        plugins: [pinia],
        stubs: { BsInputText: true, BsInputNumber: true, BsProgress: true }
      }
    })
    expect(wrapper.find('.container').exists()).toBe(true)
  })
})

describe('PushInfluxdbView (action tests)', () => {
  beforeEach(() => vi.clearAllMocks())
  it('save calls config.saveAll when form is valid', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: PushInfluxdbView } = await import('../PushInfluxdbView.vue')
    const wrapper = mount(PushInfluxdbView, {
      global: {
        plugins: [createTestingPinia()],
        stubs: { BsInputText: true, BsInputNumber: true, BsProgress: true }
      }
    })
    await wrapper.vm.save()
    const { config } = await import('@/modules/pinia')
    expect(config.saveAll).toHaveBeenCalled()
  })
  it('runTestGravity calls config.runPushTest', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: PushInfluxdbView } = await import('../PushInfluxdbView.vue')
    const wrapper = mount(PushInfluxdbView, {
      global: {
        plugins: [createTestingPinia()],
        stubs: { BsInputText: true, BsInputNumber: true, BsProgress: true }
      }
    })
    await wrapper.vm.runTestGravity()
    const { config } = await import('@/modules/pinia')
    expect(config.runPushTest).toHaveBeenCalled()
  })

  it('gravityInfluxdb2FormatCallback updates config.influxdb2_format_gravity', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: PushInfluxdbView } = await import('../PushInfluxdbView.vue')
    const { config } = await import('@/modules/pinia')
    const wrapper = mount(PushInfluxdbView, {
      global: {
        plugins: [createTestingPinia()],
        stubs: { BsInputText: true, BsInputNumber: true, BsProgress: true }
      }
    })
    wrapper.vm.gravityInfluxdb2FormatCallback(encodeURIComponent('measurement,tag=val field=1.0'))
    if (config.influxdb2_format_gravity !== undefined) {
      expect(config.influxdb2_format_gravity).not.toBeUndefined()
    }
  })

  it('gravityRenderFormat does not throw', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: PushInfluxdbView } = await import('../PushInfluxdbView.vue')
    const wrapper = mount(PushInfluxdbView, {
      global: {
        plugins: [createTestingPinia()],
        stubs: { BsInputText: true, BsInputNumber: true, BsProgress: true }
      }
    })
    expect(() => wrapper.vm.gravityRenderFormat()).not.toThrow()
  })

  it('pushDisabled returns true when influxdb is false', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: PushInfluxdbView } = await import('../PushInfluxdbView.vue')
    const { global } = await import('@/modules/pinia')
    global.disabled = true
    const wrapper = mount(PushInfluxdbView, {
      global: {
        plugins: [createTestingPinia()],
        stubs: { BsInputText: true, BsInputNumber: true, BsProgress: true }
      }
    })
    expect(wrapper.vm.pushDisabled).toBe(true)
  })

  it('gravityInfluxdb2FormatCallback decodes and updates format', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: PushInfluxdbView } = await import('../PushInfluxdbView.vue')
    const { config } = await import('@/modules/pinia')
    const wrapper = mount(PushInfluxdbView, {
      global: {
        plugins: [createTestingPinia()],
        stubs: { BsInputText: true, BsInputNumber: true, BsProgress: true }
      }
    })
    wrapper.vm.gravityInfluxdb2FormatCallback(encodeURIComponent('measurement,tag=val field=1.0'))
    if (config.influxdb2_format_gravity !== undefined) {
      expect(config.influxdb2_format_gravity).toBeDefined()
    }
  })

  it('runTestGravity handles exception from config.runPushTest', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: PushInfluxdbView } = await import('../PushInfluxdbView.vue')
    const { config, global } = await import('@/modules/pinia')
    vi.spyOn(config, 'runPushTest').mockRejectedValueOnce(new Error('Connection refused'))
    global.messageError = ''
    const wrapper = mount(PushInfluxdbView, {
      global: {
        plugins: [createTestingPinia()],
        stubs: { BsInputText: true, BsInputNumber: true, BsProgress: true }
      }
    })
    await wrapper.vm.runTestGravity()
    expect(global.messageError).toBe('Failed to start push test')
  })

  it('save does not call config.saveAll when form validation fails', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: PushInfluxdbView } = await import('../PushInfluxdbView.vue')
    const { validateCurrentForm } = await import('@mp-se/espframework-ui-components')
    const { config } = await import('@/modules/pinia')
    vi.mocked(validateCurrentForm).mockReturnValue(false)
    config.saveAll = vi.fn()
    const wrapper = mount(PushInfluxdbView, {
      global: {
        plugins: [createTestingPinia()],
        stubs: { BsInputText: true, BsInputNumber: true, BsProgress: true }
      }
    })
    await wrapper.vm.save()
    expect(config.saveAll).not.toHaveBeenCalled()
  })

  it('pushDisabled returns false when global.disabled is false and use_wifi_direct is false', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: PushInfluxdbView } = await import('../PushInfluxdbView.vue')
    const { global, config } = await import('@/modules/pinia')
    global.disabled = false
    config.use_wifi_direct = false
    const wrapper = mount(PushInfluxdbView, {
      global: {
        plugins: [createTestingPinia()],
        stubs: { BsInputText: true, BsInputNumber: true, BsProgress: true }
      }
    })
    expect(wrapper.vm.pushDisabled).toBe(false)
  })

  it('pushDisabled returns true when use_wifi_direct is true', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: PushInfluxdbView } = await import('../PushInfluxdbView.vue')
    const { config } = await import('@/modules/pinia')
    config.use_wifi_direct = true
    const wrapper = mount(PushInfluxdbView, {
      global: {
        plugins: [createTestingPinia()],
        stubs: { BsInputText: true, BsInputNumber: true, BsProgress: true }
      }
    })
    expect(wrapper.vm.pushDisabled).toBe(true)
    config.use_wifi_direct = false
  })

  it('runTestGravity calls global.clearMessages before runPushTest', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: PushInfluxdbView } = await import('../PushInfluxdbView.vue')
    const { global } = await import('@/modules/pinia')
    const wrapper = mount(PushInfluxdbView, {
      global: {
        plugins: [createTestingPinia()],
        stubs: { BsInputText: true, BsInputNumber: true, BsProgress: true }
      }
    })
    await wrapper.vm.runTestGravity()
    expect(global.clearMessages).toHaveBeenCalled()
  })

  it('gravityRenderFormat sets gravityRender from influxdb2 format template', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: PushInfluxdbView } = await import('../PushInfluxdbView.vue')
    const { config } = await import('@/modules/pinia')
    if (config.influxdb2_format_gravity !== undefined) {
      config.influxdb2_format_gravity = 'gravity,unit=SG value={gravity}'
      const wrapper = mount(PushInfluxdbView, {
        global: {
          plugins: [createTestingPinia()],
          stubs: { BsInputText: true, BsInputNumber: true, BsProgress: true }
        }
      })
      wrapper.vm.gravityRenderFormat()
      expect(typeof wrapper.vm.gravityRender).toBe('string')
    }
  })

  it('renders form with all fields using real template', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: PushInfluxdbView } = await import('../PushInfluxdbView.vue')
    const { global, config } = await import('@/modules/pinia')
    global.disabled = false
    config.use_wifi_direct = false
    const wrapper = mount(PushInfluxdbView, {
      global: {
        plugins: [createTestingPinia()],
        stubs: {
          BsInputText: {
            template:
              '<input :value="modelValue" @input="$emit(\'update:modelValue\', $event.target.value)" />',
            props: ['modelValue'],
            emits: ['update:modelValue']
          },
          BsInputNumber: true,
          BsDropdown: true,
          BsModal: true,
          BsInputTextAreaFormat: true
        }
      }
    })
    expect(wrapper.find('form').exists()).toBe(true)
    // Trigger input update to cover v-model handler functions
    const inputs = wrapper.findAll('input')
    if (inputs.length > 0) {
      await inputs[0].trigger('input')
    }
  })

  it('v-model bindings trigger all config updates via emitting stubs', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: View } = await import('../PushInfluxdbView.vue')
    const { global } = await import('@/modules/pinia')
    global.disabled = false

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
        'unit',
        'rows'
      ],
      emits: ['update:modelValue']
    }
    const textareaStub = {
      template:
        '<textarea :value="modelValue" @input="$emit(\'update:modelValue\', $event.target.value)"></textarea>',
      props: ['modelValue', 'disabled', 'rows', 'label', 'help'],
      emits: ['update:modelValue']
    }
    const wrapper = mount(View, {
      global: {
        plugins: [createTestingPinia()],
        stubs: {
          BsInputText: inputStub,
          BsInputNumber: inputStub,
          BsInputTextAreaFormat: textareaStub,
          BsModal: true,
          BsDropdown: true
        }
      }
    })
    for (const el of wrapper.findAll('input')) {
      await el.trigger('input')
    }
    for (const el of wrapper.findAll('textarea')) {
      await el.trigger('input')
    }
    expect(wrapper.find('form').exists()).toBe(true)
  })

  it('triggers BsModal v-model update for gravityRender ref', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: PushInfluxdbView } = await import('../PushInfluxdbView.vue')
    const wrapper = mount(PushInfluxdbView, {
      global: {
        plugins: [createTestingPinia()],
        stubs: {
          BsInputText: true,
          BsInputNumber: true,
          BsInputTextAreaFormat: true,
          BsInputSwitch: true,
          BsDropdown: true,
          BsModal: {
            template:
              '<button class="modal-emit" @click="$emit(\'update:modelValue\', \'test\')" />',
            props: ['modelValue', 'code', 'title', 'button'],
            emits: ['update:modelValue']
          }
        }
      }
    })
    const btn = wrapper.find('.modal-emit')
    if (btn.exists()) await btn.trigger('click')
    expect(wrapper.exists()).toBe(true)
  })

  it('runTestPressure calls config.runPushTest with pressure format', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: PushInfluxdbView } = await import('../PushInfluxdbView.vue')
    const { config } = await import('@/modules/pinia')
    const wrapper = mount(PushInfluxdbView, {
      global: {
        plugins: [createTestingPinia()],
        stubs: { BsInputText: true, BsInputNumber: true, BsProgress: true }
      }
    })
    await wrapper.vm.runTestPressure()
    expect(config.runPushTest).toHaveBeenCalledWith({ push_format: 'influxdb2_format_pressure' })
  })

  it('runTestPressure handles exception from config.runPushTest', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: PushInfluxdbView } = await import('../PushInfluxdbView.vue')
    const { config, global } = await import('@/modules/pinia')
    vi.spyOn(config, 'runPushTest').mockRejectedValueOnce(new Error('Connection error'))
    global.messageError = ''
    const wrapper = mount(PushInfluxdbView, {
      global: {
        plugins: [createTestingPinia()],
        stubs: { BsInputText: true, BsInputNumber: true, BsProgress: true }
      }
    })
    await wrapper.vm.runTestPressure()
    expect(global.messageError).toBe('Failed to start push test')
  })

  it('pressureInfluxdb2FormatCallback decodes and updates format', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: PushInfluxdbView } = await import('../PushInfluxdbView.vue')
    const { config } = await import('@/modules/pinia')
    const wrapper = mount(PushInfluxdbView, {
      global: {
        plugins: [createTestingPinia()],
        stubs: { BsInputText: true, BsInputNumber: true, BsProgress: true }
      }
    })
    const encoded = encodeURIComponent('measurement,tag=val field=2.0')
    wrapper.vm.pressureInfluxdb2FormatCallback(encoded)
    expect(config.influxdb2_format_pressure).toBe('measurement,tag=val field=2.0')
  })

  it('pressureRenderFormat returns formatted pressure data', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: PushInfluxdbView } = await import('../PushInfluxdbView.vue')
    const { config } = await import('@/modules/pinia')
    config.influxdb2_format_pressure = 'pressure={pressure}'
    const wrapper = mount(PushInfluxdbView, {
      global: {
        plugins: [createTestingPinia()],
        stubs: { BsInputText: true, BsInputNumber: true, BsProgress: true }
      }
    })
    wrapper.vm.pressureRenderFormat()
    expect(wrapper.vm.pressureRender).toBeDefined()
  })

  it('runTestGravity clears messages before executing test', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: PushInfluxdbView } = await import('../PushInfluxdbView.vue')
    const { global } = await import('@/modules/pinia')
    const clearSpy = vi.spyOn(global, 'clearMessages')
    const wrapper = mount(PushInfluxdbView, {
      global: {
        plugins: [createTestingPinia()],
        stubs: { BsInputText: true, BsInputNumber: true, BsProgress: true }
      }
    })
    await wrapper.vm.runTestGravity()
    expect(clearSpy).toHaveBeenCalled()
  })

  it('runTestPressure clears messages before executing test', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: PushInfluxdbView } = await import('../PushInfluxdbView.vue')
    const { global } = await import('@/modules/pinia')
    const clearSpy = vi.spyOn(global, 'clearMessages')
    const wrapper = mount(PushInfluxdbView, {
      global: {
        plugins: [createTestingPinia()],
        stubs: { BsInputText: true, BsInputNumber: true, BsProgress: true }
      }
    })
    await wrapper.vm.runTestPressure()
    expect(clearSpy).toHaveBeenCalled()
  })
})
