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
import { computed } from 'vue'
import { createRouter, createWebHistory } from 'vue-router'
import { validateCurrentForm } from '@mp-se/espframework-ui-components'
import * as badge from '@/modules/badge'
import { global } from '@/modules/pinia'
import i18n from '@/modules/i18n'

const { t } = i18n.global

import HomeView from '@/views/HomeView.vue'
import NotFoundView from '@/views/NotFoundView.vue'
import DeviceSettingsView from '@/views/DeviceSettingsView.vue'
import DeviceHardwareView from '@/views/DeviceHardwareView.vue'
import DeviceGyroView from '@/views/DeviceGyroView.vue'
import DeviceWifiView from '@/views/DeviceWifiView.vue'
import DeviceWifi2View from '@/views/DeviceWifi2View.vue'
import GravitySettingsView from '@/views/GravitySettingsView.vue'
import GravityFormulaView from '@/views/GravityFormulaView.vue'
import PushSettingsView from '@/views/PushSettingsView.vue'
import PushWifiDirectView from '@/views/PushWifiDirectView.vue'
import PushHttpPost1View from '@/views/PushHttpPost1View.vue'
import PushHttpPost2View from '@/views/PushHttpPost2View.vue'
import PushHttpGetView from '@/views/PushHttpGetView.vue'
import PushInfluxdbView from '@/views/PushInfluxdbView.vue'
import PushMqttView from '@/views/PushMqttView.vue'
import PushBluetoothView from '@/views/PushBluetoothView.vue'
import AboutView from '@/views/AboutView.vue'
import BackupView from '@/views/BackupView.vue'
import FirmwareView from '@/views/FirmwareView.vue'
import SupportView from '@/views/SupportView.vue'
import SerialView from '@/views/SerialView.vue'
import ToolsView from '@/views/ToolsView.vue'

const routes = [
  {
    path: '/',
    name: 'home',
    component: HomeView
  },
  {
    path: '/device/settings',
    name: 'device-settings',
    component: DeviceSettingsView
  },
  {
    path: '/device/hardware',
    name: 'device-hardware',
    component: DeviceHardwareView
  },
  {
    path: '/device/gyro',
    name: 'device-gyro',
    component: DeviceGyroView
  },
  {
    path: '/device/wifi',
    name: 'device-wifi',
    component: DeviceWifiView
  },
  {
    path: '/device/wifi2',
    name: 'device-wifi-manual',
    component: DeviceWifi2View
  },
  {
    path: '/other/backup',
    name: 'backup',
    component: BackupView
  },
  {
    path: '/gravity/settings',
    name: 'gravity-settings',
    component: GravitySettingsView
  },
  {
    path: '/gravity/formula',
    name: 'gravity-formula',
    component: GravityFormulaView
  },
  {
    path: '/other/firmware',
    name: 'firmware',
    component: FirmwareView
  },
  {
    path: '/push/settings',
    name: 'push-settings',
    component: PushSettingsView
  },
  {
    path: '/push/wifi-direct',
    name: 'push-wifi-direct',
    component: PushWifiDirectView
  },
  {
    path: '/push/http-post1',
    name: 'push-http-post1',
    component: PushHttpPost1View
  },
  {
    path: '/push/http-post2',
    name: 'push-http-post2',
    component: PushHttpPost2View
  },
  {
    path: '/push/http-get',
    name: 'push-http-get',
    component: PushHttpGetView
  },
  {
    path: '/push/influxdb',
    name: 'push-influxdb',
    component: PushInfluxdbView
  },
  {
    path: '/push/mqtt',
    name: 'push-Mqtt',
    component: PushMqttView
  },
  {
    path: '/push/bluetooth',
    name: 'push-bluetooth',
    component: PushBluetoothView
  },
  {
    path: '/other/support',
    name: 'support',
    component: SupportView
  },
  {
    path: '/other/tools',
    name: 'tools',
    component: ToolsView
  },
  {
    path: '/other/serial',
    name: 'serial',
    component: SerialView
  },
  {
    path: '/other/about',
    name: 'about',
    component: AboutView
  },
  {
    path: '/:catchAll(.*)',
    name: '404',
    component: NotFoundView
  }
]

const router = createRouter({
  history: createWebHistory(import.meta.env.BASE_URL),
  routes: routes
})

export default router

router.beforeEach(() => {
  if (global.disabled) return false

  if (!validateCurrentForm()) return false

  global.clearMessages()
  return true
})

const items = computed(() => [
  {
    label: t('router.home'),
    icon: 'IconHome',
    path: '/',
    subs: []
  },
  {
    label: t('router.device'),
    icon: 'IconCpu',
    path: '/device',
    badge: badge.deviceBadge,
    subs: [
      {
        label: t('router.device_settings'),
        badge: badge.deviceSettingBadge,
        path: '/device/settings'
      },
      {
        label: t('router.device_hardware'),
        badge: badge.deviceHardwareBadge,
        path: '/device/hardware'
      },
      {
        label: t('router.device_gyro'),
        badge: badge.deviceGyroBadge,
        path: '/device/gyro'
      },
      {
        label: t('router.device_wifi'),
        badge: badge.deviceWifiBadge,
        path: '/device/wifi'
      }
    ]
  },
  {
    label: t('router.gravity'),
    icon: 'IconGraphUpArrow',
    path: '/gravity',
    badge: badge.gravityBadge,
    subs: [
      {
        label: t('router.gravity_settings'),
        badge: badge.gravitySettingBadge,
        path: '/gravity/settings'
      },
      {
        label: t('router.gravity_formula'),
        badge: badge.gravityFormulaBadge,
        path: '/gravity/formula'
      }
    ]
  },
  {
    label: t('router.push_targets'),
    icon: 'IconCloudUpArrow',
    path: '/push',
    badge: badge.pushBadge,
    subs: [
      {
        label: t('router.push_settings'),
        badge: badge.pushSettingBadge,
        path: '/push/settings'
      },
      {
        label: t('router.push_wifi_direct'),
        badge: badge.pushWifiDirectBadge,
        path: '/push/wifi-direct'
      },
      {
        label: t('router.push_http_post1'),
        badge: badge.pushHttpPost1Badge,
        path: '/push/http-post1'
      },
      {
        label: t('router.push_http_post2'),
        badge: badge.pushHttpPost2Badge,
        path: '/push/http-post2'
      },
      {
        label: t('router.push_http_get'),
        badge: badge.pushHttpGetBadge,
        path: '/push/http-get'
      },
      {
        label: t('router.push_influxdb'),
        badge: badge.pushInfluxdb2Badge,
        path: '/push/influxdb'
      },
      {
        label: t('router.push_mqtt'),
        badge: badge.pushMqttBadge,
        path: '/push/mqtt'
      },
      {
        label: t('router.push_bluetooth'),
        badge: badge.pushBluetoothBadge,
        path: '/push/bluetooth'
      }
    ]
  },
  {
    label: t('router.other'),
    icon: 'IconTools',
    path: '/other',
    subs: [
      {
        label: t('router.other_serial'),
        path: '/other/serial'
      },
      {
        label: t('router.other_backup'),
        path: '/other/backup'
      },
      {
        label: t('router.other_firmware'),
        path: '/other/firmware'
      },
      {
        label: t('router.other_support'),
        path: '/other/support'
      },
      {
        label: t('router.other_tools'),
        path: '/other/tools'
      },
      {
        label: t('router.other_about'),
        path: '/other/about'
      }
    ]
  }
])

export { items }
