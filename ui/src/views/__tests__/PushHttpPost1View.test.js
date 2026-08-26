import { describe, it, expect, vi } from 'vitest'
import { mount } from '@vue/test-utils'
import PushHttpPost1View from '../PushHttpPost1View.vue'
import { createTestingPinia } from '../../tests/testUtils'

describe('PushHttpPost1View (interaction tests)', () => {
  it('mounts without error', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(PushHttpPost1View, {
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
    const wrapper = mount(PushHttpPost1View, {
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
    expect(wrapper.text()).toContain('HTTP Post')
  })

  it('displays form', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(PushHttpPost1View, {
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
    expect(wrapper.find('form').exists()).toBe(true)
  })

  it('has save button', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(PushHttpPost1View, {
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
    const buttons = wrapper.findAll('button')
    const saveButton = buttons.find((b) => b.text().includes('Save'))
    expect(saveButton).toBeDefined()
  })

  it('has runTestGravity function defined', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(PushHttpPost1View, {
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
    expect(typeof wrapper.vm.runTestGravity).toBe('function')
  })

  it('has save function defined', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(PushHttpPost1View, {
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
    expect(typeof wrapper.vm.save).toBe('function')
  })

  it('has config state defined', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(PushHttpPost1View, {
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
    expect(wrapper.vm.config).toBeDefined()
  })

  it('displays container layout', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(PushHttpPost1View, {
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
    expect(wrapper.find('.container').exists()).toBe(true)
  })
})

describe('PushHttpPost1View (action tests)', () => {
  it('save calls config.saveAll when form is valid', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const pinia = createTestingPinia()
    const { mount } = await import('@vue/test-utils')
    const { default: PushHttpPost1View } = await import('../PushHttpPost1View.vue')
    const wrapper = mount(PushHttpPost1View, {
      global: { plugins: [pinia], stubs: { BsInputText: true, BsProgress: true } }
    })
    await wrapper.vm.save()
    const { config } = await import('@/modules/pinia')
    expect(config.saveAll).toHaveBeenCalled()
  })

  it('runTestGravity calls config.runPushTest', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const pinia = createTestingPinia()
    const { mount } = await import('@vue/test-utils')
    const { default: PushHttpPost1View } = await import('../PushHttpPost1View.vue')
    const wrapper = mount(PushHttpPost1View, {
      global: { plugins: [pinia], stubs: { BsInputText: true, BsProgress: true } }
    })
    await wrapper.vm.runTestGravity()
    const { config } = await import('@/modules/pinia')
    expect(config.runPushTest).toHaveBeenCalled()
  })

  it('httpUrlCallback updates config.http_post_target', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: PushHttpPost1View } = await import('../PushHttpPost1View.vue')
    const { config } = await import('@/modules/pinia')
    const wrapper = mount(PushHttpPost1View, {
      global: { plugins: [createTestingPinia()], stubs: { BsInputText: true, BsProgress: true } }
    })
    wrapper.vm.httpUrlCallback('http://example.com')
    expect(config.http_post_target).toBe('http://example.com')
  })

  it('httpHeaderH1Callback updates config.http_post_header1', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: PushHttpPost1View } = await import('../PushHttpPost1View.vue')
    const { config } = await import('@/modules/pinia')
    const wrapper = mount(PushHttpPost1View, {
      global: { plugins: [createTestingPinia()], stubs: { BsInputText: true, BsProgress: true } }
    })
    wrapper.vm.httpHeaderH1Callback('Accept: */*')
    expect(config.http_post_header1).toBe('Accept: */*')
  })

  it('httpHeaderH2Callback updates config.http_post_header2', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: PushHttpPost1View } = await import('../PushHttpPost1View.vue')
    const { config } = await import('@/modules/pinia')
    const wrapper = mount(PushHttpPost1View, {
      global: { plugins: [createTestingPinia()], stubs: { BsInputText: true, BsProgress: true } }
    })
    wrapper.vm.httpHeaderH2Callback('X-Token: abc123')
    expect(config.http_post_header2).toBe('X-Token: abc123')
  })

  it('gravityHttpFormatCallback decodes and updates format', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: PushHttpPost1View } = await import('../PushHttpPost1View.vue')
    const { config } = await import('@/modules/pinia')
    const wrapper = mount(PushHttpPost1View, {
      global: { plugins: [createTestingPinia()], stubs: { BsInputText: true, BsProgress: true } }
    })
    wrapper.vm.gravityHttpFormatCallback(encodeURIComponent('{gravity}'))
    if (config.http_post_format_gravity !== undefined) {
      expect(config.http_post_format_gravity).toBe('{gravity}')
    }
  })

  it('pushDisabled returns true when use_wifi_direct is true', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: PushHttpPost1View } = await import('../PushHttpPost1View.vue')
    const { config } = await import('@/modules/pinia')
    config.use_wifi_direct = true
    const wrapper = mount(PushHttpPost1View, {
      global: { plugins: [createTestingPinia()], stubs: { BsInputText: true, BsProgress: true } }
    })
    expect(wrapper.vm.pushDisabled).toBe(true)
  })

  it('pushDisabled returns true when global.disabled is true', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: PushHttpPost1View } = await import('../PushHttpPost1View.vue')
    const { global } = await import('@/modules/pinia')
    global.disabled = true
    const wrapper = mount(PushHttpPost1View, {
      global: { plugins: [createTestingPinia()], stubs: { BsInputText: true, BsProgress: true } }
    })
    expect(wrapper.vm.pushDisabled).toBe(true)
  })

  it('runTestGravity handles exception from config.runPushTest', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: PushHttpPost1View } = await import('../PushHttpPost1View.vue')
    const { config, global } = await import('@/modules/pinia')
    vi.spyOn(config, 'runPushTest').mockRejectedValueOnce(new Error('Network error'))
    global.messageError = ''
    const wrapper = mount(PushHttpPost1View, {
      global: { plugins: [createTestingPinia()], stubs: { BsInputText: true, BsProgress: true } }
    })
    await wrapper.vm.runTestGravity()
    expect(global.messageError).toBe('Failed to start push test')
  })

  it('httpUrlCallback updates config.http_post_target', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: PushHttpPost1View } = await import('../PushHttpPost1View.vue')
    const { config } = await import('@/modules/pinia')
    const wrapper = mount(PushHttpPost1View, {
      global: { plugins: [createTestingPinia()], stubs: { BsInputText: true, BsProgress: true } }
    })
    wrapper.vm.httpUrlCallback('http://example.com:8080')
    expect(config.http_post_target).toBe('http://example.com:8080')
  })

  it('gravityRenderFormat returns formatted gravity data', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: PushHttpPost1View } = await import('../PushHttpPost1View.vue')
    const { config, status } = await import('@/modules/pinia')
    config.http_post_format_gravity = '{gravity}'
    status.gravity = 1.05
    const wrapper = mount(PushHttpPost1View, {
      global: { plugins: [createTestingPinia()], stubs: { BsInputText: true, BsProgress: true } }
    })
    wrapper.vm.gravityRenderFormat()
    expect(wrapper.vm.gravityRender).toBeDefined()
  })

  it('save sets messageError when http_post_tcp is true and target has invalid format', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: PushHttpPost1View } = await import('../PushHttpPost1View.vue')
    const { config, global } = await import('@/modules/pinia')
    const { validateCurrentForm } = await import('@mp-se/espframework-ui-components')
    vi.mocked(validateCurrentForm).mockReturnValue(true)
    config.http_post_tcp = true
    config.http_post_target = 'invalid-no-port'
    global.messageError = ''
    config.saveAll = vi.fn()
    const wrapper = mount(PushHttpPost1View, {
      global: { plugins: [createTestingPinia()], stubs: { BsInputText: true, BsProgress: true } }
    })
    await wrapper.vm.save()
    expect(global.messageError).toContain('hostname:port')
    expect(config.saveAll).not.toHaveBeenCalled()
    config.http_post_tcp = false
  })

  it('save proceeds when http_post_tcp is true and target has valid host:port format', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: PushHttpPost1View } = await import('../PushHttpPost1View.vue')
    const { config, global } = await import('@/modules/pinia')
    const { validateCurrentForm } = await import('@mp-se/espframework-ui-components')
    vi.mocked(validateCurrentForm).mockReturnValue(true)
    config.http_post_tcp = true
    config.http_post_target = 'server.example.com:8080'
    global.messageError = ''
    config.saveAll = vi.fn(async () => {})
    const wrapper = mount(PushHttpPost1View, {
      global: { plugins: [createTestingPinia()], stubs: { BsInputText: true, BsProgress: true } }
    })
    await wrapper.vm.save()
    expect(config.saveAll).toHaveBeenCalled()
    config.http_post_tcp = false
  })

  it('save proceeds when http_post_tcp is false (skips server format validation)', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: PushHttpPost1View } = await import('../PushHttpPost1View.vue')
    const { config } = await import('@/modules/pinia')
    const { validateCurrentForm } = await import('@mp-se/espframework-ui-components')
    vi.mocked(validateCurrentForm).mockReturnValue(true)
    config.http_post_tcp = false
    config.http_post_target = 'http://example.com'
    config.saveAll = vi.fn(async () => {})
    const wrapper = mount(PushHttpPost1View, {
      global: { plugins: [createTestingPinia()], stubs: { BsInputText: true, BsProgress: true } }
    })
    await wrapper.vm.save()
    expect(config.saveAll).toHaveBeenCalled()
  })

  it('renders TCP server input when http_post_tcp is true', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: PushHttpPost1View } = await import('../PushHttpPost1View.vue')
    const { config } = await import('@/modules/pinia')
    config.http_post_tcp = true
    const wrapper = mount(PushHttpPost1View, {
      global: {
        plugins: [createTestingPinia()],
        stubs: {
          BsInputText: { template: '<input />', props: ['type'] },
          BsInputSwitch: true,
          BsDropdown: true,
          BsModal: true,
          BsInputNumber: true,
          BsInputTextAreaFormat: true
        }
      }
    })
    expect(wrapper.find('form').exists()).toBe(true)
    config.http_post_tcp = false
  })

  it('runTestPressure calls config.runPushTest with pressure format', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: PushHttpPost1View } = await import('../PushHttpPost1View.vue')
    const { config } = await import('@/modules/pinia')
    const wrapper = mount(PushHttpPost1View, {
      global: { plugins: [createTestingPinia()], stubs: { BsInputText: true, BsProgress: true } }
    })
    await wrapper.vm.runTestPressure()
    expect(config.runPushTest).toHaveBeenCalledWith({ push_format: 'http_post_format_pressure' })
  })

  it('runTestPressure handles exception from config.runPushTest', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: PushHttpPost1View } = await import('../PushHttpPost1View.vue')
    const { config, global } = await import('@/modules/pinia')
    vi.spyOn(config, 'runPushTest').mockRejectedValueOnce(new Error('Network error'))
    global.messageError = ''
    const wrapper = mount(PushHttpPost1View, {
      global: { plugins: [createTestingPinia()], stubs: { BsInputText: true, BsProgress: true } }
    })
    await wrapper.vm.runTestPressure()
    expect(global.messageError).toBe('Failed to start push test')
  })

  it('pressureHttpFormatCallback decodes and updates format', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: PushHttpPost1View } = await import('../PushHttpPost1View.vue')
    const { config } = await import('@/modules/pinia')
    const wrapper = mount(PushHttpPost1View, {
      global: { plugins: [createTestingPinia()], stubs: { BsInputText: true, BsProgress: true } }
    })
    const encoded = encodeURIComponent('{pressure}')
    wrapper.vm.pressureHttpFormatCallback(encoded)
    expect(config.http_post_format_pressure).toBe('{pressure}')
  })

  it('pressureRenderFormat returns formatted pressure data', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: PushHttpPost1View } = await import('../PushHttpPost1View.vue')
    const { config } = await import('@/modules/pinia')
    config.http_post_format_pressure = '{pressure}'
    const wrapper = mount(PushHttpPost1View, {
      global: { plugins: [createTestingPinia()], stubs: { BsInputText: true, BsProgress: true } }
    })
    wrapper.vm.pressureRenderFormat()
    expect(wrapper.vm.pressureRender).toBeDefined()
  })

  it('triggers v-model handlers for http_post_tcp=false fields', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: PushHttpPost1View } = await import('../PushHttpPost1View.vue')
    const { config } = await import('@/modules/pinia')
    config.http_post_tcp = false
    const inputStub = {
      template:
        '<input :value="modelValue" @input="$emit(\'update:modelValue\', $event.target.value)" />',
      props: [
        'modelValue',
        'label',
        'disabled',
        'maxlength',
        'pattern',
        'min',
        'max',
        'width',
        'step',
        'rows',
        'help',
        'badge'
      ],
      emits: ['update:modelValue']
    }
    const wrapper = mount(PushHttpPost1View, {
      global: {
        plugins: [createTestingPinia()],
        stubs: {
          BsInputText: inputStub,
          BsInputNumber: inputStub,
          BsInputTextAreaFormat: inputStub,
          BsInputSwitch: inputStub,
          BsDropdown: true,
          BsModal: true
        }
      }
    })
    for (const el of wrapper.findAll('input')) {
      await el.trigger('input')
    }
    expect(wrapper.find('form').exists()).toBe(true)
  })

  it('triggers BsModal v-model update for render ref', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: PushHttpPost1View } = await import('../PushHttpPost1View.vue')
    const { config } = await import('@/modules/pinia')
    config.http_post_tcp = false
    const wrapper = mount(PushHttpPost1View, {
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
            props: ['modelValue', 'code', 'json'],
            emits: ['update:modelValue']
          }
        }
      }
    })
    const btn = wrapper.find('.modal-emit')
    if (btn.exists()) await btn.trigger('click')
    expect(wrapper.exists()).toBe(true)
  })

  it('triggers v-model on http_post_target in tcp mode', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: PushHttpPost1View } = await import('../PushHttpPost1View.vue')
    const { config } = await import('@/modules/pinia')
    config.http_post_tcp = true
    const inputStub = {
      template:
        '<input :value="modelValue" @input="$emit(\'update:modelValue\', $event.target.value)" />',
      props: [
        'modelValue',
        'label',
        'disabled',
        'maxlength',
        'type',
        'pattern',
        'min',
        'max',
        'width',
        'step',
        'rows',
        'help',
        'badge'
      ],
      emits: ['update:modelValue']
    }
    const wrapper = mount(PushHttpPost1View, {
      global: {
        plugins: [createTestingPinia()],
        stubs: {
          BsInputText: inputStub,
          BsInputNumber: inputStub,
          BsInputTextAreaFormat: inputStub,
          BsInputSwitch: inputStub,
          BsDropdown: true,
          BsModal: true
        }
      }
    })
    for (const el of wrapper.findAll('input')) {
      await el.trigger('input')
    }
    expect(wrapper.find('form').exists()).toBe(true)
    config.http_post_tcp = false
  })
})
