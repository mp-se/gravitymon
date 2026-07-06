import { mount } from '@vue/test-utils'
import RegisterDeviceFragment from '../RegisterDeviceFragment.vue'
import { createTestingPinia } from '../../tests/testUtils'

describe('RegisterDeviceFragment (interaction tests)', () => {
  it('mounts without error', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(RegisterDeviceFragment, {
      props: { software: 'test' },
      global: { plugins: [pinia] }
    })
    expect(wrapper.exists()).toBe(true)
  })

  it('displays registration modal', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(RegisterDeviceFragment, {
      props: { software: 'test' },
      global: { plugins: [pinia] }
    })
    expect(wrapper.html()).toContain('Report usage')
  })

  it('displays registration modal header', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(RegisterDeviceFragment, {
      props: { software: 'test' },
      global: { plugins: [pinia] }
    })
    expect(wrapper.text()).toContain('Report usage')
  })

  it('has proper modal structure', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(RegisterDeviceFragment, {
      props: { software: 'test' },
      global: { plugins: [pinia] }
    })
    expect(wrapper.find('.modal').exists()).toBe(true)
    expect(wrapper.find('.modal-content').exists()).toBe(true)
    expect(wrapper.find('.modal-header').exists()).toBe(true)
    expect(wrapper.find('.modal-body').exists()).toBe(true)
    expect(wrapper.find('.modal-footer').exists()).toBe(true)
  })

  it('has cancel button', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(RegisterDeviceFragment, {
      props: { software: 'test' },
      global: { plugins: [pinia] }
    })
    const buttons = wrapper.findAll('button')
    const cancelButton = buttons.find((b) => b.text().includes('Cancel'))
    expect(cancelButton).toBeDefined()
  })

  it('has ignore button', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(RegisterDeviceFragment, {
      props: { software: 'test' },
      global: { plugins: [pinia] }
    })
    const buttons = wrapper.findAll('button')
    const ignoreButton = buttons.find((b) => b.text().includes('Ignore'))
    expect(ignoreButton).toBeDefined()
  })

  it('has send button', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(RegisterDeviceFragment, {
      props: { software: 'test' },
      global: { plugins: [pinia] }
    })
    const buttons = wrapper.findAll('button')
    const sendButton = buttons.find((b) => b.text().includes('Send'))
    expect(sendButton).toBeDefined()
  })

  it('displays registration data section', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(RegisterDeviceFragment, {
      props: { software: 'test' },
      global: { plugins: [pinia] }
    })
    expect(wrapper.html()).toContain('Anonymous data')
  })

  it('has registrationDataJson ref defined', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(RegisterDeviceFragment, {
      props: { software: 'test' },
      global: { plugins: [pinia] }
    })
    expect(wrapper.vm.registrationDataJson).toBeDefined()
  })

  it('has cancel function defined', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(RegisterDeviceFragment, {
      props: { software: 'test' },
      global: { plugins: [pinia] }
    })
    expect(typeof wrapper.vm.cancel).toBe('function')
  })

  it('has ignore function defined', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(RegisterDeviceFragment, {
      props: { software: 'test' },
      global: { plugins: [pinia] }
    })
    expect(typeof wrapper.vm.ignore).toBe('function')
  })

  it('has send function defined', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(RegisterDeviceFragment, {
      props: { software: 'test' },
      global: { plugins: [pinia] }
    })
    expect(typeof wrapper.vm.send).toBe('function')
  })

  it('displays modal backdrop', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(RegisterDeviceFragment, {
      props: { software: 'test' },
      global: { plugins: [pinia] }
    })
    expect(wrapper.find('.modal-backdrop').exists()).toBe(true)
  })

  it('shows modal as active', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(RegisterDeviceFragment, {
      props: { software: 'test' },
      global: { plugins: [pinia] }
    })
    const modal = wrapper.find('.modal')
    expect(modal.classes()).toContain('show')
    expect(modal.classes()).toContain('d-block')
  })

  it('cancel button has secondary styling', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(RegisterDeviceFragment, {
      props: { software: 'test' },
      global: { plugins: [pinia] }
    })
    const buttons = wrapper.findAll('button')
    const cancelButton = buttons.find((b) => b.text().includes('Cancel'))
    expect(cancelButton.classes()).toContain('btn-secondary')
  })

  it('send button has primary styling', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(RegisterDeviceFragment, {
      props: { software: 'test' },
      global: { plugins: [pinia] }
    })
    const buttons = wrapper.findAll('button')
    const sendButton = buttons.find((b) => b.text().includes('Send'))
    expect(sendButton.classes()).toContain('btn-primary')
  })

  it('accepts software prop', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(RegisterDeviceFragment, {
      props: { software: 'custom-software' },
      global: { plugins: [pinia] }
    })
    expect(wrapper.props('software')).toBe('custom-software')
  })

  it('emits close event on cancel', async () => {
    const pinia = createTestingPinia()
    const wrapper = mount(RegisterDeviceFragment, {
      props: { software: 'test' },
      global: { plugins: [pinia] }
    })
    await wrapper.vm.cancel()
    expect(wrapper.emitted('close')).toBeTruthy()
  })
})

describe('RegisterDeviceFragment (action tests)', () => {
  beforeEach(() => vi.clearAllMocks())

  it('ignore calls http.postJson and emits close', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: RegisterDeviceFragment } = await import('../RegisterDeviceFragment.vue')
    const { sharedHttpClient: http } = await import('@mp-se/espframework-ui-components')
    http.postJson.mockResolvedValue({ success: true })
    const pinia = createTestingPinia()
    const wrapper = mount(RegisterDeviceFragment, {
      props: { software: 'test' },
      global: { plugins: [pinia] }
    })
    await wrapper.vm.ignore()
    expect(http.postJson).toHaveBeenCalledWith('api/config', { registered: true })
    expect(wrapper.emitted('close')).toBeTruthy()
  })

  it('send calls http.postJson and emits close', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: RegisterDeviceFragment } = await import('../RegisterDeviceFragment.vue')
    const { sharedHttpClient: http } = await import('@mp-se/espframework-ui-components')
    http.postJson.mockResolvedValue({ id: '123' })
    const pinia = createTestingPinia()
    const wrapper = mount(RegisterDeviceFragment, {
      props: { software: 'test' },
      global: { plugins: [pinia] }
    })
    await wrapper.vm.send()
    expect(wrapper.emitted('close')).toBeTruthy()
  })

  it('send sets messageError when response is falsy', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: RegisterDeviceFragment } = await import('../RegisterDeviceFragment.vue')
    const { sharedHttpClient: http } = await import('@mp-se/espframework-ui-components')
    http.postJson.mockResolvedValue(null)
    const pinia = createTestingPinia()
    const wrapper = mount(RegisterDeviceFragment, {
      props: { software: 'test' },
      global: { plugins: [pinia] }
    })
    wrapper.vm.registrationDataJson = JSON.stringify({ software: 'test', chipid: 'anon' })
    await wrapper.vm.send()
    const { global: globalStore } = await import('@/modules/pinia')
    expect(globalStore.messageError).toBeTruthy()
  })

  it('send sets messageError with 429 status on rate limit error', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: RegisterDeviceFragment } = await import('../RegisterDeviceFragment.vue')
    const { sharedHttpClient: http } = await import('@mp-se/espframework-ui-components')
    http.postJson.mockRejectedValue(new Error('HTTP 429: Too Many Requests'))
    const pinia = createTestingPinia()
    const wrapper = mount(RegisterDeviceFragment, {
      props: { software: 'test' },
      global: { plugins: [pinia] }
    })
    wrapper.vm.registrationDataJson = JSON.stringify({ software: 'test', chipid: 'anon' })
    await wrapper.vm.send()
    const { global: globalStore } = await import('@/modules/pinia')
    expect(globalStore.messageError).toContain('unavailable')
  })

  it('send sets messageError with 500 status on server error', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: RegisterDeviceFragment } = await import('../RegisterDeviceFragment.vue')
    const { sharedHttpClient: http } = await import('@mp-se/espframework-ui-components')
    http.postJson.mockRejectedValue(new Error('HTTP 500: Internal Server Error'))
    const pinia = createTestingPinia()
    const wrapper = mount(RegisterDeviceFragment, {
      props: { software: 'test' },
      global: { plugins: [pinia] }
    })
    wrapper.vm.registrationDataJson = JSON.stringify({ software: 'test', chipid: 'anon' })
    await wrapper.vm.send()
    const { global: globalStore } = await import('@/modules/pinia')
    expect(globalStore.messageError).toContain('Internal server error')
  })

  it('send sets generic messageError for unrecognized error', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: RegisterDeviceFragment } = await import('../RegisterDeviceFragment.vue')
    const { sharedHttpClient: http } = await import('@mp-se/espframework-ui-components')
    http.postJson.mockRejectedValue(new Error('unknown error'))
    const pinia = createTestingPinia()
    const wrapper = mount(RegisterDeviceFragment, {
      props: { software: 'test' },
      global: { plugins: [pinia] }
    })
    wrapper.vm.registrationDataJson = JSON.stringify({ software: 'test', chipid: 'anon' })
    await wrapper.vm.send()
    const { global: globalStore } = await import('@/modules/pinia')
    expect(globalStore.messageError).toContain('error occurred')
  })

  it('ignore sets messageError on postJson failure', async () => {
    const { createTestingPinia } = await import('../../tests/testUtils')
    const { mount } = await import('@vue/test-utils')
    const { default: RegisterDeviceFragment } = await import('../RegisterDeviceFragment.vue')
    const { sharedHttpClient: http } = await import('@mp-se/espframework-ui-components')
    http.postJson.mockRejectedValue(new Error('network fail'))
    const pinia = createTestingPinia()
    const wrapper = mount(RegisterDeviceFragment, {
      props: { software: 'test' },
      global: { plugins: [pinia] }
    })
    await wrapper.vm.ignore()
    const { global: globalStore } = await import('@/modules/pinia')
    expect(globalStore.messageError).toBeTruthy()
  })
})
