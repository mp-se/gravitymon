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
  <div
    class="modal fade show d-block"
    tabindex="-1"
    aria-labelledby="registrationModalLabel"
    aria-hidden="true"
  >
    <div class="modal-dialog modal-lg modal-dialog-centered">
      <div class="modal-content">
        <div class="modal-header">
          <h1 class="modal-title fs-5" id="registrationModalLabel">Report calibration data</h1>
        </div>
        <div class="modal-body">
          <p>
            Do you want to help to identify improvements connected to device calibration please
            report your data points. Its possible to report data multiple times if you improve your
            data points. This is the data that will be sent and its anonymized. The chipid is a hash
            (SHA256) of the actual chip ID to ensure privacy.
          </p>
          <div class="mb-3">
            <label for="registrationData" class="form-label">Anonymous data:</label>
            <pre id="registrationData" class="border p-2 bg-light">{{ registrationDataJson }}</pre>
          </div>
          <p><b>Cancel</b> will close the dialog without action.</p>
          <p><b>Send</b> will send the data above to the collection service.</p>
        </div>
        <div class="modal-footer">
          <button type="button" class="btn btn-secondary" @click="cancel">Cancel</button>
          <button type="button" class="btn btn-primary" @click="send">Send</button>
        </div>
      </div>
    </div>
  </div>
  <div class="modal-backdrop fade show"></div>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import { global, config } from '@/modules/pinia'
import { sharedHttpClient as http, logError, logInfo } from '@mp-se/espframework-ui-components'

const emit = defineEmits(['close'])

const registrationDataJson = ref('')

onMounted(async () => {
  // Generate sample JSON data for registration
  const anonymizedChipId = await global.anonymizeChipId()
  const data = {
    chipid: anonymizedChipId,
    formula: config.gravity_formula,
    gravity_unit: config.gravity_unit,
    formula_calculation_data: []
  }
  registrationDataJson.value = JSON.stringify(data, null, 2)
  console.log('Calibration data:', registrationDataJson.value)

  registrationDataJson.value = registrationDataJson.value.replace(
    '[]',
    JSON.stringify(config.formula_calculation_data, null, 0)
  )
  console.log('Calibration data:', registrationDataJson.value)

  data.formula_calculation_data = config.formula_calculation_data
  console.log('Calibration data:', JSON.stringify(data, null, 2))
})

const cancel = () => {
  emit('close') // Just close the dialog
}

const send = async () => {
  global.clearMessages()
  logInfo('RegisterCalibrationFragment.send()', 'Sending calibration data')
  global.disabled = true

  try {
    const addUrl = `${global.registerBaseUrl}api/v1/calibration/add`
    const response = await http.postJson(addUrl, JSON.parse(registrationDataJson.value), {
      headers: {
        'X-API-Key': `${global.registerApiKey}`,
        'Content-Type': 'application/json'
      }
    })

    if (response) {
      global.messageSuccess = 'Calibration data reported successfully!'
    } else {
      global.messageError = 'Registration failed. Please try again.'
    }
  } catch (error) {
    // Extract status code from error message (format: "HTTP 401: Unauthorized")
    const statusMatch = error.message.match(/HTTP (\d+):/)
    const status = statusMatch ? parseInt(statusMatch[1]) : null

    if (status === 401) {
      global.messageError =
        'Reporting failed: API key has been revoked. Please update to a newer software version to report data.'
    } else if (status === 429) {
      global.messageError =
        'Reporting failed: Registration API is currently unavailable, please try again later.'
    } else if (status === 500) {
      global.messageError = 'Reporting failed: Internal server error, please try again later.'
    } else {
      logError('RegisterCalibrationFragment.send()', 'Reporting error:', error)
      global.messageError = 'An error occurred during reporting.'
    }
  } finally {
    global.disabled = false
  }

  // Always close the dialog after sending
  emit('close')
}
</script>

<style scoped>
.modal {
  z-index: 1050;
}
.modal-backdrop {
  z-index: 1040;
}

#registrationData {
  overflow: auto;
  white-space: pre-wrap;
  word-wrap: break-word;
  max-height: 300px; /* Optional: limit height if needed */
}
</style>
