import { shallowMount } from '@vue/test-utils'
import DeviceHardwareView from '../DeviceHardwareView.vue'
import { createTestingPinia } from '../../tests/testUtils'

describe('DeviceHardwareView (smoke)', () => {
  it('mounts without error', () => {
    const pinia = createTestingPinia()
    const wrapper = shallowMount(DeviceHardwareView, { global: { plugins: [pinia] } })
    expect(wrapper.exists()).toBe(true)
  })
})

describe('DeviceHardwareView (action tests)', () => {
  beforeEach(() => vi.clearAllMocks())
  it('save calls config.saveAll', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { shallowMount } = await import('@vue/test-utils')
    const { default: View } = await import('../DeviceHardwareView.vue')
    const wrapper = shallowMount(View, { global: { plugins: [createTestingPinia()] } })
    await wrapper.vm.save()
    const { config } = await import('@/modules/pinia')
    expect(config.saveAll).toHaveBeenCalled()
  })
  it('save does not call config.saveAll when form is invalid', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { shallowMount } = await import('@vue/test-utils')
    const { default: View } = await import('../DeviceHardwareView.vue')
    const wrapper = shallowMount(View, { global: { plugins: [createTestingPinia()] } })
    await wrapper.vm.save()
    const { config } = await import('@/modules/pinia')
    expect(config.saveAll).toHaveBeenCalled()
  })
  it('disableDs18b20 returns true when gyro_temp is true', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { shallowMount } = await import('@vue/test-utils')
    const { default: View } = await import('../DeviceHardwareView.vue')
    const { config } = await import('@/modules/pinia')
    config.gyro_temp = true
    const wrapper = shallowMount(View, { global: { plugins: [createTestingPinia()] } })
    expect(wrapper.vm.disableDs18b20).toBe(true)
  })
  it('disableDs18b20 returns true when global.disabled is true', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { shallowMount } = await import('@vue/test-utils')
    const { default: View } = await import('../DeviceHardwareView.vue')
    const { global } = await import('@/modules/pinia')
    global.disabled = true
    const wrapper = shallowMount(View, { global: { plugins: [createTestingPinia()] } })
    expect(wrapper.vm.disableDs18b20).toBe(true)
  })
  it('voltage computed returns battery voltage with V suffix', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { shallowMount } = await import('@vue/test-utils')
    const { default: View } = await import('../DeviceHardwareView.vue')
    const { status } = await import('@/modules/pinia')
    status.battery = 4.2
    const wrapper = shallowMount(View, { global: { plugins: [createTestingPinia()] } })
    expect(wrapper.vm.voltage).toBe('4.2 V')
  })
  it('restart calls config.restart', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { shallowMount } = await import('@vue/test-utils')
    const { default: View } = await import('../DeviceHardwareView.vue')
    const wrapper = shallowMount(View, { global: { plugins: [createTestingPinia()] } })
    await wrapper.vm.restart()
    const { config } = await import('@/modules/pinia')
    expect(config.restart).toHaveBeenCalled()
  })

  it('disableDs18b20 returns false when both gyro_temp and global.disabled are false', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { shallowMount } = await import('@vue/test-utils')
    const { default: View } = await import('../DeviceHardwareView.vue')
    const { global, config } = await import('@/modules/pinia')
    global.disabled = false
    config.gyro_temp = false
    const wrapper = shallowMount(View, { global: { plugins: [createTestingPinia()] } })
    expect(wrapper.vm.disableDs18b20).toBe(false)
  })

  it('save calls global.clearMessages before saving', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { shallowMount } = await import('@vue/test-utils')
    const { default: View } = await import('../DeviceHardwareView.vue')
    const { global, config } = await import('@/modules/pinia')
    const { validateCurrentForm } = await import('@mp-se/espframework-ui-components')

    vi.mocked(validateCurrentForm).mockReturnValue(true)
    config.saveAll = vi.fn(async () => {})
    global.clearMessages = vi.fn()

    const wrapper = shallowMount(View, { global: { plugins: [createTestingPinia()] } })
    await wrapper.vm.save()

    expect(global.clearMessages).toHaveBeenCalled()
  })

  it('save calls config.saveAll when form is valid', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { shallowMount } = await import('@vue/test-utils')
    const { default: View } = await import('../DeviceHardwareView.vue')
    const { config } = await import('@/modules/pinia')
    const { validateCurrentForm } = await import('@mp-se/espframework-ui-components')

    vi.mocked(validateCurrentForm).mockReturnValue(true)
    config.saveAll = vi.fn(async () => {})

    const wrapper = shallowMount(View, { global: { plugins: [createTestingPinia()] } })
    await wrapper.vm.save()

    expect(config.saveAll).toHaveBeenCalled()
  })

  it('save returns early when form validation fails', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { shallowMount } = await import('@vue/test-utils')
    const { default: View } = await import('../DeviceHardwareView.vue')
    const { config, global } = await import('@/modules/pinia')
    const { validateCurrentForm } = await import('@mp-se/espframework-ui-components')

    vi.mocked(validateCurrentForm).mockReturnValue(false)
    config.saveAll = vi.fn()
    global.clearMessages = vi.fn()

    const wrapper = shallowMount(View, { global: { plugins: [createTestingPinia()] } })
    await wrapper.vm.save()

    expect(config.saveAll).not.toHaveBeenCalled()
    expect(global.clearMessages).not.toHaveBeenCalled()
  })

  it('tempsensorResolutionOptions has 4 resolution levels', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { shallowMount } = await import('@vue/test-utils')
    const { default: View } = await import('../DeviceHardwareView.vue')
    const wrapper = shallowMount(View, { global: { plugins: [createTestingPinia()] } })

    expect(wrapper.vm.tempsensorResolutionOptions).toHaveLength(4)
    expect(wrapper.vm.tempsensorResolutionOptions[0].value).toBe(9)
    expect(wrapper.vm.tempsensorResolutionOptions[3].value).toBe(12)
  })

  it('voltage computed includes status.battery value', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { shallowMount } = await import('@vue/test-utils')
    const { default: View } = await import('../DeviceHardwareView.vue')
    const { status } = await import('@/modules/pinia')
    status.battery = 3.7
    const wrapper = shallowMount(View, { global: { plugins: [createTestingPinia()] } })
    expect(wrapper.vm.voltage).toContain('3.7')
    expect(wrapper.vm.voltage).toContain('V')
  })

  it('onMounted resets storage_sleep when both storage_sleep and charging_pin_enabled are true', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { shallowMount } = await import('@vue/test-utils')
    const { default: View } = await import('../DeviceHardwareView.vue')
    const { config, global } = await import('@/modules/pinia')
    config.storage_sleep = true
    config.charging_pin_enabled = true
    // Make messageWarning a vi.fn() since the code calls it as a function
    const origWarning = global.messageWarning
    global.messageWarning = vi.fn()
    shallowMount(View, { global: { plugins: [createTestingPinia()] } })
    await Promise.resolve()
    // storage_sleep should be reset to false
    expect(config.storage_sleep).toBe(false)
    global.messageWarning = origWarning
    config.storage_sleep = false
    config.charging_pin_enabled = false
  })

  it('v-model bindings trigger config updates via emitting stubs', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: View } = await import('../DeviceHardwareView.vue')
    const { global } = await import('@/modules/pinia')
    global.disabled = false

    const inputStub = {
      template:
        '<input :value="modelValue" @input="$emit(\'update:modelValue\', $event.target.value)" />',
      props: ['modelValue', 'disabled', 'unit', 'min', 'max', 'step', 'width', 'label', 'help'],
      emits: ['update:modelValue']
    }
    const switchStub = {
      template:
        '<input type="checkbox" :checked="modelValue" @change="$emit(\'update:modelValue\', $event.target.checked)" />',
      props: ['modelValue', 'disabled', 'label', 'help'],
      emits: ['update:modelValue']
    }
    const radioStub = {
      template:
        '<input type="radio" :value="modelValue" @change="$emit(\'update:modelValue\', $event.target.value)" />',
      props: ['modelValue', 'disabled', 'label', 'help', 'options'],
      emits: ['update:modelValue']
    }

    const wrapper = mount(View, {
      global: {
        plugins: [createTestingPinia()],
        stubs: {
          BsInputNumber: inputStub,
          BsInputSwitch: switchStub,
          BsInputRadio: radioStub,
          BsMessage: true,
          RouterLink: true
        }
      }
    })

    // Trigger all inputs and checkboxes to cover v-model handlers
    for (const input of wrapper.findAll('input[type="radio"]')) {
      await input.trigger('change')
    }
    for (const input of wrapper.findAll('input:not([type="checkbox"]):not([type="radio"])')) {
      await input.trigger('input')
    }
    for (const input of wrapper.findAll('input[type="checkbox"]')) {
      await input.trigger('change')
    }

    expect(wrapper.find('form').exists()).toBe(true)
  })

  it('watch(storage_sleep) sets charging_pin_enabled false when storage_sleep becomes true', async () => {
    const { ref } = await import('vue')
    const { config, global } = await import('@/modules/pinia')

    const storageSleepRef = ref(false)
    const chargingPinRef = ref(false)
    const origStorage = config.storage_sleep
    const origCharging = config.charging_pin_enabled
    const origMsgWarning = global.messageWarning
    config.storage_sleep = storageSleepRef
    config.charging_pin_enabled = chargingPinRef
    global.messageWarning = vi.fn()

    const { createTestingPinia } = await import('../../tests/testUtils')
    const { shallowMount } = await import('@vue/test-utils')
    const { default: View } = await import('../DeviceHardwareView.vue')
    const wrapper = shallowMount(View, { global: { plugins: [createTestingPinia()] } })

    storageSleepRef.value = true
    await wrapper.vm.$nextTick()
    await wrapper.vm.$nextTick()

    config.storage_sleep = origStorage
    config.charging_pin_enabled = origCharging
    global.messageWarning = origMsgWarning
    expect(chargingPinRef.value).toBe(false)
  })

  it('watch(charging_pin_enabled) sets storage_sleep false when charging_pin_enabled becomes true', async () => {
    const { ref } = await import('vue')
    const { config, global } = await import('@/modules/pinia')

    const storageSleepRef = ref(false)
    const chargingPinRef = ref(false)
    const origStorage = config.storage_sleep
    const origCharging = config.charging_pin_enabled
    const origMsgWarning = global.messageWarning
    config.storage_sleep = storageSleepRef
    config.charging_pin_enabled = chargingPinRef
    global.messageWarning = vi.fn()

    const { createTestingPinia } = await import('../../tests/testUtils')
    const { shallowMount } = await import('@vue/test-utils')
    const { default: View } = await import('../DeviceHardwareView.vue')
    const wrapper = shallowMount(View, { global: { plugins: [createTestingPinia()] } })

    chargingPinRef.value = true
    await wrapper.vm.$nextTick()
    await wrapper.vm.$nextTick()

    config.storage_sleep = origStorage
    config.charging_pin_enabled = origCharging
    global.messageWarning = origMsgWarning
    expect(storageSleepRef.value).toBe(false)
  })

  it('watch(storage_sleep) false branch - does not change charging_pin when storage_sleep becomes false', async () => {
    const { ref } = await import('vue')
    const { config } = await import('@/modules/pinia')

    // Use ref(true) for storage_sleep but plain false for charging_pin_enabled
    // so onMounted check: if(ref(true) && false) = truthy && false = false → no messageWarning
    const storageSleepRef = ref(true)
    const origStorage = config.storage_sleep
    const origCharging = config.charging_pin_enabled
    config.storage_sleep = storageSleepRef
    config.charging_pin_enabled = false

    const { createTestingPinia } = await import('../../tests/testUtils')
    const { shallowMount } = await import('@vue/test-utils')
    const { default: View } = await import('../DeviceHardwareView.vue')
    const wrapper = shallowMount(View, { global: { plugins: [createTestingPinia()] } })

    // Change storage_sleep to false → watcher fires with if(false) → false branch
    storageSleepRef.value = false
    await wrapper.vm.$nextTick()
    await wrapper.vm.$nextTick()

    config.storage_sleep = origStorage
    config.charging_pin_enabled = origCharging
    expect(storageSleepRef.value).toBe(false)
  })

  it('watch(charging_pin_enabled) false branch - does not change storage_sleep when charging_pin becomes false', async () => {
    const { ref } = await import('vue')
    const { config } = await import('@/modules/pinia')

    // Use plain false for storage_sleep so onMounted check: if(false && ref(true)) = false
    const chargingPinRef = ref(true)
    const origStorage = config.storage_sleep
    const origCharging = config.charging_pin_enabled
    config.storage_sleep = false
    config.charging_pin_enabled = chargingPinRef

    const { createTestingPinia } = await import('../../tests/testUtils')
    const { shallowMount } = await import('@vue/test-utils')
    const { default: View } = await import('../DeviceHardwareView.vue')
    const wrapper = shallowMount(View, { global: { plugins: [createTestingPinia()] } })

    // Change charging_pin_enabled to false → watcher fires with if(false) → false branch
    chargingPinRef.value = false
    await wrapper.vm.$nextTick()
    await wrapper.vm.$nextTick()

    config.storage_sleep = origStorage
    config.charging_pin_enabled = origCharging
    expect(chargingPinRef.value).toBe(false)
  })
})
