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
  <h5>{{ t('fragment_enable_cors.title') }}</h5>
  <div class="row gy-4">
    <div class="col-md-3">
      <button
        @click="enableCors"
        type="button"
        class="btn btn-secondary"
        :disabled="global.disabled"
      >
        <span
          class="spinner-border spinner-border-sm"
          role="status"
          aria-hidden="true"
          v-show="global.disabled"
        ></span>
        &nbsp;{{ t('fragment_enable_cors.enable_button') }}</button
      >&nbsp;
    </div>
  </div>
</template>

<script setup>
import { useI18n } from 'vue-i18n'
import { global } from '@/modules/pinia'
import { logInfo, logError } from '@mp-se/espframework-ui-components'
import { sharedHttpClient as http } from '@mp-se/espframework-ui-components'

const { t } = useI18n()

const enableCors = async () => {
  try {
    global.disabled = true
    global.clearMessages()

    const data = {
      cors_allowed: true
    }

    try {
      await http.postJson('api/config', data)
      logInfo('EnableCorsFragment.enableCors()', 'Sending /api/config completed')
      global.messageSuccess = t('fragment_enable_cors.success')
    } catch (err) {
      logError('EnableCorsFragment.enableCors()', 'Sending /api/config failed', err)
      global.messageError = t('fragment_enable_cors.err_failed')
    }
  } catch (err) {
    logError('EnableCorsFragment.enableCors()', 'Error enabling CORS:', err)
    global.messageError = t('fragment_enable_cors.err_failed_detail', { error: err.message || err })
  } finally {
    global.disabled = false
  }
}
</script>
