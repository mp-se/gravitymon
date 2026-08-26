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
    <p class="h3">{{ t('device_wifi.title') }}</p>
    <hr />

    <BsMessage
      v-if="scanning"
      :dismissable="false"
      :message="t('device_wifi.scanning')"
      alert="info"
    >
    </BsMessage>

    <template v-if="global.ui.enableManualWifiEntry && !scanning">
      <BsMessage dismissable="true" message="" alert="info">
        {{ t('device_wifi.manual_entry_info') }}
        <router-link
          class="link-primary link-offset-2 link-underline-opacity-25 link-underline-opacity-100-hover"
          to="/device/wifi2"
          >{{ t('device_wifi.manual_entry_link') }}</router-link
        >
      </BsMessage>
    </template>

    <BsMessage
      v-if="config.wifi_ssid === '' && config.wifi_ssid2 === ''"
      dismissable="true"
      message=""
      alert="warning"
    >
      {{ t('device_wifi.need_network') }}
    </BsMessage>

    <form @submit.prevent="save" class="needs-validation" novalidate>
      <div class="row">
        <div class="col-md-6">
          <BsSelect
            v-model="config.wifi_ssid"
            :label="t('device_wifi.ssid1_label')"
            :options="networks"
            :badge="badge.deviceWifi1Badge()"
            :disabled="global.disabled"
          />
        </div>
        <div class="col-md-6">
          <BsInputText
            v-model="config.wifi_pass"
            type="password"
            maxlength="50"
            :label="t('device_wifi.pass1_label')"
            :help="t('device_wifi.pass1_help')"
            :disabled="global.disabled"
          ></BsInputText>
        </div>

        <div class="col-md-6">
          <BsSelect
            v-model="config.wifi_ssid2"
            :label="t('device_wifi.ssid2_label')"
            :options="networks"
            :badge="badge.deviceWifi2Badge()"
            :disabled="global.disabled"
          />
        </div>
        <div class="col-md-6">
          <BsInputText
            v-model="config.wifi_pass2"
            type="password"
            maxlength="50"
            :label="t('device_wifi.pass2_label')"
            :help="t('device_wifi.pass2_help')"
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
            :label="t('device_wifi.portal_timeout_label')"
            min="10"
            max="240"
            step="1"
            width="5"
            :help="t('device_wifi.portal_timeout_help')"
            :disabled="global.disabled"
          ></BsInputNumber>
        </div>
        <div class="col-md-6">
          <BsInputNumber
            v-model="config.wifi_connect_timeout"
            unit="seconds"
            :label="t('device_wifi.connect_timeout_label')"
            min="1"
            max="60"
            step="1"
            width="5"
            :help="t('device_wifi.connect_timeout_help')"
            :disabled="global.disabled"
          >
          </BsInputNumber>
        </div>
        <div class="col-md-6" v-if="global.ui.enableScanForStrongestAp">
          <BsInputSwitch
            v-model="config.wifi_scan_ap"
            :label="t('device_wifi.scan_strongest_label')"
            :help="t('device_wifi.scan_strongest_help')"
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
            &nbsp;{{ t('device_wifi.save') }}</button
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
            &nbsp;{{ t('device_wifi.restart') }}
          </button>
        </div>
      </div>
    </form>
  </div>
</template>

<script setup>
import { validateCurrentForm } from '@mp-se/espframework-ui-components'
import { global, config } from '@/modules/pinia'
import * as badge from '@/modules/badge'
import { onMounted, ref } from 'vue'
import { useI18n } from 'vue-i18n'
import { logDebug } from '@mp-se/espframework-ui-components'

const { t } = useI18n()
const scanning = ref(false)
const networks = ref([])

function wifiName(label, rssi, encr) {
  var l = label
  if (encr) l += ' \u{1f512}'
  if (rssi > -50) l += t('device_wifi.wifi_signal_excellent')
  else if (rssi > -60) l += t('device_wifi.wifi_signal_good')
  else if (rssi > -67) l += t('device_wifi.wifi_signal_minimum')
  else l += t('device_wifi.wifi_signal_poor')
  return l
}

onMounted(async () => {
  scanning.value = true
  const res = await config.runWifiScan()
  if (res && res.success) {
    const data = res.data
    networks.value = [{ label: '-blank-', value: '', rssi: 0, encryption: 0, channel: 0 }]
    for (var n in data.networks) {
      var d = data.networks[n]
      var o = {
        label: wifiName(d.wifi_ssid, d.rssi, d.encryption),
        value: d.wifi_ssid,
        rssi: d.rssi,
        encryption: data.networks[n].encryption,
        channel: d.channel
      }

      var f = networks.value.filter((obj) => {
        return obj.value === d.wifi_ssid
      })
      logDebug('DeviceWifiView.onMounted()', 'result:', f, d.wifi_ssid)
      if (f.length === 0) networks.value.push(o)
    }
  }
  scanning.value = false
})

const save = async () => {
  if (!validateCurrentForm()) return

  await config.saveAll()
  global.messageInfo = t('device_wifi.info_restart_needed')
}

const restart = async () => {
  await config.restart()
}
</script>
