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
    <p class="h3">{{ t('device_gyro.title') }}</p>
    <hr />

    <BsMessage
      v-if="!isGyroCalibrated() && config.gyro_type == 1"
      dismissable="true"
      message=""
      alert="warning"
    >
      {{ t('device_gyro.need_calibration') }}
    </BsMessage>

    <form @submit.prevent="save" class="needs-validation" novalidate>
      <div class="row">
        <template v-if="!global.isCuckoo">
          <div class="col-md-12">
            <BsInputRadio
              v-model="config.gyro_type"
              :options="gyroOptions"
              :label="t('device_gyro.gyro_options_label')"
              :help="t('device_gyro.gyro_options_help')"
              :disabled="global.disabled"
            ></BsInputRadio>
          </div>

          <div class="col-md-12">
            <hr />
          </div>
        </template>

        <div class="col-md-6">
          <BsInputSwitch
            v-model="config.gyro_filter"
            :label="t('device_gyro.filter_label')"
            :help="t('device_gyro.filter_help')"
            :disabled="global.disabled || !global.feature.filter"
          ></BsInputSwitch>
        </div>
        <div class="col-md-6" v-if="!global.isCuckoo">
          <BsInputSwitch
            v-model="config.gyro_swap_xy"
            :label="t('device_gyro.swap_xy_label')"
            :disabled="global.disabled || config.gyro_type != 2"
            :help="t('device_gyro.swap_xy_help')"
          ></BsInputSwitch>
        </div>
        <div class="col-md-6">
          <BsInputReadonly
            v-model="calibrationValues"
            :label="t('device_gyro.calibration_label')"
            :help="t('device_gyro.calibration_help')"
          ></BsInputReadonly>
        </div>
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
            &nbsp;{{ t('device_gyro.save') }}</button
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
            &nbsp;{{ t('device_gyro.restart') }}</button
          >&nbsp;

          <template v-if="config.gyro_type == 1">
            <button
              @click.prevent="clearCalibration()"
              type="button"
              class="btn btn-secondary"
              :disabled="global.disabled || !isGyroCalibrated()"
            >
              <span
                class="spinner-border spinner-border-sm"
                role="status"
                aria-hidden="true"
                v-show="global.disabled"
              ></span>
              &nbsp;{{ t('device_gyro.clear_calibration') }}</button
            >&nbsp;
          </template>

          <button
            @click="calibrate"
            type="button"
            class="btn btn-secondary"
            v-show="config.gyro_type == 1"
            :disabled="
              global.disabled ||
              !status.self_check.gyro_connected ||
              status.wifi_setup ||
              config.gyro_type == 2
            "
          >
            <span
              class="spinner-border spinner-border-sm"
              role="status"
              aria-hidden="true"
              v-show="global.disabled"
            ></span>
            &nbsp;{{ t('device_gyro.calibrate_gyro') }}&nbsp;<span
              v-if="badge.deviceGyroCalibratedBadge()"
              class="badge text-bg-danger rounded-circle"
              >1</span
            ></button
          >&nbsp;

          <template v-if="status.ispindel_config">
            <button
              @click="ispindel"
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
              &nbsp;{{ t('device_gyro.import_ispindel') }}&nbsp;<span
                v-if="badge.deviceMigrateIspindelBadge()"
                class="badge text-bg-danger rounded-circle"
                >1</span
              >
            </button>
          </template>
        </div>
      </div>
    </form>
  </div>
</template>

<script setup>
import { ref, computed, watch } from 'vue'
import { useI18n } from 'vue-i18n'
import { isGyroCalibrated } from '@/modules/utils'
import { validateCurrentForm } from '@mp-se/espframework-ui-components'
import { global, config, status } from '@/modules/pinia'
import * as badge from '@/modules/badge'
import { logDebug, logError, logInfo } from '@mp-se/espframework-ui-components'
import { storeToRefs } from 'pinia'
import { sharedHttpClient as http } from '@mp-se/espframework-ui-components'

const { t } = useI18n()

const gyroOptions = ref([
  // value 0 is used internally at startup to check if gyro has been defined.
  { label: 'MPU 6050/6500', value: 1 },
  { label: 'ICM42670-p', value: 2 }
])

const calibrationValues = computed(() => {
  if (config.gyro_type == 1) return JSON.stringify(config.gyro_calibration_data)

  return t('device_gyro.calibration_not_needed')
})

const { gyro_type } = storeToRefs(config)

watch(gyro_type, () => {
  if (config.gyro_type == 1) config.gyro_swap_xy = false
})

const ispindel = () => {
  const data = {
    command: 'get',
    file: '/config.json'
  }

  ;(async () => {
    global.disabled = true
    const res = await http.filesystemRequest(data)
    if (res && res.success) {
      const json = JSON.parse(res.text)
      logDebug('DeviceHardwareView.ispindel()', json)

      config.gyro_calibration_data.ax = json.Offset[0]
      config.gyro_calibration_data.ay = json.Offset[1]
      config.gyro_calibration_data.az = json.Offset[2]
      config.gyro_calibration_data.gx = json.Offset[3]
      config.gyro_calibration_data.gy = json.Offset[4]
      config.gyro_calibration_data.gz = json.Offset[5]

      config.gravity_formula = json.POLY

      global.messageSuccess = t('device_gyro.err_import')
    }
    global.disabled = false
  })()
}

const clearCalibration = async () => {
  config.gyro_calibration_data.ax = 0
  config.gyro_calibration_data.ay = 0
  config.gyro_calibration_data.az = 0
  config.gyro_calibration_data.gx = 0
  config.gyro_calibration_data.gy = 0
  config.gyro_calibration_data.gz = 0
  await config.saveAll()
}

const calibrate = async () => {
  global.clearMessages()
  global.disabled = true

  try {
    logInfo('DeviceHardwareView.calibrate()', 'Sending /api/calibrate')
    const started = await http.request('api/calibrate')

    if (!started || started.ok === false) {
      global.messageError = t('device_gyro.err_start_calibration')
      return false
    }

    // Poll for calibration completion similar to config.runPushTest
    const result = await (async () => {
      while (true) {
        const statusRes = await http.getJson('api/calibrate/status')
        if (!statusRes) {
          global.messageError = t('device_gyro.err_get_status')
          return false
        }

        logInfo('DeviceHardwareView.calibrate()', statusRes)

        // statusRes.status == true means still running
        if (statusRes.status) {
          await new Promise((r) => setTimeout(r, 2000))
          continue
        }

        // Calibration completed, check success flag
        if (!statusRes.success) {
          global.messageError = t('device_gyro.err_calibration_failed', {
            code: statusRes.message || 'unknown'
          })
          return false
        }

        return true
      }
    })()

    if (result) {
      // Reload configuration after successful calibration
      const configLoaded = await config.load()
      if (configLoaded) {
        global.messageSuccess = t('device_gyro.calibration_success')
      } else {
        global.messageError = t('device_gyro.err_load_config')
      }
    }

    return result
  } catch (err) {
    logError('DeviceHardwareView.calibrate()', err)
    global.messageError = t('device_gyro.err_calibration_unexpected')
    return false
  } finally {
    global.disabled = false
  }
}

const save = async () => {
  if (!validateCurrentForm()) return

  global.clearMessages()
  await config.saveAll()
}

const restart = async () => {
  await config.restart()
}
</script>
