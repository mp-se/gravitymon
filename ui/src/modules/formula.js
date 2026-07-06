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
import { config } from '@/modules/pinia'
import { logError, logDebug } from '@mp-se/espframework-ui-components'
import { gravityToPlato } from '@mp-se/espframework-ui-components'

function applyValuesToFormula(formula, tilt) {
  let angle = tilt.toFixed(3)
  let f = formula
  f = f.replaceAll('tilt^3', angle + '*' + angle + '*' + angle)
  f = f.replaceAll('tilt^2', angle + '*' + angle)
  f = f.replaceAll('tilt', angle)
  return f
}

export function calculate(formula, tilt) {
  if (formula != '') {
    let f = applyValuesToFormula(formula, tilt)

    try {
      let g = eval(f)
      return config.gravity_unit === 'P' ? gravityToPlato(g) : g
    } catch (err) {
      logError('formula.evaluateFormula()', err)
    }
  }

  return NaN
}

export function evaluateFormula(formula) {
  var result = []

  for (let a = 25.0; a < 80.0; a += 5.0) {
    var g = calculate(formula, a)
    result.push({ x: parseFloat(a), y: parseFloat(g) })
  }

  return result
}

export function validateFormula(formula) {
  var result = true

  config.formula_calculation_data.forEach((d) => {
    let f = applyValuesToFormula(formula, d.a)

    try {
      let g = eval(f)
      if (config.gravity_unit === 'P') g = gravityToPlato(g)

      if (Math.abs(g - d.g) > config.formula_max_deviation) {
        logDebug('formula.validateFormula()', 'Formula rejected due to high deviation', d.g, g)
        result = false
      }
    } catch {
      logDebug('formula.validateFormula()', 'Failed to evalutate formula', formula)
    }
  })

  return result
}
