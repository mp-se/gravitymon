<template>
  <nav class="navbar navbar-dark bg-primary">
    <div class="container-fluid align-center">
      <router-link class="navbar-brand nav-link" to="/">{{ brand }}</router-link>
      <span class="text-white me-auto ms-2">{{ mdns }}</span>

      <span v-if="configChanged" class="badge bg-danger fs-6 me-3">{{
        t('cuckoo.nav.save_needed')
      }}</span>

      <div class="spinner-border spinner-border-sm text-white me-3" role="status" :hidden="!disabled">
        <span class="visually-hidden">Loading...</span>
      </div>

      <div class="form-check form-switch mb-0">
        <input
          class="form-check-input"
          type="checkbox"
          role="switch"
          style="border-color: white"
          :checked="darkMode"
          @change="emit('update:darkMode', $event.target.checked)"
        />
      </div>
    </div>
  </nav>
</template>

<script setup>
import { useI18n } from 'vue-i18n'

// Top bar without a menu: the pages are reached from the home page. The
// library menu bar always shows the menu button, even with no menu items.
defineProps({
  brand: { type: String, default: '' },
  mdns: { type: String, default: '' },
  disabled: { type: Boolean, default: false },
  configChanged: { type: Boolean, default: false },
  darkMode: { type: Boolean, default: false }
})
const emit = defineEmits(['update:darkMode'])

const { t } = useI18n()
</script>
