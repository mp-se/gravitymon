import { createRouter, createWebHistory } from 'vue-router'
import { global } from '@/modules/pinia'

import HomeView from '@custom/views/HomeView.vue'
import DeviceWifiView from '@custom/views/DeviceWifiView.vue'
import DeviceBatteryView from '@custom/views/DeviceBatteryView.vue'
import GravityFormulaView from '@custom/views/GravityFormulaView.vue'
import PushHttpPostView from '@custom/views/PushHttpPostView.vue'
import PushBluetoothView from '@custom/views/PushBluetoothView.vue'
import SystemToolsView from '@custom/views/SystemToolsView.vue'
import AboutView from '@custom/views/AboutView.vue'
import FirmwareView from '@custom/views/FirmwareView.vue'

const routes = [
  { path: '/', component: HomeView },
  { path: '/device/wifi', component: DeviceWifiView },
  { path: '/device/battery', component: DeviceBatteryView },
  { path: '/gravity/formula', component: GravityFormulaView },
  { path: '/push/http-post', component: PushHttpPostView },
  { path: '/push/bluetooth', component: PushBluetoothView },
  { path: '/other/system-tools', component: SystemToolsView },
  { path: '/other/firmware', component: FirmwareView },
  { path: '/other/about', component: AboutView },
  { path: '/:catchAll(.*)', redirect: '/' }
]

const router = createRouter({
  history: createWebHistory(),
  routes
})

router.beforeEach(() => {
  global.clearMessages()
  return true
})

export default router
