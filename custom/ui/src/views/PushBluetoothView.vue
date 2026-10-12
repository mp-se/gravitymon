<template>
  <div class="container">
    <p></p>
    <p class="h3">{{ t('push_bluetooth.title') }}</p>
    <hr />

    <template v-if="global.feature.ble">
      <form @submit.prevent="save" class="needs-validation" novalidate>
        <div class="row">
          <div class="col-md-9">
            <BsInputSwitch
              v-model="cuckoo.use_ble"
              :label="t('cuckoo.bluetooth.switch_label')"
              :disabled="global.disabled"
            ></BsInputSwitch>
          </div>
          <div class="col-md-9">
            <BsInputNumber
              v-model="cuckoo.ble_sleep_interval"
              :label="t('cuckoo.sleep_interval') + intervalLabel(cuckoo.ble_sleep_interval)"
              unit="s"
              min="30"
              max="3600"
              step="1"
              width="4"
              :help="t('cuckoo.bluetooth.interval_help')"
              :disabled="global.disabled"
            />
          </div>
          <div class="col-md-9">
            <BsInputRadio
              v-model="config.ble_tilt_color"
              :options="bleTiltColorOptions"
              :label="t('push_bluetooth.tilt_color_label')"
              :help="t('push_bluetooth.tilt_color_help')"
              :disabled="tilt"
            />
          </div>
          <div class="col-md-12">
            <BsInputRadio
              v-model="config.ble_format"
              :options="bleFormatOptions"
              :label="t('push_bluetooth.format_label')"
              :help="t('push_bluetooth.format_help')"
              :disabled="global.disabled32"
            />
          </div>
          <div class="col-md-12">
            <p></p>
            <p>{{ t('push_bluetooth.restart_hint') }}</p>
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
              &nbsp;{{ t('push_bluetooth.save') }}
            </button>
          </div>
        </div>
      </form>
    </template>
    <template v-else>
      <div class="row">
        <div class="col-md-12">
          <p>{{ t('push_bluetooth.not_available') }}</p>
        </div>
      </div>
    </template>
  </div>
</template>

<script setup>
import { computed, ref } from 'vue'
import { useI18n } from 'vue-i18n'
import { validateCurrentForm } from '@mp-se/espframework-ui-components'
import { global, config } from '@/modules/pinia'
import { cuckoo, saveCuckooConfig } from '@custom/cuckooConfig'
import { intervalLabel } from '@custom/utils'

const { t } = useI18n()

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

// No "Disabled" entry, the switch above turns Bluetooth on and off. The
// manufacturer's UI also lists a CuckooTilt format (8), which the firmware
// does not know yet, so it is left out here.
const bleFormatOptions = ref([
  { label: 'Tilt iBeacon', value: 1 },
  { label: 'Tilt PRO iBeacon', value: 2 },
  { label: 'RAPT v1', value: 6 },
  { label: 'RAPT v2', value: 7 },
  { label: 'Gravitymon iBeacon', value: 5 },
  { label: 'Gravitymon Eddystone', value: 4 }
])

const tilt = computed(() => {
  if (global.disabled32) return global.disabled32

  if (config.ble_format == 1 || config.ble_format == 2) return false

  return true
})

const save = async () => {
  // The firmware only sends when a format is selected (0 = disabled)
  if (cuckoo.use_ble && config.ble_format == 0) config.ble_format = 1

  if ((config.ble_format == 1 || config.ble_format == 2) && config.ble_tilt_color == '') {
    config.ble_tilt_color = 'red'
  }

  if (!validateCurrentForm()) return

  await saveCuckooConfig()
}
</script>
