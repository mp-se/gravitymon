import { createI18n } from 'vue-i18n'

// Standard sections that are reused as they are. Named imports let the bundler
// drop every other section of the standard files.
import {
  device_hardware as de_hw,
  device_wifi as de_wifi,
  firmware as de_fw,
  gravity_formula as de_gf,
  push_bluetooth as de_ble,
  push_common as de_push,
  push_wifi_direct as de_direct
} from '@/locales/de.json'
import {
  device_hardware as en_hw,
  device_wifi as en_wifi,
  firmware as en_fw,
  gravity_formula as en_gf,
  push_bluetooth as en_ble,
  push_common as en_push,
  push_wifi_direct as en_direct
} from '@/locales/en.json'
import {
  device_hardware as es_hw,
  device_wifi as es_wifi,
  firmware as es_fw,
  gravity_formula as es_gf,
  push_bluetooth as es_ble,
  push_common as es_push,
  push_wifi_direct as es_direct
} from '@/locales/es.json'
import {
  device_hardware as fr_hw,
  device_wifi as fr_wifi,
  firmware as fr_fw,
  gravity_formula as fr_gf,
  push_bluetooth as fr_ble,
  push_common as fr_push,
  push_wifi_direct as fr_direct
} from '@/locales/fr.json'
import {
  device_hardware as zh_hw,
  device_wifi as zh_wifi,
  firmware as zh_fw,
  gravity_formula as zh_gf,
  push_bluetooth as zh_ble,
  push_common as zh_push,
  push_wifi_direct as zh_direct
} from '@/locales/zh.json'

// CuckooTilt specific text
import de from '@custom/locales/de.json'
import en from '@custom/locales/en.json'
import es from '@custom/locales/es.json'
import fr from '@custom/locales/fr.json'
import zh from '@custom/locales/zh.json'

// All languages are embedded in the UI (the standard UI only embeds English
// and loads other languages as optional packs from the device).
export const languages = [
  { code: 'en', label: 'English' },
  { code: 'fr', label: 'Français' },
  { code: 'es', label: 'Español' },
  { code: 'de', label: 'Deutsch' },
  { code: 'zh', label: '中文' }
]

const i18n = createI18n({
  legacy: false,
  locale: 'en',
  fallbackLocale: 'en',
  messages: {
    en: {
      device_hardware: en_hw,
      device_wifi: en_wifi,
      firmware: en_fw,
      gravity_formula: en_gf,
      push_bluetooth: en_ble,
      push_common: en_push,
      push_wifi_direct: en_direct,
      ...en
    },
    fr: {
      device_hardware: fr_hw,
      device_wifi: fr_wifi,
      firmware: fr_fw,
      gravity_formula: fr_gf,
      push_bluetooth: fr_ble,
      push_common: fr_push,
      push_wifi_direct: fr_direct,
      ...fr
    },
    es: {
      device_hardware: es_hw,
      device_wifi: es_wifi,
      firmware: es_fw,
      gravity_formula: es_gf,
      push_bluetooth: es_ble,
      push_common: es_push,
      push_wifi_direct: es_direct,
      ...es
    },
    de: {
      device_hardware: de_hw,
      device_wifi: de_wifi,
      firmware: de_fw,
      gravity_formula: de_gf,
      push_bluetooth: de_ble,
      push_common: de_push,
      push_wifi_direct: de_direct,
      ...de
    },
    zh: {
      device_hardware: zh_hw,
      device_wifi: zh_wifi,
      firmware: zh_fw,
      gravity_formula: zh_gf,
      push_bluetooth: zh_ble,
      push_common: zh_push,
      push_wifi_direct: zh_direct,
      ...zh
    }
  }
})

export default i18n
