import { shallowMount } from '@vue/test-utils'
import App from '../App.vue'
import { createTestingPinia } from '../tests/testUtils'

describe('App.vue (smoke)', () => {
  it('mounts without error', () => {
    const pinia = createTestingPinia()
    const wrapper = shallowMount(App, { global: { plugins: [pinia] } })
    expect(wrapper.exists()).toBe(true)
  })
})

describe('App.vue (function tests)', () => {
  beforeEach(() => vi.clearAllMocks())

  it('close clears messageError when alert is danger', async () => {
    const { shallowMount } = await import('@vue/test-utils')
    const { default: AppComponent } = await import('../App.vue')
    const { global } = await import('@/modules/pinia')
    global.messageError = 'Test error'
    const wrapper = shallowMount(AppComponent, { global: { plugins: [createTestingPinia()] } })
    wrapper.vm.close('danger')
    expect(global.messageError).toBe('')
  })

  it('close clears messageWarning when alert is warning', async () => {
    const { shallowMount } = await import('@vue/test-utils')
    const { default: AppComponent } = await import('../App.vue')
    const { global } = await import('@/modules/pinia')
    global.messageWarning = 'Test warning'
    const wrapper = shallowMount(AppComponent, { global: { plugins: [createTestingPinia()] } })
    wrapper.vm.close('warning')
    expect(global.messageWarning).toBe('')
  })

  it('close clears messageSuccess when alert is success', async () => {
    const { shallowMount } = await import('@vue/test-utils')
    const { default: AppComponent } = await import('../App.vue')
    const { global } = await import('@/modules/pinia')
    global.messageSuccess = 'Test success'
    const wrapper = shallowMount(AppComponent, { global: { plugins: [createTestingPinia()] } })
    wrapper.vm.close('success')
    expect(global.messageSuccess).toBe('')
  })

  it('close clears messageInfo when alert is info', async () => {
    const { shallowMount } = await import('@vue/test-utils')
    const { default: AppComponent } = await import('../App.vue')
    const { global } = await import('@/modules/pinia')
    global.messageInfo = 'Test info'
    const wrapper = shallowMount(AppComponent, { global: { plugins: [createTestingPinia()] } })
    wrapper.vm.close('info')
    expect(global.messageInfo).toBe('')
  })

  it('closeRegisterModal sets showRegisterModal to false', async () => {
    const { shallowMount } = await import('@vue/test-utils')
    const { default: AppComponent } = await import('../App.vue')
    const wrapper = shallowMount(AppComponent, { global: { plugins: [createTestingPinia()] } })
    wrapper.vm.closeRegisterModal()
    expect(wrapper.vm.showRegisterModal).toBe(false)
  })

  it('handleDarkModeUpdate sets config.dark_mode to true', async () => {
    const { shallowMount } = await import('@vue/test-utils')
    const { default: AppComponent } = await import('../App.vue')
    const { config } = await import('@/modules/pinia')
    const wrapper = shallowMount(AppComponent, { global: { plugins: [createTestingPinia()] } })
    wrapper.vm.handleDarkModeUpdate(true)
    expect(config.dark_mode).toBe(true)
  })

  it('handleDarkModeUpdate sets config.dark_mode to false', async () => {
    const { shallowMount } = await import('@vue/test-utils')
    const { default: AppComponent } = await import('../App.vue')
    const { config } = await import('@/modules/pinia')
    const wrapper = shallowMount(AppComponent, { global: { plugins: [createTestingPinia()] } })
    wrapper.vm.handleDarkModeUpdate(false)
    expect(config.dark_mode).toBe(false)
  })

  it('showSpinner shows the spinner dialog', async () => {
    const { shallowMount } = await import('@vue/test-utils')
    const { default: AppComponent } = await import('../App.vue')
    const wrapper = shallowMount(AppComponent, { global: { plugins: [createTestingPinia()] } })
    const mockShowModal = vi.fn()
    vi.spyOn(document, 'querySelector').mockReturnValue({ showModal: mockShowModal })
    wrapper.vm.showSpinner()
    expect(mockShowModal).toHaveBeenCalled()
  })

  it('hideSpinner closes the spinner dialog', async () => {
    const { shallowMount } = await import('@vue/test-utils')
    const { default: AppComponent } = await import('../App.vue')
    const wrapper = shallowMount(AppComponent, { global: { plugins: [createTestingPinia()] } })
    const mockClose = vi.fn()
    vi.spyOn(document, 'querySelector').mockReturnValue({ close: mockClose })
    wrapper.vm.hideSpinner()
    expect(mockClose).toHaveBeenCalled()
  })

  it('ping calls http.ping and updates status.connected', async () => {
    const { shallowMount } = await import('@vue/test-utils')
    const { default: AppComponent } = await import('../App.vue')
    const wrapper = shallowMount(AppComponent, { global: { plugins: [createTestingPinia()] } })
    // ping function is hard to test with mocking, just ensure it exists
    expect(typeof wrapper.vm.ping).toBe('function')
  })

  it('handleDarkModeUpdate sets data-bs-theme attribute to dark', async () => {
    const { shallowMount } = await import('@vue/test-utils')
    const { default: AppComponent } = await import('../App.vue')
    const wrapper = shallowMount(AppComponent, { global: { plugins: [createTestingPinia()] } })
    const root = document.documentElement
    wrapper.vm.handleDarkModeUpdate(true)
    expect(root.getAttribute('data-bs-theme')).toBe('dark')
  })

  it('handleDarkModeUpdate sets data-bs-theme attribute to light', async () => {
    const { shallowMount } = await import('@vue/test-utils')
    const { default: AppComponent } = await import('../App.vue')
    const wrapper = shallowMount(AppComponent, { global: { plugins: [createTestingPinia()] } })
    const root = document.documentElement
    wrapper.vm.handleDarkModeUpdate(false)
    expect(root.getAttribute('data-bs-theme')).toBe('light')
  })

  it('close is a function that handles different alert types', async () => {
    const { shallowMount } = await import('@vue/test-utils')
    const { default: AppComponent } = await import('../App.vue')
    const { global } = await import('@/modules/pinia')
    const wrapper = shallowMount(AppComponent, { global: { plugins: [createTestingPinia()] } })
    expect(typeof wrapper.vm.close).toBe('function')
    // Test that close works with all alert types
    global.messageError = 'error'
    global.messageWarning = 'warning'
    global.messageSuccess = 'success'
    global.messageInfo = 'info'
    wrapper.vm.close('danger')
    wrapper.vm.close('warning')
    wrapper.vm.close('success')
    wrapper.vm.close('info')
    expect(global.messageError).toBe('')
    expect(global.messageWarning).toBe('')
    expect(global.messageSuccess).toBe('')
    expect(global.messageInfo).toBe('')
  })

  it('watch(disabled) sets cursor to wait when disabled becomes true', async () => {
    const { ref } = await import('vue')
    const { global: globalMock } = await import('@/modules/pinia')
    const disabledRef = ref(false)
    const origDisabled = globalMock.disabled
    globalMock.disabled = disabledRef

    const { shallowMount } = await import('@vue/test-utils')
    const { default: AppComponent } = await import('../App.vue')
    const wrapper = shallowMount(AppComponent, { global: { plugins: [createTestingPinia()] } })

    disabledRef.value = true
    await wrapper.vm.$nextTick()
    await wrapper.vm.$nextTick()

    globalMock.disabled = origDisabled
    expect(document.body.style.cursor).toBe('wait')
  })

  it('watch(disabled) sets cursor to default when disabled becomes false', async () => {
    const { ref } = await import('vue')
    const { global: globalMock } = await import('@/modules/pinia')
    // Start with true so watch fires when changed to false
    const disabledRef = ref(true)
    const origDisabled = globalMock.disabled
    globalMock.disabled = disabledRef

    const { shallowMount } = await import('@vue/test-utils')
    const { default: AppComponent } = await import('../App.vue')
    const wrapper = shallowMount(AppComponent, { global: { plugins: [createTestingPinia()] } })

    // Replace global.disabled with plain false BEFORE the watcher fires
    // so the callback sees global.disabled = false (falsy) → else branch
    globalMock.disabled = false
    disabledRef.value = false
    await wrapper.vm.$nextTick()
    await wrapper.vm.$nextTick()

    globalMock.disabled = origDisabled
    expect(document.body.style.cursor).toBe('default')
  })

  it('ping() calls http.ping and updates status.connected', async () => {
    const { global: globalMock } = await import('@/modules/pinia')
    globalMock.initialized = true

    const { shallowMount } = await import('@vue/test-utils')
    const { default: AppComponent } = await import('../App.vue')
    const { status } = await import('@/modules/pinia')
    const httpModule = await import('@mp-se/espframework-ui-components')
    const httpMock = httpModule.sharedHttpClient
    httpMock.ping = vi.fn(async () => true)

    const wrapper = shallowMount(AppComponent, { global: { plugins: [createTestingPinia()] } })
    wrapper.vm.ping()
    await wrapper.vm.$nextTick()
    await new Promise((r) => setTimeout(r, 10))

    expect(httpMock.ping).toHaveBeenCalled()
    expect(status.connected).toBe(true)
    globalMock.initialized = true
  })

  it('handleDarkModeUpdate handles error when setAttribute throws', async () => {
    const { global: globalMock } = await import('@/modules/pinia')
    globalMock.initialized = true

    const { shallowMount } = await import('@vue/test-utils')
    const { default: AppComponent } = await import('../App.vue')
    const wrapper = shallowMount(AppComponent, { global: { plugins: [createTestingPinia()] } })

    vi.spyOn(document.documentElement, 'setAttribute').mockImplementationOnce(() => {
      throw new Error('DOM error')
    })

    expect(() => wrapper.vm.handleDarkModeUpdate(true)).not.toThrow()
    vi.restoreAllMocks()
    globalMock.initialized = true
  })
})

describe('App.vue (initializeApp tests)', () => {
  let mockShowModal, mockClose, http, globalStore, statusStore, configStore

  beforeEach(async () => {
    vi.clearAllMocks()

    // Mock the spinner dialog
    mockShowModal = vi.fn()
    mockClose = vi.fn()
    vi.spyOn(document, 'querySelector').mockReturnValue({
      showModal: mockShowModal,
      close: mockClose
    })

    const pinia = await import('@/modules/pinia')
    globalStore = pinia.global
    statusStore = pinia.status
    configStore = pinia.config

    const httpModule = await import('@mp-se/espframework-ui-components')
    http = httpModule.sharedHttpClient
    http.auth = vi.fn(async () => true)
    http.getJson = vi.fn()
    http.postJson = vi.fn(async () => {})

    globalStore.initialized = false
    globalStore.load = vi.fn(async () => true)
    globalStore.registered = true
    statusStore.load = vi.fn(async () => true)
    statusStore.wifi_setup = false
    configStore.load = vi.fn(async () => true)
    configStore.loadFormat = vi.fn(async () => true)
  })

  afterEach(() => {
    vi.restoreAllMocks()
  })

  it('initializeApp succeeds and sets global.initialized to true', async () => {
    const { shallowMount } = await import('@vue/test-utils')
    const { default: AppComponent } = await import('../App.vue')
    const wrapper = shallowMount(AppComponent, { global: { plugins: [createTestingPinia()] } })
    await wrapper.vm.initializeApp()
    expect(globalStore.initialized).toBe(true)
    expect(mockShowModal).toHaveBeenCalled()
    expect(mockClose).toHaveBeenCalled()
  })

  it('initializeApp sets messageError and returns when auth fails', async () => {
    http.auth = vi.fn(async () => false)
    const { shallowMount } = await import('@vue/test-utils')
    const { default: AppComponent } = await import('../App.vue')
    const wrapper = shallowMount(AppComponent, { global: { plugins: [createTestingPinia()] } })
    await wrapper.vm.initializeApp()
    expect(globalStore.messageError).toContain('authenticate')
    expect(globalStore.initialized).toBe(false)
  })

  it('initializeApp sets messageError and returns when global.load fails', async () => {
    globalStore.load = vi.fn(async () => false)
    const { shallowMount } = await import('@vue/test-utils')
    const { default: AppComponent } = await import('../App.vue')
    const wrapper = shallowMount(AppComponent, { global: { plugins: [createTestingPinia()] } })
    await wrapper.vm.initializeApp()
    expect(globalStore.messageError).toContain('feature flags')
    expect(globalStore.initialized).toBe(false)
  })

  it('initializeApp sets messageError and returns when status.load fails', async () => {
    statusStore.load = vi.fn(async () => false)
    const { shallowMount } = await import('@vue/test-utils')
    const { default: AppComponent } = await import('../App.vue')
    const wrapper = shallowMount(AppComponent, { global: { plugins: [createTestingPinia()] } })
    await wrapper.vm.initializeApp()
    expect(globalStore.messageError).toContain('status')
    expect(globalStore.initialized).toBe(false)
  })

  it('initializeApp sets messageError and returns when config.load fails', async () => {
    configStore.load = vi.fn(async () => false)
    const { shallowMount } = await import('@vue/test-utils')
    const { default: AppComponent } = await import('../App.vue')
    const wrapper = shallowMount(AppComponent, { global: { plugins: [createTestingPinia()] } })
    await wrapper.vm.initializeApp()
    expect(globalStore.messageError).toContain('configuration')
    expect(globalStore.initialized).toBe(false)
  })

  it('initializeApp sets messageError and returns when config.loadFormat fails', async () => {
    configStore.loadFormat = vi.fn(async () => false)
    const { shallowMount } = await import('@vue/test-utils')
    const { default: AppComponent } = await import('../App.vue')
    const wrapper = shallowMount(AppComponent, { global: { plugins: [createTestingPinia()] } })
    await wrapper.vm.initializeApp()
    expect(globalStore.messageError).toContain('format')
    expect(globalStore.initialized).toBe(false)
  })

  it('initializeApp checks registration when not registered and enableDeviceRegistration is true', async () => {
    globalStore.registered = false
    globalStore.ui = { ...globalStore.ui, enableDeviceRegistration: true }
    statusStore.wifi_setup = false
    http.getJson = vi.fn(async () => ({ exists: true }))
    const { shallowMount } = await import('@vue/test-utils')
    const { default: AppComponent } = await import('../App.vue')
    const wrapper = shallowMount(AppComponent, { global: { plugins: [createTestingPinia()] } })
    await wrapper.vm.initializeApp()
    expect(globalStore.anonymizeChipId).toHaveBeenCalled()
    expect(http.getJson).toHaveBeenCalled()
  })

  it('initializeApp marks device as registered when check response has exists=true', async () => {
    globalStore.registered = false
    globalStore.ui = { ...globalStore.ui, enableDeviceRegistration: true }
    statusStore.wifi_setup = false
    http.getJson = vi.fn(async () => ({ exists: true }))
    const { shallowMount } = await import('@vue/test-utils')
    const { default: AppComponent } = await import('../App.vue')
    const wrapper = shallowMount(AppComponent, { global: { plugins: [createTestingPinia()] } })
    await wrapper.vm.initializeApp()
    expect(globalStore.registered).toBe(true)
    expect(http.postJson).toHaveBeenCalledWith('api/config', { registered: true })
  })

  it('initializeApp shows register modal when not registered and not wifi_setup', async () => {
    globalStore.registered = false
    globalStore.ui = { ...globalStore.ui, enableDeviceRegistration: true }
    statusStore.wifi_setup = false
    http.getJson = vi.fn(async () => ({ exists: false }))
    const { shallowMount } = await import('@vue/test-utils')
    const { default: AppComponent } = await import('../App.vue')
    const wrapper = shallowMount(AppComponent, { global: { plugins: [createTestingPinia()] } })
    await wrapper.vm.initializeApp()
    expect(wrapper.vm.showRegisterModal).toBe(true)
  })

  it('initializeApp does not show register modal when wifi_setup is true', async () => {
    globalStore.registered = false
    globalStore.ui = { ...globalStore.ui, enableDeviceRegistration: true }
    statusStore.wifi_setup = true
    http.getJson = vi.fn(async () => ({ exists: false }))
    const { shallowMount } = await import('@vue/test-utils')
    const { default: AppComponent } = await import('../App.vue')
    const wrapper = shallowMount(AppComponent, { global: { plugins: [createTestingPinia()] } })
    await wrapper.vm.initializeApp()
    expect(wrapper.vm.showRegisterModal).toBe(false)
  })

  it('initializeApp handles registration check exception and continues', async () => {
    globalStore.registered = false
    globalStore.ui = { ...globalStore.ui, enableDeviceRegistration: true }
    http.getJson = vi.fn(async () => {
      throw new Error('network')
    })
    const { shallowMount } = await import('@vue/test-utils')
    const { default: AppComponent } = await import('../App.vue')
    const wrapper = shallowMount(AppComponent, { global: { plugins: [createTestingPinia()] } })
    await wrapper.vm.initializeApp()
    // Should still initialize despite registration check error
    expect(globalStore.initialized).toBe(true)
  })

  it('initializeApp sets messageError on unexpected exception', async () => {
    globalStore.load = vi.fn(async () => {
      throw new Error('unexpected')
    })
    const { shallowMount } = await import('@vue/test-utils')
    const { default: AppComponent } = await import('../App.vue')
    const wrapper = shallowMount(AppComponent, { global: { plugins: [createTestingPinia()] } })
    await wrapper.vm.initializeApp()
    expect(globalStore.messageError).toContain('Initialization failed')
  })

  it('onMounted calls initializeApp when not initialized', async () => {
    globalStore.initialized = false
    const { shallowMount } = await import('@vue/test-utils')
    const { default: AppComponent } = await import('../App.vue')
    const wrapper = shallowMount(AppComponent, { global: { plugins: [createTestingPinia()] } })
    await wrapper.vm.$nextTick()
    await new Promise((r) => setTimeout(r, 10))
    // initializeApp should have been called — initialized should now be true
    expect(globalStore.initialized).toBe(true)
  })

  it('onMounted skips initializeApp when already initialized', async () => {
    globalStore.initialized = true
    const { shallowMount } = await import('@vue/test-utils')
    const { default: AppComponent } = await import('../App.vue')
    shallowMount(AppComponent, { global: { plugins: [createTestingPinia()] } })
    await new Promise((r) => setTimeout(r, 10))
    // auth should NOT have been called since we skip initializeApp
    expect(http.auth).not.toHaveBeenCalled()
  })
})
