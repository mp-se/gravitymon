import { describe, it, expect, beforeEach, vi } from 'vitest'
import { mount } from '@vue/test-utils'
import EnableCorsFragment from '../EnableCorsFragment.vue'
import { createPinia, setActivePinia } from 'pinia'

// Mock the UI components library
vi.mock('@mp-se/espframework-ui-components', () => ({
  logDebug: vi.fn(),
  logError: vi.fn(),
  logInfo: vi.fn(),
  sharedHttpClient: {
    get: vi.fn(),
    post: vi.fn(),
    postJson: vi.fn()
  }
}))

describe('EnableCorsFragment (interaction tests)', () => {
  beforeEach(() => {
    setActivePinia(createPinia())
  })

  it('mounts without error', () => {
    const wrapper = mount(EnableCorsFragment, {
      global: {
        stubs: {}
      }
    })
    expect(wrapper.exists()).toBe(true)
  })

  it('displays developer settings heading', () => {
    const wrapper = mount(EnableCorsFragment, {
      global: {
        stubs: {}
      }
    })
    expect(wrapper.text()).toContain('Developer settings')
  })

  it('has enable CORS button', () => {
    const wrapper = mount(EnableCorsFragment, {
      global: {
        stubs: {}
      }
    })
    const buttons = wrapper.findAll('button')
    const corsButton = buttons.find((b) => b.text().includes('Enable CORS'))
    expect(corsButton).toBeDefined()
  })

  it('enable CORS button is clickable', () => {
    const wrapper = mount(EnableCorsFragment, {
      global: {
        stubs: {}
      }
    })
    const buttons = wrapper.findAll('button')
    const corsButton = buttons.find((b) => b.text().includes('Enable CORS'))
    expect(corsButton.element.className).toContain('btn')
  })

  it('has enableCors function defined', () => {
    const wrapper = mount(EnableCorsFragment, {
      global: {
        stubs: {}
      }
    })
    expect(typeof wrapper.vm.enableCors).toBe('function')
  })

  it('button is initially not disabled', () => {
    const wrapper = mount(EnableCorsFragment, {
      global: {
        stubs: {}
      }
    })
    const buttons = wrapper.findAll('button')
    const corsButton = buttons.find((b) => b.text().includes('Enable CORS'))
    expect(corsButton.attributes('disabled')).toBeUndefined()
  })

  it('button has secondary styling', () => {
    const wrapper = mount(EnableCorsFragment, {
      global: {
        stubs: {}
      }
    })
    const buttons = wrapper.findAll('button')
    const corsButton = buttons.find((b) => b.text().includes('Enable CORS'))
    expect(corsButton.element.className).toContain('btn-secondary')
  })

  it('has spinner element for loading state', () => {
    const wrapper = mount(EnableCorsFragment, {
      global: {
        stubs: {}
      }
    })
    const spinners = wrapper.findAll('.spinner-border')
    expect(spinners.length).toBeGreaterThan(0)
  })

  it('has proper heading hierarchy (h5)', () => {
    const wrapper = mount(EnableCorsFragment, {
      global: {
        stubs: {}
      }
    })
    const heading = wrapper.find('h5')
    expect(heading.exists()).toBe(true)
    expect(heading.text()).toContain('Developer settings')
  })

  it('heading has correct text content', () => {
    const wrapper = mount(EnableCorsFragment, {
      global: {
        stubs: {}
      }
    })
    const heading = wrapper.find('h5')
    expect(heading.text()).toBe('Developer settings')
  })

  it('button has correct row and column grid structure', () => {
    const wrapper = mount(EnableCorsFragment, {
      global: {
        stubs: {}
      }
    })
    const row = wrapper.find('.row')
    expect(row.exists()).toBe(true)
    const col = wrapper.find('.col-md-3')
    expect(col.exists()).toBe(true)
  })

  it('has correct spacing classes', () => {
    const wrapper = mount(EnableCorsFragment, {
      global: {
        stubs: {}
      }
    })
    const row = wrapper.find('.row')
    expect(row.element.className).toContain('gy-4')
  })
})

describe('EnableCorsFragment (action tests)', () => {
  beforeEach(() => {
    setActivePinia(createPinia())
    vi.clearAllMocks()
  })

  const mountView = () =>
    mount(EnableCorsFragment, {
      global: { stubs: {} }
    })

  it('enableCors calls http.postJson', async () => {
    const { sharedHttpClient: http } = await import('@mp-se/espframework-ui-components')
    http.postJson.mockResolvedValue({})
    const wrapper = mountView()
    await wrapper.vm.enableCors()
    expect(http.postJson).toHaveBeenCalledWith('api/config', { cors_allowed: true })
  })

  it('enableCors sets messageSuccess on success', async () => {
    const { sharedHttpClient: http } = await import('@mp-se/espframework-ui-components')
    http.postJson.mockResolvedValue({})
    const wrapper = mountView()
    await wrapper.vm.enableCors()
    const { global: globalStore } = await import('@/modules/pinia')
    expect(globalStore.messageSuccess).toBeTruthy()
  })

  it('enableCors sets messageError when postJson throws (inner catch)', async () => {
    const { sharedHttpClient: http } = await import('@mp-se/espframework-ui-components')
    http.postJson.mockRejectedValue(new Error('network fail'))
    const wrapper = mountView()
    await wrapper.vm.enableCors()
    const { global: globalStore } = await import('@/modules/pinia')
    expect(globalStore.messageError).toBeTruthy()
  })

  it('enableCors handles outer try error when clearMessages throws', async () => {
    const { global: globalStore } = await import('@/modules/pinia')
    globalStore.clearMessages.mockImplementationOnce(() => {
      throw new Error('clearMessages failure')
    })
    const wrapper = mountView()
    await wrapper.vm.enableCors()
    expect(globalStore.messageError).toContain('Failed to enable CORS')
  })
})
