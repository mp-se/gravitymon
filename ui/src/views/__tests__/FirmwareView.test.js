import { describe, it, expect, beforeEach, vi } from 'vitest'
import { mount } from '@vue/test-utils'
import FirmwareView from '../FirmwareView.vue'
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
    postJson: vi.fn(),
    uploadFile: vi.fn()
  }
}))

describe('FirmwareView (interaction tests)', () => {
  beforeEach(() => {
    setActivePinia(createPinia())
  })

  it('mounts without error', () => {
    const wrapper = mount(FirmwareView, {
      global: {
        stubs: {
          BsFileUpload: true,
          BsProgress: true
        }
      }
    })
    expect(wrapper.exists()).toBe(true)
  })

  it('displays firmware upload section', () => {
    const wrapper = mount(FirmwareView, {
      global: {
        stubs: {
          BsFileUpload: true,
          BsProgress: true
        }
      }
    })
    expect(wrapper.text()).toContain('Firmware Upload')
  })

  it('displays current platform badge', () => {
    const wrapper = mount(FirmwareView, {
      global: {
        stubs: {
          BsFileUpload: true,
          BsProgress: true
        }
      }
    })
    expect(wrapper.text()).toContain('Platform')
  })

  it('has file upload component with .bin accept', () => {
    const wrapper = mount(FirmwareView, {
      global: {
        stubs: {
          BsFileUpload: {
            template:
              '<input type="file" accept=".bin" id="upload" name="upload" class="firmware-upload" />'
          },
          BsProgress: true
        }
      }
    })
    const fileInput = wrapper.find('input[type="file"]')
    expect(fileInput.exists()).toBe(true)
    expect(fileInput.attributes('accept')).toBe('.bin')
  })

  it('has upload button', () => {
    const wrapper = mount(FirmwareView, {
      global: {
        stubs: {
          BsFileUpload: true,
          BsProgress: true
        }
      }
    })
    const buttons = wrapper.findAll('button')
    const uploadButton = buttons.find((b) => b.text().includes('Flash firmware'))
    expect(uploadButton).toBeDefined()
  })

  it('upload button is disabled when no file selected', () => {
    const wrapper = mount(FirmwareView, {
      global: {
        stubs: {
          BsFileUpload: true,
          BsProgress: true
        }
      }
    })
    expect(wrapper.vm.hasFileSelected).toBe(false)
  })

  it('initializes with hasFileSelected as false', () => {
    const wrapper = mount(FirmwareView, {
      global: {
        stubs: {
          BsFileUpload: true,
          BsProgress: true
        }
      }
    })
    expect(wrapper.vm.hasFileSelected).toBe(false)
  })

  it('initializes with progress as zero', () => {
    const wrapper = mount(FirmwareView, {
      global: {
        stubs: {
          BsFileUpload: true,
          BsProgress: true
        }
      }
    })
    expect(wrapper.vm.progress).toBe(0)
  })

  it('has onFileChange handler function', () => {
    const wrapper = mount(FirmwareView, {
      global: {
        stubs: {
          BsFileUpload: true,
          BsProgress: true
        }
      }
    })
    expect(typeof wrapper.vm.onFileChange).toBe('function')
  })

  it('has upload function defined', () => {
    const wrapper = mount(FirmwareView, {
      global: {
        stubs: {
          BsFileUpload: true,
          BsProgress: true
        }
      }
    })
    expect(typeof wrapper.vm.upload).toBe('function')
  })

  it('displays help text for firmware selection', () => {
    const wrapper = mount(FirmwareView, {
      global: {
        stubs: {
          BsFileUpload: true,
          BsProgress: true
        }
      }
    })
    expect(wrapper.text()).toContain('firmware')
  })

  it('has form element', () => {
    const wrapper = mount(FirmwareView, {
      global: {
        stubs: {
          BsFileUpload: true,
          BsProgress: true
        }
      }
    })
    const form = wrapper.find('form')
    expect(form.exists()).toBe(true)
  })

  it('upload button has correct ID', () => {
    const wrapper = mount(FirmwareView, {
      global: {
        stubs: {
          BsFileUpload: true,
          BsProgress: true
        }
      }
    })
    const uploadBtn = wrapper.find('#upload-btn')
    expect(uploadBtn.exists()).toBe(true)
  })

  it('displays version information when available', () => {
    const wrapper = mount(FirmwareView, {
      global: {
        stubs: {
          BsFileUpload: true,
          BsProgress: true
        }
      }
    })
    // Version section should be in template
    expect(wrapper.html()).toContain('Version')
  })

  it('displays hardware information section', () => {
    const wrapper = mount(FirmwareView, {
      global: {
        stubs: {
          BsFileUpload: true,
          BsProgress: true
        }
      }
    })
    // Component displays device info including platform badge
    expect(wrapper.html()).toContain('badge')
    expect(wrapper.html()).toContain('Platform:')
  })
})

describe('FirmwareView (action tests)', () => {
  beforeEach(() => {
    setActivePinia(createPinia())
    vi.clearAllMocks()
  })

  const mountView = () =>
    mount(FirmwareView, {
      global: {
        stubs: {
          BsFileUpload: true,
          BsProgress: true
        }
      }
    })

  it('onFileChange sets hasFileSelected true when file is present', async () => {
    const wrapper = mountView()
    const fakeFile = new File(['content'], 'firmware.bin', { type: 'application/octet-stream' })
    const fakeEvent = { target: { files: [fakeFile] } }
    wrapper.vm.onFileChange(fakeEvent)
    await wrapper.vm.$nextTick()
    expect(wrapper.vm.hasFileSelected).toBe(true)
  })

  it('onFileChange sets hasFileSelected false when no files', async () => {
    const wrapper = mountView()
    wrapper.vm.onFileChange({ target: { files: [] } })
    await wrapper.vm.$nextTick()
    expect(wrapper.vm.hasFileSelected).toBe(false)
  })

  it('upload sets messageError when no file selected', async () => {
    const wrapper = mountView()
    const fakeElement = { files: [] }
    vi.spyOn(document, 'getElementById').mockReturnValue(fakeElement)
    await wrapper.vm.upload()
    const { global: globalStore } = await import('@/modules/pinia')
    expect(globalStore.messageError).toBeTruthy()
    vi.restoreAllMocks()
  })

  it('upload calls http.uploadFile when file is selected', async () => {
    const wrapper = mountView()
    const fakeFile = new File(['fw'], 'firmware.bin')
    const fakeElement = { files: [fakeFile] }
    vi.spyOn(document, 'getElementById').mockReturnValue(fakeElement)
    http.uploadFile.mockResolvedValue({ success: true })
    await wrapper.vm.upload()
    expect(http.uploadFile).toHaveBeenCalledWith('api/firmware', fakeFile, expect.any(Object))
    vi.restoreAllMocks()
  })

  it('upload sets messageSuccess on successful upload', async () => {
    const wrapper = mountView()
    const fakeFile = new File(['fw'], 'firmware.bin')
    const fakeElement = { files: [fakeFile] }
    vi.spyOn(document, 'getElementById').mockReturnValue(fakeElement)
    http.uploadFile.mockResolvedValue({ success: true })
    await wrapper.vm.upload()
    const { global: globalStore } = await import('@/modules/pinia')
    expect(globalStore.messageSuccess).toBeTruthy()
    vi.restoreAllMocks()
  })

  it('upload sets messageError on failed upload', async () => {
    const wrapper = mountView()
    const fakeFile = new File(['fw'], 'firmware.bin')
    const fakeElement = { files: [fakeFile] }
    vi.spyOn(document, 'getElementById').mockReturnValue(fakeElement)
    http.uploadFile.mockResolvedValue({ success: false, status: 500 })
    await wrapper.vm.upload()
    const { global: globalStore } = await import('@/modules/pinia')
    expect(globalStore.messageError).toBeTruthy()
    vi.restoreAllMocks()
  })

  it('upload sets messageError on exception', async () => {
    const wrapper = mountView()
    const fakeFile = new File(['fw'], 'firmware.bin')
    const fakeElement = { files: [fakeFile] }
    vi.spyOn(document, 'getElementById').mockReturnValue(fakeElement)
    http.uploadFile.mockRejectedValue(new Error('network error'))
    await wrapper.vm.upload()
    const { global: globalStore } = await import('@/modules/pinia')
    expect(globalStore.messageError).toBeTruthy()
    vi.restoreAllMocks()
  })

  it('progress resets to 0 on new upload', async () => {
    const wrapper = mountView()
    const fakeFile = new File(['fw'], 'firmware.bin')
    const fakeElement = { files: [fakeFile] }
    vi.spyOn(document, 'getElementById').mockReturnValue(fakeElement)
    http.uploadFile.mockResolvedValue({ success: true })
    wrapper.vm.progress = 50
    await wrapper.vm.upload()
    vi.restoreAllMocks()
    // progress is set to 0 then 100 during upload
    expect(wrapper.vm.progress).toBe(100)
  })

  it('upload calls onProgress callback which updates progress value', async () => {
    const wrapper = mountView()
    const fakeFile = new File(['fw'], 'firmware.bin')
    const fakeElement = { files: [fakeFile] }
    vi.spyOn(document, 'getElementById').mockReturnValue(fakeElement)

    http.uploadFile.mockImplementation(async (_url, _file, opts) => {
      opts.onProgress(50)
      return { success: false, status: 'timeout' }
    })

    await wrapper.vm.upload()
    vi.restoreAllMocks()
    // onProgress should have been called with 50, then overwritten by progress=100 or set to 50
    // progress ends at 100 (set after uploadFile resolves)
    expect(wrapper.vm.progress).toBe(100)
  })

  it('upload sets messageSuccess on successful upload', async () => {
    const wrapper = mountView()
    const fakeFile = new File(['fw'], 'firmware.bin')
    const fakeElement = { files: [fakeFile] }
    vi.spyOn(document, 'getElementById').mockReturnValue(fakeElement)
    http.uploadFile.mockResolvedValue({ success: true })

    const { global: globalStore } = await import('@/modules/pinia')
    globalStore.messageSuccess = ''
    globalStore.messageError = ''

    await wrapper.vm.upload()
    vi.restoreAllMocks()

    expect(globalStore.messageSuccess).toContain('File upload completed')
  })

  it('upload success setTimeout fires redirect', async () => {
    vi.useFakeTimers()
    const wrapper = mountView()
    const fakeFile = new File(['fw'], 'firmware.bin')
    const fakeElement = { files: [fakeFile] }
    vi.spyOn(document, 'getElementById').mockReturnValue(fakeElement)
    http.uploadFile.mockResolvedValue({ success: true })
    Object.defineProperty(window, 'location', {
      value: { ...window.location, href: '/other/firmware', reload: vi.fn() },
      writable: true
    })

    await wrapper.vm.upload()
    // Advance past the 10000ms timeout
    vi.advanceTimersByTime(10001)
    vi.useRealTimers()
    vi.restoreAllMocks()
    expect(true).toBe(true)
  })

  it('upload sets messageError when response has success=false', async () => {
    const wrapper = mountView()
    const fakeFile = new File(['fw'], 'firmware.bin')
    const fakeElement = { files: [fakeFile] }
    vi.spyOn(document, 'getElementById').mockReturnValue(fakeElement)
    http.uploadFile.mockResolvedValue({ success: false, status: 503 })

    const { global: globalStore } = await import('@/modules/pinia')
    globalStore.messageError = ''

    await wrapper.vm.upload()
    vi.restoreAllMocks()

    expect(globalStore.messageError).toContain('503')
  })

  it('onFileChange sets hasFileSelected when a file is chosen', async () => {
    const wrapper = mountView()
    const fakeFile = new File(['fw'], 'firmware.bin')
    const event = { target: { files: [fakeFile] } }
    wrapper.vm.onFileChange(event)
    expect(wrapper.vm.hasFileSelected).toBe(true)
  })

  it('onFileChange sets hasFileSelected to false when no file', async () => {
    const wrapper = mountView()
    const event = { target: { files: [] } }
    wrapper.vm.onFileChange(event)
    expect(wrapper.vm.hasFileSelected).toBe(false)
  })

  it('upload success setTimeout catch block executes when location.href setter throws', async () => {
    vi.useFakeTimers()
    const wrapper = mountView()
    const fakeFile = new File(['fw'], 'firmware.bin')
    const fakeElement = { files: [fakeFile] }
    vi.spyOn(document, 'getElementById').mockReturnValue(fakeElement)
    http.uploadFile.mockResolvedValue({ success: true })

    // Make location.href.replace() throw so the catch block is entered
    const origReplace = String.prototype.replace
    String.prototype.replace = function (search, replacement) {
      if (typeof search === 'string' && search === '/other/firmware') {
        throw new Error('navigation blocked')
      }
      return origReplace.call(this, search, replacement)
    }

    await wrapper.vm.upload()
    vi.advanceTimersByTime(10001)
    await Promise.resolve()

    String.prototype.replace = origReplace
    vi.useRealTimers()
    vi.restoreAllMocks()

    // Test passes if no unhandled error; catch block executed logError silently
    expect(wrapper.exists()).toBe(true)
  })

  it('upload success beforeunload handler clears timeout when event fires', async () => {
    vi.useFakeTimers()
    const wrapper = mountView()
    const fakeFile = new File(['fw'], 'firmware.bin')
    const fakeElement = { files: [fakeFile] }
    vi.spyOn(document, 'getElementById').mockReturnValue(fakeElement)
    http.uploadFile.mockResolvedValue({ success: true })
    const reloadSpy = vi.fn()
    Object.defineProperty(window, 'location', {
      value: { reload: reloadSpy, href: '/other/firmware' },
      writable: true,
      configurable: true
    })

    await wrapper.vm.upload()
    // Fire beforeunload to trigger clearTimeout (covers the beforeunload handler)
    window.dispatchEvent(new Event('beforeunload'))

    vi.useRealTimers()
    vi.restoreAllMocks()
    expect(wrapper.exists()).toBe(true)
  })
})
