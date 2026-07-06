<!--
  GravityMon
  Copyright (c) 2021-2026 Magnus

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Alternatively, this software may be used under the terms of a
  commercial license. See LICENSE_COMMERCIAL for details.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
-->
<template>
  <h5>{{ title }}</h5>
  <div class="row gy-4">
    <div class="col-md-3">
      <button
        @click="listFilesView"
        type="button"
        class="btn btn-secondary"
        :disabled="global.disabled"
      >
        <span
          class="spinner-border spinner-border-sm"
          role="status"
          aria-hidden="true"
          v-show="global.disabled"
        ></span>
        &nbsp;{{ listButtonText }}</button
      >&nbsp;
    </div>

    <div class="col-md-6">
      <div class="button-group">
        <template v-for="(f, index) in filesView" :key="index">
          <button
            type="button"
            @click.prevent="viewFile(f)"
            class="btn btn-outline-primary"
            href="#"
            :disabled="global.disabled"
          >
            {{ f }}</button
          >&nbsp;
        </template>
      </div>
    </div>
  </div>

  <div v-if="filesystemUsage > 0" class="col-md-12">
    <h6>File system usage</h6>
    <BsProgress :progress="filesystemUsage"></BsProgress>
    <p>{{ filesystemUsageText }}</p>
  </div>

  <div v-if="fileData !== null" class="col-md-12">
    <h6>
      File contents{{ dataType ? ', format: ' + dataType : '' }}, size {{ fileDataSize }} bytes
    </h6>
    <pre class="border p-2" v-html="fileData"></pre>
  </div>
</template>

<script setup>
import { ref, computed } from 'vue'
import { global } from '@/modules/pinia'
import { sharedHttpClient as http } from '@mp-se/espframework-ui-components'
import { isValidJson, isValidFormData, isValidMqttData } from '@mp-se/espframework-ui-components'

const props = defineProps({
  type: {
    type: String,
    default: 'fs', // 'fs' or 'sd'
    validator: (value) => ['fs', 'sd'].includes(value)
  }
})

// Timeout constant for HTTP requests (20 seconds)
const HTTP_TIMEOUT_MS = 20000

const filesystemUsage = ref(null)
const filesystemUsageText = ref(null)
const filesView = ref([])
const fileData = ref(null)
const fileDataSize = ref(0)
const dataType = ref('')

// Computed properties for dynamic defaults based on type
const title = computed(() => {
  return props.type === 'sd' ? 'Explore the SD file system' : 'Explore the file system'
})

const listButtonText = computed(() => {
  return props.type === 'sd' ? 'List SD files' : 'List files'
})

const sendSecureDiskRequest = async (data) => {
  global.disabled = true
  try {
    // Use postJson to send JSON payload and get the raw response
    const resp = await http.postJson('api/sd', data, { timeoutMs: HTTP_TIMEOUT_MS })
    // postJson returns a Response-like object; read text
    const text = await resp.text()
    global.disabled = false
    return { success: true, text }
  } catch {
    global.disabled = false
    return { success: false, text: '' }
  }
}

const fetchSecureDiskFile = async (fileName) => {
  global.disabled = true
  try {
    const resp = await http.request('sd' + fileName, {
      method: 'GET',
      timeoutMs: HTTP_TIMEOUT_MS
    })
    const text = await resp.text()
    global.disabled = false
    return { success: true, text }
  } catch {
    global.disabled = false
    return { success: false, text: '' }
  }
}

const rainbowColors = ['#FF0000', '#FF7F00', '#006400', '#0000FF', '#4B0082', '#9400D3']

function formatFileSize(bytes) {
  const kb = bytes / 1024
  if (kb >= 1024 * 1024) {
    // GB
    return (kb / (1024 * 1024)).toFixed(1) + ' GB'
  } else if (kb >= 1024) {
    // MB
    return (kb / 1024).toFixed(1) + ' MB'
  } else {
    // KB
    return kb.toFixed(0) + ' KB'
  }
}

function colorizeCsv(text) {
  // Normalize line endings to handle both \r\n (Windows) and \r (old Mac) properly
  const normalizedText = text.replace(/\r\n/g, '\n').replace(/\r/g, '\n')
  return normalizedText
    .split('\n')
    .map((line) => {
      if (!line.trim()) return line
      return line
        .split(',')
        .map(
          (cell, index) =>
            `<span style="color: ${rainbowColors[index % rainbowColors.length]}">${cell}</span>`
        )
        .join(',')
    })
    .join('\n')
}

function colorizeFormData(text) {
  return text
    .split('&')
    .map((pair) => {
      if (!pair.trim()) return pair
      const [key, ...valueParts] = pair.split('=')
      const value = valueParts.join('=')
      return `<span style="color: blue">${key}</span>=<span style="color: green">${value}</span>`
    })
    .join('&\n')
}

function colorizeMqtt(text) {
  return text
    .split('|')
    .map((pair) => {
      if (!pair.trim()) return pair
      const [before, ...afterParts] = pair.split(':')
      const after = afterParts.join(':')
      return `<span style="color: purple">${before}</span>:<span style="color: orange">${after}</span>`
    })
    .join('|\n')
}

function colorizeJson(obj, indent = 0) {
  const indentStr = '  '.repeat(indent)
  if (typeof obj === 'string') return `<span style="color: green">"${obj}"</span>`
  if (typeof obj === 'number') return `<span style="color: blue">${obj}</span>`
  if (typeof obj === 'boolean') return `<span style="color: red">${obj}</span>`
  if (obj === null) return `<span style="color: gray">null</span>`
  if (Array.isArray(obj)) {
    if (obj.length === 0) return '[]'
    const items = obj.map((item) => `${indentStr}  ${colorizeJson(item, indent + 1)}`).join(',\n')
    return `[\n${items}\n${indentStr}]`
  }
  if (typeof obj === 'object') {
    const entries = Object.keys(obj)
    if (entries.length === 0) return '{}'
    const props = entries
      .map(
        (key) =>
          `${indentStr}  <span style="color: purple">"${key}"</span>: ${colorizeJson(obj[key], indent + 1)}`
      )
      .join(',\n')
    return `{\n${props}\n${indentStr}}`
  }
  return obj
}

function isBinary(text) {
  // Detect binary data: check for control characters or Unicode replacement char (�) from invalid UTF-8 decoding
  if (text.includes('\uFFFD')) return true
  for (let char of text) {
    const code = char.charCodeAt(0)
    if (code < 32 && code !== 9 && code !== 10 && code !== 13) return true
  }
  return false
}

function toHex(text) {
  // Convert to hex, replacing � (from invalid UTF-8) with FF for correct binary representation
  const hexes = Array.from(text).map((c) => {
    if (c === '\uFFFD') return 'FF'
    return c.charCodeAt(0).toString(16).padStart(2, '0').toUpperCase()
  })
  const asciiChars = Array.from(text).map((c) => {
    const code = c.charCodeAt(0)
    return code >= 32 && code <= 126 ? c : '.'
  })
  const hexWidth = 20 * 3 - 1 // 59 chars for full 20 bytes: XX XX ... XX
  const lines = []
  for (let i = 0; i < hexes.length; i += 20) {
    const hexLine = hexes.slice(i, i + 20).join(' ')
    const paddedHex = hexLine.padEnd(hexWidth, ' ')
    const asciiLine = asciiChars.slice(i, i + 20).join('')
    lines.push(`${paddedHex}  ${asciiLine}`)
  }
  return lines.join('\n')
}

function isValidCsvData(text) {
  // Check for CSV format: all non-empty lines must have the same number of commas, and at least one comma per line
  // Normalize line endings first to handle \r\n and \r properly
  const normalizedText = text.replace(/\r\n/g, '\n').replace(/\r/g, '\n')
  const lines = normalizedText
    .split('\n')
    .map((line) => line.trim())
    .filter((line) => line.length > 0)
  if (lines.length === 0) return false

  const commaCounts = lines.map((line) => (line.match(/,/g) || []).length)
  const firstCount = commaCounts[0]
  return commaCounts.every((count) => count === firstCount && count > 0)
}

const viewFile = async (f) => {
  global.disabled = true
  global.clearMessages()

  fileData.value = null

  try {
    let res
    if (props.type === 'sd') {
      res = await fetchSecureDiskFile(f)
    } else {
      const data = {
        command: 'get',
        file: f,
        timeoutMs: HTTP_TIMEOUT_MS
      }
      res = await http.filesystemRequest(data)
    }

    if (res && res.success) {
      const text = res.text
      fileDataSize.value = text.length // Use the raw text length for size

      if (isValidJson(text)) {
        fileData.value = colorizeJson(JSON.parse(text))
        dataType.value = 'json'
      } else if (isValidFormData(text)) {
        fileData.value = colorizeFormData(text)
        dataType.value = 'formdata'
      } else if (isValidMqttData(text)) {
        fileData.value = colorizeMqtt(text)
        dataType.value = 'mqtt'
      } else if (isValidCsvData(text)) {
        fileData.value = colorizeCsv(text)
        dataType.value = 'csv'
      } else if (isBinary(text)) {
        fileData.value = toHex(text)
        dataType.value = 'binary'
      } else {
        fileData.value = text
        dataType.value = 'text'
      }
    }
  } catch (error) {
    console.error('Error viewing file:', error)
  } finally {
    global.disabled = false
  }
}

const listFilesView = async () => {
  global.disabled = true
  global.clearMessages()

  filesView.value = []
  filesystemUsage.value = null
  filesystemUsageText.value = null

  try {
    let res
    if (props.type === 'sd') {
      const data = { command: 'dir' }
      res = await sendSecureDiskRequest(data)
    } else {
      const data = {
        command: 'dir',
        timeoutMs: HTTP_TIMEOUT_MS
      }
      res = await http.filesystemRequest(data)
    }

    if (res && res.success) {
      const json = JSON.parse(res.text)
      if (json.used && json.total) {
        filesystemUsage.value = (json.used / json.total) * 100
        filesystemUsageText.value =
          'Total space ' +
          formatFileSize(json.total) +
          ', Free space ' +
          formatFileSize(json.free) +
          ', Used space ' +
          formatFileSize(json.used)
      }

      if (json.files) {
        for (const f of json.files) {
          filesView.value.push(f.file)
        }
      }
    }
  } catch (error) {
    console.error('Error listing files:', error)
  } finally {
    global.disabled = false
  }
}
</script>
