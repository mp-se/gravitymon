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
    <p class="h3">{{ t('serial.title', { state: connected }) }}</p>
    <hr />

    <pre>{{ serial }}</pre>

    <div class="row gy-2">
      <div class="col-md-12">
        <hr />
      </div>
      <div class="col-md-12">
        <button @click="clear" type="button" class="btn btn-primary w-2" :disabled="!isConnected">
          {{ t('serial.clear') }}</button
        >&nbsp;

        <button
          @click="connect"
          type="button"
          class="btn btn-secondary w-2"
          :disabled="isConnected"
        >
          {{ t('serial.connect') }}
        </button>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, onMounted, onUnmounted, computed } from 'vue'
import { useI18n } from 'vue-i18n'
import { sharedHttpClient as http } from '@mp-se/espframework-ui-components'

const { t } = useI18n()
const socket = ref(null)
let socketCloser = null
const serial = ref('')
const maxLines = 50

function clear() {
  serial.value = ''
}

onUnmounted(() => {
  if (typeof socketCloser === 'function') {
    try {
      socketCloser()
    } catch {
      // ignore
    }
    socketCloser = null
  } else if (socket.value) {
    try {
      socket.value.close()
    } catch {
      // ignore
    }
  }
  socket.value = null
})

const isConnected = computed(() => {
  return socket.value === null ? false : true
})

const connected = computed(() => {
  return socket.value === null ? t('serial.not_connected') : t('serial.connected')
})

function connect() {
  serial.value = 'Attempting to connect to websocket\n'
  const ws = http.createWebSocket('serialws', {
    onOpen() {
      serial.value += 'Websocket established\n'
    },
    onMessage(event) {
      var list = serial.value.split('\n')

      while (list.length > maxLines) {
        list.shift()
      }

      serial.value = list.join('\n')
      serial.value += event.data
    },
    onClose() {
      serial.value += 'Socket closed\n'
      socket.value = null
    },
    onError(err) {
      serial.value += 'Websocket error: ' + (err && err.message ? err.message : err) + '\n'
    },
    autoReconnect: true
  })

  socketCloser = ws.close
  socket.value = ws.socketGetter()
}

onMounted(() => {
  connect()
})
</script>
