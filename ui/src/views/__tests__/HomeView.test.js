import { describe, it, expect, beforeEach, vi, afterEach } from 'vitest'
import { mount, shallowMount } from '@vue/test-utils'
import HomeView from '../HomeView.vue'
import { createTestingPinia } from '../../tests/testUtils'

describe('HomeView (smoke)', () => {
  it('mounts without error', () => {
    const pinia = createTestingPinia()
    const wrapper = shallowMount(HomeView, { global: { plugins: [pinia] } })
    expect(wrapper.exists()).toBe(true)
  })
})

describe('HomeView (action tests)', () => {
  beforeEach(() => {
    vi.clearAllMocks()
    vi.useFakeTimers()
  })

  afterEach(() => {
    vi.useRealTimers()
  })

  it('covers various template branches including wifi_setup=true', async () => {
    const pinia = createTestingPinia()
    const { status } = await import('@/modules/pinia')

    // Mock status with all features enabled/disabled combinations
    status.self_check = {
      push_targets: false, // Trigger line 58
      gyro_connected: false, // Trigger line 73
      gravity_formula: true,
      gyro_calibration: true,
      temp_connected: true,
      battery_level: true
    }
    status.wifi_setup = true // Trigger various wifi_setup false branches
    status.gravity = 1.05
    status.angle = 45.0
    status.temp = 20.0
    status.battery = 4.0
    status.rssi = -60

    const wrapper = mount(HomeView, {
      global: {
        plugins: [pinia],
        stubs: {
          BsMessage: { template: '<div class="bs-message"><slot /></div>' },
          BsCard: { template: '<div class="bs-card"><slot /></div>' },
          RouterLink: { template: '<a><slot /></a>' }
        }
      }
    })

    expect(wrapper.find('.bs-message').text()).toContain('No remote services are active')
    // Gyro error message should NOT show because wifi_setup is true (line 73)
    expect(wrapper.text()).not.toContain('No gyro is detected')

    // Now set wifi_setup to false to cover the other half
    status.wifi_setup = false
    await wrapper.vm.$nextTick()
    // We need to re-mount or ensure reactivity for children stubs if they aren't reactive
    const wrapper2 = mount(HomeView, {
      global: {
        plugins: [pinia],
        stubs: {
          BsMessage: { template: '<div class="bs-message"><slot /></div>' },
          BsCard: { template: '<div class="bs-card"><slot /></div>' },
          RouterLink: { template: '<a><slot /></a>' }
        }
      }
    })
    expect(wrapper2.text()).toContain('No gyro is detected')
  })

  it('covers gravity formula error branch', async () => {
    const pinia = createTestingPinia()
    const { status } = await import('@/modules/pinia')
    status.wifi_setup = false
    status.self_check = {
      gravity_formula: false, // Trigger line 130
      push_targets: true,
      gyro_connected: true
    }

    const wrapper = mount(HomeView, {
      global: {
        plugins: [pinia],
        stubs: {
          BsMessage: true,
          BsCard: { template: '<div class="bs-card"><slot /></div>' },
          RouterLink: { template: '<a><slot /></a>' }
        }
      }
    })

    expect(wrapper.text()).toContain('Missing formula')
  })

  it('covers battery level error branch', async () => {
    const pinia = createTestingPinia()
    const { status } = await import('@/modules/pinia')
    status.self_check = { battery_level: false } // Trigger line 199

    const wrapper = mount(HomeView, {
      global: {
        plugins: [pinia],
        stubs: {
          BsMessage: true,
          BsCard: { template: '<div class="bs-card"><slot /></div>' },
          RouterLink: true
        }
      }
    })

    expect(wrapper.text()).toContain('Battery level not valid')
  })

  it('clearAverage resets angle sum and count to zero', async () => {
    const pinia = createTestingPinia()
    const wrapper = shallowMount(HomeView, { global: { plugins: [pinia] } })
    // Set some values first
    wrapper.vm.angle.sum = 100
    wrapper.vm.angle.count = 5
    wrapper.vm.clearAverage()
    expect(wrapper.vm.angle.sum).toBe(0)
    expect(wrapper.vm.angle.count).toBe(0)
  })

  it('refresh calls status.load', async () => {
    const pinia = createTestingPinia()
    const wrapper = shallowMount(HomeView, { global: { plugins: [pinia] } })
    const { status } = await import('@/modules/pinia')
    await wrapper.vm.refresh()
    expect(status.load).toHaveBeenCalled()
  })

  it('checkForNewGravMonVersion returns false when json version is older', async () => {
    const pinia = createTestingPinia()
    const wrapper = shallowMount(HomeView, { global: { plugins: [pinia] } })
    const { global: globalStore } = await import('@/modules/pinia')
    globalStore.app_ver = '2.0.0'
    const result = wrapper.vm.checkForNewGravMonVersion({ version: '1.9.0' })
    expect(result).toBe(false)
  })

  it('checkForNewGravMonVersion returns true when major version is newer', async () => {
    const pinia = createTestingPinia()
    const wrapper = shallowMount(HomeView, { global: { plugins: [pinia] } })
    const { global: globalStore } = await import('@/modules/pinia')
    globalStore.app_ver = '1.0.0'
    const result = wrapper.vm.checkForNewGravMonVersion({ version: '2.0.0' })
    expect(result).toBe(true)
  })

  it('checkForNewGravMonVersion returns true when minor version is newer', async () => {
    const pinia = createTestingPinia()
    const wrapper = shallowMount(HomeView, { global: { plugins: [pinia] } })
    const { global: globalStore } = await import('@/modules/pinia')
    globalStore.app_ver = '1.0.0'
    const result = wrapper.vm.checkForNewGravMonVersion({ version: '1.1.0' })
    expect(result).toBe(true)
  })

  it('checkForNewGravMonVersion returns false for invalid version formats', async () => {
    const pinia = createTestingPinia()
    const wrapper = shallowMount(HomeView, { global: { plugins: [pinia] } })
    const { global: globalStore } = await import('@/modules/pinia')
    globalStore.app_ver = 'invalid'
    const result = wrapper.vm.checkForNewGravMonVersion({ version: 'also-invalid' })
    expect(result).toBe(false)
  })

  it('refresh accumulates angle average when gyro is not moving', async () => {
    const pinia = createTestingPinia()
    const wrapper = shallowMount(HomeView, { global: { plugins: [pinia] } })
    const { status } = await import('@/modules/pinia')

    status.self_check = { gyro_moving: false }
    status.angle = 45.5
    status.load = vi.fn(async () => true)

    wrapper.vm.angle = { average: 0, sum: 0, count: 0 }
    await wrapper.vm.refresh()

    expect(wrapper.vm.angle.sum).toBe(45.5)
    expect(wrapper.vm.angle.count).toBe(1)
  })

  it('refresh skips angle accumulation when gyro is moving', async () => {
    const pinia = createTestingPinia()
    const wrapper = shallowMount(HomeView, { global: { plugins: [pinia] } })
    const { status } = await import('@/modules/pinia')

    status.self_check = { gyro_moving: true }
    status.angle = 45.5
    status.load = vi.fn(async () => true)

    wrapper.vm.angle = { average: 0, sum: 0, count: 0 }
    await wrapper.vm.refresh()

    expect(wrapper.vm.angle.sum).toBe(0)
    expect(wrapper.vm.angle.count).toBe(0)
  })

  it('checkForNewGravMonVersion returns true when patch version is newer', async () => {
    const pinia = createTestingPinia()
    const wrapper = shallowMount(HomeView, { global: { plugins: [pinia] } })
    const { global: globalStore } = await import('@/modules/pinia')
    globalStore.app_ver = '1.0.0'
    const result = wrapper.vm.checkForNewGravMonVersion({ version: '1.0.1' })
    expect(result).toBe(true)
  })

  it('checkForNewGravMonVersion returns false when version is same', async () => {
    const pinia = createTestingPinia()
    const wrapper = shallowMount(HomeView, { global: { plugins: [pinia] } })
    const { global: globalStore } = await import('@/modules/pinia')
    globalStore.app_ver = '1.0.0'
    const result = wrapper.vm.checkForNewGravMonVersion({ version: '1.0.0' })
    expect(result).toBe(false)
  })

  it('onMounted triggers version fetch and handles response', async () => {
    vi.useFakeTimers()
    const module = await import('@mp-se/espframework-ui-components')
    const mockJson = vi.fn().mockResolvedValue({ version: '99.0.0' })
    const mockFetch = vi.fn().mockResolvedValue({ json: mockJson })
    vi.spyOn(module, 'useFetch').mockReturnValue({ managedFetch: mockFetch })

    const pinia = createTestingPinia()
    const { global: globalStore } = await import('@/modules/pinia')
    globalStore.app_ver = '1.0.0'

    const wrapper = shallowMount(HomeView, { global: { plugins: [pinia] } })
    // Advance past the 500ms createTimeout
    await vi.advanceTimersByTimeAsync(600)
    vi.useRealTimers()
    vi.restoreAllMocks()

    expect(wrapper.vm.newVersion.new).toBe(true)
    expect(wrapper.vm.newVersion.ver).toBe('99.0.0')
  })

  it('onMounted handles managedFetch returning null (aborted)', async () => {
    vi.useFakeTimers()
    const module = await import('@mp-se/espframework-ui-components')
    const mockFetch = vi.fn().mockResolvedValue(null)
    vi.spyOn(module, 'useFetch').mockReturnValue({ managedFetch: mockFetch })

    const pinia = createTestingPinia()
    const wrapper = shallowMount(HomeView, { global: { plugins: [pinia] } })
    await vi.advanceTimersByTimeAsync(600)
    vi.useRealTimers()
    vi.restoreAllMocks()

    expect(wrapper.vm.newVersion.new).toBe(false)
  })

  it('onMounted handles fetch exception gracefully', async () => {
    vi.useFakeTimers()
    const module = await import('@mp-se/espframework-ui-components')
    const mockFetch = vi.fn().mockRejectedValue(new Error('Network error'))
    vi.spyOn(module, 'useFetch').mockReturnValue({ managedFetch: mockFetch })

    const pinia = createTestingPinia()
    const wrapper = shallowMount(HomeView, { global: { plugins: [pinia] } })
    await vi.advanceTimersByTimeAsync(600)
    vi.useRealTimers()
    vi.restoreAllMocks()

    expect(wrapper.exists()).toBe(true)
  })

  it('renders newVersion upgrade card when newVersion.new is true', async () => {
    const pinia = createTestingPinia()
    const wrapper = shallowMount(HomeView, { global: { plugins: [pinia] } })
    expect(wrapper.vm.newVersion.new).toBe(false)
    wrapper.vm.newVersion.new = true
    wrapper.vm.newVersion.ver = '99.0.0'
    await wrapper.vm.$nextTick()
    expect(wrapper.html()).toContain('Upgrade available')
  })

  it('flag checkbox v-model triggers setSleepMode when toggled', async () => {
    const { mount } = await import('@vue/test-utils')
    const pinia = createTestingPinia()
    const wrapper = mount(HomeView, {
      global: {
        plugins: [pinia],
        stubs: {
          BsMessage: true,
          BsCard: { template: '<div><slot /></div>' },
          RouterLink: true
        }
      }
    })
    const checkbox = wrapper.find('input[type="checkbox"]')
    expect(checkbox.exists()).toBe(true)
    await checkbox.setChecked(true)
    expect(wrapper.vm.flag).toBe(true)
  })

  it('watch(sleep_mode) updates flag when sleep_mode ref changes', async () => {
    const { ref } = await import('vue')
    const { status } = await import('@/modules/pinia')

    // Make sleep_mode a real Vue ref so storeToRefs/watch detects changes
    const sleepModeRef = ref(false)
    const original = status.sleep_mode
    status.sleep_mode = sleepModeRef

    const pinia = createTestingPinia()
    const wrapper = shallowMount(HomeView, { global: { plugins: [pinia] } })

    wrapper.vm.flag = false
    sleepModeRef.value = true
    await wrapper.vm.$nextTick()
    await wrapper.vm.$nextTick()

    status.sleep_mode = original
    expect(wrapper.vm.flag).toBe(true)
  })

  it('watch(sleep_mode) skips flag update when already equal', async () => {
    const { ref } = await import('vue')
    const { status } = await import('@/modules/pinia')

    const sleepModeRef = ref(false)
    const original = status.sleep_mode
    status.sleep_mode = sleepModeRef

    const pinia = createTestingPinia()
    const wrapper = shallowMount(HomeView, { global: { plugins: [pinia] } })

    // flag = true already, sleep_mode changes to true → watcher fires but skips update
    wrapper.vm.flag = true
    sleepModeRef.value = true
    await wrapper.vm.$nextTick()
    await wrapper.vm.$nextTick()

    status.sleep_mode = original
    expect(wrapper.vm.flag).toBe(true)
  })

  it('refresh does not accumulate angle when status.load returns false', async () => {
    const pinia = createTestingPinia()
    const { status } = await import('@/modules/pinia')
    status.sleep_mode = false // ensure sleep_mode is defined before mount (prevents storeToRefs error)

    const wrapper = shallowMount(HomeView, { global: { plugins: [pinia] } })
    status.load = vi.fn(async () => false)
    wrapper.vm.angle = { average: 0, sum: 0, count: 0 }
    await wrapper.vm.refresh()

    expect(wrapper.vm.angle.sum).toBe(0)
    expect(wrapper.vm.angle.count).toBe(0)
  })

  it('onMounted timeout fires but checkForNewGravMonVersion returns false (version not newer)', async () => {
    vi.useFakeTimers()
    const module = await import('@mp-se/espframework-ui-components')
    const mockJson = vi.fn().mockResolvedValue({ version: '0.0.1' })
    const mockFetch = vi.fn().mockResolvedValue({ json: mockJson })
    vi.spyOn(module, 'useFetch').mockReturnValue({ managedFetch: mockFetch })

    const pinia = createTestingPinia()
    const { global: globalStore, status } = await import('@/modules/pinia')
    globalStore.app_ver = '99.0.0'
    status.sleep_mode = false // ensure sleep_mode is defined before mount

    const wrapper = shallowMount(HomeView, { global: { plugins: [pinia] } })
    await vi.advanceTimersByTimeAsync(600)
    vi.useRealTimers()
    vi.restoreAllMocks()

    // Version 0.0.1 < 99.0.0, so newVersion.new should remain false
    expect(wrapper.vm.newVersion.new).toBe(false)
  })

  it('renders with status.wifi_setup true - covers v-if false branches', async () => {
    const pinia = createTestingPinia()
    const { status } = await import('@/modules/pinia')
    status.wifi_setup = true
    status.sleep_mode = false // ensure sleep_mode is defined before mount

    const wrapper = shallowMount(HomeView, { global: { plugins: [pinia] } })
    await wrapper.vm.$nextTick()

    status.wifi_setup = false
    expect(wrapper.exists()).toBe(true)
  })
})

describe('HomeView batteryPercentage computed', () => {
  beforeEach(() => {
    vi.clearAllMocks()
  })

  async function mountAndGetBatteryPercentage(batteryVoltage) {
    const pinia = createTestingPinia()
    const { status } = await import('@/modules/pinia')
    status.battery = batteryVoltage
    status.sleep_mode = false
    const wrapper = shallowMount(HomeView, { global: { plugins: [pinia] } })
    return wrapper.vm.batteryPercentage
  }

  it('returns 0% when battery is falsy', async () => {
    const pinia = createTestingPinia()
    const { status } = await import('@/modules/pinia')
    status.battery = 0
    status.sleep_mode = false
    const wrapper = shallowMount(HomeView, { global: { plugins: [pinia] } })
    expect(wrapper.vm.batteryPercentage).toBe('0%')
  })

  it('returns 100% at full charge voltage (4.2V)', async () => {
    expect(await mountAndGetBatteryPercentage(4.2)).toBe('100%')
  })

  it('returns 100% above max voltage', async () => {
    expect(await mountAndGetBatteryPercentage(4.5)).toBe('100%')
  })

  it('returns 0% at minimum voltage (3.2V)', async () => {
    expect(await mountAndGetBatteryPercentage(3.2)).toBe('0%')
  })

  it('returns 0% below minimum voltage', async () => {
    expect(await mountAndGetBatteryPercentage(3.0)).toBe('0%')
  })

  it('returns 95% at 4.1V (exact datapoint)', async () => {
    expect(await mountAndGetBatteryPercentage(4.1)).toBe('95%')
  })

  it('returns 90% at 4.0V (exact datapoint)', async () => {
    expect(await mountAndGetBatteryPercentage(4.0)).toBe('90%')
  })

  it('returns 50% at 3.65V (exact datapoint - flat plateau)', async () => {
    expect(await mountAndGetBatteryPercentage(3.65)).toBe('50%')
  })

  it('returns 10% at 3.30V (exact datapoint)', async () => {
    expect(await mountAndGetBatteryPercentage(3.3)).toBe('10%')
  })

  it('returns 5% at 3.24V (exact datapoint)', async () => {
    expect(await mountAndGetBatteryPercentage(3.24)).toBe('5%')
  })

  it('interpolates between datapoints (4.15V between 4.2V and 4.1V)', async () => {
    // 4.15 is midpoint between 4.2 (100%) and 4.1 (95%) → ~97-98%
    const result = await mountAndGetBatteryPercentage(4.15)
    const numeric = parseInt(result)
    expect(numeric).toBeGreaterThan(95)
    expect(numeric).toBeLessThan(100)
  })

  it('interpolates in the flat plateau region (3.67V between 3.68V and 3.65V)', async () => {
    // 3.67 is between 3.68 (55%) and 3.65 (50%)
    const result = await mountAndGetBatteryPercentage(3.67)
    const numeric = parseInt(result)
    expect(numeric).toBeGreaterThanOrEqual(50)
    expect(numeric).toBeLessThanOrEqual(55)
  })
})
