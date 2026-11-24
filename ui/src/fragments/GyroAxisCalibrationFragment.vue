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
  <div class="row gy-2">
    <div class="col-md-12">
      <hr />
      <p class="h5">{{ t('device_gyro.axis_title') }}</p>
      <p class="mb-1">{{ t('device_gyro.axis_intro') }}</p>
      <ol class="mb-2">
        <li>{{ t('device_gyro.axis_step1') }}</li>
        <li>{{ t('device_gyro.axis_step2') }}</li>
        <li>{{ t('device_gyro.axis_step3') }}</li>
        <li>{{ t('device_gyro.axis_step4') }}</li>
      </ol>
      <p class="mb-1">
        <span class="badge" :class="calibrated ? 'text-bg-success' : 'text-bg-secondary'">
          {{ calibrated ? t('device_gyro.axis_active') : t('device_gyro.axis_not_active') }}
        </span>
        <span v-if="calibrated" class="ms-2 small font-monospace">{{ currentValues }}</span>
      </p>
    </div>

    <div class="col-md-12" v-if="message">
      <div class="alert alert-warning mb-0" role="alert">{{ message }}</div>
    </div>

    <div class="col-md-12" v-if="samples.length">
      <div class="table-responsive">
        <table class="table table-sm align-middle mb-0">
          <thead>
            <tr>
              <th scope="col">#</th>
              <th scope="col">{{ t('device_gyro.axis_col_direction') }}</th>
              <th scope="col">X</th>
              <th scope="col">Y</th>
              <th scope="col">Z</th>
              <th scope="col">{{ t('device_gyro.axis_col_before') }}</th>
              <th scope="col">{{ t('device_gyro.axis_col_after') }}</th>
              <th scope="col"></th>
            </tr>
          </thead>
          <tbody>
            <tr v-for="(s, i) in samples" :key="i">
              <td>{{ i + 1 }}</td>
              <td>
                {{
                  s.group ? t('device_gyro.axis_direction_b') : t('device_gyro.axis_direction_a')
                }}
              </td>
              <td>{{ s.x.toFixed(4) }}</td>
              <td>{{ s.y.toFixed(4) }}</td>
              <td>{{ s.z.toFixed(4) }}</td>
              <td>{{ before(i) }}</td>
              <td>{{ after(i) }}</td>
              <td>
                <button
                  type="button"
                  class="btn btn-sm btn-outline-secondary"
                  :disabled="busy || global.disabled"
                  :aria-label="t('device_gyro.axis_remove_sample')"
                  @click="samples.splice(i, 1)"
                >
                  &times;
                </button>
              </td>
            </tr>
          </tbody>
        </table>
      </div>
    </div>

    <div class="col-md-12" v-if="fit && fit.success">
      <ul class="list-unstyled mb-0 small">
        <li>
          {{ t('device_gyro.axis_result_misalignment') }}:
          <b>{{ fit.misalignment.toFixed(2) }}&deg;</b>
        </li>
        <li>
          {{ t('device_gyro.axis_result_spread') }}:
          <b>{{ fit.spreadBefore.toFixed(2) }}&deg; &rarr; {{ fit.spreadAfter.toFixed(2) }}&deg;</b>
        </li>
        <li>
          {{ t('device_gyro.axis_result_bias') }}:
          <b
            >{{ fit.calibration.bx.toFixed(4) }}, {{ fit.calibration.by.toFixed(4) }},
            {{ fit.calibration.bz.toFixed(4) }}</b
          >
        </li>
        <li>
          {{ t('device_gyro.axis_result_radius') }}: <b>{{ fit.radius.toFixed(4) }}</b>
        </li>
        <li v-if="fit.slope !== null">
          {{ t('device_gyro.axis_result_slope') }}: <b>{{ fit.slope.toFixed(2) }}&deg;</b>
        </li>
      </ul>
    </div>
    <div class="col-md-12" v-else-if="fit && fit.error != 'few_samples'">
      <div class="alert alert-warning mb-0" role="alert">
        {{ t('device_gyro.axis_err_' + fit.error) }}
      </div>
    </div>

    <div class="col-md-12">
      <button type="button" class="btn btn-secondary" :disabled="!canCapture" @click="capture(0)">
        <span
          class="spinner-border spinner-border-sm"
          role="status"
          aria-hidden="true"
          v-show="busy"
        ></span>
        &nbsp;{{ t('device_gyro.axis_capture') }}</button
      >&nbsp;

      <button
        type="button"
        class="btn btn-secondary"
        :disabled="!canCapture || samplesA < minSamples"
        @click="capture(1)"
      >
        &nbsp;{{ t('device_gyro.axis_capture_flipped') }}</button
      >&nbsp;

      <button
        type="button"
        class="btn btn-primary"
        :disabled="busy || global.disabled || !fit || !fit.success"
        @click="saveAxis"
      >
        &nbsp;{{ t('device_gyro.axis_save') }}</button
      >&nbsp;

      <button
        type="button"
        class="btn btn-secondary"
        :disabled="busy || global.disabled || !samples.length"
        @click="clearSamples"
      >
        &nbsp;{{ t('device_gyro.axis_clear_samples') }}</button
      >&nbsp;

      <button
        type="button"
        class="btn btn-secondary"
        :disabled="busy || global.disabled || !calibrated"
        @click="removeAxis"
      >
        &nbsp;{{ t('device_gyro.axis_remove') }}
      </button>
    </div>
  </div>
</template>

<script setup>
import { ref, computed } from 'vue'
import { useI18n } from 'vue-i18n'
import { global, config, status } from '@/modules/pinia'
import { fitGyroAxis, isGyroAxisCalibrated, MIN_SAMPLES } from '@/modules/gyroAxis'
import { logError, logInfo } from '@mp-se/espframework-ui-components'

const { t } = useI18n()

// Number of readings that are averaged for one position and the delay between
// them, the device updates the gyro value every 200ms in configuration mode.
const props = defineProps({
  readings: { type: Number, default: 5 },
  readingDelay: { type: Number, default: 300 }
})

// Max difference in g between the readings for one position
const MAX_NOISE = 0.01

const minSamples = MIN_SAMPLES
const samples = ref([])
const busy = ref(false)
const message = ref('')

const calibrated = computed(() => isGyroAxisCalibrated(config.gyro_axis_calibration))

const currentValues = computed(() => {
  const c = config.gyro_axis_calibration
  return `n=(${c.nx}, ${c.ny}, ${c.nz}) b=(${c.bx}, ${c.by}, ${c.bz})`
})

const samplesA = computed(() => samples.value.filter((s) => !s.group).length)

const canCapture = computed(
  () => !busy.value && !global.disabled && status.self_check.gyro_connected && !status.wifi_setup
)

const fit = computed(() => {
  if (!samples.value.length) return null
  return fitGyroAxis(samples.value, config.gyro_swap_xy)
})

const before = (i) => (fit.value && fit.value.success ? fit.value.anglesBefore[i].toFixed(2) : '-')
const after = (i) => (fit.value && fit.value.success ? fit.value.anglesAfter[i].toFixed(2) : '-')

const capture = async (group) => {
  message.value = ''
  busy.value = true

  try {
    const list = []

    for (let i = 0; i < props.readings; i++) {
      if (i) await new Promise((r) => setTimeout(r, props.readingDelay))

      const res = await status.getGyro()

      if (!res.success || res.data.ax === undefined) {
        message.value = t('device_gyro.axis_err_read')
        return
      }

      if (!res.data.valid) {
        message.value = t('device_gyro.axis_err_moving')
        return
      }

      list.push([Number(res.data.ax), Number(res.data.ay), Number(res.data.az)])
    }

    const mean = [0, 1, 2].map((k) => list.reduce((s, p) => s + p[k], 0) / list.length)
    const noise = Math.max(
      ...list.map((p) => Math.max(...[0, 1, 2].map((k) => Math.abs(p[k] - mean[k]))))
    )

    if (noise > MAX_NOISE) {
      message.value = t('device_gyro.axis_err_moving')
      return
    }

    logInfo('GyroAxisCalibrationFragment.capture()', mean)
    samples.value.push({ x: mean[0], y: mean[1], z: mean[2], group: group })
  } catch (err) {
    logError('GyroAxisCalibrationFragment.capture()', err)
    message.value = t('device_gyro.axis_err_read')
  } finally {
    busy.value = false
  }
}

const clearSamples = () => {
  samples.value = []
  message.value = ''
}

const saveAxis = async () => {
  if (!fit.value || !fit.value.success) return

  global.clearMessages()
  config.gyro_axis_calibration = { ...fit.value.calibration }
  await config.saveAll()
}

const removeAxis = async () => {
  global.clearMessages()
  config.gyro_axis_calibration = { nx: 0, ny: 0, nz: 0, bx: 0, by: 0, bz: 0 }
  await config.saveAll()
}
</script>
