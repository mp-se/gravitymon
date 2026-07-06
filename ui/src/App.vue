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
  <dialog id="spinner" class="loading">
    <div class="container text-center">
      <div class="row align-items-center" style="height: 170px">
        <div class="col">
          <div class="spinner-border" role="status" style="width: 5rem; height: 5rem">
            <span class="visually-hidden">Loading...</span>
          </div>
        </div>
      </div>
    </div>
  </dialog>

  <div v-if="!global.initialized" class="container text-center">
    <BsMessage :message="t('app.initializing')" :dismissable="false" alert="info"></BsMessage>
  </div>

  <BsMenuBar
    v-if="global.initialized"
    :disabled="global.disabled"
    brand="Gravitymon"
    :menu-items="items"
    :dark-mode="config.dark_mode"
    :mdns="config.mdns"
    :config-changed="global.configChanged"
    @update:dark-mode="handleDarkModeUpdate"
  />

  <div class="container">
    <div>
      <p></p>
    </div>
    <BsMessage
      v-if="!status.connected"
      :message="t('app.no_response')"
      :dismissable="false"
      alert="danger"
    ></BsMessage>

    <BsMessage
      v-if="global.isError"
      :close="close"
      :dismissable="true"
      :message="global.messageError"
      alert="danger"
    />
    <BsMessage
      v-if="global.isWarning"
      :close="close"
      :dismissable="true"
      :message="global.messageWarning"
      alert="warning"
    />
    <BsMessage
      v-if="global.isSuccess"
      :close="close"
      :dismissable="true"
      :message="global.messageSuccess"
      alert="success"
    />
    <BsMessage
      v-if="global.isInfo"
      :close="close"
      :dismissable="true"
      :message="global.messageInfo"
      alert="info"
    />

    <BsMessage v-if="status.wifi_setup" :dismissable="false" alert="info">
      {{ t('app.wifi_setup_info') }}
      <router-link class="alert-link" to="/device/wifi">{{ t('app.wifi_setup_link') }}</router-link>
      {{ t('app.wifi_setup_suffix') }}
    </BsMessage>

    <BsMessage v-if="status.wifi_setup" :dismissable="false" alert="warning">
      {{ t('app.wifi_setup_warning') }}
    </BsMessage>

    <BsMessage v-if="status.ispindel_config" :dismissable="true" alert="info">
      {{ t('app.ispindel_found') }}
      <router-link class="alert-link" to="/device/gyro">{{ t('app.ispindel_import') }}</router-link>
      {{ t('app.ispindel_or') }}
      <router-link class="alert-link" to="/other/support">{{ t('app.ispindel_delete') }}</router-link>
      {{ t('app.ispindel_suffix') }}
    </BsMessage>
  </div>

  <RegisterDeviceFragment
    v-if="showRegisterModal"
    software="Gravitymon"
    @close="closeRegisterModal"
  />

  <router-view v-if="global.initialized" />
  <BsFooter v-if="global.initialized" :text="t('app.footer')" />
</template>

<script setup>
import { onMounted, watch, onBeforeMount, ref } from 'vue'
import { useI18n } from 'vue-i18n'
import { global, status, config, saveConfigState } from './modules/pinia'
import { sharedHttpClient as http } from '@mp-se/espframework-ui-components'
import { storeToRefs } from 'pinia'
import { useTimers } from '@mp-se/espframework-ui-components'
import { logError, logInfo, version } from '@mp-se/espframework-ui-components'
import { items } from './modules/router'

const { t, locale } = useI18n()
const { createInterval } = useTimers()
const polling = ref(null)
const showRegisterModal = ref(false)

// Keep the active vue-i18n locale in sync with the device-stored config value
watch(
  () => config.locale,
  (newValue) => {
    if (newValue) locale.value = newValue
  },
  { immediate: true }
)

const { disabled } = storeToRefs(global)

const close = (alert) => {
  if (alert == 'danger') global.messageError = ''
  else if (alert == 'warning') global.messageWarning = ''
  else if (alert == 'success') global.messageSuccess = ''
  else if (alert == 'info') global.messageInfo = ''
}

watch(disabled, () => {
  if (global.disabled) document.body.style.cursor = 'wait'
  else document.body.style.cursor = 'default'
})

function ping() {
  ;(async () => {
    const ok = await http.ping()
    status.connected = ok
  })()
}

onBeforeMount(() => {
  polling.value = createInterval(ping, 7000)
})

onMounted(async () => {
  logInfo('App.onMounted()', `Using espframework version ${version}`)

  if (!global.initialized) {
    await initializeApp()
  }
})

async function initializeApp() {
  try {
    showSpinner()

    // Step 1: Authenticate with device (http client owns token)
    const base = btoa('gravitymon:password')
    const authOk = await http.auth(base)
    if (!authOk) {
      global.messageError = t('app.err_auth_failed')
      return
    }

    // Step 2: Load feature flags
    const globalSuccess = await global.load()
    if (!globalSuccess) {
      global.messageError = t('app.err_load_features')
      return
    }

    // Step 3: Load device status
    const statusSuccess = await status.load()
    if (!statusSuccess) {
      global.messageError = t('app.err_load_status')
      return
    }

    // Step 4: Load configuration
    const configSuccess = await config.load()
    if (!configSuccess) {
      global.messageError = t('app.err_load_config')
      return
    }

    // Step 5: Load format templates
    const formatSuccess = await config.loadFormat()
    if (!formatSuccess) {
      global.messageError = t('app.err_load_format')
      return
    }

    // Step 6: Check device registration status
    if (!global.registered && global.ui.enableDeviceRegistration) {
      try {
        const anonymizedChipId = await global.anonymizeChipId()
        const checkUrl = `${global.registerBaseUrl}api/v1/device/check/${anonymizedChipId}`
        const checkResponse = await http.getJson(checkUrl, {
          headers: { 'X-API-Key': `${global.registerApiKey}` }
        })
        if (checkResponse && checkResponse.exists) {
          // Update device config to reflect registered status
          await http.postJson('api/config', { registered: true })
          global.registered = true
        }
      } catch (error) {
        logError('App.initializeApp()', 'Registration check failed:', error)
        // Continue without blocking
      }
    }

    // Success! Initialize the app
    saveConfigState()
    handleDarkModeUpdate(config.dark_mode)
    global.initialized = true

    // Trigger register modal if device is not registered
    if (!global.registered && global.ui.enableDeviceRegistration && status.wifi_setup == false) {
      showRegisterModal.value = true
    }
  } catch (error) {
    logError('App.initializeApp()', error)
    global.messageError = t('app.err_init_failed', { error: error.message })
  } finally {
    hideSpinner()
  }
}

function closeRegisterModal() {
  showRegisterModal.value = false
}

// Watch for changes to config.dark_mode and call handleDarkModeUpdate
watch(
  () => config.dark_mode,
  (newValue) => {
    handleDarkModeUpdate(newValue)
  }
)

// Handle dark mode changes
const handleDarkModeUpdate = (newValue) => {
  logInfo('App.handleDarkModeUpdate()', 'Updating dark mode settings', newValue)

  // update the store value
  config.dark_mode = newValue
  // fallback: ensure the attribute is set on the document root so Bootstrap theme rules apply
  try {
    const root = document.documentElement
    if (newValue) root.setAttribute('data-bs-theme', 'dark')
    else root.setAttribute('data-bs-theme', 'light')
  } catch (e) {
    logError('App.handleDarkModeUpdate()', 'Failed to set data-bs-theme on documentElement', e)
  }
}

function showSpinner() {
  document.querySelector('#spinner').showModal()
}

function hideSpinner() {
  document.querySelector('#spinner').close()
}
</script>

<style>
.loading {
  position: fixed;
  width: 200px;
  height: 200px;
  padding: 10px;
  top: 0;
  right: 0;
  bottom: 0;
  left: 0;
  border: 0;
}

dialog::backdrop {
  background-color: black;
  opacity: 60%;
}
</style>
