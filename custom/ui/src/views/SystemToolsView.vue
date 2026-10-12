<template>
  <main class="container system-tools">
    <div class="st-header">
      <h1 class="st-title">
        <IconTools width="20" height="20" aria-hidden="true" />{{ t('cuckoo.tools.title') }}
      </h1>
      <p class="st-text">{{ t('cuckoo.tools.intro') }}</p>
      <p class="st-text">{{ t('cuckoo.tools.version', { version: global.app_ver || '—' }) }}</p>
    </div>

    <section class="st-links">
      <router-link to="/other/firmware" class="st-link-row">
        <span class="st-link-icon"
          ><IconCloudUpArrow width="20" height="20" aria-hidden="true"
        /></span>
        <div>
          <h2>{{ t('cuckoo.tools.firmware_title') }}</h2>
          <p class="st-text">{{ t('cuckoo.tools.firmware_text') }}</p>
        </div>
        <span class="st-link-open"
          >{{ t('cuckoo.tools.open') }}<span aria-hidden="true">&rarr;</span></span
        >
      </router-link>

      <router-link to="/other/about" class="st-link-row">
        <span class="st-link-icon"
          ><IconInfoCircle width="20" height="20" aria-hidden="true"
        /></span>
        <div>
          <h2>{{ t('cuckoo.tools.about_title') }}</h2>
          <p class="st-text">{{ t('cuckoo.tools.about_text') }}</p>
        </div>
        <span class="st-link-open"
          >{{ t('cuckoo.tools.open') }}<span aria-hidden="true">&rarr;</span></span
        >
      </router-link>
    </section>

    <section class="st-card st-card-danger">
      <div>
        <h2 class="st-card-title st-title-danger">
          <IconExclamationTriangle width="16" height="16" aria-hidden="true" />{{
            t('cuckoo.tools.restore_title')
          }}
        </h2>
        <p class="st-card-desc st-text">{{ t('cuckoo.tools.restore_text') }}</p>
        <div v-if="restoreState !== 'idle'" class="st-status" role="status" aria-live="polite">
          <span
            v-if="restoreState === 'requesting'"
            class="spinner-border spinner-border-sm st-status-danger"
            role="status"
            aria-hidden="true"
          ></span>
          <span :class="{ 'st-text-danger': restoreState === 'failed' }">{{
            restoreStatusText
          }}</span>
        </div>
      </div>
      <button
        type="button"
        class="btn btn-danger"
        :disabled="restoreState === 'requesting' || restoreState === 'accepted'"
        @click="confirmRestoreDefaults"
      >
        {{ t('cuckoo.tools.restore_button') }}
      </button>
    </section>

    <BsModalConfirm
      :callback="handleRestoreDefaults"
      :message="t('cuckoo.tools.restore_confirm')"
      id="restoreDefaults"
      :title="t('cuckoo.tools.restore_title')"
    />
  </main>
</template>

<script setup>
import { computed, onBeforeUnmount, ref } from 'vue'
import { useI18n } from 'vue-i18n'
import {
  sharedHttpClient as http,
  logError,
  IconTools,
  IconCloudUpArrow,
  IconInfoCircle,
  IconExclamationTriangle
} from '@mp-se/espframework-ui-components'
import { global } from '@/modules/pinia'

const { t } = useI18n()

const restoreState = ref('idle') // idle | requesting | accepted | failed

let restoreReloadTimeout = null

const restoreStatusText = computed(() => {
  switch (restoreState.value) {
    case 'requesting':
      return t('cuckoo.tools.restore_requesting')
    case 'accepted':
      return t('cuckoo.tools.restore_accepted')
    case 'failed':
      return t('cuckoo.tools.restore_failed')
  }
  return ''
})

const confirmRestoreDefaults = () => document.getElementById('restoreDefaults').click()

const handleRestoreDefaults = async (confirmed) => {
  if (!confirmed) return

  restoreState.value = 'requesting'

  try {
    const json = await http.getJson('api/factory')
    if (json.success !== true) throw new Error(json.message)
    restoreState.value = 'accepted'
    restoreReloadTimeout = setTimeout(() => location.reload(), 4000)
  } catch (err) {
    logError('SystemToolsView.handleRestoreDefaults()', err)
    restoreState.value = 'failed'
  }
}

onBeforeUnmount(() => {
  if (restoreReloadTimeout) clearTimeout(restoreReloadTimeout)
})
</script>

<style scoped>
.system-tools {
  padding-top: 24px;
  display: flex;
  flex-direction: column;
  gap: 32px;
}

.st-header {
  display: flex;
  flex-direction: column;
  gap: 4px;
}
.st-title {
  display: flex;
  align-items: center;
  gap: 8px;
  color: #16734e;
  font-size: 24px;
  font-weight: 600;
  letter-spacing: -0.5px;
  margin: 0;
}
.st-card-title {
  display: flex;
  align-items: center;
  gap: 8px;
  font-size: 14px;
  font-weight: 600;
  margin: 0;
}
.st-text {
  color: #5e6863;
  font-size: 14px;
  margin: 0;
}
.st-title-danger {
  color: #b02a37;
}

.st-links {
  border-block: 1px solid #e4e7e5;
}
.st-link-row {
  display: grid;
  gap: 12px;
  padding: 20px 0;
  color: inherit;
  text-decoration: none;
  transition: background-color 0.15s ease-in-out;
}
.st-link-row:hover {
  background: #f6f8f7;
}
.st-link-row:hover .st-link-open span {
  transform: translateX(4px);
}
.st-link-icon {
  color: #16734e;
}
.st-link-row h2 {
  font-size: 14px;
  font-weight: 600;
  margin: 0;
}
.st-link-row .st-text {
  margin-top: 4px;
}
.st-link-open {
  display: flex;
  align-items: center;
  gap: 4px;
  color: #16734e;
  font-size: 14px;
  font-weight: 500;
  align-self: center;
}
.st-link-open span {
  display: inline-block;
  transition: transform 0.15s ease-in-out;
}

.st-card {
  display: flex;
  flex-direction: column;
  gap: 20px;
  border-radius: 8px;
  padding: 20px;
}
.st-card > div {
  flex: 1;
}
.st-card-danger {
  border: 1px solid rgba(176, 42, 55, 0.3);
}
.st-card-desc {
  max-width: 640px;
  margin-top: 8px;
  line-height: 1.5;
}

.st-status {
  display: flex;
  align-items: center;
  gap: 8px;
  margin-top: 8px;
  font-size: 14px;
  min-height: 22px;
}
.st-status-danger {
  color: #b02a37;
}
.st-text-danger {
  color: #b02a37;
}

.st-card .btn {
  display: inline-flex;
  align-items: center;
  justify-content: center;
  gap: 8px;
  flex-shrink: 0;
}

@media (min-width: 576px) {
  .st-link-row {
    grid-template-columns: 2.5rem 1fr auto;
    align-items: center;
    padding-inline: 12px;
  }
  .st-card {
    flex-direction: row;
    align-items: center;
    padding: 24px;
  }
  .st-card .btn {
    align-self: center;
  }
}
@media (max-width: 575px) {
  .st-card .btn {
    width: 100%;
  }
}
</style>
