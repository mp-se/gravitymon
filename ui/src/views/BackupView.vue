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
    <p class="h3">Backup & Restore</p>
    <hr />

    <div class="row">
      <div class="col-md-12">
        <p>Create a backup of the device configuration and store this in a textfile</p>
      </div>

      <div class="col-md-12">
        <button
          @click="backup"
          type="button"
          class="btn btn-primary w-2"
          data-bs-toggle="tooltip"
          :disabled="global.disabled"
        >
          Create backup
        </button>
      </div>

      <div class="col-md-12">
        <hr />
      </div>

      <div class="col-md-12">
        <p>Restore a previous backup of the device configuration by uploading it.</p>
      </div>
    </div>

    <div class="row">
      <form @submit.prevent="restore">
        <div class="col-md-12">
          <BsFileUpload
            name="upload"
            id="upload"
            label="Select backup file"
            accept=".txt"
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
            value="upload"
            data-bs-toggle="tooltip"
            title="Upload the configuration to the device"
            :disabled="global.disabled || !fileSelected"
          >
            <span
              class="spinner-border spinner-border-sm"
              role="status"
              aria-hidden="true"
              v-show="global.disabled"
            ></span>
            &nbsp;Restore
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
import { global, config, getConfigChanges } from '@/modules/pinia'
import { logDebug, logError } from '@mp-se/espframework-ui-components'

const progress = ref(0)
const fileSelected = ref(false)

function onFileChange(event) {
  const files = event.target.files
  fileSelected.value = files && files.length > 0
}

function backup() {
  let backup = {
    meta: {
      version: '2.2.0',
      software: 'GravityMon',
      created: new Date().toISOString().slice(0, 10)
    },
    config: JSON.parse(config.toJson())
  }

  logDebug('BackupView.backup()', backup)

  backup.config.http_post_format_gravity = encodeURIComponent(
    backup.config.http_post_format_gravity
  )
  backup.config.http_post2_format_gravity = encodeURIComponent(
    backup.config.http_post2_format_gravity
  )
  backup.config.http_get_format_gravity = encodeURIComponent(backup.config.http_get_format_gravity)
  backup.config.influxdb2_format_gravity = encodeURIComponent(
    backup.config.influxdb2_format_gravity
  )
  backup.config.mqtt_format_gravity = encodeURIComponent(backup.config.mqtt_format_gravity)

  const s = JSON.stringify(backup, null, 2)
  const name = config.mdns + '.txt'
  download(s, 'text/plain', name)
  global.messageSuccess = 'Backup file created and downloaded as: ' + name
}

async function restore() {
  const fileElement = document.getElementById('upload')

  // Validate file element exists
  if (!fileElement) {
    global.messageError = 'Upload element not found'
    return
  }

  if (fileElement.files.length === 0) {
    global.messageError = 'You need to select one file to restore configuration from'
    return
  }

  global.disabled = true
  logDebug('BackupView.restore()', 'Selected file: ' + fileElement.files[0].name)

  const reader = new FileReader()
  const file = fileElement.files[0]

  reader.addEventListener('load', async function (e) {
    let text = e.target.result
    try {
      const data = JSON.parse(text)
      if (
        data.meta.software === 'GravityMon' &&
        (data.meta.version === '2.0.0' || data.meta.version === '2.2.0')
      ) {
        await doRestore2(data.config)
      } else if (data.meta.software === 'GravityMon') {
        await doRestore1(data)
      } else {
        global.messageError = 'Unknown format, unable to process'
      }
    } catch (error) {
      logError('BackupView.restore()', error)
      global.messageError = 'Unable to parse configuration file for GravityMon.'
    } finally {
      global.disabled = false
      // Reset file selection after operation
      fileSelected.value = false
      fileElement.value = ''
    }
  })

  reader.addEventListener('error', () => {
    logError('BackupView.restore()', 'File reading failed')
    global.messageError = 'Failed to read the backup file'
    global.disabled = false
    // Reset file selection after error
    fileSelected.value = false
    fileElement.value = ''
  })

  reader.readAsText(file)
}

function download(content, mimeType, filename) {
  const a = document.createElement('a')
  const blob = new Blob([content], { type: mimeType })
  const url = URL.createObjectURL(blob)
  a.setAttribute('href', url)
  a.setAttribute('download', filename)
  a.click()

  // Clean up the object URL to prevent memory leaks
  setTimeout(() => URL.revokeObjectURL(url), 1000)
}

async function doRestore1(json) {
  /**
   * Convert the advanced
   */
  logDebug('BackupView.doRestore1()', json)

  delete json.advanced['id']

  for (const k in json.advanced) {
    let newK = k.replaceAll('-', '_')

    if (newK === 'int_http1') newK = 'http_post_int'
    if (newK === 'int_http2') newK = 'http_post2_int'
    if (newK === 'int_http3') newK = 'http_get_int'
    if (newK === 'int_influx') newK = 'influxdb2_int'
    if (newK === 'int_mqtt') newK = 'mqtt_int'

    config[newK] = json.advanced[k]
  }

  /**
   * Convert the configuration part
   */
  delete json.config['app-ver']
  delete json.config['app-build']
  delete json.config['angle']
  delete json.config['gravity']
  delete json.config['battery']
  delete json.config['runtime-average']
  delete json.config['platform']
  delete json.config['id']
  delete json.config['wifi-ssid']
  delete json.config['wifi-pass']
  delete json.config['wifi-ssid2']
  delete json.config['wifi-pass2']

  for (const k in json.config) {
    let newK = k.replaceAll('-', '_')

    if (newK === 'ble') newK = 'ble_tilt_color'

    if (newK === 'http_push') newK = 'http_post_target'
    if (newK === 'http_push_h1') newK = 'http_post_header1'
    if (newK === 'http_push_h2') newK = 'http_post_header2'

    if (newK === 'http_push2') newK = 'http_post2_target'
    if (newK === 'http_push2_h1') newK = 'http_post2_header1'
    if (newK === 'http_push2_h2') newK = 'http_post2_header2'

    if (newK === 'http_push3') newK = 'http_get_target'

    if (newK === 'influxdb2_push') newK = 'influxdb2_target'
    if (newK === 'mqtt_push') newK = 'mqtt_target'

    if (newK === 'formula_calculation_data') {
      config.formula_calculation_data = [
        {
          a: json.config['formula-calculation-data']['a1'],
          g: json.config['formula-calculation-data']['g1']
        },
        {
          a: json.config['formula-calculation-data']['a2'],
          g: json.config['formula-calculation-data']['g2']
        },
        {
          a: json.config['formula-calculation-data']['a3'],
          g: json.config['formula-calculation-data']['g3']
        },
        {
          a: json.config['formula-calculation-data']['a4'],
          g: json.config['formula-calculation-data']['g4']
        },
        {
          a: json.config['formula-calculation-data']['a5'],
          g: json.config['formula-calculation-data']['g5']
        },
        {
          a: json.config['formula-calculation-data']['a6'],
          g: json.config['formula-calculation-data']['g6']
        },
        {
          a: json.config['formula-calculation-data']['a7'],
          g: json.config['formula-calculation-data']['g7']
        },
        {
          a: json.config['formula-calculation-data']['a8'],
          g: json.config['formula-calculation-data']['g8']
        },
        {
          a: json.config['formula-calculation-data']['a9'],
          g: json.config['formula-calculation-data']['g9']
        },
        {
          a: json.config['formula-calculation-data']['a10'],
          g: json.config['formula-calculation-data']['g10']
        }
      ]
    } else {
      logDebug('BackupView.doRestore1()', k, '=>', newK)
      config[newK] = json.config[k]
    }
  }

  /**
   * Convert the format part
   */
  config.http_post_format_gravity = decodeURIComponent(json.format['http-1'])
  config.http_post2_format_gravity = decodeURIComponent(json.format['http-2'])
  config.http_get_format_gravity = decodeURIComponent(json.format['http-3'])
  config.influxdb2_format_gravity = decodeURIComponent(json.format['influxdb'])
  config.mqtt_format_gravity = decodeURIComponent(json.format['mqtt'])

  getConfigChanges()
  await config.saveAll()
}

async function doRestore2(json) {
  for (const k in json) {
    if (k.endsWith('_format')) {
      config[k] = decodeURIComponent(json[k])
    } else {
      config[k + '_gravity'] = json[k]
    }

    if (k.endsWith('_format_gravity')) {
      config[k] = decodeURIComponent(json[k])
    } else {
      config[k] = json[k]
    }
  }

  getConfigChanges()
  await config.saveAll()
}
</script>
