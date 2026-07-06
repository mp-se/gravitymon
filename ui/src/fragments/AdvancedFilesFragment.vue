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
  <!-- Upload section - only shown for filesystem type -->
  <div v-if="props.type === 'fs'">
    <h5>{{ uploadTitle }}</h5>
    <div class="row gy-4">
      <form @submit.prevent="upload">
        <div class="col-md-12">
          <BsFileUpload
            name="upload"
            id="upload"
            label="Select firmware file"
            accept=""
            help="Choose a file to upload to the file system"
            :disabled="global.disabled"
            @change="onFileChange"
          >
          </BsFileUpload>
        </div>
        <div class="col-md-3">
          <p></p>
          <button
            type="submit"
            class="btn btn-secondary"
            id="upload-btn"
            value="upload"
            data-bs-toggle="tooltip"
            title="Update the device with the selected firmware"
            :disabled="global.disabled || !hasFileSelected"
          >
            <span
              class="spinner-border spinner-border-sm"
              role="status"
              aria-hidden="true"
              v-show="global.disabled"
            ></span>
            &nbsp;Upload file
          </button>
        </div>
        <div v-if="progress > 0" class="col-md-12">
          <p></p>
          <BsProgress :progress="progress"></BsProgress>
        </div>
      </form>
    </div>

    <div class="row gy-4">
      <p></p>
      <hr />
    </div>
  </div>

  <h5>{{ deleteTitle }}</h5>
  <div class="row gy-4">
    <div class="col-md-3">
      <button
        @click="listFilesDelete"
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
        &nbsp;{{ listButtonText }}
      </button>
    </div>
    <div class="col-md-6">
      <div class="button-group">
        <template v-for="(f, index) in filesDelete" :key="index">
          <button
            type="button"
            @click.prevent="deleteFile(f)"
            class="btn btn-outline-primary"
            href="#"
            :disabled="global.disabled"
          >
            {{ f }}</button
          >&nbsp;
        </template>
      </div>
    </div>

    <BsModalConfirm
      :callback="confirmDeleteCallback"
      :message="confirmDeleteMessage"
      :id="modalId"
      title="Delete file"
      :disabled="global.disabled"
    />
  </div>
</template>

<script setup>
import { ref, computed } from 'vue'
import { global } from '@/modules/pinia'
import { sharedHttpClient as http } from '@mp-se/espframework-ui-components'
import { logDebug, logError } from '@mp-se/espframework-ui-components'

const props = defineProps({
  type: {
    type: String,
    default: 'fs', // 'fs' for filesystem, 'sd' for secure disk
    validator: (value) => ['fs', 'sd'].includes(value)
  }
})

// Timeout constant for HTTP requests (20 seconds)
const HTTP_TIMEOUT_MS = 20000

const fileData = ref(null)
const filesDelete = ref([])
const hasFileSelected = ref(false)

const confirmDeleteMessage = ref(null)
const confirmDeleteFile = ref(null)

// Computed properties for dynamic content based on type
const uploadTitle = computed(() => {
  return 'Upload files to file system'
})

const deleteTitle = computed(() => {
  return props.type === 'sd' ? 'Delete files from SD file system' : 'Delete files from file system'
})

const listButtonText = computed(() => {
  return props.type === 'sd' ? 'Delete SD files' : 'List files'
})

const modalId = computed(() => {
  return props.type === 'sd' ? 'deleteSecureDiskFile' : 'deleteFile'
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

const confirmDeleteCallback = async (result) => {
  const logPrefix = props.type === 'sd' ? 'AdvancedSecureDiskFragment' : 'AdvancedFilesFragment'
  logDebug(`${logPrefix}.confirmDeleteCallback()`, result)

  if (result) {
    global.disabled = true
    global.clearMessages()

    fileData.value = null

    const data = {
      command: 'del',
      file: confirmDeleteFile.value
    }

    if (props.type === 'sd') {
      const res = await sendSecureDiskRequest(data)
      logDebug(`${logPrefix}.confirmDeleteCallback()`, res)
    } else {
      await http.filesystemRequest(data)
    }

    filesDelete.value = []
    global.disabled = false
  }
}

const deleteFile = (f) => {
  confirmDeleteMessage.value = 'Do you really want to delete file ' + f
  confirmDeleteFile.value = f
  document.getElementById(modalId.value).click()
}

const listFilesDelete = async () => {
  global.disabled = true
  global.clearMessages()

  filesDelete.value = []

  const data = { command: 'dir' }

  if (props.type === 'sd') {
    const res = await sendSecureDiskRequest(data)
    if (res && res.success) {
      const json = JSON.parse(res.text)
      for (const f of json.files) {
        filesDelete.value.push(f.file)
      }
    }
  } else {
    const res = await http.filesystemRequest(data)
    if (res && res.success) {
      const json = JSON.parse(res.text)
      for (const f in json.files) {
        filesDelete.value.push(json.files[f].file)
      }
    }
  }

  global.disabled = false
}

const progress = ref(0)

const onFileChange = (event) => {
  hasFileSelected.value = event.target.files.length > 0
}

async function upload() {
  const fileElement = document.getElementById('upload')

  if (fileElement.files.length === 0) {
    global.messageError = 'You need to select one file with firmware to upload'
    return
  }

  global.disabled = true
  logDebug('AdvancedFilesFragment.upload()', 'Selected file: ' + fileElement.files[0].name)

  progress.value = 0

  try {
    const res = await http.uploadFile('api/filesystem/upload', fileElement.files[0], {
      timeoutMs: 40000,
      onProgress: (percent) => {
        progress.value = Math.round(percent)
      }
    })

    progress.value = 100
    if (res && res.success) {
      global.messageSuccess = 'File upload completed!'
      global.messageError = ''
    } else {
      global.messageError = `File upload failed: ${res && res.status}`
    }
  } catch (err) {
    logError('AdvancedFilesFragment.upload()', err)
    global.messageError = 'File upload failed!'
  } finally {
    global.disabled = false
    filesDelete.value = []
  }
}
</script>
