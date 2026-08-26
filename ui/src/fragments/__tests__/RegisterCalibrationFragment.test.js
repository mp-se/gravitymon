import { describe, it, expect, beforeEach, vi } from 'vitest'
import { mount } from '@vue/test-utils'
import RegisterCalibrationFragment from '../RegisterCalibrationFragment.vue'
import { createTestingPinia } from '../../tests/testUtils'
import { sharedHttpClient as http } from '@mp-se/espframework-ui-components'

describe('RegisterCalibrationFragment (smoke)', () => {
  it('mounts without error', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(RegisterCalibrationFragment, { global: { plugins: [pinia] } })
    expect(wrapper.exists()).toBe(true)
  })
})

describe('RegisterCalibrationFragment (action tests)', () => {
  beforeEach(() => vi.clearAllMocks())

  it('cancel emits close event', async () => {
    const pinia = createTestingPinia()
    const wrapper = mount(RegisterCalibrationFragment, { global: { plugins: [pinia] } })
    await wrapper.vm.cancel()
    expect(wrapper.emitted('close')).toBeTruthy()
  })

  it('send calls postJson with registration data', async () => {
    http.postJson.mockResolvedValue(true)
    const pinia = createTestingPinia()
    const wrapper = mount(RegisterCalibrationFragment, { global: { plugins: [pinia] } })
    await wrapper.vm.$nextTick()
    // Set some registration data so JSON.parse succeeds
    wrapper.vm.registrationDataJson = JSON.stringify({ chipid: 'anon-chip-123', formula: 'tilt' })
    await wrapper.vm.send()
    expect(http.postJson).toHaveBeenCalled()
  })

  it('send sets messageSuccess on successful response', async () => {
    http.postJson.mockResolvedValue(true)
    const { global: globalStore } = await import('@/modules/pinia')
    const pinia = createTestingPinia()
    const wrapper = mount(RegisterCalibrationFragment, { global: { plugins: [pinia] } })
    await wrapper.vm.$nextTick()
    wrapper.vm.registrationDataJson = JSON.stringify({ chipid: 'anon-chip-123', formula: 'tilt' })
    await wrapper.vm.send()
    expect(globalStore.messageSuccess).toBe('Calibration data reported successfully!')
  })

  it('send sets messageError when postJson returns falsy', async () => {
    http.postJson.mockResolvedValue(false)
    const { global: globalStore } = await import('@/modules/pinia')
    const pinia = createTestingPinia()
    const wrapper = mount(RegisterCalibrationFragment, { global: { plugins: [pinia] } })
    await wrapper.vm.$nextTick()
    wrapper.vm.registrationDataJson = JSON.stringify({ chipid: 'anon-chip-123', formula: 'tilt' })
    await wrapper.vm.send()
    expect(globalStore.messageError).toBe('Registration failed. Please try again.')
  })

  it('send sets messageError on 401 error', async () => {
    http.postJson.mockRejectedValue(new Error('HTTP 401: Unauthorized'))
    const { global: globalStore } = await import('@/modules/pinia')
    const pinia = createTestingPinia()
    const wrapper = mount(RegisterCalibrationFragment, { global: { plugins: [pinia] } })
    await wrapper.vm.$nextTick()
    wrapper.vm.registrationDataJson = JSON.stringify({ chipid: 'anon-chip-123', formula: 'tilt' })
    await wrapper.vm.send()
    expect(globalStore.messageError).toContain('API key has been revoked')
  })

  it('send sets messageError on 429 rate limit error', async () => {
    http.postJson.mockRejectedValue(new Error('HTTP 429: Too Many Requests'))
    const { global: globalStore } = await import('@/modules/pinia')
    const pinia = createTestingPinia()
    const wrapper = mount(RegisterCalibrationFragment, { global: { plugins: [pinia] } })
    await wrapper.vm.$nextTick()
    wrapper.vm.registrationDataJson = JSON.stringify({ chipid: 'anon-chip-123', formula: 'tilt' })
    await wrapper.vm.send()
    expect(globalStore.messageError).toContain('unavailable')
  })

  it('send sets messageError on 500 server error', async () => {
    http.postJson.mockRejectedValue(new Error('HTTP 500: Internal Server Error'))
    const { global: globalStore } = await import('@/modules/pinia')
    const pinia = createTestingPinia()
    const wrapper = mount(RegisterCalibrationFragment, { global: { plugins: [pinia] } })
    await wrapper.vm.$nextTick()
    wrapper.vm.registrationDataJson = JSON.stringify({ chipid: 'anon-chip-123', formula: 'tilt' })
    await wrapper.vm.send()
    expect(globalStore.messageError).toContain('Internal server error')
  })

  it('send sets generic messageError for unrecognized error', async () => {
    http.postJson.mockRejectedValue(new Error('some unexpected error'))
    const { global: globalStore } = await import('@/modules/pinia')
    const pinia = createTestingPinia()
    const wrapper = mount(RegisterCalibrationFragment, { global: { plugins: [pinia] } })
    await wrapper.vm.$nextTick()
    wrapper.vm.registrationDataJson = JSON.stringify({ chipid: 'anon-chip-123', formula: 'tilt' })
    await wrapper.vm.send()
    expect(globalStore.messageError).toContain('error occurred')
  })

  it('send always emits close after completion', async () => {
    http.postJson.mockResolvedValue(true)
    const pinia = createTestingPinia()
    const wrapper = mount(RegisterCalibrationFragment, { global: { plugins: [pinia] } })
    await wrapper.vm.$nextTick()
    wrapper.vm.registrationDataJson = JSON.stringify({ chipid: 'anon-chip-123', formula: 'tilt' })
    await wrapper.vm.send()
    expect(wrapper.emitted('close')).toBeTruthy()
  })
})
