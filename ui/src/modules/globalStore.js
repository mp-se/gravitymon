/*
 * GravityMon
 * Copyright (c) 2021-2026 Magnus
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Alternatively, this software may be used under the terms of a
 * commercial license. See LICENSE_COMMERCIAL for details.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
import { defineStore } from 'pinia'
import { logInfo, logDebug, logError } from '@mp-se/espframework-ui-components'
import { sharedHttpClient as http } from '@mp-se/espframework-ui-components'
import SHA256 from 'crypto-js/sha256'

export const useGlobalStore = defineStore('global', {
  state: () => {
    return {
      platform: '',

      chip_id: '',
      board: '',
      app_ver: '',
      app_build: '',
      hardware: '',
      firmware_file: '',
      registered: true,

      ui: {
        enableVoltageFragment: true,
        enableManualWifiEntry: true,
        enableScanForStrongestAp: true,
        enableDeviceRegistration: true,
        enableCalibrationRegistration: true,
        enableGravity: true,
        enablePressure: false,
        enableHttpPostTcpMode: true
      },

      feature: {
        ble: false,
        velocity: false,
        filter: false,
        charging: false
      },

      initialized: false,
      disabled: false,
      configChanged: false,

      messageError: '',
      messageWarning: '',
      messageSuccess: '',
      messageInfo: ''
    }
  },
  getters: {
    isError() {
      return this.messageError != '' ? true : false
    },
    isWarning() {
      return this.messageWarning != '' ? true : false
    },
    isSuccess() {
      return this.messageSuccess != '' ? true : false
    },
    isInfo() {
      return this.messageInfo != '' ? true : false
    },
    uiVersion() {
      logDebug('globalStore.uiVersion()', import.meta.env.VITE_APP_VERSION)
      return import.meta.env.VITE_APP_VERSION
    },
    uiBuild() {
      logDebug('globalStore.uiBuild()', import.meta.env.VITE_APP_BUILD)
      return import.meta.env.VITE_APP_BUILD
    },
    registerApiKey() {
      // eslint-disable-next-line no-undef
      const encodedKey = __REGISTER_API_KEY__
      if (!encodedKey) {
        logError('globalStore.registerApiKey()', 'Encoded API key not defined')
        return null
      }
      const key = atob(encodedKey)
      logDebug('globalStore.registerApiKey()', 'Key decoded (length: ' + key.length + ')')
      return key
    },
    registerBaseUrl() {
      return import.meta.env.VITE_APP_REGISTER_BASEURL || 'https://api.gravitymon.com/'
    },
    isEsp8266() {
      return this.platform === 'ESP8266'
    },
    isCuckoo() {
      return this.hardware === 'CUCKOO'
    },
    isISpindel() {
      return this.hardware === 'ISPINDEL'
    },
    isGravitymon() {
      return this.hardware === 'GRAVITYMON'
    },
    disabled32() {
      if (this.disabled) return true

      if (this.platform !== 'ESP8266') return false

      return true
    }
  },
  actions: {
    clearMessages() {
      this.messageError = ''
      this.messageWarning = ''
      this.messageSuccess = ''
      this.messageInfo = ''
    },
    // Helper function to anonymize chip_id using SHA-256 hash
    async anonymizeChipId() {
      logInfo('globalStore.anonymizeChipId()', this.chip_id)
      if (!this.chip_id || this.chip_id === 'unknown') return 'unknown'
      return SHA256(this.chip_id).toString()
    },
    async load() {
      try {
        logInfo('globalStore.load()', 'Fetching /api/feature')
        const json = await http.getJson('api/feature')
        logInfo('globalStore.load()', json)

        this.chip_id = json.chip_id.toLowerCase()
        this.board = json.board.toUpperCase()
        this.app_ver = json.app_ver
        this.app_build = json.app_build
        this.platform = json.platform.toUpperCase()
        this.hardware = json.hardware.toUpperCase()
        this.firmware_file = json.firmware_file.toLowerCase()
        this.registered = json.registered

        this.feature.ble = json.ble
        this.feature.velocity = json.velocity
        this.feature.filter = json.filter
        this.feature.charging = json.charging

        logInfo('globalStore.load()', 'Fetching /api/feature completed')
        return true
      } catch (err) {
        logError('globalStore.load()', err)
        return false
      }
    }
  }
})
