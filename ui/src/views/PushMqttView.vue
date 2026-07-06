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
    <p class="h3">Push - MQTT</p>
    <hr />

    <form
      @submit.prevent="save"
      class="needs-validation"
      novalidate
      :disabled="config.use_wifi_direct"
    >
      <div class="row">
        <div class="col-md-8">
          <BsInputText
            v-model="config.mqtt_target"
            maxlength="120"
            label="Server"
            help="Name of server to connect to, use format servername.com"
            :disabled="pushDisabled"
          />
        </div>
        <div class="col-md-4">
          <BsInputNumber
            v-model="config.mqtt_port"
            label="Port"
            min="0"
            max="65535"
            help="Port number: 1883 (standard), 8883 (standard SSL), or 8000+ for SSL. Leave 0 for default."
            :disabled="pushDisabled"
          />
        </div>
        <div class="col-md-4">
          <BsInputText
            v-model="config.mqtt_user"
            maxlength="20"
            label="User name"
            help="Username to use. Leave blank if authentication is disabled"
            :disabled="pushDisabled"
          />
        </div>
        <div class="col-md-4">
          <BsInputText
            v-model="config.mqtt_pass"
            type="password"
            maxlength="20"
            label="Password"
            help="Password to use. Leave blank if authentication is disabled"
            :disabled="pushDisabled"
          />
        </div>
        <div class="col-md-4">
          <BsInputNumber
            v-model="config.mqtt_int"
            label="Skip interval"
            min="0"
            max="5"
            width="4"
            help="Defines how many sleep cycles to skip between pushing data to this target, 1 = every second cycle. Default is 0."
            :disabled="pushDisabled"
          />
        </div>
        <!-- 
        <div class="col-md-6">
          <BsInputSwitch
            v-model="config.mqtt_retain"
            label="Set MQTT retain flag"
            width="4"
            help="Set the retain flag for messages sent to MQTT."
            :disabled="pushDisabled"
          />
        </div>-->
        <div class="col-md-9">
          <BsInputTextAreaFormat
            v-model="config.mqtt_format_gravity"
            rows="6"
            label="Data format (gravity)"
            help="Format template used to create the data sent to the remote service"
            :disabled="pushDisabled || config.mqtt_gravity === false"
            v-if="global.ui.enableGravity"
          />
        </div>
        <div class="col-md-3 gy-2" v-if="global.ui.enableGravity && global.ui.enablePressure">
          <BsInputSwitch
            v-model="config.mqtt_gravity"
            label="Enable gravity"
            :disabled="global.disabled"
          />
          <BsDropdown
            label="Predefined formats"
            button="Formats"
            :options="gravityMqttFormatOptions"
            :callback="gravityMqttFormatCallback"
            :disabled="pushDisabled || config.mqtt_gravity === false"
          />
          <BsModal
            @click="gravityRenderFormat"
            v-model="gravityRender"
            :code="true"
            :json="true"
            :mqtt="true"
            title="Format preview"
            button="Preview format"
            :disabled="pushDisabled || config.mqtt_gravity === false"
          />
        </div>
        <div class="col-md-3 gy-2" v-if="global.ui.enableGravity && !global.ui.enablePressure">
          <BsDropdown
            label="Predefined formats"
            button="Formats"
            :options="gravityMqttFormatOptions"
            :callback="gravityMqttFormatCallback"
            :disabled="pushDisabled"
          />
          <BsModal
            @click="gravityRenderFormat"
            v-model="gravityRender"
            :code="true"
            :json="true"
            :mqtt="true"
            title="Format preview"
            button="Preview format"
            :disabled="pushDisabled"
          />
        </div>
        <div class="col-md-9">
          <BsInputTextAreaFormat
            v-model="config.mqtt_format_pressure"
            rows="6"
            label="Data format (pressure)"
            help="Format template used to create the data sent to the remote service"
            :disabled="pushDisabled || config.mqtt_pressure === false"
            v-if="global.ui.enablePressure"
          />
        </div>
        <div class="col-md-3 gy-2" v-if="global.ui.enablePressure && global.ui.enableGravity">
          <BsInputSwitch
            v-model="config.mqtt_pressure"
            label="Enable pressure"
            :disabled="global.disabled"
          />
          <BsDropdown
            label="Predefined formats"
            button="Formats"
            :options="pressureMqttFormatOptions"
            :callback="pressureMqttFormatCallback"
            :disabled="pushDisabled || config.mqtt_pressure === false"
          />
          <BsModal
            @click="pressureRenderFormat"
            v-model="pressureRender"
            :code="true"
            :json="true"
            :mqtt="true"
            title="Format preview"
            button="Preview format"
            :disabled="pushDisabled || config.mqtt_pressure === false"
          />
        </div>
        <div class="col-md-3 gy-2" v-if="global.ui.enablePressure && !global.ui.enableGravity">
          <BsDropdown
            label="Predefined formats"
            button="Formats"
            :options="pressureMqttFormatOptions"
            :callback="pressureMqttFormatCallback"
            :disabled="pushDisabled"
          />
          <BsModal
            @click="pressureRenderFormat"
            v-model="pressureRender"
            :code="true"
            :json="true"
            :mqtt="true"
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
import { ref, computed, watch } from 'vue'
import { gravityMqttFormatOptions } from '@/modules/gravityFormatOptions'
import { pressureMqttFormatOptions } from '@/modules/pressureFormatOptions'
import { applyTemplate } from '@/modules/formatTemplate'
import { validateCurrentForm } from '@mp-se/espframework-ui-components'
import { global, status, config } from '@/modules/pinia'
import { logError } from '@mp-se/espframework-ui-components'

// Constants
const ESP8266_PAYLOAD_LIMIT = 500
const ESP8266_WARNING_MSG =
  'On an ESP8266 a large payload will likely cause a crash due to RAM limitations on device. Reduce your template.'

const gravityRender = ref('')
const pressureRender = ref('')

/**
 * Check if format template exceeds ESP8266 payload limit
 * @param {string} format - The format template to validate
 * @returns {boolean} True if payload exceeds limit
 */
const checkPayloadLimit = (format) => {
  if (!global.isEsp8266) return false
  const rendered = applyTemplate(status, config, format)
  return rendered.length > ESP8266_PAYLOAD_LIMIT
}

/**
 * Handle format template validation for payload size
 * @param {string} format - The format template to check
 */
const handleFormatValidation = (format) => {
  if (checkPayloadLimit(format)) {
    global.messageWarning = ESP8266_WARNING_MSG
  } else {
    global.messageWarning = ''
  }
}

watch(
  () => config.mqtt_format_gravity,
  () => {
    handleFormatValidation(config.mqtt_format_gravity)
  }
)

watch(
  () => config.mqtt_format_pressure,
  () => {
    handleFormatValidation(config.mqtt_format_pressure)
  }
)

const pushDisabled = computed(() => {
  return global.disabled || config.use_wifi_direct
})

/**
 * Validate MQTT port number
 * @param {number} port - Port to validate
 * @returns {object} Validation result with valid flag and message
 */
const validateMqttPort = (port) => {
  if (!port) return { valid: true } // 0 is allowed default
  if (port < 0 || port > 65535) {
    return { valid: false, message: 'Port must be between 0 and 65535' }
  }
  return { valid: true }
}

/**
 * Validate MQTT server name
 * @param {string} server - Server hostname or IP
 * @returns {object} Validation result with valid flag and message
 */
const validateMqttServer = (server) => {
  if (!server) {
    return { valid: false, message: 'Server name or IP is required' }
  }
  // Basic validation: allow hostnames, domain names, and IP addresses
  const validHostname = /^[a-zA-Z0-9.-]+$/.test(server)
  const validIp = /^(\d{1,3}\.){3}\d{1,3}$/.test(server)
  if (!validHostname && !validIp) {
    return { valid: false, message: 'Invalid server name or IP format' }
  }
  return { valid: true }
}

/**
 * Run gravity test with proper error handling
 */
const runTestGravity = async () => {
  try {
    const serverValidation = validateMqttServer(config.mqtt_target)
    if (!serverValidation.valid) {
      global.messageError = `Cannot run test: ${serverValidation.message}`
      return
    }

    const portValidation = validateMqttPort(config.mqtt_port)
    if (!portValidation.valid) {
      global.messageError = `Cannot run test: ${portValidation.message}`
      return
    }

    const data = {
      push_format: 'mqtt_format_gravity'
    }

    global.clearMessages()
    await config.runPushTest(data)
  } catch (error) {
    logError('PushMqttView.runTestGravity()', error)
    global.messageError = 'Failed to start push test for gravity data'
  }
}

/**
 * Run pressure test with proper error handling
 */
const runTestPressure = async () => {
  try {
    const serverValidation = validateMqttServer(config.mqtt_target)
    if (!serverValidation.valid) {
      global.messageError = `Cannot run test: ${serverValidation.message}`
      return
    }

    const portValidation = validateMqttPort(config.mqtt_port)
    if (!portValidation.valid) {
      global.messageError = `Cannot run test: ${portValidation.message}`
      return
    }

    const data = {
      push_format: 'mqtt_format_pressure'
    }

    global.clearMessages()
    await config.runPushTest(data)
  } catch (error) {
    logError('PushMqttView.runTestPressure()', error)
    global.messageError = 'Failed to start push test for pressure data'
  }
}

/**
 * Handle gravity format callback with proper decoding and formatting
 * @param {string} opt - URL-encoded format option
 */
const gravityMqttFormatCallback = (opt) => {
  try {
    config.mqtt_format_gravity = decodeURIComponent(opt)
    config.mqtt_format_gravity = config.mqtt_format_gravity.replaceAll('|', '|\n')
    // Trigger validation after setting
    handleFormatValidation(config.mqtt_format_gravity)
  } catch (error) {
    logError('PushMqttView.gravityMqttFormatCallback()', error)
    global.messageError = 'Failed to apply gravity format'
  }
}

/**
 * Handle pressure format callback with proper decoding and formatting
 * @param {string} opt - URL-encoded format option
 */
const pressureMqttFormatCallback = (opt) => {
  try {
    config.mqtt_format_pressure = decodeURIComponent(opt)
    config.mqtt_format_pressure = config.mqtt_format_pressure.replaceAll('|', '|\n')
    // Trigger validation after setting
    handleFormatValidation(config.mqtt_format_pressure)
  } catch (error) {
    logError('PushMqttView.pressureMqttFormatCallback()', error)
    global.messageError = 'Failed to apply pressure format'
  }
}

/**
 * Render gravity format template preview
 */
const gravityRenderFormat = () => {
  try {
    gravityRender.value = applyTemplate(status, config, config.mqtt_format_gravity)
  } catch (error) {
    logError('PushMqttView.gravityRenderFormat()', error)
    gravityRender.value = `Error rendering format: ${error.message}`
  }
}

/**
 * Render pressure format template preview
 */
const pressureRenderFormat = () => {
  try {
    pressureRender.value = applyTemplate(status, config, config.mqtt_format_pressure)
  } catch (error) {
    logError('PushMqttView.pressureRenderFormat()', error)
    pressureRender.value = `Error rendering format: ${error.message}`
  }
}

/**
 * Save configuration with validation
 */
const save = async () => {
  if (!validateCurrentForm()) {
    global.messageError = 'Please fix form validation errors'
    return
  }

  // Validate MQTT configuration
  const serverValidation = validateMqttServer(config.mqtt_target)
  if (!serverValidation.valid) {
    global.messageError = serverValidation.message
    return
  }

  const portValidation = validateMqttPort(config.mqtt_port)
  if (!portValidation.valid) {
    global.messageError = portValidation.message
    return
  }

  try {
    await config.saveAll()
  } catch (error) {
    logError('PushMqttView.save()', error)
    global.messageError = 'Failed to save configuration'
  }
}
</script>
