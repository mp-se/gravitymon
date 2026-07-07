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
  <div class="container">
    <p></p>
    <p class="h3">{{ t('firmware.title') }}</p>
    <hr />

    <div class="row">
      <form @submit.prevent="upload">
        <div style="col-md-12">
          <p>
            {{ t('firmware.select_info') }}
            <span class="badge bg-secondary">{{ global.platform }}</span>
            <template v-if="global.app_ver && global.app_build"
              >{{ t('firmware.version_label') }}<span class="badge bg-secondary">{{ global.app_ver }}</span> ({{
                global.app_build
              }})
            </template>
            <template v-if="global.hardware"
              >{{ t('firmware.hardware_label') }}<span class="badge bg-secondary">{{ global.hardware }}</span></template
            ><template v-if="global.firmware_file"
              >{{ t('firmware.filename_label') }}
              <span class="badge bg-secondary">{{ global.firmware_file }}</span></template
            >
          </p>
        </div>

        <div class="col-md-12">
          <BsFileUpload
            name="upload"
            id="upload"
            :label="t('firmware.select_file_label')"
            accept=".bin"
            :help="t('firmware.select_file_help')"
            :disabled="global.disabled"
            @change="onFileChange"
          >
          </BsFileUpload>
        </div>

        <div class="col-md-3">
          <p></p>
          <button
            type="submit"
            class="btn btn-primary"
            id="upload-btn"
            value="upload"
            data-bs-toggle="tooltip"
            :title="
              !hasFileSelected
                ? t('firmware.select_first_title')
                : t('firmware.update_title')
            "
            :disabled="global.disabled || !hasFileSelected"
          >
            <span
              class="spinner-border spinner-border-sm"
              role="status"
              aria-hidden="true"
              v-show="global.disabled"
            ></span>
            &nbsp;{{ t('firmware.flash_button') }}
          </button>
        </div>

        <div v-if="progress > 0" class="col-md-12">
          <p></p>
          <BsProgress :progress="progress"></BsProgress>
        </div>
      </form>
    </div>
  </div>
</template>

<script setup>
import { ref } from 'vue'
import { useI18n } from 'vue-i18n'
import { global } from '@/modules/pinia'
import { sharedHttpClient as http } from '@mp-se/espframework-ui-components'
import { logDebug, logError } from '@mp-se/espframework-ui-components'

const { t } = useI18n()
const progress = ref(0)
const hasFileSelected = ref(false)

const onFileChange = (event) => {
  const fileElement = event.target
  hasFileSelected.value = fileElement.files && fileElement.files.length > 0
}

async function upload() {
  const fileElement = document.getElementById('upload')

  if (fileElement.files.length === 0) {
    global.messageError = t('firmware.err_no_file')
  } else {
    global.disabled = true
    logDebug('FirmwareView.upload()', 'Selected file: ' + fileElement.files[0].name)

    progress.value = 0

    try {
      const res = await http.uploadFile('api/firmware', fileElement.files[0], {
        timeoutMs: 180000,
        onProgress: (percent) => {
          logDebug('FirmwareView.upload()', 'Upload progress: ' + Math.round(percent) + '%')
          progress.value = Math.round(percent)
        }
      })
      progress.value = 100
      if (res.success) {
        global.messageSuccess = t('firmware.upload_success')
        global.messageError = ''

        // Use a more reliable redirect with timeout cleanup
        const redirectTimeout = setTimeout(() => {
          try {
            location.href = location.href.replace('/other/firmware', '')
          } catch (error) {
            logError('FirmwareView.redirect()', error)
            // Fallback redirect
            window.location.reload()
          }
        }, 10000)

        // Clean up timeout on page unload
        window.addEventListener(
          'beforeunload',
          () => {
            clearTimeout(redirectTimeout)
          },
          { once: true }
        )
      } else {
        global.messageError = t('firmware.upload_failed', { status: res.status })
      }
    } catch (err) {
      global.messageError = t('firmware.upload_error', { error: err.message || err })
    } finally {
      global.disabled = false
    }
  }
}
</script>
