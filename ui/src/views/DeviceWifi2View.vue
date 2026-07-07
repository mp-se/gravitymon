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
    <p class="h3">{{ t('device_wifi2.title') }}</p>
    <hr />

    <BsMessage
      v-if="config.wifi_ssid === '' && config.wifi_ssid2 === ''"
      dismissable="true"
      message=""
      alert="warning"
    >
      {{ t('device_wifi2.need_network') }}
    </BsMessage>

    <form @submit.prevent="save" class="needs-validation" novalidate>
      <div class="row">
        <div class="col-md-6">
          <BsInputText
            v-model="config.wifi_ssid"
            :label="t('device_wifi2.ssid1_label')"
            maxlength="30"
            :badge="badge.deviceWifi1Badge()"
            :disabled="global.disabled"
          />
        </div>
        <div class="col-md-6">
          <BsInputText
            v-model="config.wifi_pass"
            type="password"
            maxlength="50"
            :label="t('device_wifi2.pass1_label')"
            :help="t('device_wifi2.pass1_help')"
            :disabled="global.disabled"
          ></BsInputText>
        </div>

        <div class="col-md-6">
          <BsInputText
            v-model="config.wifi_ssid2"
            :label="t('device_wifi2.ssid2_label')"
            maxlength="30"
            :badge="badge.deviceWifi2Badge()"
            :disabled="global.disabled"
          />
        </div>
        <div class="col-md-6">
          <BsInputText
            v-model="config.wifi_pass2"
            type="password"
            maxlength="50"
            :label="t('device_wifi2.pass2_label')"
            :help="t('device_wifi2.pass2_help')"
            :disabled="global.disabled"
          ></BsInputText>
        </div>

        <div class="col-md-12">
          <hr />
        </div>

        <div class="col-md-6">
          <BsInputNumber
            v-model="config.wifi_portal_timeout"
            unit="seconds"
            :label="t('device_wifi2.portal_timeout_label')"
            min="10"
            max="240"
            step="1"
            width="5"
            :help="t('device_wifi2.portal_timeout_help')"
            :disabled="global.disabled"
          ></BsInputNumber>
        </div>
        <div class="col-md-6">
          <BsInputNumber
            v-model="config.wifi_connect_timeout"
            unit="seconds"
            :label="t('device_wifi2.connect_timeout_label')"
            min="1"
            max="60"
            step="1"
            width="5"
            :help="t('device_wifi2.connect_timeout_help')"
            :disabled="global.disabled"
          >
          </BsInputNumber>
        </div>
        <div class="col-md-6" v-if="global.ui.enableScanForStrongestAp">
          <BsInputSwitch
            v-model="config.wifi_scan_ap"
            :label="t('device_wifi2.scan_strongest_label')"
            :help="t('device_wifi2.scan_strongest_help')"
            :disabled="global.disabled"
          >
          </BsInputSwitch>
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
            &nbsp;{{ t('device_wifi2.save') }}</button
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
            &nbsp;{{ t('device_wifi2.restart') }}
          </button>
        </div>
      </div>
    </form>
  </div>
</template>

<script setup>
import { useI18n } from 'vue-i18n'
import { validateCurrentForm } from '@mp-se/espframework-ui-components'
import { global, config } from '@/modules/pinia'
import * as badge from '@/modules/badge'

const { t } = useI18n()

const save = async () => {
  if (!validateCurrentForm()) return

  await config.saveAll()
  global.messageInfo = t('device_wifi2.info_restart_needed')
}

const restart = async () => {
  await config.restart()
}
</script>
