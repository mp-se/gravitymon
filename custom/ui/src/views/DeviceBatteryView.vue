<template>
  <div class="container">
    <p></p>
    <p class="h3">{{ t('cuckoo.battery.title') }}</p>
    <hr />

    <form @submit.prevent="save" class="needs-validation" novalidate>
      <div class="">
        <div class="col-md-6">
          <BsInputNumber
            v-model="config.voltage_factor"
            :label="t('device_hardware.voltage_factor_label')"
            min="0"
            max="6"
            step=".01"
            width="4"
            :unit="status.battery + ' V'"
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
              :hidden="!global.disabled"
            ></span
            >{{ t('cuckoo.battery.save') }}
          </button>
        </div>
      </div>
    </form>
  </div>
</template>

<script setup>
import { useI18n } from 'vue-i18n'
import { validateCurrentForm } from '@mp-se/espframework-ui-components'
import { global, config, status } from '@/modules/pinia'

const { t } = useI18n()

const save = async () => {
  if (!validateCurrentForm()) return

  global.clearMessages()
  await config.saveAll()
}
</script>
