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
    <p class="h3">{{ t('device_hardware.title') }}</p>
    <hr />

    <BsMessage dismissable="true" message="" alert="info">
      {{ t('device_hardware.voltage_calc_info') }}
      <router-link
        class="link-primary link-offset-2 link-underline-opacity-25 link-underline-opacity-100-hover"
        to="/other/tools"
        >{{ t('device_hardware.voltage_calc_link') }}</router-link
      >
    </BsMessage>

    <form @submit.prevent="save" class="needs-validation" novalidate>
      <div class="row">
        <div class="col-md-6">
          <BsInputNumber
            v-model="config.voltage_factor"
            :label="t('device_hardware.voltage_factor_label')"
            min="0"
            max="6"
            step=".01"
            width="4"
            :unit="voltage"
            :help="t('device_hardware.voltage_factor_help')"
            :disabled="global.disabled"
          >
          </BsInputNumber>
        </div>
        <div class="col-md-6">
          <BsInputNumber
            v-model="config.voltage_config"
            unit="V"
            :label="t('device_hardware.voltage_config_label')"
            min="3"
            max="6"
            step=".01"
            width="4"
            :help="t('device_hardware.voltage_config_help')"
            :disabled="global.disabled"
          ></BsInputNumber>
        </div>

        <div class="col-md-6">
          <BsInputSwitch
            v-model="config.storage_sleep"
            :label="t('device_hardware.storage_sleep_label')"
            :help="t('device_hardware.storage_sleep_help')"
            :disabled="global.disabled"
          ></BsInputSwitch>
        </div>
        <div class="col-md-6" v-if="!global.isEsp8266 && !global.isCuckoo">
          <BsInputSwitch
            v-model="config.charging_pin_enabled"
            :label="t('device_hardware.charging_pin_label')"
            :help="t('device_hardware.charging_pin_help')"
            :disabled="global.disabled || !global.feature.charging"
          ></BsInputSwitch>
        </div>
        <div class="col-md-6">
          <BsInputSwitch
            v-model="config.battery_saving"
            :label="t('device_hardware.battery_saving_label')"
            :help="t('device_hardware.battery_saving_help')"
            :disabled="global.disabled"
          ></BsInputSwitch>
        </div>
        <div class="col-md-12">
          <hr />
        </div>
        <div class="col-md-12">
          <BsInputRadio
            v-model="config.tempsensor_resolution"
            :options="tempsensorResolutionOptions"
            :label="t('device_hardware.ds18b20_label')"
            :help="t('device_hardware.ds18b20_help')"
            :disabled="disableDs18b20"
          ></BsInputRadio>
        </div>
        <div class="col-md-6">
          <BsInputNumber
            v-model="config.temp_adjustment_value"
            :unit="config.temp_unit"
            :label="t('device_hardware.temp_adjustment_label')"
            min="-20"
            max="20"
            step=".01"
            width="4"
            :help="t('device_hardware.temp_adjustment_help')"
            :disabled="global.disabled"
          ></BsInputNumber>
        </div>
        <div class="col-md-6" v-if="!global.isCuckoo">
          <BsInputSwitch
            v-model="config.gyro_temp"
            :label="t('device_hardware.gyro_temp_label')"
            :help="t('device_hardware.gyro_temp_help')"
            :disabled="global.disabled"
          ></BsInputSwitch>
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
            &nbsp;{{ t('device_hardware.save') }}</button
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
            &nbsp;{{ t('device_hardware.restart') }}</button
          >&nbsp;
        </div>
      </div>
    </form>
  </div>
</template>

<script setup>
import { ref, computed, watch, onMounted } from 'vue'
import { useI18n } from 'vue-i18n'
import { validateCurrentForm } from '@mp-se/espframework-ui-components'
import { global, config, status } from '@/modules/pinia'
import { storeToRefs } from 'pinia'

const { t } = useI18n()

const tempsensorResolutionOptions = ref([
  { label: '0.5°C (93 ms)', value: 9 },
  { label: '0.25°C (187 ms)', value: 10 },
  { label: '0.125°C (375 ms)', value: 11 },
  { label: '0.0625°C (850 ms)', value: 12 }
])

const disableDs18b20 = computed(() => {
  return config.gyro_temp || global.disabled
})

const voltage = computed(() => {
  return status.battery + ' V'
})

const { charging_pin_enabled, storage_sleep } = storeToRefs(config)

onMounted(() => {
  if (config.storage_sleep && config.charging_pin_enabled) {
    global.messageWarning(t('device_hardware.warn_sleep_charging_conflict'))
    config.storage_sleep = false
  }
})

watch(storage_sleep, () => {
  if (storage_sleep.value) charging_pin_enabled.value = false
})

watch(charging_pin_enabled, () => {
  if (charging_pin_enabled.value) storage_sleep.value = false
})

const save = async () => {
  if (!validateCurrentForm()) return

  global.clearMessages()
  await config.saveAll()
}

const restart = async () => {
  await config.restart()
}
</script>
