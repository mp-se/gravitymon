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
    <p class="h2">Device - Settings</p>
    <hr />

    <BsMessage v-if="config.mdns === ''" dismissable="true" message="" alert="warning">
      You need to define a mdns name for the device
    </BsMessage>

    <form @submit.prevent="saveSettings" class="needs-validation" novalidate>
      <div class="row">
        <div class="col-md-12">
          <BsInputText
            v-model="config.mdns"
            maxlength="63"
            minlength="1"
            label="MDNS"
            help="Enter device name used on the network, the suffix .local will be added to this name"
            :badge="badge.deviceMdnsBadge()"
            :disabled="global.disabled"
          >
          </BsInputText>
        </div>

        <div class="col-md-12">
          <hr />
        </div>

        <div class="col-md-4">
          <BsInputRadio
            v-model="config.temp_unit"
            :options="tempOptions"
            label="Temperature Format"
            width=""
            :disabled="global.disabled"
          ></BsInputRadio>
        </div>
        <div class="col-md-4">
          <BsInputRadio
            v-model="config.gravity_unit"
            :options="gravityOptions"
            label="Gravity Format"
            width=""
            :disabled="global.disabled"
          ></BsInputRadio>
        </div>

        <div class="col-md-4">
          <BsInputRadio
            v-model="config.dark_mode"
            :options="uiOptions"
            label="User Interface"
            width=""
            :disabled="global.disabled"
          ></BsInputRadio>
        </div>

        <div class="col-md-12">
          <hr />
        </div>

        <div class="col-md-9">
          <BsInputText
            v-model="config.ota_url"
            type="url"
            maxlength="80"
            label="OTA URL"
            help="Base URL to where firmware and version.json file can be found. Needs to end with '/'', example: http://www.mysite.com/firmware/"
            :disabled="global.disabled"
          >
          </BsInputText>
        </div>

        <div class="col-md-3">
          <BsDropdown
            label="Predefined ota"
            button="URL"
            :options="otaOptions"
            :callback="otaCallback"
            :disabled="global.disabled"
          />
        </div>

        <template v-if="global.ui.enableDeviceRegistration">
          <div class="col-md-12">
            <hr />
          </div>

          <div class="col-md-12">
            <BsInputReadonly v-model="registrationStatus" label="Device usage reporting">
            </BsInputReadonly>
          </div>

          <div class="col-md-12">
            <button
              @click.prevent="checkReported"
              type="button"
              class="btn btn-secondary"
              :disabled="global.disabled || status.wifi_setup == true"
            >
              <span
                class="spinner-border spinner-border-sm"
                role="status"
                aria-hidden="true"
                v-show="global.disabled"
              ></span>
              &nbsp;Check status
            </button>
          </div>
        </template>
      </div>

      <div class="row gy-2">
        <div class="col-md-12">
          <hr />
        </div>
        <div class="col-md-12">
          <button
            type="submit"
            class="btn btn-primary w-2"
            :disabled="global.disabled || !global.configChanged"
          >
            <span
              class="spinner-border spinner-border-sm"
              role="status"
              aria-hidden="true"
              v-show="global.disabled"
            ></span>
            &nbsp;Save</button
          >&nbsp;

          <button
            @click.prevent="restart"
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
            &nbsp;Restart device</button
          >&nbsp;

          <button
            @click="factory"
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
            &nbsp;Restore factory defaults
          </button>

          <template v-if="global.ui.enableDeviceRegistration">
            &nbsp;
            <button
              @click.prevent="reportUsage"
              type="button"
              class="btn btn-secondary"
              :disabled="global.disabled || status.wifi_setup == true"
            >
              <span
                class="spinner-border spinner-border-sm"
                role="status"
                aria-hidden="true"
                v-show="global.disabled"
              ></span>
              &nbsp;Report Usage
            </button>
          </template>
        </div>
      </div>
    </form>
  </div>

  <RegisterDeviceFragment
    v-if="showRegisterModal"
    software="Gravitymon"
    @close="closeRegisterModal"
  />
</template>

<script setup>
import { ref } from 'vue'
import { BsInputReadonly, validateCurrentForm } from '@mp-se/espframework-ui-components'
import { global, status, config } from '@/modules/pinia'
import * as badge from '@/modules/badge'
import { logError, logInfo } from '@mp-se/espframework-ui-components'
import { sharedHttpClient as http } from '@mp-se/espframework-ui-components'

const otaOptions = ref([
  { label: '-blank-', value: '' },
  { label: 'Gravitymon.com', value: 'https://www.gravitymon.com/firmware/' }
])

const tempOptions = ref([
  { label: 'Celsius °C', value: 'C' },
  { label: 'Fahrenheit °F', value: 'F' }
])

const gravityOptions = ref([
  { label: 'Specific Gravity', value: 'G' },
  { label: 'Plato', value: 'P' }
])

const uiOptions = ref([
  { label: 'Day mode', value: false },
  { label: 'Dark mode', value: true }
])

const registrationStatus = ref('Unknown')
const showRegisterModal = ref(false)

const otaCallback = (opt) => {
  config.ota_url = opt
}

const checkReported = async () => {
  global.clearMessages()
  logInfo('DeviceSettingsView.checkReported()', 'Checking device registration status')
  global.disabled = true

  try {
    const anonymizedChipId = await global.anonymizeChipId()
    const checkUrl = `${global.registerBaseUrl}api/v1/device/check/${anonymizedChipId}`
    const checkResponse = await http.getJson(checkUrl, {
      headers: { 'X-API-Key': `${global.registerApiKey}` }
    })
    if (checkResponse && checkResponse.exists) {
      registrationStatus.value = `This device has been reported using ${checkResponse.software}.`
    } else {
      registrationStatus.value = 'This device has not been reported'
    }
  } catch (error) {
    logError('DeviceSettingsView.checkReported()', 'Failed to check registration status:', error)
    registrationStatus.value = 'Error checking registration status'
  } finally {
    global.disabled = false
  }
}

const reportUsage = async () => {
  // Show the registration modal for manual usage reporting
  showRegisterModal.value = true
}

const factory = async () => {
  global.clearMessages()
  logInfo('DeviceSettingsView.factory()', 'Sending /api/factory')
  global.disabled = true

  try {
    const json = await http.getJson('api/factory')

    if (json.success == true) {
      global.messageSuccess = json.message
      const reloadTimeout = setTimeout(() => {
        try {
          location.reload(true)
        } catch (error) {
          logError('DeviceSettingsView.factory.reload()', error)
          window.location.reload()
        }
      }, 2000)

      // Clean up timeout on component unmount
      window.addEventListener(
        'beforeunload',
        () => {
          clearTimeout(reloadTimeout)
        },
        { once: true }
      )
    } else {
      global.messageError = json.message
    }
  } catch (err) {
    logError('DeviceSettingsView.factory()', err)
    global.messageError = 'Failed to do factory restore'
  } finally {
    global.disabled = false
  }
}

const restart = async () => {
  await config.restart()
}

const saveSettings = async () => {
  if (!validateCurrentForm()) return

  await config.saveAll()
}

const closeRegisterModal = () => {
  checkReported()
  showRegisterModal.value = false
}
</script>
