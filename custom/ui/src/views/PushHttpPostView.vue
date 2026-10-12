<template>
  <div class="container">
    <p></p>
    <p class="h3">{{ t('cuckoo.http_post.title') }}</p>
    <hr />

    <form @submit.prevent="save" class="needs-validation" novalidate>
      <div class="row">
        <div class="col-md-9">
          <BsInputSwitch
            v-model="cuckoo.use_http_post"
            :label="t('cuckoo.http_post.switch_label')"
            :disabled="global.disabled"
          ></BsInputSwitch>
        </div>
        <div class="col-md-9">
          <BsInputNumber
            v-model="cuckoo.http_post_sleep_interval"
            :label="t('cuckoo.sleep_interval') + intervalLabel(cuckoo.http_post_sleep_interval)"
            unit="s"
            min="300"
            max="3600"
            step="1"
            width="4"
            :help="t('cuckoo.http_post.interval_help')"
            :disabled="global.disabled"
          />
        </div>
        <div class="col-md-9">
          <BsInputText
            v-model="config.http_post2_target"
            type="url"
            maxlength="120"
            :label="t('push_common.http_url_label')"
            :help="t('push_common.http_url_help')"
            :disabled="pushDisabled"
          />
          <details>
            <summary class="pt-2 fw-bold" style="cursor: pointer">
              {{ t('cuckoo.http_post.data_format') }}
            </summary>
            <div>
              <BsInputTextAreaFormat
                v-model="config.http_post2_format_gravity"
                rows="6"
                :help="t('push_common.data_format_help')"
                :disabled="pushDisabled"
              />
              <BsModal
                @click="renderFormat"
                v-model="render"
                :code="true"
                :json="true"
                :title="t('push_common.format_preview_title')"
                :button="t('push_common.preview_format_button')"
                :disabled="pushDisabled"
              />
            </div>
          </details>
        </div>
        <div class="col-md-3">
          <BsDropdown
            :label="t('push_common.predefined_formats_label')"
            :button="t('push_common.formats_button')"
            :options="gravityHttpPostFormatOptions"
            :callback="httpFormatCallback"
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
            {{ t('push_common.save') }}
          </button>
          <button
            @click="runTest"
            type="button"
            class="btn btn-secondary mx-2"
            :disabled="pushDisabled"
          >
            <span
              class="spinner-border spinner-border-sm"
              role="status"
              aria-hidden="true"
              :hidden="!global.disabled"
            ></span>
            {{ t('cuckoo.http_post.run_test') }}
          </button>
        </div>
      </div>
    </form>
  </div>
</template>

<script setup>
import { computed, ref } from 'vue'
import { useI18n } from 'vue-i18n'
import { logError, validateCurrentForm } from '@mp-se/espframework-ui-components'
import { global, status, config } from '@/modules/pinia'
import { applyTemplate } from '@/modules/formatTemplate'
import { gravityHttpPostFormatOptions } from '@/modules/gravityFormatOptions'
import { cuckoo, saveCuckooConfig } from '@custom/cuckooConfig'
import { intervalLabel } from '@custom/utils'

const { t } = useI18n()
const render = ref('')

const pushDisabled = computed(() => global.disabled || config.use_wifi_direct)

const renderFormat = () => {
  render.value = applyTemplate(status, config, config.http_post2_format_gravity)
}

const httpFormatCallback = (opt) => {
  config.http_post2_format_gravity = decodeURIComponent(opt)
}

const save = async () => {
  if (!validateCurrentForm()) return

  await saveCuckooConfig()
}

const runTest = async () => {
  try {
    global.clearMessages()
    await config.runPushTest({ push_format: 'http_post2_format_gravity' })
  } catch (error) {
    logError('PushHttpPostView.runTest()', error)
    global.messageError = t('push_common.err_start_push_test')
  }
}
</script>
