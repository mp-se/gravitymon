// Inline version helper — replaces semver since we only need major.minor comparison.
function parseMajorMinor(ver) {
  if (!ver) return null
  const m = String(ver).match(/^[^\d]*(\d+)\.(\d+)/)
  if (!m) return null
  return { major: parseInt(m[1], 10), minor: parseInt(m[2], 10) }
}

// Public hosting base URL (constant)
const languageURL = 'https://www.gravitymon.com'

export async function fetchManifest(product = 'gravitymon', version) {
  if (!version) throw new Error('fetchManifest requires a version string')
  const url = new URL(`/language/${encodeURIComponent(product)}/${encodeURIComponent(version)}/lang_manifest.json`, languageURL).href
  const res = await fetch(url, { cache: 'no-cache' })
  if (!res.ok) throw new Error(`Manifest fetch failed: ${res.status}`)
  const manifest = await res.json()

  // Resolve pack URLs from filename relative to the manifest's hosting base
  const manifestBase = new URL(`/language/${encodeURIComponent(product)}/${encodeURIComponent(version)}/`, languageURL).href
  for (const p of manifest.packs || []) {
    if (p.filename) p.url = new URL(p.filename, manifestBase).href
  }

  return manifest
}

function packMatchesFirmware(manifest, firmwareVersion) {
  // Compatibility is determined by the manifest header version, not per-pack fields.
  const fw = parseMajorMinor(firmwareVersion)
  const c = parseMajorMinor(manifest.version)
  if (!fw || !c) return true
  return fw.major === c.major && fw.minor === c.minor
}

function pickBestPack(packs) {
  // All packs in a manifest share the same version; just return first.
  return packs[0]
}

export async function findCompatiblePack(manifest, firmwareVersion, lang) {
  if (!packMatchesFirmware(manifest, firmwareVersion)) return null
  const pack = (manifest.packs || []).find((p) => p.lang === lang)
  return pack || null
}

export function getCompatibilityMessage(manifest, firmwareVersion, lang) {
  const fw = parseMajorMinor(firmwareVersion)
  if (!fw) return `Invalid firmware version: ${firmwareVersion}`

  const hasPack = (manifest.packs || []).some((p) => p.lang === lang)
  if (!hasPack) return `No language packs available for ${lang}`

  if (packMatchesFirmware(manifest, firmwareVersion)) return null

  const c = parseMajorMinor(manifest.version)
  const target = c ? `${c.major}.${c.minor}` : manifest.version
  return `Language pack requires firmware ${target} but device is on ${fw.major}.${fw.minor}. Please download a matching pack from settings.`
}

async function decompressGzipToString(arrayBuffer) {
  const ds = new DecompressionStream('gzip')
  return new Response(new Response(arrayBuffer).body.pipeThrough(ds)).text()
}

export async function downloadAndVerifyPack(entry) {
  // entry.url is expected to point to a .gz compressed JSON file
  const res = await fetch(entry.url, { cache: 'no-cache' })
  if (!res.ok) throw new Error(`Pack download failed: ${res.status}`)
  const buffer = await res.arrayBuffer()
  const jsonText = await decompressGzipToString(buffer)
  const pack = JSON.parse(jsonText)

  // Basic sanity checks
  if (pack.product && pack.product !== entry.product && pack.product !== undefined) {
    // not necessarily fatal, but warn
    console.warn('Downloaded pack product mismatch', pack.product, entry.product)
  }
  if (pack.pack_version && entry.pack_version && pack.pack_version !== entry.pack_version) {
    console.warn('Pack version differs between manifest and pack file', entry.pack_version, pack.pack_version)
  }

  return pack
}

export default {
  fetchManifest,
  findCompatiblePack,
  downloadAndVerifyPack
}
