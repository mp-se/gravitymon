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
    <p class="h3">{{ t('push_http_post2.title') }}</p>
    <hr />

    <form
      @submit.prevent="save"
      class="needs-validation"
      novalidate
      :disabled="config.use_wifi_direct"
    >
      <div class="row">
        <div class="col-md-9">
          <BsInputText
            v-model="config.http_post2_target"
            type="url"
            maxlength="120"
            :label="t('push_common.http_url_label')"
            :help="t('push_common.http_url_help')"
            :disabled="pushDisabled"
          />
        </div>
        <div class="col-md-3">
          <BsDropdown
            :label="t('push_common.predefined_urls_label')"
            :button="t('push_common.url_button')"
            :options="httpPostUrlOptions"
            :callback="httpUrlCallback"
            :disabled="pushDisabled"
          />
        </div>
        <div class="col-md-9">
          <BsInputText
            v-model="config.http_post2_header1"
            maxlength="120"
            pattern="(.+): (.+)"
            :label="t('push_common.header1_label')"
            help=""
            :disabled="pushDisabled"
          />
        </div>
        <div class="col-md-3">
          <BsDropdown
            :label="t('push_common.predefined_headers_label')"
            :button="t('push_common.header_button')"
            :options="httpHeaderOptions"
            :callback="httpHeaderH1Callback"
            :disabled="pushDisabled"
          />
        </div>
        <div class="col-md-9">
          <BsInputText
            v-model="config.http_post2_header2"
            maxlength="120"
            pattern="(.+): (.+)"
            :label="t('push_common.header2_label')"
            :help="t('push_common.header2_help')"
            :disabled="pushDisabled"
          />
        </div>
        <div class="col-md-3">
          <BsDropdown
            :label="t('push_common.predefined_headers_label')"
            :button="t('push_common.header_button')"
            :options="httpHeaderOptions"
            :callback="httpHeaderH2Callback"
            :disabled="pushDisabled"
          />
        </div>
        <div class="col-md-6">
          <BsInputNumber
            v-model="config.http_post2_int"
            :label="t('push_common.skip_interval_label')"
            min="0"
            max="5"
            width="4"
            :help="t('push_common.skip_interval_help')"
            :disabled="pushDisabled"
          />
        </div>
      </div>
      <div class="row">
        <div class="col-md-9">
          <BsInputTextAreaFormat
            v-model="config.http_post2_format_gravity"
            rows="6"
            :label="t('push_common.data_format_gravity_label')"
            :help="t('push_common.data_format_help')"
            :disabled="pushDisabled || config.http_post2_gravity === false"
            v-if="global.ui.enableGravity"
          />
        </div>
        <div class="col-md-3" v-if="global.ui.enableGravity && global.ui.enablePressure">
          <BsInputSwitch
            v-model="config.http_post2_gravity"
            :label="t('push_common.enable_gravity_label')"
            :disabled="global.disabled"
          />
          <BsDropdown
            :label="t('push_common.predefined_formats_label')"
            :button="t('push_common.formats_button')"
            :options="gravityHttpPostFormatOptions"
            :callback="gravityHttpFormatCallback"
            :disabled="pushDisabled || config.http_post2_gravity === false"
          />
          <BsModal
            @click="gravityRenderFormat"
            v-model="gravityRender"
            :code="true"
            :json="true"
            :title="t('push_common.format_preview_title')"
            :button="t('push_common.preview_format_button')"
            :disabled="pushDisabled || config.http_post2_gravity === false"
          />
        </div>
        <div class="col-md-3" v-if="global.ui.enableGravity && !global.ui.enablePressure">
          <BsDropdown
            :label="t('push_common.predefined_formats_label')"
            :button="t('push_common.formats_button')"
            :options="gravityHttpPostFormatOptions"
            :callback="gravityHttpFormatCallback"
            :disabled="pushDisabled"
          />
          <BsModal
            @click="gravityRenderFormat"
            v-model="gravityRender"
            :code="true"
            :json="true"
            :title="t('push_common.format_preview_title')"
            :button="t('push_common.preview_format_button')"
            :disabled="pushDisabled"
          />
        </div>
        <div class="col-md-9">
          <BsInputTextAreaFormat
            v-model="config.http_post2_format_pressure"
            rows="6"
            :label="t('push_common.data_format_pressure_label')"
            :help="t('push_common.data_format_help')"
            :disabled="pushDisabled || config.http_post2_pressure === false"
            v-if="global.ui.enablePressure"
          />
        </div>
        <div class="col-md-3" v-if="global.ui.enablePressure && global.ui.enableGravity">
          <BsInputSwitch
            v-model="config.http_post2_pressure"
            :label="t('push_common.enable_pressure_label')"
            :disabled="global.disabled"
          />
          <BsDropdown
            :label="t('push_common.predefined_formats_label')"
            :button="t('push_common.formats_button')"
            :options="pressureHttpPostFormatOptions"
            :callback="pressureHttpFormatCallback"
            :disabled="pushDisabled || config.http_post2_pressure === false"
          />
          <BsModal
            @click="pressureRenderFormat"
            v-model="pressureRender"
            :code="true"
            :json="true"
            :title="t('push_common.format_preview_title')"
            :button="t('push_common.preview_format_button')"
            :disabled="pushDisabled || config.http_post2_pressure === false"
          />
        </div>
        <div class="col-md-3" v-if="global.ui.enablePressure && !global.ui.enableGravity">
          <BsDropdown
            :label="t('push_common.predefined_formats_label')"
            :button="t('push_common.formats_button')"
            :options="pressureHttpPostFormatOptions"
            :callback="pressureHttpFormatCallback"
            :disabled="pushDisabled"
          />
          <BsModal
            @click="pressureRenderFormat"
            v-model="pressureRender"
            :code="true"
            :json="true"
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
import { httpHeaderOptions, httpPostUrlOptions } from '@/modules/utils'
import { gravityHttpPostFormatOptions } from '@/modules/gravityFormatOptions'
import { pressureHttpPostFormatOptions } from '@/modules/pressureFormatOptions'
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
      push_format: 'http_post2_format_gravity'
    }

    global.clearMessages()
    await config.runPushTest(data)
  } catch (error) {
    logError('PushHttpPost2View.runTestGravity()', error)
    global.messageError = t('push_common.err_start_push_test')
  }
}

const runTestPressure = async () => {
  try {
    const data = {
      push_format: 'http_post2_format_pressure'
    }

    global.clearMessages()
    await config.runPushTest(data)
  } catch (error) {
    logError('PushHttpPost2View.runTestPressure()', error)
    global.messageError = t('push_common.err_start_push_test')
  }
}

const httpUrlCallback = (opt) => {
  config.http_post2_target = opt
}

const httpHeaderH1Callback = (opt) => {
  config.http_post2_header1 = opt
}

const httpHeaderH2Callback = (opt) => {
  config.http_post2_header2 = opt
}

const gravityHttpFormatCallback = (opt) => {
  config.http_post2_format_gravity = decodeURIComponent(opt)
}

const pressureHttpFormatCallback = (opt) => {
  config.http_post2_format_pressure = decodeURIComponent(opt)
}

const gravityRenderFormat = () => {
  gravityRender.value = applyTemplate(status, config, config.http_post2_format_gravity)
}

const pressureRenderFormat = () => {
  pressureRender.value = applyTemplate(status, config, config.http_post2_format_pressure)
}

const save = async () => {
  if (!validateCurrentForm()) return

  await config.saveAll()
}
</script>
