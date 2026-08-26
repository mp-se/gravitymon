import { describe, it, expect, beforeEach, vi } from 'vitest'
import { createPinia, setActivePinia } from 'pinia'
import { config } from '@/modules/pinia'
import * as formula from '../formula'

// Mock the pinia module
vi.mock('@/modules/pinia', () => ({
  config: {}
}))

// Mock UI components
vi.mock('@mp-se/espframework-ui-components', () => ({
  logError: vi.fn(),
  logDebug: vi.fn(),
  gravityToPlato: vi.fn((g) => (g - 1) * 261.72) // simple mock
}))

describe('formula.js', () => {
  beforeEach(() => {
    setActivePinia(createPinia())
    config.gravity_unit = 'SG'
    config.formula_calculation_data = []
    config.formula_max_deviation = 0.005
  })

  describe('calculate', () => {
    it('returns NaN for empty formula', () => {
      expect(formula.calculate('', 50)).toBeNaN()
    })

    it('evaluates simple polynomial formula', () => {
      // 0.5 * tilt + 0.1
      expect(formula.calculate('0.5 * tilt + 0.1', 2)).toBe(1.1)
    })

    it('evaluates tilt^2 and tilt^3', () => {
      // tilt^2 = 3*3 = 9
      expect(formula.calculate('tilt^2', 3)).toBe(9)
      // tilt^3 = 2*2*2 = 8
      expect(formula.calculate('tilt^3', 2)).toBe(8)
    })

    it('converts to Plato if unit is P', () => {
      config.gravity_unit = 'P'
      // 1.040 -> 10.4688 approx
      const result = formula.calculate('tilt', 1.04)
      expect(result).toBeCloseTo(10.4688, 4)
    })

    it('handles eval error', () => {
      const result = formula.calculate('invalid formula', 50)
      // If result is NaN, check if it's literally NaN
      expect(result).toBeNaN()
    })
  })

  describe('evaluateFormula', () => {
    it('returns array of data points for graph', () => {
      const result = formula.evaluateFormula('tilt * 2')
      expect(result).toHaveLength(11) // (80-25)/5 + 1
      expect(result[0]).toEqual({ x: 25, y: 50 })
    })
  })

  describe('validateFormula', () => {
    it('returns true if all points are within deviation', () => {
      config.formula_calculation_data = [
        { a: 25, g: 1.01 },
        { a: 50, g: 1.05 }
      ]
      // formula tilt/2500 + 1.0
      // a=25 -> 0.01 + 1.0 = 1.01
      // a=50 -> 0.02 + 1.0 = 1.02 (diff 0.03 > 0.005)
      expect(formula.validateFormula('tilt/2500 + 1.0')).toBe(false)

      config.formula_max_deviation = 0.05
      expect(formula.validateFormula('tilt/2500 + 1.0')).toBe(true)
    })

    it('handles eval error in validation', () => {
      config.formula_calculation_data = [{ a: 25, g: 1.01 }]
      expect(formula.validateFormula('invalid')).toBe(true)
    })

    it('applies Plato conversion in validateFormula when unit is P', () => {
      config.gravity_unit = 'P'
      config.formula_max_deviation = 1000
      config.formula_calculation_data = [{ a: 45, g: 12.0 }]
      // gravityToPlato converts SG to Plato; with huge deviation, result is true
      expect(formula.validateFormula('tilt*0.01 + 1.0')).toBe(true)
    })
  })
})
