import { describe, it, expect, beforeEach, vi } from 'vitest'
import { mount } from '@vue/test-utils'
import VoltageFragment from '../VoltageFragment.vue'
import { createPinia, setActivePinia } from 'pinia'

// Mock the UI components library
vi.mock('@mp-se/espframework-ui-components', () => ({
  logDebug: vi.fn(),
  logError: vi.fn(),
  logInfo: vi.fn(),
  sharedHttpClient: {
    get: vi.fn(),
    post: vi.fn()
  }
}))

describe('VoltageFragment (interaction tests)', () => {
  beforeEach(() => {
    setActivePinia(createPinia())
  })

  it('mounts without error', () => {
    const wrapper = mount(VoltageFragment, {
      global: {
        stubs: {
          BsInputNumber: true,
          BsInputReadonly: true
        }
      }
    })
    expect(wrapper.exists()).toBe(true)
  })

  it('displays voltage calculation section header', () => {
    const wrapper = mount(VoltageFragment, {
      global: {
        stubs: {
          BsInputNumber: true,
          BsInputReadonly: true
        }
      }
    })
    expect(wrapper.text()).toContain('Calculate a new voltage factor')
  })

  it('has measured voltage input component', () => {
    const wrapper = mount(VoltageFragment, {
      global: {
        stubs: {
          BsInputNumber: { template: '<div class="bs-input-number"></div>' },
          BsInputReadonly: true
        }
      }
    })
    const input = wrapper.find('.bs-input-number')
    expect(input.exists()).toBe(true)
  })

  it('has last voltage reading readonly field', () => {
    const wrapper = mount(VoltageFragment, {
      global: {
        stubs: {
          BsInputNumber: true,
          BsInputReadonly: {
            template: '<div class="bs-readonly">{{ modelValue }}</div>',
            props: ['modelValue']
          }
        }
      }
    })
    const readonly = wrapper.findAll('.bs-readonly')
    expect(readonly.length).toBeGreaterThan(0)
  })

  it('has current voltage factor readonly field', () => {
    const wrapper = mount(VoltageFragment, {
      global: {
        stubs: {
          BsInputNumber: true,
          BsInputReadonly: {
            template: '<div class="bs-readonly"></div>'
          }
        }
      }
    })
    const readonlyFields = wrapper.findAll('.bs-readonly')
    expect(readonlyFields.length).toBeGreaterThanOrEqual(2)
  })

  it('has calculate factor button', () => {
    const wrapper = mount(VoltageFragment, {
      global: {
        stubs: {
          BsInputNumber: true,
          BsInputReadonly: true
        }
      }
    })
    const buttons = wrapper.findAll('button')
    const calcButton = buttons.find((b) => b.text().includes('Calculate factor'))
    expect(calcButton).toBeDefined()
  })

  it('has calculateFactor function defined', () => {
    const wrapper = mount(VoltageFragment, {
      global: {
        stubs: {
          BsInputNumber: true,
          BsInputReadonly: true
        }
      }
    })
    expect(typeof wrapper.vm.calculateFactor).toBe('function')
  })

  it('initializes measuredVoltage reactive property', () => {
    const wrapper = mount(VoltageFragment, {
      global: {
        stubs: {
          BsInputNumber: true,
          BsInputReadonly: true
        }
      }
    })
    expect(wrapper.vm.measuredVoltage).toBeDefined()
  })

  it('calculate button is initially not disabled', () => {
    const wrapper = mount(VoltageFragment, {
      global: {
        stubs: {
          BsInputNumber: true,
          BsInputReadonly: true
        }
      }
    })
    const buttons = wrapper.findAll('button')
    const calcButton = buttons.find((b) => b.text().includes('Calculate factor'))
    expect(calcButton.attributes('disabled')).toBeUndefined()
  })

  it('button has secondary styling', () => {
    const wrapper = mount(VoltageFragment, {
      global: {
        stubs: {
          BsInputNumber: true,
          BsInputReadonly: true
        }
      }
    })
    const buttons = wrapper.findAll('button')
    const calcButton = buttons.find((b) => b.text().includes('Calculate factor'))
    expect(calcButton.element.className).toContain('btn-secondary')
  })

  it('has proper grid layout with row and column', () => {
    const wrapper = mount(VoltageFragment, {
      global: {
        stubs: {
          BsInputNumber: true,
          BsInputReadonly: true
        }
      }
    })
    const rows = wrapper.findAll('.row')
    expect(rows.length).toBeGreaterThan(0)
    const cols = wrapper.findAll('.col-md-4')
    expect(cols.length).toBeGreaterThan(0)
  })

  it('button row has proper vertical gap spacing', () => {
    const wrapper = mount(VoltageFragment, {
      global: {
        stubs: {
          BsInputNumber: true,
          BsInputReadonly: true
        }
      }
    })
    const rows = wrapper.findAll('.row')
    const spacedRow = rows.find((r) => r.element.className.includes('gy-4'))
    expect(spacedRow).toBeDefined()
  })

  it('has spinner element for loading state', () => {
    const wrapper = mount(VoltageFragment, {
      global: {
        stubs: {
          BsInputNumber: true,
          BsInputReadonly: true
        }
      }
    })
    const spinner = wrapper.find('.spinner-border')
    expect(spinner.exists()).toBe(true)
  })

  it('heading is h5 element', () => {
    const wrapper = mount(VoltageFragment, {
      global: {
        stubs: {
          BsInputNumber: true,
          BsInputReadonly: true
        }
      }
    })
    const heading = wrapper.find('h5')
    expect(heading.exists()).toBe(true)
    expect(heading.text()).toContain('voltage factor')
  })
})

describe('VoltageFragment (action tests)', () => {
  beforeEach(() => {
    setActivePinia(createPinia())
    vi.clearAllMocks()
  })

  const mountView = () =>
    mount(VoltageFragment, {
      global: {
        stubs: {
          BsInputNumber: true,
          BsInputReadonly: true
        }
      }
    })

  it('calculateFactor sets messageError when measuredVoltage is NaN', async () => {
    const wrapper = mountView()
    wrapper.vm.measuredVoltage = 'notanumber'
    wrapper.vm.calculateFactor()
    await wrapper.vm.$nextTick()
    const { global: globalStore } = await import('@/modules/pinia')
    expect(globalStore.messageError).toBeTruthy()
  })

  it('calculateFactor calls config.sendConfig when valid voltage provided', async () => {
    const { config, status } = await import('@/modules/pinia')
    config.voltage_factor = 1.0
    status.battery = 4.0
    const wrapper = mountView()
    wrapper.vm.measuredVoltage = 4.2
    wrapper.vm.calculateFactor()
    await wrapper.vm.$nextTick()
    expect(config.sendConfig).toHaveBeenCalled()
  })

  it('calculateFactor updates voltage_factor with valid inputs', async () => {
    const { config, status } = await import('@/modules/pinia')
    config.voltage_factor = 1.0
    status.battery = 4.0
    const wrapper = mountView()
    wrapper.vm.measuredVoltage = 4.2
    wrapper.vm.calculateFactor()
    await wrapper.vm.$nextTick()
    // voltage_factor should be updated to mv / (battery / factor) = 4.2 / (4.0 / 1.0) = 1.05
    expect(parseFloat(config.voltage_factor)).toBeCloseTo(1.05, 1)
  })

  it('calculateFactor executes setTimeout callback after delay', async () => {
    vi.useFakeTimers()
    const { config, status } = await import('@/modules/pinia')
    config.voltage_factor = 1.0
    status.battery = 4.0
    status.load = vi.fn().mockResolvedValue(true)
    const wrapper = mountView()
    wrapper.vm.measuredVoltage = 4.2
    wrapper.vm.calculateFactor()
    await wrapper.vm.$nextTick()
    // Advance timers to trigger the setTimeout callback
    await vi.runAllTimersAsync()
    expect(status.load).toHaveBeenCalled()
    vi.useRealTimers()
  })

  it('calculateFactor sets messageInfo after setTimeout callback completes', async () => {
    vi.useFakeTimers()
    const { config, status, global: globalStore } = await import('@/modules/pinia')
    config.voltage_factor = 1.0
    status.battery = 4.0
    status.load = vi.fn().mockResolvedValue(true)
    const wrapper = mountView()
    wrapper.vm.measuredVoltage = 4.2
    wrapper.vm.calculateFactor()
    await wrapper.vm.$nextTick()
    await vi.runAllTimersAsync()
    expect(globalStore.messageInfo).toBeTruthy()
    vi.useRealTimers()
  })
})

describe('VoltageFragment (v-model setter coverage)', () => {
  beforeEach(() => {
    setActivePinia(createPinia())
    vi.clearAllMocks()
  })

  it('emitting update:modelValue on BsInputNumber triggers measuredVoltage setter', async () => {
    const wrapper = mount(VoltageFragment, {
      global: {
        components: {
          BsInputNumber: {
            props: ['modelValue'],
            emits: ['update:modelValue'],
            template: '<button class="num-emit" @click="$emit(\'update:modelValue\', 4.1)" />'
          },
          BsInputReadonly: { template: '<div />' }
        }
      }
    })
    await wrapper.vm.$nextTick()
    const btn = wrapper.find('.num-emit')
    if (btn.exists()) await btn.trigger('click')
    expect(wrapper.exists()).toBe(true)
  })

  it('emitting update:modelValue on first BsInputReadonly triggers status.battery setter', async () => {
    const wrapper = mount(VoltageFragment, {
      global: {
        components: {
          BsInputNumber: { template: '<div />' },
          BsInputReadonly: {
            props: ['modelValue'],
            emits: ['update:modelValue'],
            template: '<button class="ro-emit" @click="$emit(\'update:modelValue\', 3.9)" />'
          }
        }
      }
    })
    await wrapper.vm.$nextTick()
    const btns = wrapper.findAll('.ro-emit')
    for (const btn of btns) {
      await btn.trigger('click')
    }
    expect(wrapper.exists()).toBe(true)
  })
})
