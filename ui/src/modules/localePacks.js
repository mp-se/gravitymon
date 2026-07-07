/*
 * GravityMon
 * Copyright (c) 2021-2026 Magnus
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Alternatively, this software may be used under the terms of a
 * commercial license. See LICENSE_COMMERCIAL for details.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
import i18n from '@/modules/i18n'
import { sharedHttpClient as http, logError, logInfo } from '@mp-se/espframework-ui-components'

// Language packs are stored gzip-compressed both when hosted for download and
// on the device's LittleFS, as /lang_<code>.json.gz. Decompression happens
// entirely client-side via the browser's native DecompressionStream, so no
// gzip library is added to the bundle and no firmware changes are needed to
// serve/store them as raw bytes. gzip's CRC32 also means a corrupted file
// simply fails to decompress instead of silently loading garbage.
function packFileName(code) {
  return `lang_${code}.json.gz`
}

async function gunzip(blob) {
  const stream = blob.stream().pipeThrough(new DecompressionStream('gzip'))
  return await new Response(stream).text()
}

const GZIP_MAGIC = [0x1f, 0x8b]

async function isGzip(blob) {
  const head = new Uint8Array(await blob.slice(0, 2).arrayBuffer())
  return head[0] === GZIP_MAGIC[0] && head[1] === GZIP_MAGIC[1]
}

// Some static hosts negotiate Content-Encoding: gzip for .gz files when the
// browser sends Accept-Encoding: gzip, which makes fetch() transparently
// decompress the body - we'd then get the original plain JSON instead of the
// gzip bytes we asked for (curl doesn't hit this since it doesn't request
// compression by default, which is why this only showed up against real
// hosting, not in manual curl checks). Re-compress in that case so the
// device always receives real gzip bytes regardless of what the host did.
async function ensureGzipBlob(blob) {
  if (await isGzip(blob)) return blob
  const stream = blob.stream().pipeThrough(new CompressionStream('gzip'))
  return await new Response(stream).blob()
}

// Fetches raw bytes for a device filesystem file via a dedicated GET download
// route (not the generic POST /api/filesystem 'get' command) - a plain GET
// with the response consumed as a blob is the reliable way to pull binary
// gzip content off the device.
async function fetchFileBlob(file) {
  const res = await http.request(`api/language?file=${encodeURIComponent(file)}`, {
    method: 'GET'
  })
  if (!res || !res.ok) return null
  return res.blob()
}

export async function loadLocalePack(code) {
  if (code === 'en' || i18n.global.availableLocales.includes(code)) return true

  try {
    const blob = await fetchFileBlob(`/${packFileName(code)}`)
    if (!blob) return false
    const text = await gunzip(blob)
    i18n.global.setLocaleMessage(code, JSON.parse(text))
    logInfo('localePacks.loadLocalePack()', `Loaded language pack for ${code}`)
    return true
  } catch (error) {
    logError('localePacks.loadLocalePack()', error)
    return false
  }
}

// After an upload, the device's HTTP response can land slightly before the
// LittleFS write is actually flushed/visible, so an immediate get/dir right
// after can race and fail (missing file or a network-level fetch failure).
// Retry a few times with a short delay to ride out that window.
export async function loadLocalePackWithRetry(code, { retries = 3, delayMs = 400 } = {}) {
  for (let attempt = 0; attempt < retries; attempt++) {
    if (attempt > 0) await new Promise((resolve) => setTimeout(resolve, delayMs))
    if (await loadLocalePack(code)) return true
  }
  return false
}

export async function installPackFromUrl(baseUrl, entry, opts = {}) {
  // entry: { code, name, file } from version.json's "languages" array
  const res = await fetch(baseUrl + entry.file)
  if (!res.ok) throw new Error(`HTTP ${res.status}`)
  const rawBlob = await res.blob()
  const blob = await ensureGzipBlob(rawBlob)
  const file = new File([blob], entry.file, { type: 'application/gzip' })
  // uploadFile() never rejects - it always resolves with { success, status, text }
  // - so a failed upload must be checked explicitly, or callers would silently
  // carry on as if the pack were installed.
  const uploadResult = await http.uploadFile('api/filesystem/upload', file, opts)
  if (!uploadResult || !uploadResult.success) {
    throw new Error(`Upload failed: HTTP ${uploadResult ? uploadResult.status : 'unknown'}`)
  }
  return uploadResult
}

export async function removeLocalePack(code) {
  return http.filesystemRequest({ command: 'del', file: `/${packFileName(code)}` })
}

export async function listInstalledPacks() {
  const res = await http.filesystemRequest({ command: 'dir' })
  if (!res.success) return []
  try {
    const json = JSON.parse(res.text)
    return json.files
      .map((f) => f.file)
      .filter((f) => /^\/?lang_.*\.json\.gz$/.test(f))
      .map((f) => f.replace(/^\/?lang_/, '').replace(/\.json\.gz$/, ''))
  } catch (error) {
    logError('localePacks.listInstalledPacks()', error)
    return []
  }
}
