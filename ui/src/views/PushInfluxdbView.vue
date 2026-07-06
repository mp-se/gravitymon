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
    <p class="h3">{{ t('push_influxdb.title') }}</p>
    <hr />

    <form
      @submit.prevent="save"
      class="needs-validation"
      novalidate
      :disabled="config.use_wifi_direct"
    >
      <div class="row">
        <div class="col-md-12">
          <BsInputText
            v-model="config.influxdb2_target"
            type="url"
            maxlength="120"
            :label="t('push_common.server_label')"
            :help="t('push_common.http_url_help')"
            :disabled="pushDisabled"
          />
        </div>
        <div class="col-md-6">
          <BsInputText
            v-model="config.influxdb2_org"
            maxlength="50"
            :label="t('push_influxdb.org_label')"
            :help="t('push_influxdb.org_help')"
            :disabled="pushDisabled"
          />
        </div>
        <div class="col-md-6">
          <BsInputText
            v-model="config.influxdb2_bucket"
            maxlength="50"
            :label="t('push_influxdb.bucket_label')"
            :help="t('push_influxdb.bucket_help')"
            :disabled="pushDisabled"
          />
        </div>
        <div class="col-md-6">
          <BsInputText
            v-model="config.influxdb2_token"
            type="password"
            maxlength="100"
            :label="t('push_influxdb.token_label')"
            :help="t('push_influxdb.token_help')"
            :disabled="pushDisabled"
          />
        </div>
        <div class="col-md-6">
          <BsInputNumber
            v-model="config.influxdb2_int"
            :label="t('push_common.skip_interval_label')"
            min="0"
            max="5"
            width="4"
            :help="t('push_common.skip_interval_help')"
            :disabled="pushDisabled"
          />
        </div>
        <div class="col-md-9">
          <BsInputTextAreaFormat
            v-model="config.influxdb2_format_gravity"
            rows="6"
            :label="t('push_common.data_format_gravity_label')"
            :help="t('push_common.data_format_help')"
            :disabled="pushDisabled || config.influxdb2_gravity === false"
            v-if="global.ui.enableGravity"
          />
        </div>
        <div class="col-md-3 gy-2" v-if="global.ui.enableGravity && global.ui.enablePressure">
          <BsInputSwitch
            v-model="config.influxdb2_gravity"
            :label="t('push_common.enable_gravity_label')"
            :disabled="global.disabled"
          />
          <BsDropdown
            :label="t('push_common.predefined_formats_label')"
            :button="t('push_common.formats_button')"
            :options="gravityInfluxdb2FormatOptions"
            :callback="gravityInfluxdb2FormatCallback"
            :disabled="pushDisabled || config.influxdb2_gravity === false"
          />
          <BsModal
            @click="gravityRenderFormat"
            v-model="gravityRender"
            :code="true"
            :title="t('push_common.format_preview_title')"
            :button="t('push_common.preview_format_button')"
            :disabled="pushDisabled || config.influxdb2_gravity === false"
          />
        </div>
        <div class="col-md-3 gy-2" v-if="global.ui.enableGravity && !global.ui.enablePressure">
          <BsDropdown
            :label="t('push_common.predefined_formats_label')"
            :button="t('push_common.formats_button')"
            :options="gravityInfluxdb2FormatOptions"
            :callback="gravityInfluxdb2FormatCallback"
            :disabled="pushDisabled"
          />
          <BsModal
            @click="gravityRenderFormat"
            v-model="gravityRender"
            :code="true"
            :title="t('push_common.format_preview_title')"
            :button="t('push_common.preview_format_button')"
            :disabled="pushDisabled"
          />
        </div>
        <div class="col-md-9">
          <BsInputTextAreaFormat
            v-model="config.influxdb2_format_pressure"
            rows="6"
            :label="t('push_common.data_format_pressure_label')"
            :help="t('push_common.data_format_help')"
            :disabled="pushDisabled || config.influxdb2_pressure === false"
            v-if="global.ui.enablePressure"
          />
        </div>
        <div class="col-md-3 gy-2" v-if="global.ui.enablePressure && global.ui.enableGravity">
          <BsInputSwitch
            v-model="config.influxdb2_pressure"
            :label="t('push_common.enable_pressure_label')"
            :disabled="global.disabled"
          />
          <BsDropdown
            :label="t('push_common.predefined_formats_label')"
            :button="t('push_common.formats_button')"
            :options="pressureInfluxdb2FormatOptions"
            :callback="pressureInfluxdb2FormatCallback"
            :disabled="pushDisabled || config.influxdb2_pressure === false"
          />
          <BsModal
            @click="pressureRenderFormat"
            v-model="pressureRender"
            :code="true"
            :title="t('push_common.format_preview_title')"
            :button="t('push_common.preview_format_button')"
            :disabled="pushDisabled || config.influxdb2_pressure === false"
          />
        </div>
        <div class="col-md-3 gy-2" v-if="global.ui.enablePressure && !global.ui.enableGravity">
          <BsDropdown
            :label="t('push_common.predefined_formats_label')"
            :button="t('push_common.formats_button')"
            :options="pressureInfluxdb2FormatOptions"
            :callback="pressureInfluxdb2FormatCallback"
            :disabled="pushDisabled"
          />
          <BsModal
            @click="pressureRenderFormat"
            v-model="pressureRender"
            :code="true"
            :title="t('push_common.format_preview_title')"
            :button="t('push_common.preview_format_button')"
            :disabled="pushDisabled"
          />
        </div>
      </div>
      <div class="row gy-2">
        <div class="col-md-12">
          <hr />
        </div>
        <div class="col-sm-12">
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
            &nbsp;{{ t('push_common.save') }}</button
          >&nbsp;

          <button
            v-if="global.ui.enableGravity"
            @click="runTestGravity"
            type="button"
            class="btn btn-secondary"
            :disabled="pushDisabled"
          >
            <span
              class="spinner-border spinner-border-sm"
              role="status"
              aria-hidden="true"
              :hidden="!global.disabled"
            ></span>
            &nbsp;{{ t('push_common.run_push_gravity_test') }}</button
          >&nbsp;
          <button
            v-if="global.ui.enablePressure"
            @click="runTestPressure"
            type="button"
            class="btn btn-secondary"
            :disabled="pushDisabled"
          >
            <span
              class="spinner-border spinner-border-sm"
              role="status"
              aria-hidden="true"
              :hidden="!global.disabled"
            ></span>
            &nbsp;{{ t('push_common.run_push_pressure_test') }}
          </button>
        </div>
      </div>
    </form>
  </div>
</template>

<script setup>
import { ref, computed } from 'vue'
import { useI18n } from 'vue-i18n'
import { gravityInfluxdb2FormatOptions } from '@/modules/gravityFormatOptions'
import { pressureInfluxdb2FormatOptions } from '@/modules/pressureFormatOptions'
import { applyTemplate } from '@/modules/formatTemplate'
import { validateCurrentForm } from '@mp-se/espframework-ui-components'
import { global, status, config } from '@/modules/pinia'
import { logError } from '@mp-se/espframework-ui-components'

const { t } = useI18n()
const gravityRender = ref('')
const pressureRender = ref('')

const pushDisabled = computed(() => {
  return global.disabled || config.use_wifi_direct
})

const runTestGravity = async () => {
  try {
    const data = {
      push_format: 'influxdb2_format_gravity'
    }

    global.clearMessages()
    await config.runPushTest(data)
  } catch (error) {
    logError('PushInfluxdbView.runTestGravity()', error)
    global.messageError = t('push_common.err_start_push_test')
  }
}

const runTestPressure = async () => {
  try {
    const data = {
      push_format: 'influxdb2_format_pressure'
    }

    global.clearMessages()
    await config.runPushTest(data)
  } catch (error) {
    logError('PushInfluxdbView.runTestPressure()', error)
    global.messageError = t('push_common.err_start_push_test')
  }
}

const gravityInfluxdb2FormatCallback = (opt) => {
  config.influxdb2_format_gravity = decodeURIComponent(opt)
}

const pressureInfluxdb2FormatCallback = (opt) => {
  config.influxdb2_format_pressure = decodeURIComponent(opt)
}

const gravityRenderFormat = () => {
  gravityRender.value = applyTemplate(status, config, config.influxdb2_format_gravity)
}

const pressureRenderFormat = () => {
  pressureRender.value = applyTemplate(status, config, config.influxdb2_format_pressure)
}

const save = async () => {
  if (!validateCurrentForm()) return

  await config.saveAll()
}
</script>
