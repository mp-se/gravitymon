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
    <p class="h3">Push - Influxdb v2</p>
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
            label="Server"
            help="URL to push target, use format http://servername.com/resource (Supports http and https)"
            :disabled="pushDisabled"
          />
        </div>
        <div class="col-md-6">
          <BsInputText
            v-model="config.influxdb2_org"
            maxlength="50"
            label="Organisation"
            help="Identifier to what organisation to use"
            :disabled="pushDisabled"
          />
        </div>
        <div class="col-md-6">
          <BsInputText
            v-model="config.influxdb2_bucket"
            maxlength="50"
            label="Bucket"
            help="Identifier for the data bucket to use"
            :disabled="pushDisabled"
          />
        </div>
        <div class="col-md-6">
          <BsInputText
            v-model="config.influxdb2_token"
            type="password"
            maxlength="100"
            label="Authentication token"
            help="Authentication token for accessing data bucket"
            :disabled="pushDisabled"
          />
        </div>
        <div class="col-md-6">
          <BsInputNumber
            v-model="config.influxdb2_int"
            label="Skip interval"
            min="0"
            max="5"
            width="4"
            help="Defines how many sleep cycles to skip between pushing data to this target, 1 = every second cycle. Default is 0."
            :disabled="pushDisabled"
          />
        </div>
        <div class="col-md-9">
          <BsInputTextAreaFormat
            v-model="config.influxdb2_format_gravity"
            rows="6"
            label="Data format (gravity)"
            help="Format template used to create the data sent to the remote service"
            :disabled="pushDisabled || config.influxdb2_gravity === false"
            v-if="global.ui.enableGravity"
          />
        </div>
        <div class="col-md-3 gy-2" v-if="global.ui.enableGravity && global.ui.enablePressure">
          <BsInputSwitch
            v-model="config.influxdb2_gravity"
            label="Enable gravity"
            :disabled="global.disabled"
          />
          <BsDropdown
            label="Predefined formats"
            button="Formats"
            :options="gravityInfluxdb2FormatOptions"
            :callback="gravityInfluxdb2FormatCallback"
            :disabled="pushDisabled || config.influxdb2_gravity === false"
          />
          <BsModal
            @click="gravityRenderFormat"
            v-model="gravityRender"
            :code="true"
            title="Format preview"
            button="Preview format"
            :disabled="pushDisabled || config.influxdb2_gravity === false"
          />
        </div>
        <div class="col-md-3 gy-2" v-if="global.ui.enableGravity && !global.ui.enablePressure">
          <BsDropdown
            label="Predefined formats"
            button="Formats"
            :options="gravityInfluxdb2FormatOptions"
            :callback="gravityInfluxdb2FormatCallback"
            :disabled="pushDisabled"
          />
          <BsModal
            @click="gravityRenderFormat"
            v-model="gravityRender"
            :code="true"
            title="Format preview"
            button="Preview format"
            :disabled="pushDisabled"
          />
        </div>
        <div class="col-md-9">
          <BsInputTextAreaFormat
            v-model="config.influxdb2_format_pressure"
            rows="6"
            label="Data format (pressure)"
            help="Format template used to create the data sent to the remote service"
            :disabled="pushDisabled || config.influxdb2_pressure === false"
            v-if="global.ui.enablePressure"
          />
        </div>
        <div class="col-md-3 gy-2" v-if="global.ui.enablePressure && global.ui.enableGravity">
          <BsInputSwitch
            v-model="config.influxdb2_pressure"
            label="Enable pressure"
            :disabled="global.disabled"
          />
          <BsDropdown
            label="Predefined formats"
            button="Formats"
            :options="pressureInfluxdb2FormatOptions"
            :callback="pressureInfluxdb2FormatCallback"
            :disabled="pushDisabled || config.influxdb2_pressure === false"
          />
          <BsModal
            @click="pressureRenderFormat"
            v-model="pressureRender"
            :code="true"
            title="Format preview"
            button="Preview format"
            :disabled="pushDisabled || config.influxdb2_pressure === false"
          />
        </div>
        <div class="col-md-3 gy-2" v-if="global.ui.enablePressure && !global.ui.enableGravity">
          <BsDropdown
            label="Predefined formats"
            button="Formats"
            :options="pressureInfluxdb2FormatOptions"
            :callback="pressureInfluxdb2FormatCallback"
            :disabled="pushDisabled"
          />
          <BsModal
            @click="pressureRenderFormat"
            v-model="pressureRender"
            :code="true"
            title="Format preview"
            button="Preview format"
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
            &nbsp;Save</button
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
            &nbsp;Run push gravity test</button
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
            &nbsp;Run push pressure test
          </button>
        </div>
      </div>
    </form>
  </div>
</template>

<script setup>
import { ref, computed } from 'vue'
import { gravityInfluxdb2FormatOptions } from '@/modules/gravityFormatOptions'
import { pressureInfluxdb2FormatOptions } from '@/modules/pressureFormatOptions'
import { applyTemplate } from '@/modules/formatTemplate'
import { validateCurrentForm } from '@mp-se/espframework-ui-components'
import { global, status, config } from '@/modules/pinia'
import { logError } from '@mp-se/espframework-ui-components'

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
    global.messageError = 'Failed to start push test'
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
    global.messageError = 'Failed to start push test'
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
