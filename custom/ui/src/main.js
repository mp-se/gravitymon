import { createApp } from 'vue'
import App from './App.vue'
import piniaInstance, { global } from '@/modules/pinia.js'
import router from './router.js'
import i18n from './i18n.js'
import { registerCuckooConfig } from './cuckooConfig.js'

// Only the components used by the pages in this UI
import {
  BsMessage,
  BsFileUpload,
  BsProgress,
  BsInputBase,
  BsInputText,
  BsInputNumber,
  BsInputSwitch,
  BsInputRadio,
  BsInputTextAreaFormat,
  BsSelect,
  BsDropdown,
  BsModal,
  BsModalConfirm
} from '@mp-se/espframework-ui-components'

import 'bootstrap/dist/css/bootstrap.css'
import '@/modules/bootstrapMenu'

// UI feature flags (standard defaults are in the global store). Everything is
// off for CuckooTilt except gravity and scanning for the strongest access point.
Object.assign(global.ui, {
  enableVoltageFragment: false,
  enableManualWifiEntry: false,
  enableScanForStrongestAp: true,
  enableDeviceRegistration: false,
  enableCalibrationRegistration: false,
  enableGravity: true,
  enablePressure: false,
  enableHttpPostTcpMode: false,
  enableLanguageDownload: false
})

// Must be registered before the configuration is loaded
registerCuckooConfig()

const app = createApp(App)
app.use(piniaInstance)
app.use(router)
app.use(i18n)

app.component('BsMessage', BsMessage)
app.component('BsFileUpload', BsFileUpload)
app.component('BsProgress', BsProgress)
// The label, help text and spacing of every input component come from
// BsInputBase, which the library looks up by name: it must be registered, even
// though no page uses it directly.
app.component('BsInputBase', BsInputBase)
app.component('BsInputText', BsInputText)
app.component('BsInputNumber', BsInputNumber)
app.component('BsInputSwitch', BsInputSwitch)
app.component('BsInputRadio', BsInputRadio)
app.component('BsInputTextAreaFormat', BsInputTextAreaFormat)
app.component('BsSelect', BsSelect)
app.component('BsDropdown', BsDropdown)
app.component('BsModal', BsModal)
app.component('BsModalConfirm', BsModalConfirm)

app.mount('#app')
