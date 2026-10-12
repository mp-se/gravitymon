<template>
  <dialog id="spinner" class="loading">
    <div class="container text-center">
      <div class="row align-items-center" style="height: 170px">
        <div class="col">
          <div class="spinner-border" role="status" style="width: 5rem; height: 5rem">
            <span class="visually-hidden">Loading...</span>
          </div>
        </div>
      </div>
    </div>
  </dialog>

  <div v-if="!global.initialized" class="container text-center">
    <BsMessage :message="t('cuckoo.app.initializing')" :dismissable="false" alert="info" />
  </div>

  <CuckooNavBar
    v-if="global.initialized"
    brand="CuckooTilt"
    :disabled="global.disabled"
    :mdns="config.mdns"
    :config-changed="global.configChanged"
    :dark-mode="config.dark_mode"
    @update:dark-mode="handleDarkModeUpdate"
  />

  <div class="container">
    <p></p>
    <div v-if="global.initialized" class="d-flex justify-content-between align-items-center gap-2">
      <router-link v-if="route.path !== '/'" to="/" class="btn btn-sm btn-outline-secondary">
        &larr; {{ t('cuckoo.nav.back') }}
      </router-link>
      <span v-else></span>

      <div class="d-flex align-items-center gap-2">
        <label for="language" class="form-label mb-0">{{ t('cuckoo.nav.language') }}</label>
        <select id="language" v-model="config.locale" class="form-select form-select-sm w-auto">
          <option v-for="l in languages" :key="l.code" :value="l.code">{{ l.label }}</option>
        </select>
      </div>
    </div>

    <BsMessage
      v-if="!status.connected"
      :message="t('cuckoo.app.no_response')"
      :dismissable="false"
      alert="danger"
    />
    <BsMessage
      v-if="global.isError"
      :close="close"
      :dismissable="true"
      :message="global.messageError"
      alert="danger"
    />
    <BsMessage
      v-if="global.isWarning"
      :close="close"
      :dismissable="true"
      :message="global.messageWarning"
      alert="warning"
    />
    <BsMessage
      v-if="global.isSuccess"
      :close="close"
      :dismissable="true"
      :message="global.messageSuccess"
      alert="success"
    />
    <BsMessage
      v-if="global.isInfo"
      :close="close"
      :dismissable="true"
      :message="global.messageInfo"
      alert="info"
    />
  </div>

  <router-view v-if="global.initialized" />
  <div
    v-if="global.initialized"
    class="container-fluid text-center text-secondary small pt-3 pb-3"
  >
    CuckooTilt &middot; {{ t('cuckoo.app.footer_by') }} Cuckoo (ckbrew.com) &middot;
    {{ t('cuckoo.app.footer_based_on') }}
    <a class="link-secondary" href="https://github.com/mp-se/gravitymon" target="_blank">GravityMon</a>
    &copy; Magnus Persson
  </div>
</template>

<script setup>
import { onMounted, onBeforeMount, ref, watch } from 'vue'
import { useI18n } from 'vue-i18n'
import { useRoute } from 'vue-router'
import {
  sharedHttpClient as http,
  useTimers,
  logError,
  logInfo
} from '@mp-se/espframework-ui-components'
import { global, status, config, saveConfigState } from '@/modules/pinia'
import { languages } from '@custom/i18n'
import CuckooNavBar from '@custom/components/CuckooNavBar.vue'

const { t, locale } = useI18n()
const route = useRoute()
const { createInterval } = useTimers()
const polling = ref(null)

// Keep the active language in sync with the language stored on the device.
// All languages are embedded, so there is nothing to load.
watch(
  () => config.locale,
  (code) => {
    locale.value = languages.some((l) => l.code === code) ? code : 'en'
  },
  { immediate: true }
)

const close = (alert) => {
  if (alert == 'danger') global.messageError = ''
  else if (alert == 'warning') global.messageWarning = ''
  else if (alert == 'success') global.messageSuccess = ''
  else if (alert == 'info') global.messageInfo = ''
}

watch(
  () => global.disabled,
  (disabled) => {
    document.body.style.cursor = disabled ? 'wait' : 'default'
  }
)

onBeforeMount(() => {
  polling.value = createInterval(() => {
    ;(async () => {
      status.connected = await http.ping()
    })()
  }, 7000)
})

onMounted(async () => {
  if (!global.initialized) await initializeApp()
})

async function initializeApp() {
  try {
    document.querySelector('#spinner').showModal()

    if (!(await http.auth(btoa('gravitymon:password')))) {
      global.messageError = t('cuckoo.app.err_auth')
      return
    }

    // Features, status, configuration (including the Cuckoo settings, see
    // cuckooConfig.js) and push formats
    if (
      !(await global.load()) ||
      !(await status.load()) ||
      !(await config.load()) ||
      !(await config.loadFormat())
    ) {
      global.messageError = t('cuckoo.app.err_load')
      return
    }

    saveConfigState()
    handleDarkModeUpdate(config.dark_mode)
    global.initialized = true
  } catch (error) {
    logError('App.initializeApp()', error)
    global.messageError = t('cuckoo.app.err_load')
  } finally {
    document.querySelector('#spinner').close()
  }
}

watch(
  () => config.dark_mode,
  (value) => handleDarkModeUpdate(value)
)

const handleDarkModeUpdate = (value) => {
  logInfo('App.handleDarkModeUpdate()', 'Updating dark mode settings', value)
  config.dark_mode = value
  document.documentElement.setAttribute('data-bs-theme', value ? 'dark' : 'light')
}
</script>

<style>
.loading {
  position: fixed;
  width: 200px;
  height: 200px;
  padding: 10px;
  top: 0;
  right: 0;
  bottom: 0;
  left: 0;
  margin: auto;
  border: none;
  background: transparent;
}
</style>
