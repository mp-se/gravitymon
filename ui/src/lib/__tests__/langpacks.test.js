import { gzipSync } from 'zlib'
import { afterEach, beforeEach, describe, expect, it, vi } from 'vitest'
import {
  downloadAndVerifyPack,
  fetchManifest,
  findCompatiblePack,
  getCompatibilityMessage
} from '@/lib/langpacks'

describe('hosted language packs', () => {
  beforeEach(() => {
    vi.stubGlobal('fetch', vi.fn())
  })

  afterEach(() => {
    vi.unstubAllGlobals()
    vi.restoreAllMocks()
  })

  it('requires a version before fetching a manifest', async () => {
    await expect(fetchManifest()).rejects.toThrow('requires a version string')
    expect(fetch).not.toHaveBeenCalled()
  })

  it('fetches a manifest and resolves each pack URL', async () => {
    fetch.mockResolvedValue({
      ok: true,
      json: async () => ({
        version: '2.6.0',
        packs: [{ lang: 'de', filename: 'lang_de.json.gz' }, { lang: 'sv' }]
      })
    })

    const manifest = await fetchManifest('gravity mon', '2.6.0')

    expect(fetch).toHaveBeenCalledWith(
      'https://www.gravitymon.com/language/gravity%20mon/2.6.0/lang_manifest.json',
      { cache: 'no-cache' }
    )
    expect(manifest.packs[0].url).toBe(
      'https://www.gravitymon.com/language/gravity%20mon/2.6.0/lang_de.json.gz'
    )
    expect(manifest.packs[1].url).toBeUndefined()
  })

  it('surfaces failed manifest requests', async () => {
    fetch.mockResolvedValue({ ok: false, status: 503 })
    await expect(fetchManifest('gravitymon', '2.6.0')).rejects.toThrow('Manifest fetch failed: 503')
  })

  it('finds only packs compatible with the firmware major and minor version', async () => {
    const manifest = { version: '2.6.4', packs: [{ lang: 'de' }, { lang: 'sv' }] }

    await expect(findCompatiblePack(manifest, 'v2.6.1', 'sv')).resolves.toEqual({ lang: 'sv' })
    await expect(findCompatiblePack(manifest, '2.7.0', 'sv')).resolves.toBeNull()
    await expect(findCompatiblePack(manifest, '2.6.0', 'fr')).resolves.toBeNull()
    await expect(findCompatiblePack({ version: 'not-a-version', packs: [{ lang: 'sv' }] }, '2.6.0', 'sv')).resolves.toEqual({ lang: 'sv' })
  })

  it('explains invalid, unavailable, and incompatible pack selections', () => {
    const manifest = { version: '2.7.0', packs: [{ lang: 'de' }] }

    expect(getCompatibilityMessage(manifest, 'unknown', 'de')).toBe('Invalid firmware version: unknown')
    expect(getCompatibilityMessage(manifest, '2.7.1', 'sv')).toBe('No language packs available for sv')
    expect(getCompatibilityMessage(manifest, '2.7.1', 'de')).toBeNull()
    expect(getCompatibilityMessage(manifest, '2.6.1', 'de')).toContain('requires firmware 2.7')
  })

  it('decompresses downloaded packs and warns about non-fatal metadata differences', async () => {
    const warning = vi.spyOn(console, 'warn').mockImplementation(() => {})
    const bytes = gzipSync(JSON.stringify({ product: 'other', pack_version: '3', greeting: 'Hej' }))
    fetch.mockResolvedValue({ ok: true, arrayBuffer: async () => bytes.buffer.slice(bytes.byteOffset, bytes.byteOffset + bytes.byteLength) })

    await expect(downloadAndVerifyPack({ url: 'https://example.test/sv.gz', product: 'gravitymon', pack_version: '2' }))
      .resolves.toEqual({ product: 'other', pack_version: '3', greeting: 'Hej' })
    expect(warning).toHaveBeenCalledTimes(2)
  })

  it('surfaces failed pack downloads', async () => {
    fetch.mockResolvedValue({ ok: false, status: 404 })
    await expect(downloadAndVerifyPack({ url: 'https://example.test/missing.gz' })).rejects.toThrow('Pack download failed: 404')
  })
})
