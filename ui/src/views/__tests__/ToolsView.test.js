import { describe, it, expect, beforeEach, vi } from 'vitest'
import { shallowMount } from '@vue/test-utils'
import ToolsView from '../ToolsView.vue'
import { createTestingPinia } from '../../tests/testUtils'

describe('ToolsView (smoke)', () => {
  it('mounts without error', () => {
    const pinia = createTestingPinia()
    const wrapper = shallowMount(ToolsView, { global: { plugins: [pinia] } })
    expect(wrapper.exists()).toBe(true)
  })
})

describe('ToolsView (action tests)', () => {
  beforeEach(() => vi.clearAllMocks())

  it('enableAdvanced toggles hideAdvanced from true to false', async () => {
    const pinia = createTestingPinia()
    const wrapper = shallowMount(ToolsView, { global: { plugins: [pinia] } })
    expect(wrapper.vm.hideAdvanced).toBe(true)
    wrapper.vm.enableAdvanced()
    expect(wrapper.vm.hideAdvanced).toBe(false)
  })

  it('enableAdvanced toggles hideAdvanced back to true on second call', async () => {
    const pinia = createTestingPinia()
    const wrapper = shallowMount(ToolsView, { global: { plugins: [pinia] } })
    wrapper.vm.enableAdvanced()
    wrapper.vm.enableAdvanced()
    expect(wrapper.vm.hideAdvanced).toBe(true)
  })

  it('enable advanced button is visible when hideAdvanced is true', async () => {
    const pinia = createTestingPinia()
    const wrapper = shallowMount(ToolsView, { global: { plugins: [pinia] } })
    expect(wrapper.find('button').exists()).toBe(true)
  })

  it('clicking enable advanced button toggles hideAdvanced via click event', async () => {
    const pinia = createTestingPinia()
    const wrapper = shallowMount(ToolsView, { global: { plugins: [pinia] } })
    expect(wrapper.vm.hideAdvanced).toBe(true)
    await wrapper.find('button').trigger('click')
    expect(wrapper.vm.hideAdvanced).toBe(false)
  })

  it('VoltageFragment is shown when enableVoltageFragment flag is true', async () => {
    const pinia = createTestingPinia()
    const { global } = await import('@/modules/pinia')
    global.ui.enableVoltageFragment = true
    const wrapper = shallowMount(ToolsView, { global: { plugins: [pinia], stubs: {} } })
    expect(wrapper.findComponent({ name: 'VoltageFragment' }).exists()).toBe(true)
  })

  it('VoltageFragment is hidden when enableVoltageFragment flag is false', async () => {
    const pinia = createTestingPinia()
    const { global } = await import('@/modules/pinia')
    global.ui.enableVoltageFragment = false
    const wrapper = shallowMount(ToolsView, { global: { plugins: [pinia], stubs: {} } })
    expect(wrapper.findComponent({ name: 'VoltageFragment' }).exists()).toBe(false)
  })

  it('AdvancedFilesFragment is hidden when hideAdvanced is true', async () => {
    const pinia = createTestingPinia()
    const wrapper = shallowMount(ToolsView, { global: { plugins: [pinia], stubs: {} } })
    expect(wrapper.vm.hideAdvanced).toBe(true)
    expect(wrapper.findComponent({ name: 'AdvancedFilesFragment' }).exists()).toBe(false)
  })

  it('AdvancedFilesFragment is shown when hideAdvanced is toggled to false', async () => {
    const pinia = createTestingPinia()
    const wrapper = shallowMount(ToolsView, { global: { plugins: [pinia], stubs: {} } })
    wrapper.vm.hideAdvanced = false
    await wrapper.vm.$nextTick()
    expect(wrapper.findComponent({ name: 'AdvancedFilesFragment' }).exists()).toBe(true)
  })

  it('EnableCorsFragment is hidden when hideAdvanced is true', async () => {
    const pinia = createTestingPinia()
    const wrapper = shallowMount(ToolsView, { global: { plugins: [pinia], stubs: {} } })
    expect(wrapper.vm.hideAdvanced).toBe(true)
    expect(wrapper.findComponent({ name: 'EnableCorsFragment' }).exists()).toBe(false)
  })

  it('EnableCorsFragment is shown when hideAdvanced is toggled to false', async () => {
    const pinia = createTestingPinia()
    const wrapper = shallowMount(ToolsView, { global: { plugins: [pinia], stubs: {} } })
    wrapper.vm.hideAdvanced = false
    await wrapper.vm.$nextTick()
    expect(wrapper.findComponent({ name: 'EnableCorsFragment' }).exists()).toBe(true)
  })

  it('ListFilesFragment is always visible regardless of hideAdvanced state', async () => {
    const pinia = createTestingPinia()
    const wrapper = shallowMount(ToolsView, { global: { plugins: [pinia], stubs: {} } })
    expect(wrapper.findComponent({ name: 'ListFilesFragment' }).exists()).toBe(true)
    wrapper.vm.hideAdvanced = false
    await wrapper.vm.$nextTick()
    expect(wrapper.findComponent({ name: 'ListFilesFragment' }).exists()).toBe(true)
  })

  it('Enable Advanced button is only visible when hideAdvanced is true', async () => {
    const pinia = createTestingPinia()
    const wrapper = shallowMount(ToolsView, { global: { plugins: [pinia] } })
    const buttons = wrapper.findAll('button')
    expect(buttons.length).toBeGreaterThan(0)
    wrapper.vm.hideAdvanced = false
    await wrapper.vm.$nextTick()
    const buttonsAfter = wrapper.findAll('button')
    expect(buttonsAfter.length).toBeLessThan(buttons.length)
  })
})
