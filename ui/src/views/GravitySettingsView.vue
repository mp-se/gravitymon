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
    <p class="h2">{{ t('gravity_settings.title') }}</p>
    <hr />

    <form @submit.prevent="save" class="needs-validation" novalidate>
      <div class="row">
        <div class="col-md-6">
          <BsInputSwitch
            v-model="config.gravity_temp_adjustment"
            :label="t('gravity_settings.temp_adjust_label')"
            :help="t('gravity_settings.temp_adjust_help')"
            :disabled="global.disabled"
          ></BsInputSwitch>
        </div>
        <div class="col-md-6">
          <BsInputNumber
            v-model="config.formula_calibration_temp"
            :unit="'°' + config.temp_unit"
            :label="t('gravity_settings.cal_temp_label')"
            min="0"
            max="100"
            step=".01"
            width="4"
            :help="t('gravity_settings.cal_temp_help')"
            :disabled="calTempAdj"
          ></BsInputNumber>
        </div>
        <div class="col-md-6">
          <BsInputSwitch
            v-model="config.ignore_low_angles"
            :label="t('gravity_settings.ignore_low_label')"
            :help="t('gravity_settings.ignore_low_help')"
            :disabled="global.disabled"
          ></BsInputSwitch>
        </div>
        <div class="col-md-6">
          <BsInputNumber
            v-model="config.gyro_read_count"
            :label="t('gravity_settings.gyro_reads_label')"
            min="10"
            max="100"
            width="4"
            :help="t('gravity_settings.gyro_reads_help')"
            :disabled="global.disabled"
          ></BsInputNumber>
        </div>
        <div class="col-md-6">
          <BsInputNumber
            v-model="config.gyro_moving_threashold"
            :label="t('gravity_settings.gyro_threshold_label')"
            min="50"
            max="1000"
            width="4"
            :help="t('gravity_settings.gyro_threshold_help')"
            :disabled="global.disabled"
          ></BsInputNumber>
        </div>
      </div>
      <div class="row gy-2">
        <div class="col-md-12">
          <hr />
        </div>
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
            &nbsp;{{ t('gravity_settings.save') }}
          </button>
        </div>
      </div>
    </form>
  </div>
</template>

<script setup>
import { computed } from 'vue'
import { useI18n } from 'vue-i18n'
import { validateCurrentForm } from '@mp-se/espframework-ui-components'
import { global, config } from '@/modules/pinia'

const { t } = useI18n()

const calTempAdj = computed(() => {
  return !config.gravity_temp_adjustment || global.disabled
})

const save = async () => {
  if (!validateCurrentForm()) return

  await config.saveAll()
}
</script>
