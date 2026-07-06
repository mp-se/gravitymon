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

    <template v-if="status">
      <BsMessage
        v-if="!status.self_check.push_targets"
        dismissable="true"
        message=""
        alert="danger"
      >
        No remote services are active. Check your push settings and enable at least one service.
      </BsMessage>

      <BsMessage
        v-if="!status.self_check.gyro_connected && status.wifi_setup == false"
        dismissable="true"
        message=""
        alert="danger"
      >
        No gyro is detected. Try to reboot / power-off. If this persists, check for hardware issues.
      </BsMessage>
    </template>

    <div v-if="status" class="container overflow-hidden text-center">
      <div class="row gy-4">
        <div
          class="col-md-4"
          v-if="
            status.self_check.gravity_formula &&
            status.wifi_setup == false &&
            status.gravity !== undefined
          "
        >
          <BsCard header="Measurement" color="info" title="Gravity">
            <p class="text-center">
              {{ status.gravity }}
              {{ status.gravity_unit === 'G' ? ' SG' : ' P' }}
            </p>
          </BsCard>
        </div>

        <div
          class="col-md-4"
          v-if="!status.self_check.gravity_formula && status.wifi_setup == false"
        >
          <BsCard header="Measurement" title="Error" :iserr="true" icon="bi-x-circle">
            <p class="text-center">
              Missing
              <router-link
                class="link-primary link-offset-2 link-underline-opacity-25 link-underline-opacity-100-hover"
                to="/gravity/formula"
                >formula</router-link
              >, unable to calculate gravity
            </p>
          </BsCard>
        </div>

        <div
          class="col-md-4"
          v-if="
            status.self_check.gyro_calibration &&
            status.self_check.gyro_connected &&
            status.wifi_setup == false &&
            status.angle !== undefined
          "
        >
          <BsCard header="Measurement" color="info" title="Angle">
            <p class="text-center">
              <template v-if="status.self_check.gyro_moving"> Gyro is moving </template>
              <template v-else>
                {{ status.angle }}
              </template>
            </p>
          </BsCard>
        </div>
        <div
          class="col-md-4"
          v-if="
            status.self_check.gyro_calibration &&
            status.self_check.gyro_connected &&
            status.wifi_setup == false &&
            status.angle !== undefined
          "
        >
          <BsCard header="Measurement" color="info" title="Average Angle">
            <p class="text-center">
              {{ angle.average }} ({{ angle.count }})
              <button
                @click="clearAverage"
                type="button"
                class="btn btn-outline-info btn-sm"
                style="font-size: 0.7rem"
              >
                Clear
              </button>
            </p>
          </BsCard>
        </div>
        <div
          class="col-md-4"
          v-if="!status.self_check.gyro_calibration && status.wifi_setup == false"
        >
          <BsCard header="Measurement" title="Error" :iserr="true" icon="bi-x-circle">
            <p class="text-center">
              Gyro has not been
              <router-link
                class="link-primary link-offset-2 link-underline-opacity-25 link-underline-opacity-100-hover"
                to="/device/gyro"
                >calibrated</router-link
              >
              at 90 degrees
            </p>
          </BsCard>
        </div>

        <div
          class="col-md-4"
          v-if="
            status.self_check.temp_connected &&
            status.wifi_setup == false &&
            status.temp !== undefined
          "
        >
          <BsCard header="Measurement" color="info" title="Temperature">
            <p class="text-center">{{ status.temp }} °{{ status.temp_unit }}</p>
          </BsCard>
        </div>

        <div
          class="col-md-4"
          v-if="!status.self_check.temp_connected && status.wifi_setup == false"
        >
          <BsCard header="Measurement" title="Error" :iserr="true" icon="bi-x-circle">
            <p class="text-center">No temperature sensor detected</p>
          </BsCard>
        </div>

        <div
          class="col-md-4"
          v-if="status.self_check.battery_level && status.battery !== undefined"
        >
          <BsCard header="Measurement" color="info" title="Battery">
            <p class="text-center">{{ status.battery }} V ({{ batteryPercentage }})</p>
          </BsCard>
        </div>
        <div class="col-md-4" v-if="!status.self_check.battery_level">
          <BsCard header="Measurement" title="Error" :iserr="true" icon="bi-x-circle">
            <p class="text-center">Battery level not valid</p>
          </BsCard>
        </div>

        <div class="col-md-4">
          <BsCard header="Device" title="WIFI">
            <p class="text-center">{{ status.rssi }} dBm - {{ status.wifi_ssid }}</p>
          </BsCard>
        </div>

        <div class="col-md-4">
          <BsCard header="Device" title="IP Address">
            <p class="text-center">
              {{ status.ip }}
            </p>
          </BsCard>
        </div>

        <div class="col-md-4">
          <BsCard header="Device" title="Memory">
            <p class="text-center">
              Free: {{ status.free_heap }} kb, Total: {{ status.total_heap }} kb
            </p>
          </BsCard>
        </div>

        <div class="col-md-4" v-if="newVersion.new">
          <BsCard header="Device" title="Upgrade available">
            <p class="text-center">
              {{ newVersion.ver }} available on
              <a
                class="link-primary link-offset-2 link-underline-opacity-25 link-underline-opacity-100-hover"
                href="https://github.com/mp-se/gravitymon/releases"
                target="_blank"
                >github.com</a
              >
            </p>
          </BsCard>
        </div>

        <div class="col-md-4">
          <BsCard header="Device" title="Software version">
            <p class="text-center">
              Firmware: {{ global.app_ver }} ({{ global.app_build }}) UI: {{ global.uiVersion }} ({{
                global.uiBuild
              }})
            </p>
          </BsCard>
        </div>

        <div class="col-md-4">
          <BsCard header="Device" title="Platform">
            <p class="text-center">
              Platform: <span class="badge bg-secondary">{{ global.platform }}</span> Variant:
              <span class="badge bg-secondary">{{ global.hardware }}</span>
            </p>
          </BsCard>
        </div>

        <div class="col-md-4" v-if="status.wifi_setup == false">
          <BsCard header="Device" title="Hardware">
            <p class="text-center">
              Board: <span class="badge bg-secondary">{{ global.board }}</span> Gyro:
              <span class="badge bg-secondary">{{ status.gyro_family }}</span>
            </p>
          </BsCard>
        </div>

        <div class="col-md-4">
          <BsCard header="Device" title="ID">
            <p class="text-center">{{ status.id }}</p>
          </BsCard>
        </div>

        <div class="col-md-4">
          <BsCard header="Device" title="Force config mode">
            <div class="d-flex justify-content-center">
              <div class="form-check form-switch">
                <input
                  v-model="flag"
                  class="form-check-input p-2"
                  type="checkbox"
                  role="switch"
                  style="
                    transform: scale(1.5);
                    transform-origin: left center;
                    display: inline-block;
                  "
                />
              </div>
            </div>
          </BsCard>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, computed, watch, onMounted, onBeforeMount } from 'vue'
import { status, global, config } from '@/modules/pinia'
import { logDebug, logError, logInfo } from '@mp-se/espframework-ui-components'
import { useTimers, useFetch } from '@mp-se/espframework-ui-components'
import { storeToRefs } from 'pinia'

const { createInterval, createTimeout } = useTimers()
const { managedFetch } = useFetch()

const polling = ref(null)
const flag = ref(false)
const angle = ref({ average: 0, sum: 0, count: 0 })
const newVersion = ref({ new: false, ver: '' })
const { sleep_mode } = storeToRefs(status)

const batteryPercentage = computed(() => {
  if (!status.battery) return '0%'
  
  // 18650 battery voltage to percentage mapping based on actual non-linear discharge curve
  // Voltages are non-uniform to reflect real capacity distribution (flat plateau ~3.6-3.7V)
  const batteryDataPoints = [
    { voltage: 4.2, percentage: 100 },
    { voltage: 4.1, percentage: 95 },
    { voltage: 4.0, percentage: 90 },
    { voltage: 3.93, percentage: 85 },
    { voltage: 3.87, percentage: 80 },
    { voltage: 3.83, percentage: 75 },
    { voltage: 3.79, percentage: 70 },
    { voltage: 3.75, percentage: 65 },
    { voltage: 3.71, percentage: 60 },
    { voltage: 3.68, percentage: 55 },
    { voltage: 3.65, percentage: 50 },
    { voltage: 3.62, percentage: 45 },
    { voltage: 3.58, percentage: 40 },
    { voltage: 3.55, percentage: 35 },
    { voltage: 3.52, percentage: 30 },
    { voltage: 3.48, percentage: 25 },
    { voltage: 3.43, percentage: 20 },
    { voltage: 3.37, percentage: 15 },
    { voltage: 3.30, percentage: 10 },
    { voltage: 3.24, percentage: 5 },
    { voltage: 3.2, percentage: 0 }
  ]
  
  const voltage = status.battery
  
  // Find surrounding datapoints for interpolation
  for (let i = 0; i < batteryDataPoints.length - 1; i++) {
    if (voltage >= batteryDataPoints[i + 1].voltage && voltage <= batteryDataPoints[i].voltage) {
      const upper = batteryDataPoints[i]
      const lower = batteryDataPoints[i + 1]
      const ratio = (voltage - lower.voltage) / (upper.voltage - lower.voltage)
      const percentage = lower.percentage + ratio * (upper.percentage - lower.percentage)
      return `${Math.round(percentage)}%`
    }
  }
  
  // If voltage is above max or below min
  if (voltage > batteryDataPoints[0].voltage) return '100%'
  if (voltage < batteryDataPoints[batteryDataPoints.length - 1].voltage) return '0%'
  
  return '0%'
})

watch(flag, async () => {
  try {
    await config.setSleepMode(flag.value)
  } catch (err) {
    logError('HomeView.setSleepMode()', err)
  }
})

watch(sleep_mode, () => {
  logDebug(
    'HomeView.watch(sleep_mode)',
    `sleep_mode changed to ${sleep_mode.value}, flag: ${flag.value}`
  )
  if (flag.value !== sleep_mode.value) {
    flag.value = sleep_mode.value
  }
})

function clearAverage() {
  angle.value.sum = 0
  angle.value.count = 0
  angle.value.sum = 0
}

async function refresh() {
  const success = await status.load()
  if (success) {
    if (!status.self_check.gyro_moving) {
      angle.value.sum += parseFloat(status.angle)
      angle.value.count++
      angle.value.average = (Math.round((angle.value.sum / angle.value.count) * 100) / 100).toFixed(
        2
      )
    }
  }
}

onMounted(async () => {
  flag.value = status.sleep_mode

  createTimeout(async () => {
    try {
      logInfo('HomeView.onMounted()', 'Checking for new sw')
      const response = await managedFetch('https://www.gravitymon.com/firmware/version.json')

      if (!response) {
        // Request was aborted
        return
      }

      const json = await response.json()

      logDebug('HomeView.onMounted()', json)
      if (checkForNewGravMonVersion(json)) {
        newVersion.value.new = true
        newVersion.value.ver = json.version
        logInfo('HomeView.onMounted()', 'Newer version found')
      }

      logInfo('HomeView.onMounted()', 'Fetching latest gravtmon version completed')
    } catch (err) {
      logError('HomeView.onMounted()', err)
    }
  }, 500)
})

onBeforeMount(() => {
  refresh()
  polling.value = createInterval(refresh, 4000)
})

function checkForNewGravMonVersion(json) {
  const current = global.app_ver
  const latest = json.version

  const newVer = latest.split('.')
  const curVer = current.split('.')

  if (newVer.length != 3 && curVer.length != 3) return false

  if (newVer[0] > curVer[0]) return true
  else if (newVer[0] == curVer[0] && newVer[1] > curVer[1]) return true
  else if (newVer[0] == curVer[0] && newVer[1] == curVer[1] && newVer[2] > curVer[2]) return true

  return false
}
</script>

<style></style>
