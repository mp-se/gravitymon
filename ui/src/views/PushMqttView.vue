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
    <p class="h3">{{ t('push_mqtt.title') }}</p>
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
            :label="t('push_mqtt.server_label')"
            :help="t('push_mqtt.server_help')"
            :disabled="pushDisabled"
          />
        </div>
        <div class="col-md-4">
          <BsInputNumber
            v-model="config.mqtt_port"
            :label="t('push_mqtt.port_label')"
            min="0"
            max="65535"
            :help="t('push_mqtt.port_help')"
            :disabled="pushDisabled"
          />
        </div>
        <div class="col-md-4">
          <BsInputText
            v-model="config.mqtt_user"
            maxlength="20"
            :label="t('push_mqtt.user_label')"
            :help="t('push_mqtt.user_help')"
            :disabled="pushDisabled"
          />
        </div>
        <div class="col-md-4">
          <BsInputText
            v-model="config.mqtt_pass"
            type="password"
            maxlength="20"
            :label="t('push_mqtt.pass_label')"
            :help="t('push_mqtt.pass_help')"
            :disabled="pushDisabled"
          />
        </div>
        <div class="col-md-4">
          <BsInputNumber
            v-model="config.mqtt_int"
            :label="t('push_common.skip_interval_label')"
            min="0"
            max="5"
            width="4"
            :help="t('push_common.skip_interval_help')"
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
            :label="t('push_common.data_format_gravity_label')"
            :help="t('push_common.data_format_help')"
            :disabled="pushDisabled || config.mqtt_gravity === false"
            v-if="global.ui.enableGravity"
          />
        </div>
        <div class="col-md-3 gy-2" v-if="global.ui.enableGravity && global.ui.enablePressure">
          <BsInputSwitch
            v-model="config.mqtt_gravity"
            :label="t('push_common.enable_gravity_label')"
            :disabled="global.disabled"
          />
          <BsDropdown
            :label="t('push_common.predefined_formats_label')"
            :button="t('push_common.formats_button')"
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
            :title="t('push_common.format_preview_title')"
            :button="t('push_common.preview_format_button')"
            :disabled="pushDisabled || config.mqtt_gravity === false"
          />
        </div>
        <div class="col-md-3 gy-2" v-if="global.ui.enableGravity && !global.ui.enablePressure">
          <BsDropdown
            :label="t('push_common.predefined_formats_label')"
            :button="t('push_common.formats_button')"
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
            :title="t('push_common.format_preview_title')"
            :button="t('push_common.preview_format_button')"
            :disabled="pushDisabled"
          />
        </div>
        <div class="col-md-9">
          <BsInputTextAreaFormat
            v-model="config.mqtt_format_pressure"
            rows="6"
            :label="t('push_common.data_format_pressure_label')"
            :help="t('push_common.data_format_help')"
            :disabled="pushDisabled || config.mqtt_pressure === false"
            v-if="global.ui.enablePressure"
          />
        </div>
        <div class="col-md-3 gy-2" v-if="global.ui.enablePressure && global.ui.enableGravity">
          <BsInputSwitch
            v-model="config.mqtt_pressure"
            :label="t('push_common.enable_pressure_label')"
            :disabled="global.disabled"
          />
          <BsDropdown
            :label="t('push_common.predefined_formats_label')"
            :button="t('push_common.formats_button')"
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
            :title="t('push_common.format_preview_title')"
            :button="t('push_common.preview_format_button')"
            :disabled="pushDisabled || config.mqtt_pressure === false"
          />
        </div>
        <div class="col-md-3 gy-2" v-if="global.ui.enablePressure && !global.ui.enableGravity">
          <BsDropdown
            :label="t('push_common.predefined_formats_label')"
            :button="t('push_common.formats_button')"
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
import { ref, computed, watch } from 'vue'
import { useI18n } from 'vue-i18n'
import { gravityMqttFormatOptions } from '@/modules/gravityFormatOptions'
import { pressureMqttFormatOptions } from '@/modules/pressureFormatOptions'
import { applyTemplate } from '@/modules/formatTemplate'
import { validateCurrentForm } from '@mp-se/espframework-ui-components'
import { global, status, config } from '@/modules/pinia'
import { logError } from '@mp-se/espframework-ui-components'

const { t } = useI18n()

// Constants
const ESP8266_PAYLOAD_LIMIT = 500

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
    global.messageWarning = t('push_mqtt.warn_esp8266_payload')
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
    return { valid: false, message: t('push_mqtt.err_port_range') }
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
    return { valid: false, message: t('push_mqtt.err_server_required') }
  }
  // Basic validation: allow hostnames, domain names, and IP addresses
  const validHostname = /^[a-zA-Z0-9.-]+$/.test(server)
  const validIp = /^(\d{1,3}\.){3}\d{1,3}$/.test(server)
  if (!validHostname && !validIp) {
    return { valid: false, message: t('push_mqtt.err_server_format') }
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
      global.messageError = t('push_mqtt.err_test_prefix', { message: serverValidation.message })
      return
    }

    const portValidation = validateMqttPort(config.mqtt_port)
    if (!portValidation.valid) {
      global.messageError = t('push_mqtt.err_test_prefix', { message: portValidation.message })
      return
    }

    const data = {
      push_format: 'mqtt_format_gravity'
    }

    global.clearMessages()
    await config.runPushTest(data)
  } catch (error) {
    logError('PushMqttView.runTestGravity()', error)
    global.messageError = t('push_mqtt.err_gravity_test')
  }
}

/**
 * Run pressure test with proper error handling
 */
const runTestPressure = async () => {
  try {
    const serverValidation = validateMqttServer(config.mqtt_target)
    if (!serverValidation.valid) {
      global.messageError = t('push_mqtt.err_test_prefix', { message: serverValidation.message })
      return
    }

    const portValidation = validateMqttPort(config.mqtt_port)
    if (!portValidation.valid) {
      global.messageError = t('push_mqtt.err_test_prefix', { message: portValidation.message })
      return
    }

    const data = {
      push_format: 'mqtt_format_pressure'
    }

    global.clearMessages()
    await config.runPushTest(data)
  } catch (error) {
    logError('PushMqttView.runTestPressure()', error)
    global.messageError = t('push_mqtt.err_pressure_test')
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
    global.messageError = t('push_mqtt.err_apply_gravity_format')
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
    global.messageError = t('push_mqtt.err_apply_pressure_format')
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
    global.messageError = t('push_mqtt.err_validation')
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
    global.messageError = t('push_mqtt.err_save')
  }
}
</script>
