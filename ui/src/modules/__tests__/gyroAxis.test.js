import { describe, it, expect } from 'vitest'
import {
  fitGyroAxis,
  axisAngle,
  legacyAngle,
  eigenSymmetric3,
  isGyroAxisCalibrated
} from '../gyroAxis'

const DEG = Math.PI / 180

// Rotation matrix helpers (column vectors, R * v)
const rotX = (a) => [
  [1, 0, 0],
  [0, Math.cos(a), -Math.sin(a)],
  [0, Math.sin(a), Math.cos(a)]
]
const rotZ = (a) => [
  [Math.cos(a), -Math.sin(a), 0],
  [Math.sin(a), Math.cos(a), 0],
  [0, 0, 1]
]
const rotY = (a) => [
  [Math.cos(a), 0, Math.sin(a)],
  [0, 1, 0],
  [-Math.sin(a), 0, Math.cos(a)]
]
const mul = (m, v) => m.map((r) => r[0] * v[0] + r[1] * v[1] + r[2] * v[2])
const mulM = (a, b) =>
  a.map((r, i) => [0, 1, 2].map((j) => a[i][0] * b[0][j] + a[i][1] * b[1][j] + a[i][2] * b[2][j]))

// Simulates a device. The case frame has Y as the long axis. The sensor is
// mounted with a small rotation inside the case and has a bias.
//   tilt  angle between the long axis and vertical (90 = flat on the table)
//   roll  rotation of the case around its long axis
function simulate({ mount, bias, tilt, roll, noise = 0, seed = 1 }) {
  const t = tilt * DEG
  // gravity in the case frame, before roll
  const gCase = mul(rotY(roll * DEG), [0, Math.cos(t), Math.sin(t)])
  const g = mul(mount, gCase)
  let s = seed
  const rnd = () => {
    s = (s * 16807) % 2147483647
    return (s / 2147483647 - 0.5) * 2 * noise
  }
  return { x: g[0] + bias[0] + rnd(), y: g[1] + bias[1] + rnd(), z: g[2] + bias[2] + rnd() }
}

const mount = mulM(rotX(2.5 * DEG), rotZ(-1.5 * DEG))
const bias = [0.03, -0.02, 0.05]

const table = (rolls, slope = 0, group = 0, extra = {}) =>
  rolls.map((roll, i) => ({
    ...simulate({ mount, bias, tilt: 90 - slope, roll, seed: i + 1 + group * 100, ...extra }),
    group
  }))

describe('gyroAxis.js', () => {
  describe('eigenSymmetric3', () => {
    it('finds eigenvalues of a diagonal matrix in ascending order', () => {
      const e = eigenSymmetric3([
        [3, 0, 0],
        [0, 1, 0],
        [0, 0, 2]
      ])
      expect(e.values[0]).toBeCloseTo(1)
      expect(e.values[1]).toBeCloseTo(2)
      expect(e.values[2]).toBeCloseTo(3)
      expect(Math.abs(e.vectors[0][1])).toBeCloseTo(1)
    })

    it('satisfies A*v = lambda*v for a full matrix', () => {
      const a = [
        [2, 1, 0.5],
        [1, 3, -0.2],
        [0.5, -0.2, 1]
      ]
      const e = eigenSymmetric3(a)
      for (let i = 0; i < 3; i++) {
        const av = mul(a, e.vectors[i])
        for (let k = 0; k < 3; k++) expect(av[k]).toBeCloseTo(e.values[i] * e.vectors[i][k], 8)
      }
    })
  })

  describe('fitGyroAxis', () => {
    it('reproduces the problem, legacy angle depends on roll', () => {
      const a = legacyAngle(Object.values(simulate({ mount, bias: [0, 0, 0], tilt: 90, roll: 0 })))
      const b = legacyAngle(
        Object.values(simulate({ mount, bias: [0, 0, 0], tilt: 90, roll: 180 }))
      )
      // Folded around 90 so both read below 90, but at an angle like 40 the
      // difference is visible.
      const c = legacyAngle(Object.values(simulate({ mount, bias: [0, 0, 0], tilt: 40, roll: 0 })))
      const d = legacyAngle(
        Object.values(simulate({ mount, bias: [0, 0, 0], tilt: 40, roll: 180 }))
      )
      expect(a).toBeLessThanOrEqual(90)
      expect(b).toBeLessThanOrEqual(90)
      expect(Math.abs(c - d)).toBeGreaterThan(3)
    })

    it('recovers the axis and in-plane bias from four rolls on a level table', () => {
      const res = fitGyroAxis(table([0, 90, 180, 270]))
      expect(res.success).toBe(true)

      const n = mul(mount, [0, 1, 0])
      expect(res.calibration.nx).toBeCloseTo(n[0], 4)
      expect(res.calibration.ny).toBeCloseTo(n[1], 4)
      expect(res.calibration.nz).toBeCloseTo(n[2], 4)
      expect(res.calibration.bx).toBeCloseTo(bias[0], 4)
      expect(res.calibration.by).toBeCloseTo(bias[1], 4)
      expect(res.calibration.bz).toBeCloseTo(bias[2], 4)
      expect(res.radius).toBeCloseTo(1, 4)
      expect(res.spreadAfter).toBeLessThan(0.01)
      expect(res.slope).toBeNull()
    })

    it('works with three unevenly spaced rolls', () => {
      const res = fitGyroAxis(table([10, 130, 215]))
      expect(res.success).toBe(true)
      expect(res.calibration.bx).toBeCloseTo(bias[0], 4)
      expect(res.spreadAfter).toBeLessThan(0.01)
    })

    it('gives a roll independent tilt at any angle', () => {
      const cal = fitGyroAxis(table([0, 90, 180, 270])).calibration

      for (const tilt of [20, 35, 50, 65, 80]) {
        for (const roll of [0, 45, 120, 180, 300]) {
          const p = simulate({ mount, bias, tilt, roll })
          expect(axisAngle([p.x, p.y, p.z], cal)).toBeCloseTo(tilt, 2)
        }
      }
    })

    it('reports the mounting misalignment', () => {
      const res = fitGyroAxis(table([0, 90, 180, 270]))
      const n = mul(mount, [0, 1, 0])
      expect(res.misalignment).toBeCloseTo(Math.acos(n[1]) / DEG, 2)
    })

    it('a sloping table becomes a constant offset with only one group', () => {
      const cal = fitGyroAxis(table([0, 90, 180, 270], 1.0)).calibration
      const p = simulate({ mount, bias, tilt: 40, roll: 77 })
      const q = simulate({ mount, bias, tilt: 40, roll: 250 })
      const a = axisAngle([p.x, p.y, p.z], cal)
      const b = axisAngle([q.x, q.y, q.z], cal)
      // Still independent of roll, but offset by roughly the slope
      expect(Math.abs(a - b)).toBeLessThan(0.02)
      expect(Math.abs(a - 40)).toBeGreaterThan(0.5)
    })

    it('cancels a sloping table when the device is also measured end for end', () => {
      const res = fitGyroAxis([
        ...table([0, 90, 180, 270], 1.0, 0),
        ...table([0, 90, 180, 270], -1.0, 1)
      ])
      expect(res.success).toBe(true)
      expect(res.calibration.by).toBeCloseTo(bias[1], 3)
      expect(Math.abs(res.slope)).toBeCloseTo(1.0, 2)

      const p = simulate({ mount, bias, tilt: 40, roll: 77 })
      expect(axisAngle([p.x, p.y, p.z], res.calibration)).toBeCloseTo(40, 2)
    })

    it('accepts a single end for end sample', () => {
      const res = fitGyroAxis([...table([0, 90, 180, 270], 1.0, 0), ...table([45], -1.0, 1)])
      expect(res.success).toBe(true)
      expect(res.calibration.by).toBeCloseTo(bias[1], 3)
    })

    it('is stable with sensor noise', () => {
      const res = fitGyroAxis(table([0, 60, 120, 180, 240, 300], 0, 0, { noise: 0.002 }))
      expect(res.success).toBe(true)
      const p = simulate({ mount, bias, tilt: 40, roll: 200 })
      expect(Math.abs(axisAngle([p.x, p.y, p.z], res.calibration) - 40)).toBeLessThan(0.3)
    })

    it('uses the X axis as reference when swap is enabled', () => {
      // Same device but the long axis of the case is the sensor X axis
      const swap = [
        [0, 1, 0],
        [1, 0, 0],
        [0, 0, -1]
      ]
      const samples = table([0, 90, 180, 270]).map((s) => {
        const p = mul(swap, [s.x, s.y, s.z])
        return { x: p[0], y: p[1], z: p[2], group: 0 }
      })
      const res = fitGyroAxis(samples, true)
      expect(res.success).toBe(true)
      expect(res.calibration.nx).toBeGreaterThan(0.99)
      expect(res.misalignment).toBeLessThan(5)
    })

    it('rejects too few samples', () => {
      expect(fitGyroAxis(table([0, 180])).error).toBe('few_samples')
      expect(fitGyroAxis([]).error).toBe('few_samples')
    })

    it('rejects samples taken at the same roll', () => {
      expect(fitGyroAxis(table([0, 1, 2, 3])).error).toBe('no_spread')
    })

    it('rejects data that is not a gravity circle', () => {
      const samples = table([0, 90, 180, 270]).map((s) => ({
        x: s.x * 0.5,
        y: s.y * 0.5,
        z: s.z * 0.5,
        group: 0
      }))
      expect(fitGyroAxis(samples).error).toBe('bad_fit')
    })
  })

  describe('isGyroAxisCalibrated', () => {
    it('is false for missing or zero data', () => {
      expect(isGyroAxisCalibrated(undefined)).toBe(false)
      expect(isGyroAxisCalibrated({})).toBe(false)
      expect(isGyroAxisCalibrated({ nx: 0, ny: 0, nz: 0, bx: 0, by: 0, bz: 0 })).toBe(false)
    })

    it('is true for a unit vector', () => {
      expect(isGyroAxisCalibrated({ nx: 0, ny: 1, nz: 0 })).toBe(true)
    })
  })
})
