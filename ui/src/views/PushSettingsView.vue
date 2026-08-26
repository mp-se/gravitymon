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
    <p class="h3">{{ t('push_settings.title') }}</p>
    <hr />

    <BsMessage v-if="config.sleep_interval < 300" dismissable="true" message="" alert="warning">
      {{ t('push_settings.warn_short_interval') }}
    </BsMessage>

    <BsMessage
      v-if="config.gyro_temp && config.sleep_interval < 300"
      dismissable="true"
      message=""
      alert="warning"
    >
      {{ t('push_settings.warn_gyro_temp_interval') }}
    </BsMessage>

    <form @submit.prevent="save" class="needs-validation" novalidate>
      <div class="row">
        <div class="col-md-6">
          <BsInputText
            v-model="config.token"
            maxlength="50"
            :label="t('push_settings.token1_label')"
            :help="t('push_settings.token_help')"
            :disabled="global.disabled"
          />
        </div>
        <div class="col-md-6">
          <BsInputText
            v-model="config.token2"
            maxlength="50"
            :label="t('push_settings.token2_label')"
            :help="t('push_settings.token_help')"
            :disabled="global.disabled"
          />
        </div>
        <div class="col-md-6">
          <BsInputNumber
            v-model="config.sleep_interval"
            :label="t('push_settings.sleep_interval_label') + sleepLabel"
            unit="s"
            min="10"
            max="3600"
            step="1"
            width="5"
            :help="t('push_settings.sleep_interval_help')"
            :disabled="global.disabled"
          />
        </div>

        <div class="col-md-6">
          <BsInputReadonly
            v-model="batteryLife"
            :label="t('push_settings.battery_life_label')"
            :help="t('push_settings.battery_life_help')"
            :disabled="global.disabled"
          ></BsInputReadonly>
        </div>

        <div class="col-md-6">
          <BsInputNumber
            v-model="config.push_timeout"
            :label="t('push_settings.push_timeout_label')"
            unit="s"
            min="10"
            max="60"
            step="1"
            width="5"
            :help="t('push_settings.push_timeout_help')"
            :disabled="global.disabled"
          />
        </div>
        <div v-if="global.platform === 'esp8266'" class="col-md-6">
          <BsInputSwitch
            v-model="config.skip_ssl_on_test"
            :label="t('push_settings.skip_ssl_label')"
            :help="t('push_settings.skip_ssl_help')"
            :disabled="global.disabled"
          />
        </div>

        <div class="col-md-12">
          <hr />
        </div>
      </div>

      <div class="row gy-2">
        <div class="col-md-3">
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
            &nbsp;{{ t('push_settings.save') }}
          </button>
        </div>
      </div>
    </form>
  </div>
</template>

<script setup>
import { onMounted, ref, watch } from 'vue'
import { useI18n } from 'vue-i18n'
import { validateCurrentForm } from '@mp-se/espframework-ui-components'
import { global, config } from '@/modules/pinia'
import { storeToRefs } from 'pinia'
import { logDebug, logError } from '@mp-se/espframework-ui-components'

const { t } = useI18n()
const { sleep_interval } = storeToRefs(config)
const batteryLife = ref('')
const sleepLabel = ref('')

const save = async () => {
  if (!validateCurrentForm()) return

  await config.saveAll()
}

watch(sleep_interval, () => {
  createSleepLabel()
  calculateBatteryLife()
})

onMounted(() => {
  createSleepLabel()
  calculateBatteryLife()
})

const createSleepLabel = () => {
  const s = Math.floor(sleep_interval.value / 60) + ' min ' + (sleep_interval.value % 60) + ' sec'
  sleepLabel.value = '(' + s + ')'
}

const calculateBatteryLife = () => {
  // ESP8266 consumes between 140-170mA when WIFI is on. Deep sleep is 20uA.
  // MPU-6050 consumes 4mA
  // DS18B20 consumes 1mA
  // For this estimation we use an average of 160mA
  let pwrActive = 160 // mA per hour (120-170 mA)
  const pwrSleep = 15 // mA per day (include all pheripials as well)
  const batt = 2200 // mA
  const rt = 5 // Assume 10 seconds per run
  const ble = config.ble_format === 0 ? false : true
  const wifi =
    config.http_post_target.length +
      config.http_post2_target.length +
      config.http_get_target.length +
      config.influxdb2_target.length +
      config.mqtt_target.length >
    0
      ? true
      : false

  if (!wifi && !ble) {
    logError(
      'PushSettingsView.calculateBatteryLife()',
      'No push targets defined, cannot estimate battery life'
    )
    return
  }

  if (wifi) {
    switch (global.platform) {
      case 'ESP8266':
        pwrActive = 160
        break
      case 'ESP32C3':
        pwrActive = 320 // mA per hour (290-350 mA)
        break
      case 'ESP32S2':
        pwrActive = 280 // mA per hour (260-300 mA)
        break
      case 'ESP32S3':
        pwrActive = 300 // mA per hour (285-355 mA)
        break
      default:
        logError('PushSettingsView.calculateBatteryLife()', 'Unknown platform', global.platform)
        break
    }
  } else {
    switch (global.platform) {
      case 'ESP8266':
      case 'ESP32C3':
      case 'ESP32S2':
        pwrActive = 160
        break
      case 'ESP32S3':
        pwrActive = 180
        break
      default:
        logError('PushSettingsView.calculateBatteryLife()', 'Unknown platform', global.platform)
        break
    }
  }

  // if (rt < 4) rt = 4 // Assume that this takes at least 4 seconds to read sensors and do push.

  // The deep sleep will consume approx 1mA per day.
  const powerPerDay =
    ((24 * 3600) / (config.sleep_interval + rt)) * (rt / 3600) * pwrActive + pwrSleep
  const days = batt / powerPerDay

  logDebug(
    'PushSettingsView.calculateBatteryLife()',
    'Estimated power per hour = ' + pwrActive.toString() + 'mA on platform = ' + global.platform
  )
  logDebug('PushSettingsView.calculateBatteryLife()', 'Estimated number of days = ' + days)

  batteryLife.value = t('push_settings.battery_life_weeks', {
    weeks: Math.floor(days / 7),
    days: Math.floor(days % 7)
  })
}
</script>
