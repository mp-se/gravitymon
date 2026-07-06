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
  <h5>{{ t('fragment_voltage.title') }}</h5>
  <div class="row">
    <div class="col-md-4">
      <BsInputNumber
        v-model="measuredVoltage"
        :label="t('fragment_voltage.measured_label')"
        min="0"
        max="6"
        step=".01"
        width="4"
        unit="V"
        :help="t('fragment_voltage.measured_help')"
        :disabled="global.disabled"
      >
      </BsInputNumber>
    </div>
    <div class="col-md-4">
      <BsInputReadonly
        v-model="status.battery"
        unit="V"
        :label="t('fragment_voltage.last_label')"
        width="4"
        :help="t('fragment_voltage.last_help')"
        :disabled="global.disabled"
      ></BsInputReadonly>
    </div>
    <div class="col-md-4">
      <BsInputReadonly
        v-model="config.voltage_factor"
        :label="t('fragment_voltage.current_label')"
        width="4"
        :help="t('fragment_voltage.current_help')"
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
        &nbsp;{{ t('fragment_voltage.calculate_button') }}
      </button>
    </div>
  </div>
</template>

<script setup>
import { ref } from 'vue'
import { useI18n } from 'vue-i18n'
import { global, config, status, saveConfigState } from '@/modules/pinia'
import { logDebug } from '@mp-se/espframework-ui-components'

const { t } = useI18n()
const measuredVoltage = ref(0)

const calculateFactor = () => {
  global.disabled = true
  global.clearMessages()

  var mv = parseFloat(measuredVoltage.value)

  if (isNaN(mv)) {
    global.messageError = t('fragment_voltage.err_invalid')
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
      global.messageInfo = t('fragment_voltage.info_new_factor')
      global.disabled = false
    }, 1000)
  })()
}
</script>
