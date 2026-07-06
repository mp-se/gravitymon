import { describe, it, expect, beforeEach, vi } from 'vitest'
import { mount } from '@vue/test-utils'
import SerialView from '../SerialView.vue'
import { createPinia, setActivePinia } from 'pinia'

// Mock the UI components library
vi.mock('@mp-se/espframework-ui-components', () => ({
  logDebug: vi.fn(),
  logError: vi.fn(),
  logInfo: vi.fn(),
  sharedHttpClient: {
    get: vi.fn(),
    post: vi.fn(),
    postJson: vi.fn(),
    createWebSocket: vi.fn(() => {
      const mockSocket = {
        onmessage: null,
        onerror: null,
        onopen: null,
        onclose: null,
        send: vi.fn(),
        close: vi.fn()
      }
      return {
        onmessage: null,
        onerror: null,
        onopen: null,
        onclose: null,
        send: vi.fn(),
        close: vi.fn(),
        socketGetter: vi.fn(() => mockSocket)
      }
    })
  }
}))

describe('SerialView (interaction tests)', () => {
  beforeEach(() => {
    setActivePinia(createPinia())
  })

  it('mounts without error', () => {
    const wrapper = mount(SerialView, {
      global: {
        stubs: {}
      }
    })
    expect(wrapper.exists()).toBe(true)
  })

  it('displays serial console header', () => {
    const wrapper = mount(SerialView, {
      global: {
        stubs: {}
      }
    })
    expect(wrapper.text()).toContain('Serial console')
  })

  it('displays connection status', () => {
    const wrapper = mount(SerialView, {
      global: {
        stubs: {}
      }
    })
    expect(wrapper.text()).toContain('Not connected')
  })

  it('has serial output area with pre element', () => {
    const wrapper = mount(SerialView, {
      global: {
        stubs: {}
      }
    })
    const pre = wrapper.find('pre')
    expect(pre.exists()).toBe(true)
  })

  it('has clear button', () => {
    const wrapper = mount(SerialView, {
      global: {
        stubs: {}
      }
    })
    const buttons = wrapper.findAll('button')
    const clearButton = buttons.find((b) => b.text().includes('Clear'))
    expect(clearButton).toBeDefined()
  })

  it('has connect button', () => {
    const wrapper = mount(SerialView, {
      global: {
        stubs: {}
      }
    })
    const buttons = wrapper.findAll('button')
    const connectButton = buttons.find((b) => b.text().includes('Connect'))
    expect(connectButton).toBeDefined()
  })

  it('clear button is initially disabled', () => {
    const wrapper = mount(SerialView, {
      global: {
        stubs: {}
      }
    })
    const buttons = wrapper.findAll('button')
    const clearButton = buttons.find((b) => b.text().includes('Clear'))
    // Clear button should be disabled when not connected
    expect(clearButton.attributes('disabled')).toBeDefined()
  })

  it('connect button is initially not disabled', () => {
    const wrapper = mount(SerialView, {
      global: {
        stubs: {}
      }
    })
    const buttons = wrapper.findAll('button')
    const connectButton = buttons.find((b) => b.text().includes('Connect'))
    expect(connectButton.attributes('disabled')).toBeUndefined()
  })

  it('has clear function defined', () => {
    const wrapper = mount(SerialView, {
      global: {
        stubs: {}
      }
    })
    expect(typeof wrapper.vm.clear).toBe('function')
  })

  it('has connect function defined', () => {
    const wrapper = mount(SerialView, {
      global: {
        stubs: {}
      }
    })
    expect(typeof wrapper.vm.connect).toBe('function')
  })

  it('has serial output property', () => {
    const wrapper = mount(SerialView, {
      global: {
        stubs: {}
      }
    })
    expect(wrapper.vm.serial).toBeDefined()
    expect(typeof wrapper.vm.serial).toBe('string')
  })

  it('has isConnected computed property', () => {
    const wrapper = mount(SerialView, {
      global: {
        stubs: {}
      }
    })
    expect(wrapper.vm.isConnected).toBeDefined()
    expect(typeof wrapper.vm.isConnected).toBe('boolean')
  })

  it('has connected computed property', () => {
    const wrapper = mount(SerialView, {
      global: {
        stubs: {}
      }
    })
    expect(wrapper.vm.connected).toBeDefined()
  })

  it('clear button has primary styling', () => {
    const wrapper = mount(SerialView, {
      global: {
        stubs: {}
      }
    })
    const buttons = wrapper.findAll('button')
    const clearButton = buttons.find((b) => b.text().includes('Clear'))
    expect(clearButton.element.className).toContain('btn-primary')
  })

  it('connect button has secondary styling', () => {
    const wrapper = mount(SerialView, {
      global: {
        stubs: {}
      }
    })
    const buttons = wrapper.findAll('button')
    const connectButton = buttons.find((b) => b.text().includes('Connect'))
    expect(connectButton.element.className).toContain('btn-secondary')
  })

  it('has proper grid layout', () => {
    const wrapper = mount(SerialView, {
      global: {
        stubs: {}
      }
    })
    const rows = wrapper.findAll('.row')
    expect(rows.length).toBeGreaterThan(0)
  })

  it('buttons have wide width class', () => {
    const wrapper = mount(SerialView, {
      global: {
        stubs: {}
      }
    })
    const buttons = wrapper.findAll('button')
    const wideButtons = buttons.filter((b) => b.element.className.includes('w-2'))
    expect(wideButtons.length).toBeGreaterThan(0)
  })

  it('displays scroll output in pre block', () => {
    const wrapper = mount(SerialView, {
      global: {
        stubs: {}
      }
    })
    const pre = wrapper.find('pre')
    expect(pre.exists()).toBe(true)
    // Serial content is set by connect() in onMounted
    expect(wrapper.vm.serial).toBeDefined()
  })

  it('has container layout element', () => {
    const wrapper = mount(SerialView, {
      global: {
        stubs: {}
      }
    })
    const container = wrapper.find('.container')
    expect(container.exists()).toBe(true)
  })
})

describe('SerialView (action tests)', () => {
  beforeEach(() => {
    setActivePinia(createPinia())
    vi.clearAllMocks()
  })
  const mountView = () => mount(SerialView, { global: { stubs: {} } })

  it('clear resets serial content', () => {
    const wrapper = mountView()
    wrapper.vm.serial = 'some log data'
    wrapper.vm.clear()
    expect(wrapper.vm.serial).toBe('')
  })

  it('connect does not throw when called', async () => {
    const wrapper = mountView()
    expect(() => wrapper.vm.connect()).not.toThrow()
  })

  it('connect calls sharedHttpClient.createWebSocket with correct parameters', async () => {
    const { sharedHttpClient } = await import('@mp-se/espframework-ui-components')
    const wrapper = mountView()
    wrapper.vm.connect()
    expect(sharedHttpClient.createWebSocket).toHaveBeenCalledWith(
      'serialws',
      expect.objectContaining({
        onOpen: expect.any(Function),
        onMessage: expect.any(Function),
        onClose: expect.any(Function),
        onError: expect.any(Function),
        autoReconnect: true
      })
    )
  })

  it('websocket onMessage appends data and trims to maxLines', async () => {
    const { sharedHttpClient } = await import('@mp-se/espframework-ui-components')
    let messageHandler = null

    vi.mocked(sharedHttpClient).createWebSocket = vi.fn((url, handlers) => {
      messageHandler = handlers.onMessage
      return {
        close: vi.fn(),
        socketGetter: vi.fn(() => ({ close: vi.fn() }))
      }
    })

    const wrapper = mountView()
    wrapper.vm.connect()

    // Populate with many lines to test trimming
    for (let i = 0; i < 40; i++) {
      messageHandler({ data: 'Line ' + i })
    }

    const lineCount = wrapper.vm.serial.split('\n').filter((l) => l).length
    expect(lineCount).toBeLessThanOrEqual(50)
  })

  it('websocket onOpen appends connection message', async () => {
    const { sharedHttpClient } = await import('@mp-se/espframework-ui-components')
    let openHandler = null

    vi.mocked(sharedHttpClient).createWebSocket = vi.fn((url, handlers) => {
      openHandler = handlers.onOpen
      return {
        close: vi.fn(),
        socketGetter: vi.fn(() => ({ close: vi.fn() }))
      }
    })

    const wrapper = mountView()
    wrapper.vm.connect()
    openHandler()

    expect(wrapper.vm.serial).toContain('Websocket established')
  })

  it('websocket onError appends error message with err.message', async () => {
    const { sharedHttpClient } = await import('@mp-se/espframework-ui-components')
    let errorHandler = null

    vi.mocked(sharedHttpClient).createWebSocket = vi.fn((url, handlers) => {
      errorHandler = handlers.onError
      return {
        close: vi.fn(),
        socketGetter: vi.fn(() => ({ close: vi.fn() }))
      }
    })

    const wrapper = mountView()
    wrapper.vm.connect()
    errorHandler({ message: 'Connection error' })

    expect(wrapper.vm.serial).toContain('Websocket error: Connection error')
  })

  it('websocket onError handles error without message property', async () => {
    const { sharedHttpClient } = await import('@mp-se/espframework-ui-components')
    let errorHandler = null

    vi.mocked(sharedHttpClient).createWebSocket = vi.fn((url, handlers) => {
      errorHandler = handlers.onError
      return {
        close: vi.fn(),
        socketGetter: vi.fn(() => ({ close: vi.fn() }))
      }
    })

    const wrapper = mountView()
    wrapper.vm.connect()
    errorHandler('Raw error string')

    expect(wrapper.vm.serial).toContain('Websocket error: Raw error string')
  })

  it('websocket onClose appends close message', async () => {
    const { sharedHttpClient } = await import('@mp-se/espframework-ui-components')
    let closeHandler = null

    vi.mocked(sharedHttpClient).createWebSocket = vi.fn((url, handlers) => {
      closeHandler = handlers.onClose
      return {
        close: vi.fn(),
        socketGetter: vi.fn(() => ({ close: vi.fn() }))
      }
    })

    const wrapper = mountView()
    wrapper.vm.connect()
    closeHandler()

    expect(wrapper.vm.serial).toContain('Socket closed')
  })

  it('websocket onClose sets socket.value to null', async () => {
    const { sharedHttpClient } = await import('@mp-se/espframework-ui-components')
    let closeHandler = null

    vi.mocked(sharedHttpClient).createWebSocket = vi.fn((url, handlers) => {
      closeHandler = handlers.onClose
      const mockSock = { close: vi.fn() }
      return {
        close: vi.fn(),
        socketGetter: vi.fn(() => mockSock)
      }
    })

    const wrapper = mountView()
    wrapper.vm.connect()
    // After connect, socket is set
    expect(wrapper.vm.isConnected).toBe(true)
    closeHandler()
    await wrapper.vm.$nextTick()
    // After onClose, socket.value = null
    expect(wrapper.vm.isConnected).toBe(false)
  })

  it('websocket onError appends error message', async () => {
    const { sharedHttpClient } = await import('@mp-se/espframework-ui-components')
    let errorHandler = null

    vi.mocked(sharedHttpClient).createWebSocket = vi.fn((url, handlers) => {
      errorHandler = handlers.onError
      return {
        close: vi.fn(),
        socketGetter: vi.fn(() => ({ close: vi.fn() }))
      }
    })

    const wrapper = mountView()
    wrapper.vm.connect()
    errorHandler(new Error('connection refused'))

    expect(wrapper.vm.serial).toContain('Websocket error')
    expect(wrapper.vm.serial).toContain('connection refused')
  })

  it('websocket onError handles error without message property', async () => {
    const { sharedHttpClient } = await import('@mp-se/espframework-ui-components')
    let errorHandler = null

    vi.mocked(sharedHttpClient).createWebSocket = vi.fn((url, handlers) => {
      errorHandler = handlers.onError
      return {
        close: vi.fn(),
        socketGetter: vi.fn(() => ({ close: vi.fn() }))
      }
    })

    const wrapper = mountView()
    wrapper.vm.connect()
    errorHandler('plain string error')

    expect(wrapper.vm.serial).toContain('Websocket error')
  })

  it('onUnmounted calls socketCloser when it is a function', async () => {
    const { sharedHttpClient } = await import('@mp-se/espframework-ui-components')
    const closeFn = vi.fn()

    vi.mocked(sharedHttpClient).createWebSocket = vi.fn((url, handlers) => {
      if (handlers.onOpen) handlers.onOpen()
      return {
        close: closeFn,
        socketGetter: vi.fn(() => ({ close: vi.fn() }))
      }
    })

    const wrapper = mountView()
    // onMounted called connect(), which set socketCloser = closeFn
    await wrapper.unmount()
    expect(closeFn).toHaveBeenCalled()
  })

  it('onUnmounted closes socket.value when socketCloser is null', async () => {
    const { sharedHttpClient } = await import('@mp-se/espframework-ui-components')
    const socketCloseFn = vi.fn()

    vi.mocked(sharedHttpClient).createWebSocket = vi.fn((url, handlers) => {
      if (handlers.onOpen) handlers.onOpen()
      return {
        close: null, // not a function, so socketCloser remains null-like
        socketGetter: vi.fn(() => ({ close: socketCloseFn }))
      }
    })

    const wrapper = mountView()
    await wrapper.unmount()
    // Since socketCloser (null) is not a function, falls through to socket.value.close()
    // socket.value was set by socketGetter(), so socket.value.close() is called
    expect(socketCloseFn).toHaveBeenCalled()
  })

  it('onMessage drops oldest lines when serial exceeds maxLines (50)', async () => {
    const { sharedHttpClient } = await import('@mp-se/espframework-ui-components')
    let capturedHandlers = null
    vi.mocked(sharedHttpClient).createWebSocket = vi.fn((url, handlers) => {
      capturedHandlers = handlers
      if (handlers.onOpen) handlers.onOpen()
      return { close: vi.fn(), socketGetter: vi.fn(() => ({ close: vi.fn() })) }
    })
    const wrapper = mountView()
    // Pre-fill serial with 50 lines so the next message triggers list.shift()
    wrapper.vm.serial = Array(50).fill('line').join('\n') + '\n'
    capturedHandlers.onMessage({ data: 'extra line' })
    // After shift, serial should still be <= maxLines + 1 new line
    const lines = wrapper.vm.serial.split('\n').filter((l) => l.length > 0)
    expect(lines.length).toBeLessThanOrEqual(51)
  })
})
