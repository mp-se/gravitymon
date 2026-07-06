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
    <p class="h2">{{ t('device_settings.title') }}</p>
    <hr />

    <BsMessage v-if="config.mdns === ''" dismissable="true" message="" alert="warning">
      {{ t('device_settings.mdns_warning') }}
    </BsMessage>

    <form @submit.prevent="saveSettings" class="needs-validation" novalidate>
      <div class="row">
        <div class="col-md-12">
          <BsInputText
            v-model="config.mdns"
            maxlength="63"
            minlength="1"
            :label="t('device_settings.mdns_label')"
            :help="t('device_settings.mdns_help')"
            :badge="badge.deviceMdnsBadge()"
            :disabled="global.disabled"
          >
          </BsInputText>
        </div>

        <div class="col-md-12">
          <hr />
        </div>

        <div class="col-md-3">
          <BsInputRadio
            v-model="config.temp_unit"
            :options="tempOptions"
            :label="t('device_settings.temp_format_label')"
            width=""
            :disabled="global.disabled"
          ></BsInputRadio>
        </div>
        <div class="col-md-3">
          <BsInputRadio
            v-model="config.gravity_unit"
            :options="gravityOptions"
            :label="t('device_settings.gravity_format_label')"
            width=""
            :disabled="global.disabled"
          ></BsInputRadio>
        </div>

        <div class="col-md-3">
          <BsInputRadio
            v-model="config.dark_mode"
            :options="uiOptions"
            :label="t('device_settings.ui_label')"
            width=""
            :disabled="global.disabled"
          ></BsInputRadio>
        </div>

        <div class="col-md-3">
          <BsSelect
            v-model="config.locale"
            :options="localeOptions"
            :label="t('device_settings.locale_label')"
            :disabled="global.disabled"
          ></BsSelect>
        </div>

        <div class="col-md-12">
          <hr />
        </div>

        <div class="col-md-9">
          <BsInputText
            v-model="config.ota_url"
            type="url"
            maxlength="80"
            :label="t('device_settings.ota_url_label')"
            :help="t('device_settings.ota_url_help')"
            :disabled="global.disabled"
          >
          </BsInputText>
        </div>

        <div class="col-md-3">
          <BsDropdown
            :label="t('device_settings.predefined_ota_label')"
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
            <BsInputReadonly v-model="registrationStatus" :label="t('device_settings.reporting_label')">
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
              &nbsp;{{ t('device_settings.check_status') }}
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
            &nbsp;{{ t('device_settings.save') }}</button
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
            &nbsp;{{ t('device_settings.restart') }}</button
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
            &nbsp;{{ t('device_settings.factory_defaults') }}
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
              &nbsp;{{ t('device_settings.report_usage') }}
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
import { ref, computed } from 'vue'
import { useI18n } from 'vue-i18n'
import { BsInputReadonly, validateCurrentForm } from '@mp-se/espframework-ui-components'
import { global, status, config } from '@/modules/pinia'
import * as badge from '@/modules/badge'
import { logError, logInfo } from '@mp-se/espframework-ui-components'
import { sharedHttpClient as http } from '@mp-se/espframework-ui-components'
import { resolveMessage } from '@/modules/utils'

const { t } = useI18n()

const otaOptions = ref([
  { label: '-blank-', value: '' },
  { label: 'Gravitymon.com', value: 'https://www.gravitymon.com/firmware/' }
])

const tempOptions = computed(() => [
  { label: t('device_settings.temp_celsius'), value: 'C' },
  { label: t('device_settings.temp_fahrenheit'), value: 'F' }
])

const gravityOptions = computed(() => [
  { label: t('device_settings.gravity_sg'), value: 'G' },
  { label: t('device_settings.gravity_plato'), value: 'P' }
])

const uiOptions = computed(() => [
  { label: t('device_settings.ui_day_mode'), value: false },
  { label: t('device_settings.ui_dark_mode'), value: true }
])

const localeOptions = ref([
  { label: 'English', value: 'en' },
  { label: 'Svenska', value: 'sv' },
  { label: 'Deutsch', value: 'de' },
  { label: 'Français', value: 'fr' },
  { label: 'Español', value: 'es' },
  { label: '中文', value: 'zh' },
  { label: 'Polski', value: 'pl' },
  { label: 'Norsk', value: 'no' },
  { label: 'Dansk', value: 'da' },
  { label: 'Nederlands', value: 'nl' },
  { label: 'Português', value: 'pt' },
  { label: 'Italiano', value: 'it' }
])

const registrationStatus = ref(t('device_settings.reporting_unknown'))
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
      registrationStatus.value = t('device_settings.reporting_found', {
        software: checkResponse.software
      })
    } else {
      registrationStatus.value = t('device_settings.reporting_not_found')
    }
  } catch (error) {
    logError('DeviceSettingsView.checkReported()', 'Failed to check registration status:', error)
    registrationStatus.value = t('device_settings.reporting_error')
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
      global.messageSuccess = resolveMessage(json.message_code, json.message)
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
      global.messageError = resolveMessage(json.message_code, json.message)
    }
  } catch (err) {
    logError('DeviceSettingsView.factory()', err)
    global.messageError = t('device_settings.err_factory_restore')
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
