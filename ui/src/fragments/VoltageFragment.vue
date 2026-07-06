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
  <h5>Calculate a new voltage factor</h5>
  <div class="row">
    <div class="col-md-4">
      <BsInputNumber
        v-model="measuredVoltage"
        label="Measured voltage"
        min="0"
        max="6"
        step=".01"
        width="4"
        unit="V"
        help="Enter the measured voltage on the device"
        :disabled="global.disabled"
      >
      </BsInputNumber>
    </div>
    <div class="col-md-4">
      <BsInputReadonly
        v-model="status.battery"
        unit="V"
        label="Last voltage reading"
        width="4"
        help="Last measured battery voltage"
        :disabled="global.disabled"
      ></BsInputReadonly>
    </div>
    <div class="col-md-4">
      <BsInputReadonly
        v-model="config.voltage_factor"
        label="Current voltage factor"
        width="4"
        help="Current voltage factor"
        :disabled="global.disabled"
      ></BsInputReadonly>
    </div>
  </div>

  <div class="row gy-4">
    <div class="col-md-12"></div>
    <div class="col-md-3">
      <button
        @click="calculateFactor"
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
        &nbsp;Calculate factor
      </button>
    </div>
  </div>
</template>

<script setup>
import { ref } from 'vue'
import { global, config, status, saveConfigState } from '@/modules/pinia'
import { logDebug } from '@mp-se/espframework-ui-components'

const measuredVoltage = ref(0)

const calculateFactor = () => {
  global.disabled = true
  global.clearMessages()

  var mv = parseFloat(measuredVoltage.value)

  if (isNaN(mv)) {
    global.messageError = 'Not a valid measurement'
    return
  }

  config.voltage_factor = parseFloat(mv / (status.battery / config.voltage_factor)).toFixed(2)
  ;(async () => {
    const success = await config.sendConfig()
    logDebug('VoltageFragment.calculateFactor()', success)
    saveConfigState()
    global.disabled = true

    setTimeout(async () => {
      const s2 = await status.load()
      logDebug('VoltageFragment.calculateFactor()', s2, status.battery)
      global.messageInfo = 'New factor applied, check if the current battery reading is correct'
      global.disabled = false
    }, 1000)
  })()
}
</script>
