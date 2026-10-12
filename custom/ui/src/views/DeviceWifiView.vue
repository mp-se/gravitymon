<template>
  <div class="container">
    <p></p>
    <p class="h3">{{ t('cuckoo.wifi.title') }}</p>
    <hr />

    <BsMessage
      v-if="scanning"
      :dismissable="false"
      :message="t('device_wifi.scanning')"
      alert="info"
    >
    </BsMessage>

    <BsMessage
      v-if="config.wifi_ssid === '' && config.wifi_ssid2 === ''"
      :dismissable="true"
      :message="t('device_wifi.need_network')"
      alert="warning"
    >
    </BsMessage>

    <form @submit.prevent="save" class="needs-validation" novalidate>
      <div class="row">
        <div class="col-md-9">
          <BsSelect
            v-model="config.wifi_ssid"
            :label="t('cuckoo.wifi.ssid_label')"
            width="6"
            :options="networks"
            :badge="badge.deviceWifi1Badge()"
            :disabled="global.disabled"
          />

          <BsInputText
            v-model="config.wifi_pass"
            type="password"
            width="6"
            maxlength="50"
            :label="t('cuckoo.wifi.pass_label')"
            :help="t('device_wifi.pass1_help')"
            :disabled="global.disabled"
          ></BsInputText>

          <div class="has-validation pt-2">
            <label class="form-label fw-bold"
              >{{ t('cuckoo.wifi.device_id') }}: {{ status.id }}</label
            >
          </div>
        </div>
      </div>

      <div class="row gy-2">
        <div class="col-md-12">
          <hr />
        </div>

        <div class="col-md-9">
          <details>
            <summary class="pt-2 fw-bold" style="cursor: pointer">
              {{ t('cuckoo.wifi.direct_title') }}
            </summary>
            <div>
              <p>{{ t('push_wifi_direct.intro') }}</p>

              <BsInputText
                v-model="config.wifi_direct_ssid"
                :label="t('push_wifi_direct.ssid_label')"
                :help="t('push_wifi_direct.ssid_help')"
                :disabled="global.disabled"
              />

              <BsInputText
                v-model="config.wifi_direct_pass"
                type="password"
                maxlength="50"
                :label="t('push_wifi_direct.pass_label')"
                :help="t('push_wifi_direct.pass_help')"
                :disabled="global.disabled"
              ></BsInputText>

              <BsInputSwitch
                v-model="config.use_wifi_direct"
                :label="t('push_wifi_direct.use_label')"
                :help="t('push_wifi_direct.use_help')"
                :disabled="global.disabled"
              ></BsInputSwitch>
            </div>
          </details>
        </div>
      </div>

      <div class="row gy-2">
        <div class="col-md-12">
          <hr />
        </div>
        <div class="col-md-12">
          <button type="button" class="btn btn-primary" @click="copy(status.id)">
            {{ copied ? t('cuckoo.home.copied') : t('cuckoo.home.copy_id') }}</button
          >&nbsp;

          <button
            type="submit"
            class="btn btn-primary w-2"
            :disabled="global.disabled || !global.configChanged"
          >
            <span
              class="spinner-border spinner-border-sm"
              role="status"
              aria-hidden="true"
              :hidden="!global.disabled"
            ></span>
            &nbsp;{{ t('device_wifi.save') }}</button
          >&nbsp;

          <button
            @click="config.restart()"
            type="button"
            class="btn btn-secondary"
            :disabled="global.disabled"
          >
            <span
              class="spinner-border spinner-border-sm"
              role="status"
              aria-hidden="true"
              :hidden="!global.disabled"
            ></span>
            &nbsp;{{ t('device_wifi.restart') }}
          </button>
        </div>
      </div>
    </form>
  </div>
</template>

<script setup>
import { onMounted, ref } from 'vue'
import { useI18n } from 'vue-i18n'
import { validateCurrentForm } from '@mp-se/espframework-ui-components'
import { config, global, status } from '@/modules/pinia'
import * as badge from '@/modules/badge'
import { useCopied } from '@custom/utils'

const { t } = useI18n()
const scanning = ref(false)
const networks = ref([])
const { copied, copy } = useCopied()

function wifiName(label, rssi, encr) {
  let l = label
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
    networks.value = [{ label: '-blank-', value: '', rssi: 0, encryption: 0, channel: 0 }]
    for (const d of res.data.networks) {
      if (networks.value.some((n) => n.value === d.wifi_ssid)) continue
      networks.value.push({
        label: wifiName(d.wifi_ssid, d.rssi, d.encryption),
        value: d.wifi_ssid,
        rssi: d.rssi,
        encryption: d.encryption,
        channel: d.channel
      })
    }
  }
  scanning.value = false
})

const save = async () => {
  if (!validateCurrentForm()) return

  await config.saveAll()
  global.messageInfo = t('device_wifi.info_restart_needed')
}
</script>
