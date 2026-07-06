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
    <p class="h2">Gravity - Settings</p>
    <hr />

    <form @submit.prevent="save" class="needs-validation" novalidate>
      <div class="row">
        <div class="col-md-6">
          <BsInputSwitch
            v-model="config.gravity_temp_adjustment"
            label="Temperature adjust gravity"
            help="Adjust the calculated gravity based on the current temperature. Assumes that calibration is done using 20C / 68F"
            :disabled="global.disabled"
          ></BsInputSwitch>
        </div>
        <div class="col-md-6">
          <BsInputNumber
            v-model="config.formula_calibration_temp"
            :unit="'°' + config.temp_unit"
            label="Gravity calibration temp"
            min="0"
            max="100"
            step=".01"
            width="4"
            help="Calibration temperature, used in temperatur correction formula, default 20C/68F"
            :disabled="calTempAdj"
          ></BsInputNumber>
        </div>
        <div class="col-md-6">
          <BsInputSwitch
            v-model="config.ignore_low_angles"
            label="Ignore low angles"
            help="When active, angles below water will be ignored. Note! Angle must be defined under calibration, first field."
            :disabled="global.disabled"
          ></BsInputSwitch>
        </div>
        <div class="col-md-6">
          <BsInputNumber
            v-model="config.gyro_read_count"
            label="Gyro reads"
            min="10"
            max="100"
            width="4"
            help="How many times should we read the gyro to get an accurate angle. More reads = better accuracy but higher battery drain"
            :disabled="global.disabled"
          ></BsInputNumber>
        </div>
        <div class="col-md-6">
          <BsInputNumber
            v-model="config.gyro_moving_threashold"
            label="Gyro moving threshold"
            min="50"
            max="1000"
            width="4"
            help="How much deviation between gyro reads are acceptable in order to regard this as a valid angle"
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
            &nbsp;Save
          </button>
        </div>
      </div>
    </form>
  </div>
</template>

<script setup>
import { computed } from 'vue'
import { validateCurrentForm } from '@mp-se/espframework-ui-components'
import { global, config } from '@/modules/pinia'

const calTempAdj = computed(() => {
  return !config.gravity_temp_adjustment || global.disabled
})

const save = async () => {
  if (!validateCurrentForm()) return

  await config.saveAll()
}
</script>
