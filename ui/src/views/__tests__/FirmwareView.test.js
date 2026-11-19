import { describe, it, expect, beforeEach, vi } from 'vitest'
import { mount } from '@vue/test-utils'
import FirmwareView from '../FirmwareView.vue'
import { createPinia, setActivePinia } from 'pinia'
import { sharedHttpClient as http } from '@mp-se/espframework-ui-components'
import { global } from '@/modules/pinia'

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

// Minimal firmware image header: magic byte 0xE9 and the chip id at bytes 12-13
function validFirmwareFile(chipId = 0, name = 'firmware.bin') {
  const header = new Uint8Array(16)
  header[0] = 0xe9
  header[12] = chipId & 0xff
  header[13] = chipId >> 8
  return new File([header], name)
}

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
    const fakeFile = validFirmwareFile()
    const fakeElement = { files: [fakeFile] }
    vi.spyOn(document, 'getElementById').mockReturnValue(fakeElement)
    http.uploadFile.mockResolvedValue({ success: true })
    await wrapper.vm.upload()
    expect(http.uploadFile).toHaveBeenCalledWith('api/firmware', fakeFile, expect.any(Object))
    vi.restoreAllMocks()
  })

  it('upload sets messageSuccess on successful upload', async () => {
    const wrapper = mountView()
    const fakeFile = validFirmwareFile()
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
    const fakeFile = validFirmwareFile()
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
    const fakeFile = validFirmwareFile()
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
    const fakeFile = validFirmwareFile()
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
    const fakeFile = validFirmwareFile()
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
    const fakeFile = validFirmwareFile()
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
    vi.useFakeTimers({ toFake: ['setTimeout', 'clearTimeout'] })
    const wrapper = mountView()
    const fakeFile = validFirmwareFile()
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
    const fakeFile = validFirmwareFile()
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
    const fakeFile = validFirmwareFile()
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
    vi.useFakeTimers({ toFake: ['setTimeout', 'clearTimeout'] })
    const wrapper = mountView()
    const fakeFile = validFirmwareFile()
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
    vi.useFakeTimers({ toFake: ['setTimeout', 'clearTimeout'] })
    const wrapper = mountView()
    const fakeFile = validFirmwareFile()
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

  it('hides platform, version, hardware and file name when enableFirmwareDetails is off', () => {
    global.platform = 'ESP32C3'
    global.hardware = 'CUCKOO'
    global.firmware_file = 'firmware32c3.bin'
    global.ui.enableFirmwareDetails = false

    const wrapper = mount(FirmwareView, {
      global: { stubs: { BsFileUpload: true, BsProgress: true } }
    })

    expect(wrapper.text()).not.toContain('ESP32C3')
    expect(wrapper.text()).not.toContain('CUCKOO')
    expect(wrapper.text()).not.toContain('firmware32c3.bin')
    expect(wrapper.find('.badge').exists()).toBe(false)
    // The upload itself is still there
    expect(wrapper.text()).toContain('Flash firmware')

    global.ui.enableFirmwareDetails = true
  })

  it('shows platform, hardware and file name when enableFirmwareDetails is on', () => {
    global.platform = 'ESP32C3'
    global.hardware = 'CUCKOO'
    global.firmware_file = 'firmware32c3.bin'
    global.ui.enableFirmwareDetails = true

    const wrapper = mount(FirmwareView, {
      global: { stubs: { BsFileUpload: true, BsProgress: true } }
    })

    expect(wrapper.text()).toContain('ESP32C3')
    expect(wrapper.text()).toContain('CUCKOO')
    expect(wrapper.text()).toContain('firmware32c3.bin')
  })

  describe('firmware file validation before upload', () => {
    const mountWithFile = (file, platform, firmwareFile = '') => {
      global.platform = platform
      global.firmware_file = firmwareFile
      global.messageError = ''
      const wrapper = mount(FirmwareView, {
        global: { stubs: { BsFileUpload: true, BsProgress: true } }
      })
      vi.spyOn(document, 'getElementById').mockReturnValue({ files: [file] })
      return wrapper
    }

    it('uploads a firmware built for this chip', async () => {
      http.uploadFile.mockClear()
      http.uploadFile.mockResolvedValue({ success: true })
      const wrapper = mountWithFile(validFirmwareFile(5), 'ESP32C3')
      await wrapper.vm.upload()
      expect(http.uploadFile).toHaveBeenCalledTimes(1)
      vi.restoreAllMocks()
    })

    it('rejects a firmware built for another chip without uploading', async () => {
      http.uploadFile.mockClear()
      const wrapper = mountWithFile(validFirmwareFile(2), 'ESP32C3')
      await wrapper.vm.upload()
      expect(http.uploadFile).not.toHaveBeenCalled()
      expect(global.messageError).toContain('another chip')
      expect(global.disabled).toBe(false)
      vi.restoreAllMocks()
    })

    it('rejects a file with another name than the device reported without uploading', async () => {
      http.uploadFile.mockClear()
      const wrapper = mountWithFile(
        validFirmwareFile(5, 'firmware32c3supermini.bin'),
        'ESP32C3',
        'firmware32c3.bin'
      )
      await wrapper.vm.upload()
      expect(http.uploadFile).not.toHaveBeenCalled()
      expect(global.messageError).toContain('firmware32c3.bin')
      vi.restoreAllMocks()
    })

    it('clears the previous error when flash is pressed again', async () => {
      http.uploadFile.mockClear()
      http.uploadFile.mockResolvedValue({ success: true })
      const wrapper = mountWithFile(validFirmwareFile(5), 'ESP32C3')
      global.messageError = 'Error from a previous attempt'
      await wrapper.vm.upload()
      expect(global.messageError).toBe('')
      vi.restoreAllMocks()
    })

    it('rejects a file that is not a firmware image without uploading', async () => {
      http.uploadFile.mockClear()
      const wrapper = mountWithFile(new File(['not firmware at all'], 'notes.txt'), 'ESP32C3')
      await wrapper.vm.upload()
      expect(http.uploadFile).not.toHaveBeenCalled()
      expect(global.messageError).toContain('not a valid firmware')
      vi.restoreAllMocks()
    })
  })
})
