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
import { global } from '@/modules/pinia'
import {
  logError,
  tempToF,
  tempToC,
  psiToBar,
  psiToKPa,
  barToPsi,
  kpaToPsi
} from '@mp-se/espframework-ui-components'

export function applyTemplate(status, config, template) {
  let s = template

  s = s.replaceAll('${temp}', status.temp)

  let c = status.temp
  let f = status.temp

  if (config.temp_unit === 'C') {
    f = tempToF(status.temp)
  } else {
    c = tempToC(status.temp)
  }

  s = s.replaceAll('${temp-c}', c)
  s = s.replaceAll('${temp-f}', f)

  // NOTE: Gravity handling (gated by feature flag)
  if (global.ui.enableGravity) {
    s = s.replaceAll('${angle}', status.angle)
    s = s.replaceAll('${tilt}', status.angle)
    s = s.replaceAll('${velocity}', -12)
    s = s.replaceAll('${corr-gravity}', status.gravity)

    if (config.gravity_unit === 'G') {
      const sg = status.gravity
      s = s.replaceAll('${gravity}', sg)
      s = s.replaceAll('${gravity-sg}', sg)
      s = s.replaceAll('${corr-gravity-sg}', sg)
      const plato = 259 - (259 - sg)
      s = s.replaceAll('${gravity-plato}', plato)
      s = s.replaceAll('${corr-gravity-plato}', plato)
    } else {
      const plato = status.gravity
      s = s.replaceAll('${gravity}', plato)
      s = s.replaceAll('${gravity-plato}', plato)
      s = s.replaceAll('${corr-gravity-plato}', plato)
      const sg = 259 / (259 - plato)
      s = s.replaceAll('${gravity-sg}', sg)
      s = s.replaceAll('${corr-gravity-sg}', sg)
    }
  }

  // NOTE: Pressure handling (gated by feature flag)
  if (global.ui.enablePressure) {
    let p = status.pressure

    if (status.isKPa) {
      p = kpaToPsi(p)
    } else if (status.isBar) {
      p = barToPsi(p)
    }

    s = s.replaceAll('${pressure}', p)
    s = s.replaceAll('${pressure-psi}', p)
    s = s.replaceAll('${pressure-bar}', psiToBar(p))
    s = s.replaceAll('${pressure-kpa}', psiToKPa(p))

    let p1 = status.pressure1

    if (status.isKPa) {
      p1 = kpaToPsi(p1)
    } else if (status.isBar) {
      p1 = barToPsi(p1)
    }

    s = s.replaceAll('${pressure1}', p1)
    s = s.replaceAll('${pressure1-psi}', p1)
    s = s.replaceAll('${pressure1-bar}', psiToBar(p1))
    s = s.replaceAll('${pressure1-kpa}', psiToKPa(p1))
  }

  s = s.replaceAll('${app-ver}', global.app_ver)
  s = s.replaceAll('${app-build}', global.app_build)
  s = s.replaceAll('${battery-percent}', 100)
  s = s.replaceAll('${rssi}', status.rssi)
  s = s.replaceAll('${run-time}', 5)
  s = s.replaceAll('${battery}', status.battery)

  s = s.replaceAll('${mdns}', config.mdns)
  s = s.replaceAll('${id}', config.id)
  s = s.replaceAll('${sleep-interval}', config.sleep_interval)
  s = s.replaceAll('${token}', config.token)
  s = s.replaceAll('${token2}', config.token2)
  s = s.replaceAll('${temp-unit}', config.temp_unit)
  s = s.replaceAll('${gravity-unit}', config.gravity_unit)
  s = s.replaceAll('${pressure-unit}', config.pressure_unit)

  try {
    return JSON.stringify(JSON.parse(s), null, 2)
  } catch {
    logError('formatTemplate.applyTemplate()', 'Not a valid json document, returning string')
  }

  return s
}
