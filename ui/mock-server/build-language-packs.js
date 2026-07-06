/*
 * Dev helper: builds gzip language pack files + a version.json manifest into
 * mock-server/firmware/ so the Device Settings "available languages" toggle
 * list (fetched from <ota_url>/version.json) can be tested locally against
 * `npm run mock`. Mirrors what script/create_versionjson.py does for real
 * firmware builds. Run with: node mock-server/build-language-packs.js
 */
import { createRequire } from 'module'
import { readdirSync, readFileSync, writeFileSync, mkdirSync } from 'fs'
import { gzipSync } from 'zlib'
import path from 'path'
import { fileURLToPath } from 'url'

const require = createRequire(import.meta.url)
const __dirname = path.dirname(fileURLToPath(import.meta.url))

const LANGUAGE_NAMES = {
  sv: 'Svenska',
  de: 'Deutsch',
  fr: 'Français',
  es: 'Español',
  zh: '中文',
  pl: 'Polski',
  no: 'Norsk',
  da: 'Dansk',
  nl: 'Nederlands',
  pt: 'Português',
  it: 'Italiano'
}

const localesDir = path.join(__dirname, '..', 'src', 'locales')
const outDir = path.join(__dirname, 'firmware')
mkdirSync(outDir, { recursive: true })

const languages = []
for (const file of readdirSync(localesDir)) {
  const code = file.replace(/\.json$/, '')
  if (!(code in LANGUAGE_NAMES)) continue

  const json = readFileSync(path.join(localesDir, file))
  const gz = gzipSync(json)
  const packFile = `lang_${code}.json.gz`
  writeFileSync(path.join(outDir, packFile), gz)
  languages.push({ code, name: LANGUAGE_NAMES[code], file: packFile })
  console.log(`Built ${packFile} (${json.length} -> ${gz.length} bytes)`)
}

const versionJson = { project: 'gravmon', version: '2.5.0', html: [], languages }
writeFileSync(path.join(outDir, 'version.json'), JSON.stringify(versionJson))
console.log(`Wrote version.json with ${languages.length} languages`)
