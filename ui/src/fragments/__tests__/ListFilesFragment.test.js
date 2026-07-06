import { describe, it, expect, beforeEach, vi } from 'vitest'
import { mount } from '@vue/test-utils'
import ListFilesFragment from '../ListFilesFragment.vue'
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
    request: vi.fn(),
    filesystemRequest: vi.fn()
  },
  isValidJson: vi.fn(() => true),
  isValidFormData: vi.fn(() => false),
  isValidMqttData: vi.fn(() => false)
}))

describe('ListFilesFragment (interaction tests)', () => {
  beforeEach(() => {
    setActivePinia(createPinia())
  })

  it('mounts without error', () => {
    const wrapper = mount(ListFilesFragment, {
      props: {
        type: 'fs'
      },
      global: {
        stubs: {
          BsProgress: true
        }
      }
    })
    expect(wrapper.exists()).toBe(true)
  })

  it('mounts with sd type', () => {
    const wrapper = mount(ListFilesFragment, {
      props: {
        type: 'sd'
      },
      global: {
        stubs: {
          BsProgress: true
        }
      }
    })
    expect(wrapper.exists()).toBe(true)
  })

  it('displays title for filesystem when type is fs', () => {
    const wrapper = mount(ListFilesFragment, {
      props: {
        type: 'fs'
      },
      global: {
        stubs: {
          BsProgress: true
        }
      }
    })
    expect(wrapper.text()).toContain('Explore the file system')
  })

  it('displays title for SD filesystem when type is sd', () => {
    const wrapper = mount(ListFilesFragment, {
      props: {
        type: 'sd'
      },
      global: {
        stubs: {
          BsProgress: true
        }
      }
    })
    expect(wrapper.text()).toContain('Explore the SD file system')
  })

  it('has list button with filesystem text', () => {
    const wrapper = mount(ListFilesFragment, {
      props: {
        type: 'fs'
      },
      global: {
        stubs: {
          BsProgress: true
        }
      }
    })
    expect(wrapper.text()).toContain('List files')
  })

  it('has list button with SD text when type is sd', () => {
    const wrapper = mount(ListFilesFragment, {
      props: {
        type: 'sd'
      },
      global: {
        stubs: {
          BsProgress: true
        }
      }
    })
    expect(wrapper.text()).toContain('List SD files')
  })

  it('has list button element', () => {
    const wrapper = mount(ListFilesFragment, {
      props: {
        type: 'fs'
      },
      global: {
        stubs: {
          BsProgress: true
        }
      }
    })
    const buttons = wrapper.findAll('button')
    const listButton = buttons.find((b) => b.text().includes('List files'))
    expect(listButton).toBeDefined()
    expect(listButton.element.className).toContain('btn-secondary')
  })

  it('initializes filesystemUsage as null', () => {
    const wrapper = mount(ListFilesFragment, {
      props: {
        type: 'fs'
      },
      global: {
        stubs: {
          BsProgress: true
        }
      }
    })
    expect(wrapper.vm.filesystemUsage).toBeNull()
  })

  it('initializes fileData as null', () => {
    const wrapper = mount(ListFilesFragment, {
      props: {
        type: 'fs'
      },
      global: {
        stubs: {
          BsProgress: true
        }
      }
    })
    expect(wrapper.vm.fileData).toBeNull()
  })

  it('initializes filesView as empty array', () => {
    const wrapper = mount(ListFilesFragment, {
      props: {
        type: 'fs'
      },
      global: {
        stubs: {
          BsProgress: true
        }
      }
    })
    expect(wrapper.vm.filesView).toEqual([])
  })

  it('initializes fileDataSize as zero', () => {
    const wrapper = mount(ListFilesFragment, {
      props: {
        type: 'fs'
      },
      global: {
        stubs: {
          BsProgress: true
        }
      }
    })
    expect(wrapper.vm.fileDataSize).toBe(0)
  })

  it('initializes dataType as empty string', () => {
    const wrapper = mount(ListFilesFragment, {
      props: {
        type: 'fs'
      },
      global: {
        stubs: {
          BsProgress: true
        }
      }
    })
    expect(wrapper.vm.dataType).toBe('')
  })

  it('has listFilesView function defined', () => {
    const wrapper = mount(ListFilesFragment, {
      props: {
        type: 'fs'
      },
      global: {
        stubs: {
          BsProgress: true
        }
      }
    })
    expect(typeof wrapper.vm.listFilesView).toBe('function')
  })

  it('has viewFile function defined', () => {
    const wrapper = mount(ListFilesFragment, {
      props: {
        type: 'fs'
      },
      global: {
        stubs: {
          BsProgress: true
        }
      }
    })
    expect(typeof wrapper.vm.viewFile).toBe('function')
  })

  it('has formatFileSize function defined', () => {
    const wrapper = mount(ListFilesFragment, {
      props: {
        type: 'fs'
      },
      global: {
        stubs: {
          BsProgress: true
        }
      }
    })
    expect(typeof wrapper.vm.formatFileSize).toBe('function')
  })

  it('list button is initially not disabled', () => {
    const wrapper = mount(ListFilesFragment, {
      props: {
        type: 'fs'
      },
      global: {
        stubs: {
          BsProgress: true
        }
      }
    })
    const buttons = wrapper.findAll('button')
    const listButton = buttons.find((b) => b.text().includes('List files'))
    expect(listButton.attributes('disabled')).toBeUndefined()
  })

  it('displays button group for file list', () => {
    const wrapper = mount(ListFilesFragment, {
      props: {
        type: 'fs'
      },
      global: {
        stubs: {
          BsProgress: true
        }
      }
    })
    const buttonGroup = wrapper.find('.button-group')
    expect(buttonGroup.exists()).toBe(true)
  })

  it('has HTTP timeout constant defined', () => {
    const wrapper = mount(ListFilesFragment, {
      props: {
        type: 'fs'
      },
      global: {
        stubs: {
          BsProgress: true
        }
      }
    })
    // HTTP_TIMEOUT_MS should be 20000
    expect(wrapper.vm.HTTP_TIMEOUT_MS).toBe(20000)
  })
})

describe('ListFilesFragment (action tests)', () => {
  beforeEach(() => {
    setActivePinia(createPinia())
    vi.clearAllMocks()
  })

  const mountView = (type = 'fs') =>
    mount(ListFilesFragment, {
      props: { type },
      global: { stubs: { BsProgress: true } }
    })

  it('listFilesView calls filesystemRequest for fs type', async () => {
    http.filesystemRequest.mockResolvedValue({
      success: true,
      text: JSON.stringify({ files: [], used: 0, total: 0, free: 0 })
    })
    const wrapper = mountView('fs')
    await wrapper.vm.listFilesView()
    expect(http.filesystemRequest).toHaveBeenCalledWith(expect.objectContaining({ command: 'dir' }))
  })

  it('listFilesView populates filesView from response', async () => {
    http.filesystemRequest.mockResolvedValue({
      success: true,
      text: JSON.stringify({
        files: [{ file: '/config.json' }, { file: '/log.txt' }],
        used: 100,
        total: 1000,
        free: 900
      })
    })
    const wrapper = mountView('fs')
    await wrapper.vm.listFilesView()
    expect(wrapper.vm.filesView).toContain('/config.json')
  })

  it('listFilesView computes filesystemUsage when used/total present', async () => {
    http.filesystemRequest.mockResolvedValue({
      success: true,
      text: JSON.stringify({ files: [], used: 500, total: 1000, free: 500 })
    })
    const wrapper = mountView('fs')
    await wrapper.vm.listFilesView()
    expect(wrapper.vm.filesystemUsage).toBe(50)
  })

  it('viewFile calls filesystemRequest with get command', async () => {
    http.filesystemRequest.mockResolvedValue({
      success: true,
      text: '{"key":"value"}'
    })
    const wrapper = mountView('fs')
    await wrapper.vm.viewFile('/config.json')
    expect(http.filesystemRequest).toHaveBeenCalledWith(
      expect.objectContaining({ command: 'get', file: '/config.json' })
    )
  })

  it('viewFile sets fileData on success', async () => {
    http.filesystemRequest.mockResolvedValue({
      success: true,
      text: '{"key":"value"}'
    })
    const wrapper = mountView('fs')
    await wrapper.vm.viewFile('/config.json')
    expect(wrapper.vm.fileData).not.toBeNull()
    expect(wrapper.vm.dataType).toBe('json')
  })

  it('viewFile does not set fileData on failure', async () => {
    http.filesystemRequest.mockResolvedValue({ success: false, text: '' })
    const wrapper = mountView('fs')
    await wrapper.vm.viewFile('/missing.json')
    expect(wrapper.vm.fileData).toBeNull()
  })
})

describe('ListFilesFragment (utility functions)', () => {
  const mountView = (type = 'fs') =>
    mount(ListFilesFragment, {
      props: { type },
      global: { stubs: { BsProgress: true } }
    })

  let wrapper

  beforeEach(() => {
    setActivePinia(createPinia())
    vi.clearAllMocks()
    wrapper = mountView('fs')
  })

  it('formatFileSize returns KB for small values', () => {
    expect(wrapper.vm.formatFileSize(2048)).toBe('2 KB')
  })

  it('formatFileSize returns MB for large values', () => {
    expect(wrapper.vm.formatFileSize(2 * 1024 * 1024)).toBe('2.0 MB')
  })

  it('formatFileSize returns GB for very large values', () => {
    expect(wrapper.vm.formatFileSize(2 * 1024 * 1024 * 1024)).toBe('2.0 GB')
  })

  it('colorizeCsv wraps cells in span elements', () => {
    const result = wrapper.vm.colorizeCsv('a,b,c')
    expect(result).toContain('<span')
    expect(result).toContain('a')
  })

  it('colorizeFormData wraps key/value in span elements', () => {
    const result = wrapper.vm.colorizeFormData('key=value')
    expect(result).toContain('<span')
    expect(result).toContain('key')
    expect(result).toContain('value')
  })

  it('colorizeMqtt wraps parts in span elements', () => {
    const result = wrapper.vm.colorizeMqtt('topic:payload')
    expect(result).toContain('<span')
    expect(result).toContain('topic')
  })

  it('colorizeJson handles string value', () => {
    const result = wrapper.vm.colorizeJson('hello')
    expect(result).toContain('hello')
    expect(result).toContain('green')
  })

  it('colorizeJson handles number value', () => {
    const result = wrapper.vm.colorizeJson(42)
    expect(result).toContain('42')
    expect(result).toContain('blue')
  })

  it('colorizeJson handles boolean value', () => {
    const result = wrapper.vm.colorizeJson(true)
    expect(result).toContain('true')
    expect(result).toContain('red')
  })

  it('colorizeJson handles null', () => {
    const result = wrapper.vm.colorizeJson(null)
    expect(result).toContain('null')
  })

  it('colorizeJson handles empty array', () => {
    expect(wrapper.vm.colorizeJson([])).toBe('[]')
  })

  it('colorizeJson handles empty object', () => {
    expect(wrapper.vm.colorizeJson({})).toBe('{}')
  })

  it('colorizeJson handles object with keys', () => {
    const result = wrapper.vm.colorizeJson({ key: 'val' })
    expect(result).toContain('key')
    expect(result).toContain('val')
  })

  it('isBinary returns false for normal text', () => {
    expect(wrapper.vm.isBinary('hello world')).toBe(false)
  })

  it('isBinary returns true for text with Unicode replacement char', () => {
    expect(wrapper.vm.isBinary('data\uFFFD')).toBe(true)
  })

  it('isBinary returns true for text with control characters', () => {
    expect(wrapper.vm.isBinary('data\x01')).toBe(true)
  })

  it('toHex converts text to hex representation', () => {
    const result = wrapper.vm.toHex('A')
    expect(result).toContain('41')
  })

  it('isValidCsvData returns false for empty text', () => {
    expect(wrapper.vm.isValidCsvData('')).toBe(false)
  })

  it('isValidCsvData returns true for valid CSV', () => {
    expect(wrapper.vm.isValidCsvData('a,b,c\n1,2,3')).toBe(true)
  })
})

describe('ListFilesFragment (viewFile data types)', () => {
  let isValidJson, isValidFormData, isValidMqttData

  beforeEach(async () => {
    setActivePinia(createPinia())
    vi.clearAllMocks()
    const mods = await import('@mp-se/espframework-ui-components')
    isValidJson = mods.isValidJson
    isValidFormData = mods.isValidFormData
    isValidMqttData = mods.isValidMqttData
  })

  const mountView = () =>
    mount(ListFilesFragment, {
      props: { type: 'fs' },
      global: { stubs: { BsProgress: true } }
    })

  it('viewFile sets dataType to formdata when isValidFormData returns true', async () => {
    isValidJson.mockReturnValueOnce(false)
    isValidFormData.mockReturnValueOnce(true)
    http.filesystemRequest.mockResolvedValue({ success: true, text: 'key=value&foo=bar' })
    const wrapper = mountView()
    await wrapper.vm.viewFile('/data.form')
    expect(wrapper.vm.dataType).toBe('formdata')
    expect(wrapper.vm.fileData).toContain('key')
  })

  it('viewFile sets dataType to mqtt when isValidMqttData returns true', async () => {
    isValidJson.mockReturnValueOnce(false)
    isValidFormData.mockReturnValueOnce(false)
    isValidMqttData.mockReturnValueOnce(true)
    http.filesystemRequest.mockResolvedValue({ success: true, text: 'topic:payload' })
    const wrapper = mountView()
    await wrapper.vm.viewFile('/data.mqtt')
    expect(wrapper.vm.dataType).toBe('mqtt')
    expect(wrapper.vm.fileData).toContain('topic')
  })

  it('viewFile sets dataType to csv when content is valid CSV', async () => {
    isValidJson.mockReturnValueOnce(false)
    isValidFormData.mockReturnValueOnce(false)
    isValidMqttData.mockReturnValueOnce(false)
    http.filesystemRequest.mockResolvedValue({ success: true, text: 'a,b,c\n1,2,3' })
    const wrapper = mountView()
    await wrapper.vm.viewFile('/data.csv')
    expect(wrapper.vm.dataType).toBe('csv')
  })

  it('viewFile sets dataType to binary when content is binary', async () => {
    isValidJson.mockReturnValueOnce(false)
    isValidFormData.mockReturnValueOnce(false)
    isValidMqttData.mockReturnValueOnce(false)
    http.filesystemRequest.mockResolvedValue({ success: true, text: 'data\x01\x02' })
    const wrapper = mountView()
    await wrapper.vm.viewFile('/data.bin')
    expect(wrapper.vm.dataType).toBe('binary')
  })

  it('viewFile sets dataType to text for plain text content', async () => {
    isValidJson.mockReturnValueOnce(false)
    isValidFormData.mockReturnValueOnce(false)
    isValidMqttData.mockReturnValueOnce(false)
    http.filesystemRequest.mockResolvedValue({ success: true, text: 'plain text content' })
    const wrapper = mountView()
    await wrapper.vm.viewFile('/readme.txt')
    expect(wrapper.vm.dataType).toBe('text')
    expect(wrapper.vm.fileData).toBe('plain text content')
  })

  it('viewFile handles error thrown by filesystemRequest gracefully', async () => {
    http.filesystemRequest.mockRejectedValue(new Error('network error'))
    const wrapper = mountView()
    await wrapper.vm.viewFile('/bad.json')
    expect(wrapper.vm.fileData).toBeNull()
    expect(wrapper.vm.dataType).toBe('')
  })

  it('listFilesView handles error thrown by filesystemRequest gracefully', async () => {
    http.filesystemRequest.mockRejectedValue(new Error('dir error'))
    const wrapper = mountView()
    await wrapper.vm.listFilesView()
    expect(wrapper.vm.filesView).toEqual([])
  })
})

describe('ListFilesFragment (SD type)', () => {
  beforeEach(() => {
    setActivePinia(createPinia())
    vi.clearAllMocks()
  })

  const mountSd = () =>
    mount(ListFilesFragment, {
      props: { type: 'sd' },
      global: { stubs: { BsProgress: true } }
    })

  it('listFilesView uses postJson for SD type', async () => {
    http.postJson.mockResolvedValue({
      text: vi.fn().mockResolvedValue(JSON.stringify({ files: [{ file: '/sd/test.csv' }] }))
    })
    const wrapper = mountSd()
    await wrapper.vm.listFilesView()
    expect(http.postJson).toHaveBeenCalledWith(
      'api/sd',
      expect.objectContaining({ command: 'dir' }),
      expect.any(Object)
    )
  })

  it('listFilesView populates filesView from SD response', async () => {
    http.postJson.mockResolvedValue({
      text: vi.fn().mockResolvedValue(JSON.stringify({ files: [{ file: '/sd/log.txt' }] }))
    })
    const wrapper = mountSd()
    await wrapper.vm.listFilesView()
    expect(wrapper.vm.filesView).toContain('/sd/log.txt')
  })

  it('viewFile uses request for SD type', async () => {
    http.request.mockResolvedValue({
      text: vi.fn().mockResolvedValue('{"key":"value"}')
    })
    const wrapper = mountSd()
    await wrapper.vm.viewFile('/sd/config.json')
    expect(http.request).toHaveBeenCalledWith('sd/sd/config.json', expect.any(Object))
  })
})
