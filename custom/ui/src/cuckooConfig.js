import { config } from '@/modules/pinia'

// The settings that only exist in the CuckooTilt firmware (CuckooConfig). The
// standard config store does not know these keys, so they are added to its state
// when the configuration is loaded. They are then part of the change detection
// ("Save needed") and are sent by config.saveAll() in the same request as the
// standard settings.
const KEYS = ['http_post_sleep_interval', 'ble_sleep_interval', 'use_http_post', 'use_ble']

export function registerCuckooConfig() {
  config.registerLoadCallback((json, store) => {
    const values = {}
    for (const key of KEYS) {
      if (json[key] !== undefined) values[key] = json[key]
    }
    store.$patch(values)
  })
}

// The device wakes at sleep_interval and the HTTP post targets are pushed every
// (http_post_int + 1) wakes, so the intervals of the two channels are turned
// into those standard settings before saving, as the manufacturer's UI does.
export function calculateIntervals() {
  let sleep = 3600
  let skip = 0

  if (cuckoo.use_http_post && cuckoo.use_ble) {
    sleep = Math.min(cuckoo.http_post_sleep_interval, cuckoo.ble_sleep_interval)
    skip = Math.floor(cuckoo.http_post_sleep_interval / sleep) - 1
  } else if (cuckoo.use_http_post) {
    sleep = cuckoo.http_post_sleep_interval
  } else if (cuckoo.use_ble) {
    sleep = cuckoo.ble_sleep_interval
  }

  // Anything below the minimum (30 s) is invalid, use the default
  if (!(sleep >= 30)) {
    sleep = 900
    skip = 0
  }

  config.sleep_interval = sleep
  config.http_post_int = skip
  config.http_post2_int = skip
}

export async function saveCuckooConfig() {
  calculateIntervals()
  await config.saveAll()
}

// A store only exposes the state keys it declared, so the added settings are
// reached through the state object itself (reactive, use it with v-model):
// cuckoo.use_http_post, cuckoo.http_post_sleep_interval, cuckoo.use_ble and
// cuckoo.ble_sleep_interval.
export const cuckoo = config.$state
