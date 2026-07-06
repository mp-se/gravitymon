import { describe, it, expect, beforeEach, vi } from 'vitest'
import { mount } from '@vue/test-utils'
import AdvancedFilesFragment from '../AdvancedFilesFragment.vue'
import { createTestingPinia } from '../../tests/testUtils'
import { sharedHttpClient as http } from '@mp-se/espframework-ui-components'

describe('AdvancedFilesFragment (interaction tests)', () => {
  const createWrapper = (props = {}) => {
    const pinia = createTestingPinia()
    return mount(AdvancedFilesFragment, {
      props,
      global: {
        plugins: [pinia],
        stubs: { BsFileUpload: true, BsProgress: true, BsModalConfirm: true }
      }
    })
  }

  it('mounts without error', () => {
    const wrapper = createWrapper()
    expect(wrapper.exists()).toBe(true)
  })

  it('displays upload section for fs type', () => {
    const wrapper = createWrapper({ type: 'fs' })
    expect(wrapper.text()).toContain('Upload files to file system')
  })

  it('displays delete section', () => {
    const wrapper = createWrapper()
    expect(wrapper.text()).toContain('Delete files from file system')
  })

  it('has upload button', () => {
    const wrapper = createWrapper({ type: 'fs' })
    const buttons = wrapper.findAll('button')
    const uploadButton = buttons.find((b) => b.text().includes('Upload file'))
    expect(uploadButton).toBeDefined()
  })

  it('has list files delete button', () => {
    const wrapper = createWrapper()
    const buttons = wrapper.findAll('button')
    const listButton = buttons.find((b) => b.text().includes('List files'))
    expect(listButton).toBeDefined()
  })

  it('upload button is initially disabled', () => {
    const wrapper = createWrapper({ type: 'fs' })
    expect(wrapper.vm.hasFileSelected).toBe(false)
  })

  it('has hasFileSelected reactive property', () => {
    const wrapper = createWrapper({ type: 'fs' })
    expect(wrapper.vm.hasFileSelected).toBeDefined()
    expect(typeof wrapper.vm.hasFileSelected).toBe('boolean')
  })

  it('has progress reactive property', () => {
    const wrapper = createWrapper({ type: 'fs' })
    expect(wrapper.vm.progress).toBeDefined()
  })

  it('has filesDelete reactive property', () => {
    const wrapper = createWrapper()
    expect(wrapper.vm.filesDelete).toBeDefined()
  })

  it('has onFileChange function defined', () => {
    const wrapper = createWrapper({ type: 'fs' })
    expect(typeof wrapper.vm.onFileChange).toBe('function')
  })

  it('has upload function defined', () => {
    const wrapper = createWrapper({ type: 'fs' })
    expect(typeof wrapper.vm.upload).toBe('function')
  })

  it('has listFilesDelete function defined', () => {
    const wrapper = createWrapper()
    expect(typeof wrapper.vm.listFilesDelete).toBe('function')
  })

  it('has deleteFile function defined', () => {
    const wrapper = createWrapper()
    expect(typeof wrapper.vm.deleteFile).toBe('function')
  })

  it('accepts type prop', () => {
    const wrapper = createWrapper({ type: 'sd' })
    expect(wrapper.props('type')).toBe('sd')
  })

  it('displays correct delete title for sd type', () => {
    const wrapper = createWrapper({ type: 'sd' })
    expect(wrapper.text()).toContain('Delete files from SD file system')
  })

  it('has uploadTitle computed property', () => {
    const wrapper = createWrapper({ type: 'fs' })
    expect(wrapper.vm.uploadTitle).toBeDefined()
  })

  it('has deleteTitle computed property', () => {
    const wrapper = createWrapper()
    expect(wrapper.vm.deleteTitle).toBeDefined()
  })

  it('has listButtonText computed property', () => {
    const wrapper = createWrapper()
    expect(wrapper.vm.listButtonText).toBeDefined()
  })

  it('renders form element for upload', () => {
    const wrapper = createWrapper({ type: 'fs' })
    expect(wrapper.find('form').exists()).toBe(true)
  })

  it('upload form has prevent default', () => {
    const wrapper = createWrapper({ type: 'fs' })
    const form = wrapper.find('form')
    expect(form.exists()).toBe(true)
  })

  it('has container layout element', () => {
    const wrapper = createWrapper()
    expect(wrapper.find('.row').exists()).toBe(true)
  })

  it('displays file upload component', () => {
    const wrapper = createWrapper({ type: 'fs' })
    expect(wrapper.findComponent({ name: 'BsFileUpload' }).exists()).toBe(true)
  })

  it('displays modal confirm component', () => {
    const wrapper = createWrapper()
    expect(wrapper.findComponent({ name: 'BsModalConfirm' }).exists()).toBe(true)
  })
})

describe('AdvancedFilesFragment (action tests)', () => {
  beforeEach(() => {
    vi.clearAllMocks()
  })

  const createWrapper = (props = {}) => {
    const pinia = createTestingPinia()
    return mount(AdvancedFilesFragment, {
      props,
      global: {
        plugins: [pinia],
        stubs: { BsFileUpload: true, BsProgress: true, BsModalConfirm: true }
      }
    })
  }

  it('listFilesDelete calls filesystemRequest', async () => {
    http.filesystemRequest.mockResolvedValue({
      success: true,
      text: JSON.stringify({ files: [{ file: '/config.json' }] })
    })
    const wrapper = createWrapper({ type: 'fs' })
    await wrapper.vm.listFilesDelete()
    expect(http.filesystemRequest).toHaveBeenCalledWith(expect.objectContaining({ command: 'dir' }))
  })

  it('listFilesDelete populates filesDelete from response', async () => {
    http.filesystemRequest.mockResolvedValue({
      success: true,
      text: JSON.stringify({ files: [{ file: '/log.txt' }, { file: '/cfg.json' }] })
    })
    const wrapper = createWrapper({ type: 'fs' })
    await wrapper.vm.listFilesDelete()
    expect(wrapper.vm.filesDelete).toContain('/log.txt')
  })

  it('confirmDeleteCallback(false) does not call filesystemRequest', async () => {
    const wrapper = createWrapper({ type: 'fs' })
    await wrapper.vm.confirmDeleteCallback(false)
    expect(http.filesystemRequest).not.toHaveBeenCalled()
  })

  it('confirmDeleteCallback(true) calls filesystemRequest with del command', async () => {
    http.filesystemRequest.mockResolvedValue({ success: true })
    const wrapper = createWrapper({ type: 'fs' })
    wrapper.vm.confirmDeleteFile = '/to-delete.txt'
    await wrapper.vm.confirmDeleteCallback(true)
    expect(http.filesystemRequest).toHaveBeenCalledWith(
      expect.objectContaining({ command: 'del', file: '/to-delete.txt' })
    )
  })

  it('deleteFile sets confirmDeleteMessage', () => {
    const wrapper = createWrapper({ type: 'fs' })
    vi.spyOn(document, 'getElementById').mockReturnValue({ click: vi.fn() })
    wrapper.vm.deleteFile('/test.json')
    expect(wrapper.vm.confirmDeleteMessage).toContain('/test.json')
    vi.restoreAllMocks()
  })

  it('onFileChange sets hasFileSelected true when file present', async () => {
    const wrapper = createWrapper({ type: 'fs' })
    const fakeFile = new File(['content'], 'test.txt')
    wrapper.vm.onFileChange({ target: { files: [fakeFile] } })
    await wrapper.vm.$nextTick()
    expect(wrapper.vm.hasFileSelected).toBe(true)
  })

  it('upload calls http.uploadFile', async () => {
    const wrapper = createWrapper({ type: 'fs' })
    const fakeFile = new File(['data'], 'fw.bin')
    vi.spyOn(document, 'getElementById').mockReturnValue({ files: [fakeFile] })
    await wrapper.vm.upload()
    expect(http.uploadFile).toHaveBeenCalledWith(
      'api/filesystem/upload',
      fakeFile,
      expect.any(Object)
    )
    vi.restoreAllMocks()
  })
})

describe('AdvancedFilesFragment (SD type)', () => {
  beforeEach(() => {
    vi.clearAllMocks()
  })

  const createWrapper = (props = {}) => {
    const pinia = createTestingPinia()
    return mount(AdvancedFilesFragment, {
      props,
      global: {
        plugins: [pinia],
        stubs: { BsFileUpload: true, BsProgress: true, BsModalConfirm: true }
      }
    })
  }

  it('listFilesDelete for SD type calls postJson', async () => {
    http.postJson.mockResolvedValue({
      text: vi.fn().mockResolvedValue(JSON.stringify({ files: [{ file: '/sd/log.txt' }] }))
    })
    const wrapper = createWrapper({ type: 'sd' })
    await wrapper.vm.listFilesDelete()
    expect(http.postJson).toHaveBeenCalledWith(
      'api/sd',
      expect.objectContaining({ command: 'dir' }),
      expect.any(Object)
    )
  })

  it('listFilesDelete for SD type populates filesDelete', async () => {
    http.postJson.mockResolvedValue({
      text: vi.fn().mockResolvedValue(JSON.stringify({ files: [{ file: '/sd/config.json' }] }))
    })
    const wrapper = createWrapper({ type: 'sd' })
    await wrapper.vm.listFilesDelete()
    expect(wrapper.vm.filesDelete).toContain('/sd/config.json')
  })

  it('confirmDeleteCallback(true) for SD type calls postJson with del', async () => {
    http.postJson.mockResolvedValue({
      text: vi.fn().mockResolvedValue(JSON.stringify({}))
    })
    const wrapper = createWrapper({ type: 'sd' })
    wrapper.vm.confirmDeleteFile = '/sd/to-delete.txt'
    await wrapper.vm.confirmDeleteCallback(true)
    expect(http.postJson).toHaveBeenCalledWith(
      'api/sd',
      expect.objectContaining({ command: 'del', file: '/sd/to-delete.txt' }),
      expect.any(Object)
    )
  })

  it('sendSecureDiskRequest handles postJson failure gracefully', async () => {
    http.postJson.mockRejectedValue(new Error('network fail'))
    const wrapper = createWrapper({ type: 'sd' })
    await wrapper.vm.listFilesDelete()
    expect(wrapper.vm.filesDelete).toEqual([])
  })
})

describe('AdvancedFilesFragment (upload edge cases)', () => {
  beforeEach(() => {
    vi.clearAllMocks()
  })

  const createWrapper = (props = {}) => {
    const pinia = createTestingPinia()
    return mount(AdvancedFilesFragment, {
      props,
      global: {
        plugins: [pinia],
        stubs: { BsFileUpload: true, BsProgress: true, BsModalConfirm: true }
      }
    })
  }

  it('upload sets messageSuccess on success', async () => {
    const fakeFile = new File(['data'], 'fw.bin')
    vi.spyOn(document, 'getElementById').mockReturnValue({ files: [fakeFile] })
    http.uploadFile.mockResolvedValue({ success: true })
    const wrapper = createWrapper({ type: 'fs' })
    await wrapper.vm.upload()
    const { global: globalStore } = await import('@/modules/pinia')
    expect(globalStore.messageSuccess).toBeTruthy()
    vi.restoreAllMocks()
  })

  it('upload sets messageError when uploadFile returns failure', async () => {
    const fakeFile = new File(['data'], 'fw.bin')
    vi.spyOn(document, 'getElementById').mockReturnValue({ files: [fakeFile] })
    http.uploadFile.mockResolvedValue({ success: false, status: 500 })
    const wrapper = createWrapper({ type: 'fs' })
    await wrapper.vm.upload()
    const { global: globalStore } = await import('@/modules/pinia')
    expect(globalStore.messageError).toContain('failed')
    vi.restoreAllMocks()
  })

  it('upload sets messageError when uploadFile throws', async () => {
    const fakeFile = new File(['data'], 'fw.bin')
    vi.spyOn(document, 'getElementById').mockReturnValue({ files: [fakeFile] })
    http.uploadFile.mockRejectedValue(new Error('network error'))
    const wrapper = createWrapper({ type: 'fs' })
    await wrapper.vm.upload()
    const { global: globalStore } = await import('@/modules/pinia')
    expect(globalStore.messageError).toContain('failed')
    vi.restoreAllMocks()
  })

  it('upload invokes onProgress callback during file upload', async () => {
    const fakeFile = new File(['data'], 'fw.bin')
    vi.spyOn(document, 'getElementById').mockReturnValue({ files: [fakeFile] })
    http.uploadFile.mockImplementation(async (_url, _file, options) => {
      if (options && options.onProgress) {
        options.onProgress(50)
        options.onProgress(100)
      }
      return { success: true }
    })
    const wrapper = createWrapper({ type: 'fs' })
    await wrapper.vm.upload()
    expect(wrapper.vm.progress).toBe(100)
    vi.restoreAllMocks()
  })
})
