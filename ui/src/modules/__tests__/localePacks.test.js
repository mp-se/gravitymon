import { describe, it, expect, vi, beforeEach, afterEach } from 'vitest'
import { Blob, File } from 'buffer'
import { gzipSync } from 'zlib'


const filesystemRequestMock = vi.fn()
const uploadFileMock = vi.fn()
const setLocaleMessageMock = vi.fn()
const availableLocalesMock = []

vi.mock('@mp-se/espframework-ui-components', () => ({
  sharedHttpClient: {
    request: vi.fn(),
    filesystemRequest: (...args) => filesystemRequestMock(...args),
    uploadFile: (...args) => uploadFileMock(...args)
  },
  logInfo: vi.fn(),
  logError: vi.fn()
}))

vi.mock('@/modules/i18n', () => ({
  default: {
    global: {
      get availableLocales() {
        return availableLocalesMock
      },
      setLocaleMessage: (...args) => setLocaleMessageMock(...args)
    }
  }
}))

import { sharedHttpClient as http } from '@mp-se/espframework-ui-components'
import {
  loadLocalePack,
  loadLocalePackWithRetry,
  installPackFromUrl,
  removeLocalePack,
  listInstalledPacks
} from '@/modules/localePacks'

function gzipBlobOf(text) {
  // jsdom's global Blob doesn't implement .stream(), unlike real browsers
  // (and unlike loadLocalePack's actual runtime environment), so use Node's
  // native Blob here to exercise the same gunzip() code path faithfully.
  return new Blob([gzipSync(Buffer.from(text))])
}

function plainBlobOf(text) {
  return new Blob([Buffer.from(text)])
}


describe('localePacks', () => {
  beforeEach(() => {
    vi.clearAllMocks()
    availableLocalesMock.length = 0
    // jsdom's global Blob/File don't implement .stream()/.arrayBuffer()
    // reliably (particularly when a Blob part crosses between jsdom's and
    // Node's implementations), unlike real browsers - use Node's native File
    // so installPackFromUrl's new File(...) call behaves like it does at
    // runtime.
    vi.stubGlobal('File', File)
  })

  afterEach(() => {
    vi.unstubAllGlobals()
  })

  describe('loadLocalePack', () => {
    it('returns true immediately for en without any request', async () => {
      const result = await loadLocalePack('en')
      expect(result).toBe(true)
      expect(http.request).not.toHaveBeenCalled()
    })

    it('returns true without re-fetching if already loaded', async () => {
      availableLocalesMock.push('de')
      const result = await loadLocalePack('de')
      expect(result).toBe(true)
      expect(http.request).not.toHaveBeenCalled()
    })

    it('fetches, decompresses and registers messages on success', async () => {
      http.request.mockResolvedValue({
        ok: true,
        blob: async () => gzipBlobOf(JSON.stringify({ hello: 'world' }))
      })

      const result = await loadLocalePack('de')

      expect(result).toBe(true)
      expect(http.request).toHaveBeenCalledWith(
        'api/language?file=%2Flang_de.json.gz',
        expect.objectContaining({ method: 'GET' })
      )
      expect(setLocaleMessageMock).toHaveBeenCalledWith('de', { hello: 'world' })
    })

    it('returns false when the pack file is missing on device', async () => {
      http.request.mockResolvedValue({ ok: false })
      const result = await loadLocalePack('de')
      expect(result).toBe(false)
      expect(setLocaleMessageMock).not.toHaveBeenCalled()
    })

    it('returns false when the pack contents fail to decompress/parse', async () => {
      http.request.mockResolvedValue({
        ok: true,
        blob: async () => new Blob(['not actually gzip data'])
      })
      const result = await loadLocalePack('de')
      expect(result).toBe(false)
      expect(setLocaleMessageMock).not.toHaveBeenCalled()
    })
  })

  describe('loadLocalePackWithRetry', () => {
    it('succeeds on first attempt without retrying', async () => {
      http.request.mockResolvedValue({
        ok: true,
        blob: async () => gzipBlobOf(JSON.stringify({ hello: 'world' }))
      })

      const result = await loadLocalePackWithRetry('de', { delayMs: 1 })

      expect(result).toBe(true)
      expect(http.request).toHaveBeenCalledTimes(1)
    })

    it('rides out a get-right-after-upload race by retrying', async () => {
      // Simulates the device's HTTP response landing slightly before the
      // LittleFS write is actually flushed: the first get fails (network
      // error, matching what a not-yet-visible file produces), then a
      // subsequent retry succeeds once the write has settled.
      http.request
        .mockRejectedValueOnce(new TypeError('Failed to fetch'))
        .mockResolvedValueOnce({
          ok: true,
          blob: async () => gzipBlobOf(JSON.stringify({ hello: 'world' }))
        })

      const result = await loadLocalePackWithRetry('de', { delayMs: 1 })

      expect(result).toBe(true)
      expect(http.request).toHaveBeenCalledTimes(2)
    })

    it('gives up and returns false after exhausting retries', async () => {
      http.request.mockRejectedValue(new TypeError('Failed to fetch'))

      const result = await loadLocalePackWithRetry('de', { retries: 2, delayMs: 1 })

      expect(result).toBe(false)
      expect(http.request).toHaveBeenCalledTimes(2)
    })
  })

  describe('installPackFromUrl', () => {
    const entry = { code: 'de', name: 'Deutsch', file: 'lang_de.json.gz' }

    it('uploads the downloaded blob as-is when it is already gzip', async () => {
      const gz = gzipBlobOf(JSON.stringify({ hello: 'world' }))
      vi.stubGlobal(
        'fetch',
        vi.fn(async () => ({ ok: true, blob: async () => gz }))
      )
      uploadFileMock.mockResolvedValue({ success: true, status: 200 })

      await installPackFromUrl('https://example.com/', entry)

      // Already-gzip input should pass through unmodified (no recompression).
      const uploadedFile = uploadFileMock.mock.calls[0][1]
      expect(uploadedFile.size).toBe(gz.size)
    })

    it('re-compresses the blob when the host already transparently decompressed it', async () => {
      // Some static hosts set Content-Encoding: gzip for .gz files when the
      // browser requests compression, so fetch() hands back the original
      // plain content instead of the gzip bytes we asked for.
      const plain = plainBlobOf(JSON.stringify({ hello: 'world' }))
      vi.stubGlobal(
        'fetch',
        vi.fn(async () => ({ ok: true, blob: async () => plain }))
      )
      uploadFileMock.mockResolvedValue({ success: true, status: 200 })

      await installPackFromUrl('https://example.com/', entry)

      // Recompressed output should be smaller than the plain input (proving
      // it actually went through CompressionStream, not just passed along).
      const uploadedFile = uploadFileMock.mock.calls[0][1]
      expect(uploadedFile.size).toBeLessThan(plain.size)
    })

    it('throws when the download itself fails', async () => {
      vi.stubGlobal('fetch', vi.fn(async () => ({ ok: false, status: 404 })))
      await expect(installPackFromUrl('https://example.com/', entry)).rejects.toThrow('HTTP 404')
      expect(uploadFileMock).not.toHaveBeenCalled()
    })

    it('throws when the upload fails, even though fetch resolved', async () => {
      const gz = gzipBlobOf(JSON.stringify({ hello: 'world' }))
      vi.stubGlobal(
        'fetch',
        vi.fn(async () => ({ ok: true, blob: async () => gz }))
      )
      uploadFileMock.mockResolvedValue({ success: false, status: 500 })

      await expect(installPackFromUrl('https://example.com/', entry)).rejects.toThrow(
        'Upload failed'
      )
    })
  })

  describe('removeLocalePack', () => {
    it('deletes the pack file by naming convention', async () => {
      filesystemRequestMock.mockResolvedValue({ success: true })
      const result = await removeLocalePack('sv')
      expect(filesystemRequestMock).toHaveBeenCalledWith({
        command: 'del',
        file: '/lang_sv.json.gz'
      })
      expect(result).toEqual({ success: true })
    })
  })

  describe('listInstalledPacks', () => {
    it('filters dir listing down to lang_*.json.gz files and extracts codes', async () => {
      filesystemRequestMock.mockResolvedValue({
        success: true,
        text: JSON.stringify({
          files: [
            { file: '/config.json' },
            { file: '/lang_de.json.gz' },
            { file: '/lang_sv.json.gz' },
            { file: '/error.log' }
          ]
        })
      })

      const result = await listInstalledPacks()
      expect(result).toEqual(['de', 'sv'])
    })

    it('returns an empty array when the request fails', async () => {
      filesystemRequestMock.mockResolvedValue({ success: false })
      const result = await listInstalledPacks()
      expect(result).toEqual([])
    })
  })
})
