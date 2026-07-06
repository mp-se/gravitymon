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
    <p class="h3">Push - Wifi Direct</p>
    <hr />

    <form @submit.prevent="save" class="needs-validation" novalidate>
      <div class="row">
        <div class="col-md-12">
          <p>
            Using the WIFI direct feature means that the device will connect to the AP and send data
            using HTTP post to the Gravitymon Gateway.
          </p>
        </div>

        <div class="col-md-6">
          <BsInputText
            v-model="config.wifi_direct_ssid"
            label="Direct SSID"
            help="Enter the SSID for the wifi direct functionallity"
            :disabled="global.disabled"
          />
        </div>
        <div class="col-md-6">
          <BsInputText
            v-model="config.wifi_direct_pass"
            type="password"
            maxlength="50"
            label="Direct Password"
            help="Enter password for the wifi direct network"
            :disabled="global.disabled"
          ></BsInputText>
        </div>

        <div class="col-md-6">
          <BsInputSwitch
            v-model="config.use_wifi_direct"
            label="Use wifi direct in gravity mode"
            help="In gravity mode the wifi direct SSID/Password will be used for connection"
            :disabled="global.disabled"
          ></BsInputSwitch>
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
import { validateCurrentForm } from '@mp-se/espframework-ui-components'
import { global, config } from '@/modules/pinia'

const save = async () => {
  if (!validateCurrentForm()) return

  if (config.use_wifi_direct) {
    if (config.wifi_direct_ssid === '' || config.wifi_direct_pass === '') {
      global.messageError = 'SSID and Password are required when using wifi direct'
      return
    }
  }

  await config.saveAll()
}
</script>
