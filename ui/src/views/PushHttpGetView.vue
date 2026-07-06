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
    <p class="h3">Push - HTTP Get</p>
    <hr />

    <form @submit.prevent="save" class="needs-validation" novalidate>
      <div class="row">
        <div class="col-md-9">
          <BsInputText
            v-model="config.http_get_target"
            type="url"
            maxlength="120"
            label="HTTP URL"
            help="URL to push target, use format http://servername.com/resource (Supports http and https)"
            :disabled="pushDisabled"
          />
        </div>
        <div class="col-md-3">
          <BsDropdown
            label="Predefined URLs"
            button="URL"
            :options="httpGetUrlOptions"
            :callback="httpUrlCallback"
            :disabled="pushDisabled"
          />
        </div>
        <div class="col-md-9">
          <BsInputText
            v-model="config.http_get_header1"
            maxlength="120"
            pattern="(.+): (.+)"
            label="HTTP Header #1"
            help=""
            :disabled="pushDisabled"
          />
        </div>
        <div class="col-md-3">
          <BsDropdown
            label="Predefined headers"
            button="Header"
            :options="httpHeaderOptions"
            :callback="httpHeaderH1Callback"
            :disabled="pushDisabled"
          />
        </div>
        <div class="col-md-9">
          <BsInputText
            v-model="config.http_get_header2"
            maxlength="120"
            pattern="(.+): (.+)"
            label="HTTP Header #2"
            help="Set a http headers, empty string is skipped, example: Content-Type: application/json"
            :disabled="pushDisabled"
          />
        </div>
        <div class="col-md-3">
          <BsDropdown
            label="Predefined headers"
            button="Header"
            :options="httpHeaderOptions"
            :callback="httpHeaderH2Callback"
            :disabled="pushDisabled"
          />
        </div>
        <div class="col-md-6">
          <BsInputNumber
            v-model="config.http_get_int"
            label="Skip interval"
            min="0"
            max="5"
            width="4"
            help="Defines how many sleep cycles to skip between pushing data to this target, 1 = every second cycle. Default is 0."
            :disabled="pushDisabled"
          />
        </div>

        <!-- GRAVITY SECTION -->
        <div v-if="global.ui.enableGravity" class="col-md-9">
          <BsInputTextAreaFormat
            v-model="config.http_get_format_gravity"
            rows="6"
            label="Data format (gravity)"
            help="Format template used to create the data sent to the remote service"
            :disabled="pushDisabled || config.http_get_gravity === false"
          />
        </div>
        <div v-if="global.ui.enableGravity && global.ui.enablePressure" class="col-md-3">
          <BsInputSwitch
            v-model="config.http_get_gravity"
            label="Enable gravity"
            :disabled="global.disabled"
          />
          <BsDropdown
            label="Predefined formats"
            button="Formats"
            :options="gravityHttpGetFormatOptions"
            :callback="gravityHttpFormatCallback"
            :disabled="pushDisabled || config.http_get_gravity === false"
          />
          <BsModal
            @click="gravityRenderFormat"
            v-model="gravityRender"
            :code="true"
            title="Format preview"
            button="Preview format"
            :disabled="pushDisabled || config.http_get_gravity === false"
          />
        </div>
        <div v-if="global.ui.enableGravity && !global.ui.enablePressure" class="col-md-3">
          <BsDropdown
            label="Predefined formats"
            button="Formats"
            :options="gravityHttpGetFormatOptions"
            :callback="gravityHttpFormatCallback"
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

        <!-- PRESSURE SECTION -->
        <div v-if="global.ui.enablePressure" class="col-md-9">
          <BsInputTextAreaFormat
            v-model="config.http_get_format_pressure"
            rows="6"
            label="Data format (pressure)"
            help="Format template used to create the data sent to the remote service"
            :disabled="pushDisabled || config.http_get_pressure === false"
          />
        </div>
        <div v-if="global.ui.enablePressure && global.ui.enableGravity" class="col-md-3">
          <BsInputSwitch
            v-model="config.http_get_pressure"
            label="Enable pressure"
            :disabled="global.disabled"
          />
          <BsDropdown
            label="Predefined formats"
            button="Formats"
            :options="pressureHttpGetFormatOptions"
            :callback="pressureHttpFormatCallback"
            :disabled="pushDisabled || config.http_get_pressure === false"
          />
          <BsModal
            @click="pressureRenderFormat"
            v-model="pressureRender"
            :code="true"
            title="Format preview"
            button="Preview format"
            :disabled="pushDisabled || config.http_get_pressure === false"
          />
        </div>
        <div v-if="global.ui.enablePressure && !global.ui.enableGravity" class="col-md-3">
          <BsDropdown
            label="Predefined formats"
            button="Formats"
            :options="pressureHttpGetFormatOptions"
            :callback="pressureHttpFormatCallback"
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
import { validateCurrentForm, logError } from '@mp-se/espframework-ui-components'
import { httpGetUrlOptions, httpHeaderOptions } from '@/modules/utils'
import { gravityHttpGetFormatOptions } from '@/modules/gravityFormatOptions'
import { pressureHttpGetFormatOptions } from '@/modules/pressureFormatOptions'
import { applyTemplate } from '@/modules/formatTemplate'
import { global, status, config } from '@/modules/pinia'

const gravityRender = ref('')
const pressureRender = ref('')

const pushDisabled = computed(() => {
  return global.disabled || config.use_wifi_direct
})

const runTestGravity = async () => {
  try {
    const data = { push_format: 'http_get_format_gravity' }
    global.clearMessages()
    await config.runPushTest(data)
  } catch (error) {
    logError('PushHttpGetView.runTestGravity()', error)
    global.messageError = 'Failed to start push test'
  }
}

const runTestPressure = async () => {
  try {
    const data = { push_format: 'http_get_format_pressure' }
    global.clearMessages()
    await config.runPushTest(data)
  } catch (error) {
    logError('PushHttpGetView.runTestPressure()', error)
    global.messageError = 'Failed to start push test'
  }
}

const httpUrlCallback = (opt) => {
  config.http_get_target = opt
}

const httpHeaderH1Callback = (opt) => {
  config.http_get_header1 = opt
}

const httpHeaderH2Callback = (opt) => {
  config.http_get_header2 = opt
}

const gravityHttpFormatCallback = (opt) => {
  config.http_get_format_gravity = decodeURIComponent(opt)
}

const pressureHttpFormatCallback = (opt) => {
  config.http_get_format_pressure = decodeURIComponent(opt)
}

const gravityRenderFormat = () => {
  var s = applyTemplate(status, config, config.http_get_format_gravity)
  gravityRender.value = s
}

const pressureRenderFormat = () => {
  var s = applyTemplate(status, config, config.http_get_format_pressure)
  pressureRender.value = s
}

const save = async () => {
  if (!validateCurrentForm()) return

  await config.saveAll()
}
</script>
