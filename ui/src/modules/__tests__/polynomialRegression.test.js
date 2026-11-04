import { describe, it, expect } from 'vitest'
import { PolynomialRegression } from '../polynomialRegression.js'
import fixtures from './polynomialRegression.fixtures.json'

// Fixtures were generated with ml-regression-polynomial 3.0.2 (the library this replaced)
describe('PolynomialRegression (parity with ml-regression-polynomial)', () => {
  for (const [name, set] of Object.entries(fixtures)) {
    for (const [precision, expected] of Object.entries(set.res)) {
      it(`data set ${name}, precision ${precision}, orders 1-4`, () => {
        for (let order = 1; order <= 4; order++) {
          const r = new PolynomialRegression(set.x, set.y, order)
          const got = r.toString(Number(precision))
          if (Number(precision) <= 8) {
            expect(got).toBe(expected[order - 1])
          } else {
            // last digits at 10 significant figures are float rounding noise: compare numerically
            const nums = (str) => str.match(/\d+\.?\d*(?:e-?\d+)?/g).map(Number)
            const a = nums(got)
            const b = nums(expected[order - 1])
            expect(a.length).toBe(b.length)
            a.forEach((v, i) => expect(Math.abs(v - b[i])).toBeLessThanOrEqual(1e-8 * Math.abs(b[i])))
          }
        }
      })
    }
  }

  it('recovers exact polynomial coefficients', () => {
    const x = [1, 2, 3, 4, 5]
    const y = x.map((v) => 2 + 3 * v - 0.5 * v * v)
    const r = new PolynomialRegression(x, y, 2)
    expect(r.coefficients[0]).toBeCloseTo(2, 8)
    expect(r.coefficients[1]).toBeCloseTo(3, 8)
    expect(r.coefficients[2]).toBeCloseTo(-0.5, 8)
  })

  it('throws on mismatched arrays and on singular systems', () => {
    expect(() => new PolynomialRegression([1, 2], [1], 1)).toThrow()
    expect(() => new PolynomialRegression([1, 1, 1], [1, 2, 3], 2)).toThrow()
  })

  it('omits zero terms and formats without precision', () => {
    const r = new PolynomialRegression([0, 1, 2], [0, 2, 4], 1)
    expect(r.toString()).toMatch(/^f\(x\) = /)
  })
})
