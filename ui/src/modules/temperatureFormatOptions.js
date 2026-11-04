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
import { ref } from 'vue'

// Placeholder temperature format options for ui
// Gravitymon only measures gravity, but the multi-type push views need these for completeness.
// The real options live in the tempmon repo (like the pressure options live in pressuremon).
export const temperatureHttpPostFormatOptions = ref([
  {
    label: 'Empty',
    value: '{}'
  }
])

export const temperatureHttpGetFormatOptions = ref([
  {
    label: 'Empty',
    value: ''
  }
])

export const temperatureInfluxdb2FormatOptions = ref([
  {
    label: 'Empty',
    value: ''
  }
])

export const temperatureMqttFormatOptions = ref([
  {
    label: 'Empty',
    value: '|'
  }
])
