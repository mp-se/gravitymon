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
import { global, saveConfigState, getConfigChanges } from '@/modules/pinia'
import { logDebug, logError, logInfo } from '@mp-se/espframework-ui-components'
import { tempToC, tempToF, roundVal } from '@mp-se/espframework-ui-components'
import { sharedHttpClient as http } from '@mp-se/espframework-ui-components'

export const useConfigStore = defineStore('config', {
  state: () => {
    return {
      // Device
      id: '',
      mdns: '',
      temp_unit: '',
      gravity_unit: '',
      // Hardware
      ota_url: '',
      storage_sleep: false,
      voltage_factor: 0,
      voltage_config: 0,
      gyro_temp: false,
      gyro_swap_xy: false,
      gyro_filter: false,
      battery_saving: false,
      tempsensor_resolution: 0,
      temp_adjustment_value: 0, // C or F
      charging_pin_enabled: false,
      // Wifi
      wifi_portal_timeout: 0,
      wifi_connect_timeout: 0,
      wifi_ssid: '',
      wifi_ssid2: '',
      wifi_pass: '',
      wifi_pass2: '',
      wifi_direct_ssid: '',
      wifi_direct_pass: '',
      use_wifi_direct: false,
      wifi_scan_ap: false,
      // Push - Generic
      token: '',
      token2: '',
      sleep_interval: 0,
      push_timeout: 0,
      skip_ssl_on_test: false,
      // Push - Http Post 1
      http_post_target: '',
      http_post_header1: '',
      http_post_header2: '',
      http_post_tcp: false,
      http_post_int: 0,
      http_post_format_gravity: '',
      // Push - Http Post 2
      http_post2_target: '',
      http_post2_header1: '',
      http_post2_header2: '',
      http_post2_int: 0,
      http_post2_format_gravity: '',
      // Push - Http Get
      http_get_target: '',
      http_get_header1: '',
      http_get_header2: '',
      http_get_int: 0,
      http_get_format_gravity: '',
      // Push - Influx
      influxdb2_target: '',
      influxdb2_org: '',
      influxdb2_bucket: '',
      influxdb2_token: '',
      influxdb2_int: 0,
      influxdb2_format_gravity: '',
      // Push - MQTT
      mqtt_target: '',
      mqtt_port: '',
      mqtt_user: '',
      mqtt_pass: '',
      mqtt_int: 0,
      mqtt_format_gravity: '',
      // Push BLE
      ble_tilt_color: '',
      ble_format: 0,
      // Gravity formula
      gyro_type: 0,
      gravity_formula: '',
      gravity_temp_adjustment: false,
      formula_calculation_data: [], // SG or P
      gyro_read_count: 0,
      gyro_moving_threashold: 0,
      formula_max_deviation: 0,
      formula_calibration_temp: 0, // C or F
      ignore_low_angles: false,
      gyro_calibration_data: [],
      dark_mode: false
    }
  },
  actions: {
    convertTemp() {
      if (this.temp_unit == this.internal_temp_unit) return
      if (this.temp_unit == 'C') this.convertTempToC()
      if (this.temp_unit == 'F') this.convertTempToF()
    },
    convertTempToC() {
      if (this.internal_temp_unit == 'C') return

      this.temp_adjustment_value = roundVal(this.temp_adjustment_value / 1.8, 2)
      this.formula_calibration_temp = roundVal(tempToC(this.formula_calibration_temp), 2)
      this.internal_temp_unit = 'C'
    },
    convertTempToF() {
      if (this.internal_temp_unit == 'F') return

      this.temp_adjustment_value = roundVal(this.temp_adjustment_value * 1.8, 2) // Delta value
      this.formula_calibration_temp = roundVal(tempToF(this.formula_calibration_temp), 2)
      this.internal_temp_unit = 'F'
    },
    toJson() {
      logInfo('configStore.toJSON()')
      const dest = {}

      for (const key in this.$state) {
        if (!key.startsWith('$')) {
          dest[key] = this[key]
        }
      }

      logInfo('configStore.toJSON()', dest)
      return JSON.stringify(dest, null, 2)
    },
    async load() {
      global.disabled = true
      logInfo('configStore.load()', 'Fetching /api/config')
      try {
        const json = await http.getJson('api/config')
        logDebug('configStore.load()', json)
        global.disabled = false
        this.id = json.id
        // Device
        this.mdns = json.mdns
        this.temp_unit = json.temp_unit
        this.gravity_unit = json.gravity_unit
        // Hardware
        this.ota_url = json.ota_url
        this.storage_sleep = json.storage_sleep
        this.voltage_factor = json.voltage_factor
        this.voltage_config = json.voltage_config
        this.gyro_type = json.gyro_type
        this.gyro_temp = json.gyro_temp
        this.gyro_swap_xy = json.gyro_swap_xy
        this.gyro_filter = json.gyro_filter
        this.battery_saving = json.battery_saving
        this.tempsensor_resolution = json.tempsensor_resolution
        this.temp_adjustment_value = json.temp_adjustment_value
        this.charging_pin_enabled = json.charging_pin_enabled
        // Wifi
        this.wifi_portal_timeout = json.wifi_portal_timeout
        this.wifi_connect_timeout = json.wifi_connect_timeout
        this.wifi_ssid = json.wifi_ssid
        this.wifi_ssid2 = json.wifi_ssid2
        this.wifi_pass = json.wifi_pass
        this.wifi_pass2 = json.wifi_pass2
        this.wifi_direct_ssid = json.wifi_direct_ssid
        this.wifi_direct_pass = json.wifi_direct_pass
        this.use_wifi_direct = json.use_wifi_direct
        this.wifi_scan_ap = json.wifi_scan_ap
        // Push - Generic
        this.token = json.token
        this.token2 = json.token2
        this.sleep_interval = json.sleep_interval
        this.push_timeout = json.push_timeout
        this.skip_ssl_on_test = json.skip_ssl_on_test
        // Push - Http Post 1
        this.http_post_target = json.http_post_target
        this.http_post_header1 = json.http_post_header1
        this.http_post_header2 = json.http_post_header2
        this.http_post_int = json.http_post_int
        this.http_post_tcp = json.http_post_tcp
        // this.http_post_format_gravity = json.http_post_format_gravity
        // Push - Http Post 2
        this.http_post2_target = json.http_post2_target
        this.http_post2_header1 = json.http_post2_header1
        this.http_post2_header2 = json.http_post2_header2
        this.http_post2_int = json.http_post2_int
        // this.http_post2_format_gravity = json.http_post2_format_gravity
        // Push - Http Get
        this.http_get_target = json.http_get_target
        this.http_get_header1 = json.http_get_header1
        this.http_get_header2 = json.http_get_header2
        this.http_get_int = json.http_get_int
        // this.http_get_format_gravity = json.http_get_format_gravity
        // Push - Influx
        this.influxdb2_target = json.influxdb2_target
        this.influxdb2_org = json.influxdb2_org
        this.influxdb2_bucket = json.influxdb2_bucket
        this.influxdb2_token = json.influxdb2_token
        this.influxdb2_int = json.influxdb2_int
        // this.influxdb2_format_gravity = json.influxdb2_format_gravity
        // Push - MQTT
        this.mqtt_target = json.mqtt_target
        this.mqtt_port = json.mqtt_port
        this.mqtt_user = json.mqtt_user
        this.mqtt_pass = json.mqtt_pass
        this.mqtt_int = json.mqtt_int
        // this.mqtt_format_gravity = json.mqtt_format_gravity
        // Push BLE
        this.ble_tilt_color = json.ble_tilt_color
        this.ble_format = json.ble_format
        // Gravity formula
        this.gravity_formula = json.gravity_formula
        this.gravity_temp_adjustment = json.gravity_temp_adjustment
        this.gyro_read_count = json.gyro_read_count
        this.gyro_moving_threashold = json.gyro_moving_threashold
        this.formula_max_deviation = json.formula_max_deviation
        this.formula_calibration_temp = json.formula_calibration_temp
        this.ignore_low_angles = json.ignore_low_angles
        this.formula_calculation_data = json.formula_calculation_data
        this.gyro_calibration_data = json.gyro_calibration_data
        this.dark_mode = json.dark_mode

        this.internal_temp_unit = 'C'
        this.convertTemp()
        return true
      } catch (err) {
        global.disabled = false
        logError('configStore.load()', err)
        return false
      }
    },
    async loadFormat() {
      global.disabled = true
      logInfo('configStore.loadFormat()', 'Fetching /api/format')
      try {
        const json = await http.getJson('api/format')
        logDebug('configStore.loadFormat()', json)
        global.disabled = false
        this.http_post_format_gravity = decodeURIComponent(json.http_post_format_gravity)
        this.http_post2_format_gravity = decodeURIComponent(json.http_post2_format_gravity)
        this.http_get_format_gravity = decodeURIComponent(json.http_get_format_gravity)
        this.influxdb2_format_gravity = decodeURIComponent(json.influxdb2_format_gravity)
        this.mqtt_format_gravity = decodeURIComponent(json.mqtt_format_gravity)

        // Add linebreaks so the editor shows the data correctly
        this.mqtt_format_gravity = this.mqtt_format_gravity.replaceAll('|', '|\n')
        return true
      } catch (err) {
        global.disabled = false
        logError('configStore.loadFormat()', err)
        return false
      }
    },
    async sendConfig() {
      global.disabled = true
      logInfo('configStore.sendConfig()', 'Sending /api/config')

      this.convertTempToC() // Device use C internally

      const data = getConfigChanges()
      delete data.http_post_format_gravity
      delete data.http_post2_format_gravity
      delete data.http_get_format_gravity
      delete data.influxdb2_format_gravity
      delete data.mqtt_format_gravity
      logDebug('configStore.sendConfig()', data)

      if (JSON.stringify(data).length == 2) {
        logInfo('configStore.sendConfig()', 'No config data to store, skipping step')
        global.disabled = false
        this.convertTemp()
        return true
      }

      try {
        await http.postJson('api/config', data)
        global.disabled = false
        logInfo('configStore.sendConfig()', 'Sending /api/config completed')
        this.convertTemp()
        return true
      } catch (err) {
        logError('configStore.sendConfig()', err)
        this.convertTemp()
        global.disabled = false
        return false
      }
    },
    async sendFormat() {
      global.disabled = true
      logInfo('configStore.sendFormat()', 'Sending /api/format')

      const data2 = getConfigChanges()
      let data = {}
      let cnt = 0

      logDebug('configStore.sendFormat()', data)
      try {
        data =
          data2.http_post_format_gravity !== undefined
            ? { http_post_format_gravity: encodeURIComponent(data2.http_post_format_gravity) }
            : {}
        if (await this.sendOneFormat(data)) cnt += 1

        data =
          data2.http_post2_format_gravity !== undefined
            ? { http_post2_format_gravity: encodeURIComponent(data2.http_post2_format_gravity) }
            : {}
        if (await this.sendOneFormat(data)) cnt += 1

        data =
          data2.http_get_format_gravity !== undefined
            ? { http_get_format_gravity: encodeURIComponent(data2.http_get_format_gravity) }
            : {}
        if (await this.sendOneFormat(data)) cnt += 1

        data =
          data2.influxdb2_format_gravity !== undefined
            ? { influxdb2_format_gravity: encodeURIComponent(data2.influxdb2_format_gravity) }
            : {}
        if (await this.sendOneFormat(data)) cnt += 1

        if (data2.mqtt_format_gravity !== undefined) {
          data2.mqtt_format_gravity = data2.mqtt_format_gravity.replaceAll('\n', '')
          data2.mqtt_format_gravity = data2.mqtt_format_gravity.replaceAll('\r', '')
        }

        data =
          data2.mqtt_format_gravity !== undefined
            ? { mqtt_format_gravity: encodeURIComponent(data2.mqtt_format_gravity) }
            : {}
        if (await this.sendOneFormat(data)) cnt += 1

        return cnt == 5
      } finally {
        global.disabled = false
      }
    },
    async sendOneFormat(data) {
      logInfo('configStore.sendOneFormat()', 'Sending /api/format')

      if (JSON.stringify(data).length == 2) {
        logInfo('configStore.sendOneFormat()', 'No format data to store, skipping step')
        return true
      }

      try {
        await http.postJson('api/format', data)
        global.disabled = false
        logInfo('configStore.sendOneFormat()', 'Sending /api/format completed')
        return true
      } catch (err) {
        logError('configStore.sendOneFormat()', err)
        return false
      }
    },
    async sendPushTest(data) {
      global.disabled = true
      logInfo('configStore.sendPushTest()', 'Sending /api/push')
      try {
        await http.postJson('api/push', data)
        return true
      } catch (err) {
        logError('configStore.sendPushTest()', err)
        return false
      }
    },

    async setSleepMode(flag) {
      try {
        logInfo('configStore.setSleepMode()', 'Sending /api/sleepmode')
        const response = await http.request('api/sleepmode', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ sleep_mode: flag })
        })
        if (!response.ok) {
          logError('configStore.setSleepMode()', `HTTP ${response.status}: ${response.statusText}`)
          return false
        }

        await response.json()
        logInfo('configStore.setSleepMode()', 'Sending /api/sleepmode completed')
        return true
      } catch (err) {
        logError('configStore.setSleepMode()', err)
        return false
      }
    },
    async restart() {
      global.clearMessages()
      global.disabled = true
      try {
        const res = await http.restart(this.mdns, { redirectDelayMs: 8000 })
        if (res.success && res.json && res.json.status === true) {
          global.messageSuccess =
            (res.json.message || '') +
            ' Redirecting to http://' +
            this.mdns +
            '.local in 8 seconds.'
          logInfo('configStore.restart()', 'Restart requested, redirect scheduled')
        } else if (res.success && res.json) {
          global.messageError = res.json.message || 'Failed to restart device'
        } else {
          global.messageError = 'Failed to request restart'
        }
      } catch (err) {
        logError('configStore.restart()', err)
        global.messageError = 'Failed to do restart'
      } finally {
        global.disabled = false
      }
    },
    async getPushTestStatus() {
      logInfo('configStore.getPushTest()', 'Fetching /api/push/status')
      try {
        const json = await http.getJson('api/push/status')
        logDebug('configStore.getPushTest()', json)
        logInfo('configStore.getPushTest()', 'Fetching /api/push/status completed')
        return { success: true, data: json }
      } catch (err) {
        logError('configStore.getPushTest()', err)
        return { success: false, data: null }
      }
    },
    async sendWifiScan() {
      global.disabled = true
      logInfo('configStore.sendWifiScan()', 'Sending /api/wifi')
      try {
        await http.request('api/wifi')
        logInfo('configStore.sendWifiScan()', 'Sending /api/wifi completed')
        return true
      } catch (err) {
        logError('configStore.sendWifiScan()', err)
        return false
      }
    },
    async getWifiScanStatus() {
      logInfo('configStore.getWifiScanStatus()', 'Fetching /api/wifi/status')
      try {
        const json = await http.getJson('api/wifi/status')
        logDebug('configStore.getWifiScanStatus()', json)
        logInfo('configStore.getWifiScanStatus()', 'Fetching /api/wifi/status completed')
        return { success: true, data: json }
      } catch (err) {
        logError('configStore.getWifiScanStatus()', err)
        return { success: false, data: null }
      }
    },
    async sendHardwareScan() {
      global.disabled = true
      logInfo('configStore.sendHardwareScan()', 'Sending /api/hardware')
      try {
        await http.request('api/hardware')
        logInfo('configStore.sendHardwareScan()', 'Sending /api/hardware completed')
        return true
      } catch (err) {
        logError('configStore.sendHardwareScan()', err)
        return false
      }
    },
    async getHardwareScanStatus() {
      logInfo('configStore.getHardwareScanStatus()', 'Fetching /api/hardware/status')
      try {
        const json = await http.getJson('api/hardware/status')
        logDebug('configStore.getHardwareScanStatus()', json)
        logInfo('configStore.getHardwareScanStatus()', 'Fetching /api/hardware/status completed')
        return { success: true, data: json }
      } catch (err) {
        logError('configStore.getHardwareScanStatus()', err)
        return { success: false, data: null }
      }
    },
    async saveAll() {
      global.clearMessages()
      global.disabled = true

      try {
        const configSuccess = await this.sendConfig()
        if (!configSuccess) {
          global.messageError = 'Failed to store configuration to device'
          return
        }

        const formatSuccess = await this.sendFormat()
        if (!formatSuccess) {
          global.messageError = 'Failed to store format to device'
          return
        }

        global.messageSuccess = 'Configuration has been saved to device'
        saveConfigState()
      } catch (error) {
        logError('configStore.saveAll()', error)
        global.messageError = 'Failed to save configuration'
      } finally {
        global.disabled = false
      }
    },
    async runPushTest(data) {
      global.disabled = true
      logInfo('configStore.runPushTest()', 'Starting push test')

      try {
        const pushStarted = await this.sendPushTest(data)
        if (!pushStarted) {
          global.messageError = 'Failed to start push test'
          return false
        }

        // Poll for test completion
        const result = await (async () => {
          while (true) {
            const statusRes = await this.getPushTestStatus()
            if (!statusRes.success) {
              global.messageError = 'Failed to get push test status'
              return false
            }

            const d = statusRes.data
            if (d.status) {
              // still running
              await new Promise((r) => setTimeout(r, 2000))
              continue
            }

            if (!d.success) {
              global.messageError = 'Test failed with error code (' + d.push_return_code + ')'
              return true
            } else {
              if (!d.push_enabled) {
                global.messageWarning = 'No endpoint is defined for this target. Cannot run test.'
              } else if (!d.success && d.push_return_code > 0) {
                global.messageError =
                  'Test failed with error code (' + http.getErrorString(d.push_return_code) + ')'
              } else if (!d.success && d.push_return_code == 0) {
                global.messageError =
                  'Test not started. Might be blocked due to skip SSL flag enabled on esp8266'
              } else {
                global.messageSuccess = 'Test was successful'
              }
              return true
            }
          }
        })()

        return result
      } catch (error) {
        logError('configStore.runPushTest()', error)
        global.messageError = 'Push test failed unexpectedly'
        return false
      } finally {
        global.disabled = false
      }
    },

    async runWifiScan() {
      global.disabled = true
      logInfo('configStore.runWifiScan()', 'Starting wifi scan')

      try {
        const started = await this.sendWifiScan()
        if (!started) {
          global.messageError = 'Failed to start wifi scan'
          return { success: false }
        }

        while (true) {
          const statusRes = await this.getWifiScanStatus()
          if (!statusRes.success) {
            global.messageError = 'Failed to get wifi scan status'
            return { success: false }
          }

          if (statusRes.data.status) {
            await new Promise((r) => setTimeout(r, 2000))
            continue
          }

          global.disabled = false
          return { success: statusRes.data.success, data: statusRes.data }
        }
      } finally {
        global.disabled = false
      }
    },

    async runHardwareScan() {
      global.disabled = true
      logInfo('configStore.runHardwareScan()', 'Starting hardware scan')

      try {
        const started = await this.sendHardwareScan()
        if (!started) {
          global.messageError = 'Failed to start hardware scan'
          return { success: false }
        }

        while (true) {
          const statusRes = await this.getHardwareScanStatus()
          if (!statusRes.success) {
            global.messageError = 'Failed to get hardware scan status'
            return { success: false }
          }

          if (statusRes.data.status) {
            await new Promise((r) => setTimeout(r, 2000))
            continue
          }

          global.disabled = false
          return { success: statusRes.data.success, data: statusRes.data }
        }
      } finally {
        global.disabled = false
      }
    }
  }
})
