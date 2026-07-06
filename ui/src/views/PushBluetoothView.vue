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
    <p class="h3">Push - Bluetooth</p>
    <hr />

    <template v-if="global.feature.ble">
      <form @submit.prevent="save" class="needs-validation" novalidate>
        <div class="row">
          <div class="col-md-12">
            <BsInputRadio
              v-model="config.ble_tilt_color"
              :options="bleTiltColorOptions"
              label="Tilt color"
              help="Tilt color beacon. Only used when tilt type is selected."
              :disabled="tilt"
            />
          </div>
          <div class="col-md-12">
            <BsInputRadio
              v-model="config.ble_format"
              :options="bleFormatOptions"
              label="Bluetooth data format"
              help="Select the type of bluetooth transmission used."
              :disabled="global.disabled32"
            />
          </div>
          <div class="col-md-12">
            <p></p>
            <p>Changing bluetooth settings might require a restart to function properly</p>
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
              :disabled="global.disabled32 || !global.configChanged"
            >
              <span
                class="spinner-border spinner-border-sm"
                role="status"
                aria-hidden="true"
                :hidden="!global.disabled32"
              ></span>
              &nbsp;Save
            </button>
          </div>
        </div>
      </form>
    </template>
    <template v-else>
      <div class="row">
        <div class="col-md-12">
          <p>Bluetooth is not available on this platform</p>
        </div>
      </div>
    </template>
  </div>
</template>

<script setup>
import { ref, computed } from 'vue'
import { validateCurrentForm } from '@mp-se/espframework-ui-components'
import { global, config } from '@/modules/pinia'

const bleTiltColorOptions = ref([
  { label: 'red', value: 'red' },
  { label: 'green', value: 'green' },
  { label: 'black', value: 'black' },
  { label: 'purple', value: 'purple' },
  { label: 'orange', value: 'orange' },
  { label: 'blue', value: 'blue' },
  { label: 'yellow', value: 'yellow' },
  { label: 'pink', value: 'pink' }
])

const bleFormatOptions = ref([
  { label: 'Disabled', value: 0 },
  { label: 'Tilt iBeacon', value: 1 },
  { label: 'Tilt PRO iBeacon', value: 2 },
  { label: 'Gravitymon iBeacon', value: 5 },
  // { label: 'Gravitymon Service', value: 3 },
  { label: 'Gravitymon Eddystone', value: 4 },
  { label: 'RAPT v1', value: 6 },
  { label: 'RAPT v2', value: 7 }
])

const tilt = computed(() => {
  if (global.disabled32) return global.disabled32

  if (config.ble_format == 1 || config.ble_format == 2) return false

  return true
})

const save = async () => {
  if ((config.ble_format == 1 || config.ble_format == 2) && config.ble_tilt_color == '') {
    config.ble_tilt_color = 'red'
  }

  if (!validateCurrentForm()) return

  global.clearMessages()
  await config.saveAll()
}
</script>
