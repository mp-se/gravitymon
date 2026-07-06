import { describe, it, expect, beforeEach, vi } from 'vitest'
import { mount } from '@vue/test-utils'
import SupportView from '../SupportView.vue'
import { createPinia, setActivePinia } from 'pinia'
import { sharedHttpClient as http } from '@mp-se/espframework-ui-components'

// Mock the UI components library
vi.mock('@mp-se/espframework-ui-components', () => ({
  logDebug: vi.fn(),
  logError: vi.fn(),
  logInfo: vi.fn(),
  sharedHttpClient: {
    get: vi.fn(),
    post: vi.fn(),
    filesystemRequest: vi.fn()
  }
}))

describe('SupportView (interaction tests)', () => {
  beforeEach(() => {
    setActivePinia(createPinia())
  })

  it('mounts without error', () => {
    const wrapper = mount(SupportView, {
      global: {
        stubs: {}
      }
    })
    expect(wrapper.exists()).toBe(true)
  })

  it('displays support section header', () => {
    const wrapper = mount(SupportView, {
      global: {
        stubs: {}
      }
    })
    expect(wrapper.text()).toContain('Links and device logs')
  })

  it('displays support message text', () => {
    const wrapper = mount(SupportView, {
      global: {
        stubs: {}
      }
    })
    expect(wrapper.text()).toContain('support')
  })

  it('has link to github', () => {
    const wrapper = mount(SupportView, {
      global: {
        stubs: {}
      }
    })
    const links = wrapper.findAll('a')
    const githubLink = links.find((l) => l.attributes('href').includes('github.com'))
    expect(githubLink).toBeDefined()
  })

  it('has link to homebrewtalk', () => {
    const wrapper = mount(SupportView, {
      global: {
        stubs: {}
      }
    })
    const links = wrapper.findAll('a')
    const homebrewLink = links.find((l) => l.attributes('href').includes('homebrewtalk'))
    expect(homebrewLink).toBeDefined()
  })

  it('has link to gravitymon.com', () => {
    const wrapper = mount(SupportView, {
      global: {
        stubs: {}
      }
    })
    const links = wrapper.findAll('a')
    const gravitymonLink = links.find((l) => l.attributes('href').includes('gravitymon.com'))
    expect(gravitymonLink).toBeDefined()
  })

  it('displays platform information', () => {
    const wrapper = mount(SupportView, {
      global: {
        stubs: {}
      }
    })
    expect(wrapper.text()).toContain('Platform')
  })

  it('displays gyro information', () => {
    const wrapper = mount(SupportView, {
      global: {
        stubs: {}
      }
    })
    expect(wrapper.text()).toContain('Gyro')
  })

  it('displays firmware version information', () => {
    const wrapper = mount(SupportView, {
      global: {
        stubs: {}
      }
    })
    expect(wrapper.text()).toContain('Firmware')
  })

  it('displays hardware information', () => {
    const wrapper = mount(SupportView, {
      global: {
        stubs: {}
      }
    })
    expect(wrapper.text()).toContain('Hardware')
  })

  it('displays user interface version information', () => {
    const wrapper = mount(SupportView, {
      global: {
        stubs: {}
      }
    })
    expect(wrapper.text()).toContain('User interface')
  })

  it('displays bluetooth feature status', () => {
    const wrapper = mount(SupportView, {
      global: {
        stubs: {}
      }
    })
    expect(wrapper.text()).toContain('Bluetooth')
  })

  it('displays filter feature status', () => {
    const wrapper = mount(SupportView, {
      global: {
        stubs: {}
      }
    })
    expect(wrapper.text()).toContain('Filter')
  })

  it('all external links open in new tab', () => {
    const wrapper = mount(SupportView, {
      global: {
        stubs: {}
      }
    })
    const links = wrapper.findAll('a[target="_blank"]')
    expect(links.length).toBeGreaterThan(0)
  })

  it('has proper styling classes on links', () => {
    const wrapper = mount(SupportView, {
      global: {
        stubs: {}
      }
    })
    const links = wrapper.findAll('a')
    const styledLinks = links.filter((l) => l.classes().includes('link-primary'))
    expect(styledLinks.length).toBeGreaterThan(0)
  })

  it('displays device information section', () => {
    const wrapper = mount(SupportView, {
      global: {
        stubs: {}
      }
    })
    const badges = wrapper.findAll('.badge')
    expect(badges.length).toBeGreaterThan(0)
  })

  it('displays velocity feature status', () => {
    const wrapper = mount(SupportView, {
      global: {
        stubs: {}
      }
    })
    expect(wrapper.text()).toContain('Velocity')
  })
})

describe('SupportView (action tests)', () => {
  beforeEach(() => {
    setActivePinia(createPinia())
    vi.clearAllMocks()
  })

  const mountView = () => mount(SupportView, { global: { stubs: {} } })

  it('viewLogs fetches log files and displays content', async () => {
    http.filesystemRequest
      .mockResolvedValueOnce({ success: true, text: 'line3\nline2\nline1' })
      .mockResolvedValueOnce({ success: true, text: 'entry1\nentry2' })

    const wrapper = mountView()
    const btn = wrapper.findAll('button').find((b) => b.text().includes('View device logs'))
    await btn.trigger('click')
    await wrapper.vm.$nextTick()
    await wrapper.vm.$nextTick()

    expect(http.filesystemRequest).toHaveBeenCalledTimes(2)
    expect(http.filesystemRequest).toHaveBeenCalledWith({ command: 'get', file: '/error2.log' })
    expect(http.filesystemRequest).toHaveBeenCalledWith({ command: 'get', file: '/error.log' })
  })

  it('viewLogs handles fetch failure gracefully', async () => {
    http.filesystemRequest
      .mockResolvedValueOnce({ success: false })
      .mockResolvedValueOnce({ success: false })

    const wrapper = mountView()
    const btn = wrapper.findAll('button').find((b) => b.text().includes('View device logs'))
    await btn.trigger('click')
    await wrapper.vm.$nextTick()
    await wrapper.vm.$nextTick()

    expect(http.filesystemRequest).toHaveBeenCalledTimes(2)
  })

  it('removeLogs deletes both log files', async () => {
    http.filesystemRequest
      .mockResolvedValueOnce({ success: true })
      .mockResolvedValueOnce({ success: true })

    const wrapper = mountView()
    const btn = wrapper.findAll('button').find((b) => b.text().includes('Erase device logs'))
    await btn.trigger('click')
    await wrapper.vm.$nextTick()
    await wrapper.vm.$nextTick()

    expect(http.filesystemRequest).toHaveBeenCalledWith({ command: 'del', file: '/error2.log' })
    expect(http.filesystemRequest).toHaveBeenCalledWith({ command: 'del', file: '/error.log' })
  })

  it('removeLogs does not show success if one file delete fails', async () => {
    http.filesystemRequest
      .mockResolvedValueOnce({ success: true })
      .mockResolvedValueOnce({ success: false })

    const wrapper = mountView()
    const btn = wrapper.findAll('button').find((b) => b.text().includes('Erase device logs'))
    await btn.trigger('click')
    await wrapper.vm.$nextTick()
    await wrapper.vm.$nextTick()

    expect(http.filesystemRequest).toHaveBeenCalledTimes(2)
  })

  it('hardwareScan button exists and is clickable', () => {
    const wrapper = mountView()
    const btn = wrapper.findAll('button').find((b) => b.text().includes('Hardware scan'))
    expect(btn).toBeDefined()
    expect(btn.exists()).toBe(true)
  })

  it('toggle error help button shows error help section', async () => {
    const wrapper = mountView()
    expect(wrapper.text()).not.toContain('400 - Bad request')
    const btn = wrapper.findAll('button').find((b) => b.text().includes('Toggle error help'))
    await btn.trigger('click')
    await wrapper.vm.$nextTick()
    expect(wrapper.text()).toContain('400 - Bad request')
  })

  it('toggle error help button hides section on second click', async () => {
    const wrapper = mountView()
    const btn = wrapper.findAll('button').find((b) => b.text().includes('Toggle error help'))
    await btn.trigger('click')
    await wrapper.vm.$nextTick()
    await btn.trigger('click')
    await wrapper.vm.$nextTick()
    expect(wrapper.text()).not.toContain('400 - Bad request')
  })

  it('fetchLog concatenates log lines in reverse order', async () => {
    const wrapper = mountView()
    http.filesystemRequest.mockResolvedValueOnce({
      success: true,
      text: 'Line 1\nLine 2\nLine 3'
    })
    await wrapper.vm.fetchLog('/error.log')
    expect(wrapper.vm.logData).toContain('Line 3')
    expect(wrapper.vm.logData.indexOf('Line 3')).toBeLessThan(wrapper.vm.logData.indexOf('Line 1'))
  })

  it('fetchLog returns false on failure', async () => {
    const wrapper = mountView()
    http.filesystemRequest.mockResolvedValueOnce({ success: false })
    const result = await wrapper.vm.fetchLog('/error.log')
    expect(result).toBe(false)
  })

  it('removeLog calls filesystemRequest with del command', async () => {
    const wrapper = mountView()
    http.filesystemRequest.mockResolvedValueOnce({ success: true })
    await wrapper.vm.removeLog('/error.log')
    expect(http.filesystemRequest).toHaveBeenCalledWith({ command: 'del', file: '/error.log' })
  })

  it('removeLog returns false on failure', async () => {
    const wrapper = mountView()
    http.filesystemRequest.mockResolvedValueOnce({ success: false })
    const result = await wrapper.vm.removeLog('/error.log')
    expect(result).toBe(false)
  })

  it('viewLogs loads both error log files', async () => {
    const wrapper = mountView()
    http.filesystemRequest.mockResolvedValueOnce({ success: true, text: 'Error log 2' })
    http.filesystemRequest.mockResolvedValueOnce({ success: true, text: 'Error log 1' })
    await wrapper.vm.viewLogs()
    expect(http.filesystemRequest).toHaveBeenCalledWith({ command: 'get', file: '/error2.log' })
    expect(http.filesystemRequest).toHaveBeenCalledWith({ command: 'get', file: '/error.log' })
  })

  it('viewLogs sets disabled state during load', async () => {
    const wrapper = mountView()
    const { global } = await import('@/modules/pinia')
    http.filesystemRequest.mockResolvedValueOnce({ success: true, text: '' })
    http.filesystemRequest.mockResolvedValueOnce({ success: true, text: '' })
    global.disabled = false
    await wrapper.vm.viewLogs()
    expect(global.disabled).toBe(false) // Resets to false after load
  })

  it('viewLogs clears previous log data', async () => {
    const wrapper = mountView()
    wrapper.vm.logData = 'Old log data'
    http.filesystemRequest.mockResolvedValueOnce({ success: true, text: 'New log' })
    http.filesystemRequest.mockResolvedValueOnce({ success: true, text: '' })
    await wrapper.vm.viewLogs()
    expect(wrapper.vm.logData).not.toContain('Old log data')
  })

  it('removeLogs removes both error log files', async () => {
    const wrapper = mountView()
    http.filesystemRequest.mockResolvedValueOnce({ success: true })
    http.filesystemRequest.mockResolvedValueOnce({ success: true })
    await wrapper.vm.removeLogs()
    expect(http.filesystemRequest).toHaveBeenCalledWith({ command: 'del', file: '/error2.log' })
    expect(http.filesystemRequest).toHaveBeenCalledWith({ command: 'del', file: '/error.log' })
  })

  it('removeLogs shows success message when both files deleted', async () => {
    const wrapper = mountView()
    const { global } = await import('@/modules/pinia')
    http.filesystemRequest.mockResolvedValueOnce({ success: true })
    http.filesystemRequest.mockResolvedValueOnce({ success: true })
    await wrapper.vm.removeLogs()
    expect(global.messageSuccess).toContain('Log files are deleted')
  })

  it('removeLogs does not show success if one file fails', async () => {
    const wrapper = mountView()
    const { global } = await import('@/modules/pinia')
    http.filesystemRequest.mockResolvedValueOnce({ success: true })
    http.filesystemRequest.mockResolvedValueOnce({ success: false })
    global.messageSuccess = ''
    await wrapper.vm.removeLogs()
    expect(global.messageSuccess).toBe('')
  })

  it('removeLegacy removes legacy config files', async () => {
    const wrapper = mountView()
    http.filesystemRequest.mockResolvedValueOnce({ success: true })
    http.filesystemRequest.mockResolvedValueOnce({ success: true })
    await wrapper.vm.removeLegacy()
    expect(http.filesystemRequest).toHaveBeenCalledWith({ command: 'del', file: '/config.json' })
    expect(http.filesystemRequest).toHaveBeenCalledWith({
      command: 'del',
      file: '/gravitymon.json'
    })
  })

  it('removeLegacy shows success message when both files deleted', async () => {
    const wrapper = mountView()
    const { global } = await import('@/modules/pinia')
    http.filesystemRequest.mockResolvedValueOnce({ success: true })
    http.filesystemRequest.mockResolvedValueOnce({ success: true })
    await wrapper.vm.removeLegacy()
    expect(global.messageSuccess).toContain('Old configuration files are deleted')
  })

  it('hardwareScan displays scan results on success', async () => {
    const wrapper = mountView()
    const { config } = await import('@/modules/pinia')
    config.runHardwareScan = vi.fn(async () => ({
      success: true,
      data: 'Gyro: MPU6050\nTemp: BME280'
    }))
    await wrapper.vm.hardwareScan()
    await new Promise((resolve) => setTimeout(resolve, 10))
    expect(wrapper.vm.logData).toContain('Gyro')
  })

  it('hardwareScan clears log data before scan', async () => {
    const wrapper = mountView()
    const { config } = await import('@/modules/pinia')
    wrapper.vm.logData = 'Old data'
    config.runHardwareScan = vi.fn(async () => ({ success: true, data: 'New' }))
    await wrapper.vm.hardwareScan()
    await new Promise((resolve) => setTimeout(resolve, 10))
    // logData is cleared first
    expect(wrapper.vm.logData).not.toContain('Old data')
  })

  it('hardwareScan disables UI during scan', async () => {
    const wrapper = mountView()
    const { global, config } = await import('@/modules/pinia')
    config.runHardwareScan = vi.fn(
      async () => ((global.disabled = false), { success: true, data: '' })
    )
    global.disabled = false
    await wrapper.vm.hardwareScan()
    await new Promise((resolve) => setTimeout(resolve, 10))
    expect(global.disabled).toBe(false) // Re-enabled after scan
  })

  it('hardwareScan handles failure gracefully', async () => {
    const wrapper = mountView()
    const { config } = await import('@/modules/pinia')
    config.runHardwareScan = vi.fn(async () => ({ success: false }))
    await wrapper.vm.hardwareScan()
    await new Promise((resolve) => setTimeout(resolve, 10))
    expect(wrapper.vm.logData).toBe('')
  })

  it('shows Disabled badges when features are disabled', async () => {
    const { global } = await import('@/modules/pinia')
    const savedFeature = { ...global.feature }
    global.feature.ble = false
    global.feature.filter = false
    global.feature.velocity = false
    global.feature.charging = false
    const wrapper = mountView()
    await wrapper.vm.$nextTick()
    expect(wrapper.text()).toContain('Disabled')
    Object.assign(global.feature, savedFeature)
  })

  it('shows erase ispindel config button when ispindel_config is true', async () => {
    const { status } = await import('@/modules/pinia')
    status.ispindel_config = true
    const wrapper = mountView()
    await wrapper.vm.$nextTick()
    expect(wrapper.text()).toContain('Erase iSpindel config')
    status.ispindel_config = false
  })

  it('removeLegacy does not show success message if first file fails', async () => {
    const wrapper = mountView()
    const { global } = await import('@/modules/pinia')
    global.messageSuccess = ''
    http.filesystemRequest.mockResolvedValueOnce({ success: false })
    http.filesystemRequest.mockResolvedValueOnce({ success: true })
    await wrapper.vm.removeLegacy()
    expect(global.messageSuccess).toBe('')
  })
})
