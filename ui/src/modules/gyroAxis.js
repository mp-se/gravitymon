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
 */

// Gyro axis calibration.
//
// When the device lies on a table and is rolled around its long axis, the
// gravity vector seen by the accelerometer traces a circle. The normal of that
// circle is the long axis of the case expressed in sensor coordinates, and the
// centre of the circle is the accelerometer bias. Knowing both we can calculate
// the tilt of the case instead of the tilt of the sensor, so the result no
// longer depends on how the sensor is mounted or how the device is rolled.
//
// Samples are { x, y, z, group } in g. Group 0 are the normal samples, group 1
// (optional) are taken with the device turned end for end. A table that slopes
// along the device moves the two circles in opposite directions along the axis,
// so the average of the two gives the bias along the axis.

export const MIN_SAMPLES = 3

const RAD2DEG = 180 / Math.PI

const dot = (a, b) => a[0] * b[0] + a[1] * b[1] + a[2] * b[2]
const sub = (a, b) => [a[0] - b[0], a[1] - b[1], a[2] - b[2]]
const scale = (a, s) => [a[0] * s, a[1] * s, a[2] * s]
const norm = (a) => Math.sqrt(dot(a, a))
const cross = (a, b) => [
  a[1] * b[2] - a[2] * b[1],
  a[2] * b[0] - a[0] * b[2],
  a[0] * b[1] - a[1] * b[0]
]
const clamp1 = (v) => Math.min(1, Math.max(-1, v))

// Eigen decomposition of a symmetric 3x3 matrix using Jacobi rotations.
// Returns { values: [..], vectors: [[..],[..],[..]] } sorted ascending.
export function eigenSymmetric3(m) {
  const a = m.map((r) => r.slice())
  const v = [
    [1, 0, 0],
    [0, 1, 0],
    [0, 0, 1]
  ]

  for (let sweep = 0; sweep < 50; sweep++) {
    const off = Math.abs(a[0][1]) + Math.abs(a[0][2]) + Math.abs(a[1][2])
    if (off < 1e-15) break

    for (let p = 0; p < 2; p++) {
      for (let q = p + 1; q < 3; q++) {
        if (Math.abs(a[p][q]) < 1e-18) continue

        const theta = (a[q][q] - a[p][p]) / (2 * a[p][q])
        const t = Math.sign(theta || 1) / (Math.abs(theta) + Math.sqrt(theta * theta + 1))
        const c = 1 / Math.sqrt(t * t + 1)
        const s = t * c

        for (let k = 0; k < 3; k++) {
          const akp = a[k][p]
          const akq = a[k][q]
          a[k][p] = c * akp - s * akq
          a[k][q] = s * akp + c * akq
        }
        for (let k = 0; k < 3; k++) {
          const apk = a[p][k]
          const aqk = a[q][k]
          a[p][k] = c * apk - s * aqk
          a[q][k] = s * apk + c * aqk
        }
        for (let k = 0; k < 3; k++) {
          const vkp = v[k][p]
          const vkq = v[k][q]
          v[k][p] = c * vkp - s * vkq
          v[k][q] = s * vkp + c * vkq
        }
      }
    }
  }

  const order = [0, 1, 2].sort((i, j) => a[i][i] - a[j][j])
  return {
    values: order.map((i) => a[i][i]),
    vectors: order.map((i) => [v[0][i], v[1][i], v[2][i]])
  }
}

// Solve a 3x3 linear system using Cramer's rule. Returns null if singular.
function solve3(m, r) {
  const det = (x) =>
    x[0][0] * (x[1][1] * x[2][2] - x[1][2] * x[2][1]) -
    x[0][1] * (x[1][0] * x[2][2] - x[1][2] * x[2][0]) +
    x[0][2] * (x[1][0] * x[2][1] - x[1][1] * x[2][0])

  const d = det(m)
  if (Math.abs(d) < 1e-12) return null

  const col = (i) => m.map((row, k) => row.map((val, j) => (j === i ? r[k] : val)))
  return [det(col(0)) / d, det(col(1)) / d, det(col(2)) / d]
}

// Tilt as the firmware calculates it without axis calibration.
export function legacyAngle(p, swapXY = false) {
  const len = norm(p)
  if (len === 0) return NaN
  return Math.acos(clamp1(Math.abs(swapXY ? p[0] : p[1]) / len)) * RAD2DEG
}

// Tilt as the firmware calculates it with axis calibration.
export function axisAngle(p, cal) {
  const a = sub(p, [cal.bx, cal.by, cal.bz])
  const len = norm(a)
  if (len === 0) return NaN
  return Math.acos(clamp1(Math.abs(dot(a, [cal.nx, cal.ny, cal.nz])) / len)) * RAD2DEG
}

const spread = (list) => (list.length ? Math.max(...list) - Math.min(...list) : 0)
const round = (v, d) => Number(v.toFixed(d))

// Fit the case axis and accelerometer bias to the samples.
//
// Returns { success: false, error } where error is one of
//   'few_samples'  less than MIN_SAMPLES in group 0
//   'no_spread'    the samples do not cover enough roll angles
//   'bad_fit'      the result is not a plausible gravity circle
// or { success: true, calibration: { nx, ny, nz, bx, by, bz }, ... diagnostics }
export function fitGyroAxis(samples, swapXY = false) {
  const pts = samples.map((s) => ({ p: [s.x, s.y, s.z], group: s.group ? 1 : 0 }))
  const groups = [0, 1].map((g) => pts.filter((s) => s.group === g).map((s) => s.p))

  if (groups[0].length < MIN_SAMPLES) return { success: false, error: 'few_samples' }

  // Plane fit, each group is centered on its own centroid since a sloping table
  // shifts the two groups apart along the axis we are looking for.
  const scatter = [
    [0, 0, 0],
    [0, 0, 0],
    [0, 0, 0]
  ]

  for (const g of groups) {
    if (!g.length) continue
    const c = scale(
      g.reduce((s, p) => [s[0] + p[0], s[1] + p[1], s[2] + p[2]], [0, 0, 0]),
      1 / g.length
    )
    for (const p of g) {
      const d = sub(p, c)
      for (let i = 0; i < 3; i++) for (let j = 0; j < 3; j++) scatter[i][j] += d[i] * d[j]
    }
  }

  const eig = eigenSymmetric3(scatter)

  // The two largest eigenvalues describe how well the samples cover the circle,
  // if the second one is small all samples are more or less at the same roll.
  if (eig.values[1] < 0.05) return { success: false, error: 'no_spread' }

  let n = eig.vectors[0]
  n = scale(n, 1 / norm(n))
  const ref = swapXY ? [1, 0, 0] : [0, 1, 0]
  if (dot(n, ref) < 0) n = scale(n, -1)

  // Circle fit in the plane (Kasa), x^2 + y^2 = 2*cx*x + 2*cy*y + k
  let u = cross(n, Math.abs(n[2]) < 0.9 ? [0, 0, 1] : [1, 0, 0])
  u = scale(u, 1 / norm(u))
  const v = cross(n, u)

  const m = [
    [0, 0, 0],
    [0, 0, 0],
    [0, 0, 0]
  ]
  const r = [0, 0, 0]

  for (const s of pts) {
    const x = dot(s.p, u)
    const y = dot(s.p, v)
    const row = [2 * x, 2 * y, 1]
    const rhs = x * x + y * y
    for (let i = 0; i < 3; i++) {
      r[i] += row[i] * rhs
      for (let j = 0; j < 3; j++) m[i][j] += row[i] * row[j]
    }
  }

  const sol = solve3(m, r)
  if (!sol) return { success: false, error: 'no_spread' }

  const [cx, cy, k] = sol
  const radius = Math.sqrt(Math.max(0, k + cx * cx + cy * cy))

  // Bias along the axis, mean of the groups so a table slope cancels out.
  const along = groups
    .filter((g) => g.length)
    .map((g) => g.reduce((s, p) => s + dot(p, n), 0) / g.length)
  const bn = along.reduce((s, a) => s + a, 0) / along.length

  const b = [0, 1, 2].map((i) => cx * u[i] + cy * v[i] + bn * n[i])

  // A real gravity circle has a radius of 1g, the accelerometer is not that far
  // off even when it is uncalibrated.
  if (!(radius > 0.85 && radius < 1.15) || norm(b) > 0.5) {
    return { success: false, error: 'bad_fit', radius: radius }
  }

  const calibration = {
    nx: round(n[0], 5),
    ny: round(n[1], 5),
    nz: round(n[2], 5),
    bx: round(b[0], 5),
    by: round(b[1], 5),
    bz: round(b[2], 5)
  }

  const before = pts.map((s) => legacyAngle(s.p, swapXY))
  const after = pts.map((s) => axisAngle(s.p, calibration))

  return {
    success: true,
    calibration: calibration,
    radius: radius,
    // Angle between the sensor axis used so far and the real axis of the case
    misalignment: Math.acos(clamp1(Math.abs(dot(n, ref)))) * RAD2DEG,
    // Slope of the table along the device, only known when both groups exist
    slope:
      along.length > 1 ? Math.asin(clamp1((along[0] - along[1]) / 2 / radius)) * RAD2DEG : null,
    anglesBefore: before,
    anglesAfter: after,
    // Max variation in angle within a group, should be zero for a perfect fit
    spreadBefore: Math.max(
      ...[0, 1].map((g) => spread(before.filter((a, i) => pts[i].group === g)))
    ),
    spreadAfter: Math.max(...[0, 1].map((g) => spread(after.filter((a, i) => pts[i].group === g))))
  }
}

export function isGyroAxisCalibrated(cal) {
  if (!cal) return false
  const len = (cal.nx || 0) ** 2 + (cal.ny || 0) ** 2 + (cal.nz || 0) ** 2
  return len > 0.5
}
