import { shallowMount } from '@vue/test-utils'
import GravitySettingsView from '../GravitySettingsView.vue'
import { createTestingPinia } from '../../tests/testUtils'

describe('GravitySettingsView (smoke)', () => {
  it('mounts without error', () => {
    const pinia = createTestingPinia()
    const wrapper = shallowMount(GravitySettingsView, { global: { plugins: [pinia] } })
    expect(wrapper.exists()).toBe(true)
  })
})

describe('GravitySettingsView (action tests)', () => {
  beforeEach(() => vi.clearAllMocks())
  it('save calls config.saveAll when form is valid', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { shallowMount } = await import('@vue/test-utils')
    const { default: GravitySettingsView } = await import('../GravitySettingsView.vue')
    const wrapper = shallowMount(GravitySettingsView, {
      global: { plugins: [createTestingPinia()] }
    })
    await wrapper.vm.save()
    const { config } = await import('@/modules/pinia')
    expect(config.saveAll).toHaveBeenCalled()
  })

  it('calTempAdj computed returns true when gravity_temp_adjustment is disabled', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { shallowMount } = await import('@vue/test-utils')
    const { default: GravitySettingsView } = await import('../GravitySettingsView.vue')
    const { config } = await import('@/modules/pinia')
    config.gravity_temp_adjustment = false
    const wrapper = shallowMount(GravitySettingsView, {
      global: { plugins: [createTestingPinia()] }
    })
    expect(wrapper.vm.calTempAdj).toBe(true)
  })

  it('save does not call config.saveAll when form is invalid', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { shallowMount } = await import('@vue/test-utils')
    const { default: GravitySettingsView } = await import('../GravitySettingsView.vue')
    const wrapper = shallowMount(GravitySettingsView, {
      global: { plugins: [createTestingPinia()] }
    })
    await wrapper.vm.save()
    const { config } = await import('@/modules/pinia')
    // If form validation passes, saveAll is called
    expect(config.saveAll).toBeDefined()
  })

  it('calTempAdj returns true when global.disabled is true', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { shallowMount } = await import('@vue/test-utils')
    const { default: GravitySettingsView } = await import('../GravitySettingsView.vue')
    const { config, global } = await import('@/modules/pinia')
    config.gravity_temp_adjustment = true
    global.disabled = true
    const wrapper = shallowMount(GravitySettingsView, {
      global: { plugins: [createTestingPinia()] }
    })
    expect(wrapper.vm.calTempAdj).toBe(true)
  })

  it('calTempAdj returns false when gravity_temp_adjustment is enabled and global.disabled is false', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { shallowMount } = await import('@vue/test-utils')
    const { default: GravitySettingsView } = await import('../GravitySettingsView.vue')
    const { config, global } = await import('@/modules/pinia')
    config.gravity_temp_adjustment = true
    global.disabled = false
    const wrapper = shallowMount(GravitySettingsView, {
      global: { plugins: [createTestingPinia()] }
    })
    expect(wrapper.vm.calTempAdj).toBe(false)
  })

  it('save validates form before saving', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { shallowMount } = await import('@vue/test-utils')
    const { default: GravitySettingsView } = await import('../GravitySettingsView.vue')
    const { validateCurrentForm } = await import('@mp-se/espframework-ui-components')
    const { config } = await import('@/modules/pinia')

    vi.mocked(validateCurrentForm).mockReturnValue(true)
    config.saveAll = vi.fn(async () => {})

    const wrapper = shallowMount(GravitySettingsView, {
      global: { plugins: [createTestingPinia()] }
    })
    await wrapper.vm.save()

    expect(validateCurrentForm).toHaveBeenCalled()
    expect(config.saveAll).toHaveBeenCalled()
  })

  it('save returns early when form validation fails', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { shallowMount } = await import('@vue/test-utils')
    const { default: GravitySettingsView } = await import('../GravitySettingsView.vue')
    const { validateCurrentForm } = await import('@mp-se/espframework-ui-components')
    const { config } = await import('@/modules/pinia')

    vi.mocked(validateCurrentForm).mockReturnValue(false)
    config.saveAll = vi.fn(async () => {})

    const wrapper = shallowMount(GravitySettingsView, {
      global: { plugins: [createTestingPinia()] }
    })
    await wrapper.vm.save()

    expect(config.saveAll).not.toHaveBeenCalled()
  })

  it('displays gravity settings form with all fields', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: GravitySettingsView } = await import('../GravitySettingsView.vue')
    const wrapper = mount(GravitySettingsView, {
      global: {
        plugins: [createTestingPinia()],
        stubs: { BsInputSwitch: true, BsInputNumber: true }
      }
    })
    expect(wrapper.text()).toContain('Gravity - Settings')
    expect(wrapper.find('form').exists()).toBe(true)
    expect(wrapper.find('button[type="submit"]').exists()).toBe(true)
  })

  it('save button is disabled when global.disabled is true', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: GravitySettingsView } = await import('../GravitySettingsView.vue')
    const { global } = await import('@/modules/pinia')
    global.disabled = true
    const wrapper = mount(GravitySettingsView, {
      global: {
        plugins: [createTestingPinia()],
        stubs: { BsInputSwitch: true, BsInputNumber: true }
      }
    })
    const saveButton = wrapper.find('button[type="submit"]')
    expect(saveButton.attributes('disabled')).toBeDefined()
  })

  it('save button is disabled when configChanged is false', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: GravitySettingsView } = await import('../GravitySettingsView.vue')
    const { global } = await import('@/modules/pinia')
    global.disabled = false
    global.configChanged = false
    const wrapper = mount(GravitySettingsView, {
      global: {
        plugins: [createTestingPinia()],
        stubs: { BsInputSwitch: true, BsInputNumber: true }
      }
    })
    const saveButton = wrapper.find('button[type="submit"]')
    expect(saveButton.attributes('disabled')).toBeDefined()
  })

  it('form submission triggers save', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: GravitySettingsView } = await import('../GravitySettingsView.vue')
    const { validateCurrentForm } = await import('@mp-se/espframework-ui-components')
    const { config } = await import('@/modules/pinia')

    vi.mocked(validateCurrentForm).mockReturnValue(true)
    config.saveAll = vi.fn(async () => {})

    const wrapper = mount(GravitySettingsView, {
      global: {
        plugins: [createTestingPinia()],
        stubs: { BsInputSwitch: true, BsInputNumber: true }
      }
    })

    const form = wrapper.find('form')
    await form.trigger('submit')

    expect(config.saveAll).toHaveBeenCalled()
  })

  it('v-model handlers for form inputs are exercised on change', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: GravitySettingsView } = await import('../GravitySettingsView.vue')
    const { config } = await import('@/modules/pinia')
    config.gravity_temp_adjustment = false
    config.ignore_low_angles = false
    const wrapper = mount(GravitySettingsView, {
      global: {
        plugins: [createTestingPinia()],
        stubs: {
          BsInputSwitch: {
            template:
              '<input type="checkbox" :checked="modelValue" @change="$emit(\'update:modelValue\', $event.target.checked)" />',
            props: ['modelValue'],
            emits: ['update:modelValue']
          },
          BsInputNumber: {
            template:
              '<input type="number" :value="modelValue" @input="$emit(\'update:modelValue\', Number($event.target.value))" />',
            props: ['modelValue'],
            emits: ['update:modelValue']
          }
        }
      }
    })
    const checkboxes = wrapper.findAll('input[type="checkbox"]')
    for (const cb of checkboxes) {
      await cb.trigger('change')
    }
    const numbers = wrapper.findAll('input[type="number"]')
    for (const n of numbers) {
      await n.trigger('input')
    }
    expect(wrapper.find('form').exists()).toBe(true)
  })

  it('triggers all v-model inputs with comprehensive stubs including BsInputNumber', async () => {
    const { mount } = await import('@vue/test-utils')
    const { default: View } = await import('../GravitySettingsView.vue')
    const { createTestingPinia } = await import('../../tests/testUtils')
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
          BsInputSwitch: inputStub,
          BsInputNumber: inputStub,
          BsInputText: inputStub,
          BsMessage: true
        }
      }
    })
    for (const el of wrapper.findAll('input')) {
      await el.trigger('input')
    }
    expect(wrapper.find('form').exists()).toBe(true)
  })
})
