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
    <p class="h3">{{ t('push_wifi_direct.title') }}</p>
    <hr />

    <form @submit.prevent="save" class="needs-validation" novalidate>
      <div class="row">
        <div class="col-md-12">
          <p>
            {{ t('push_wifi_direct.intro') }}
          </p>
        </div>

        <div class="col-md-6">
          <BsInputText
            v-model="config.wifi_direct_ssid"
            :label="t('push_wifi_direct.ssid_label')"
            :help="t('push_wifi_direct.ssid_help')"
            :disabled="global.disabled"
          />
        </div>
        <div class="col-md-6">
          <BsInputText
            v-model="config.wifi_direct_pass"
            type="password"
            maxlength="50"
            :label="t('push_wifi_direct.pass_label')"
            :help="t('push_wifi_direct.pass_help')"
            :disabled="global.disabled"
          ></BsInputText>
        </div>

        <div class="col-md-6">
          <BsInputSwitch
            v-model="config.use_wifi_direct"
            :label="t('push_wifi_direct.use_label')"
            :help="t('push_wifi_direct.use_help')"
            :disabled="global.disabled"
          ></BsInputSwitch>
        </div>
      </div>

      <div class="row gy-2">
        <div class="col-md-12">
          <hr />
        </div>
        <div class="col-md-3">
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
            &nbsp;{{ t('push_wifi_direct.save') }}
          </button>
        </div>
      </div>
    </form>
  </div>
</template>

<script setup>
import { useI18n } from 'vue-i18n'
import { validateCurrentForm } from '@mp-se/espframework-ui-components'
import { global, config } from '@/modules/pinia'

const { t } = useI18n()

const save = async () => {
  if (!validateCurrentForm()) return

  if (config.use_wifi_direct) {
    if (config.wifi_direct_ssid === '' || config.wifi_direct_pass === '') {
      global.messageError = t('push_wifi_direct.err_missing_credentials')
      return
    }
  }

  await config.saveAll()
}
</script>
