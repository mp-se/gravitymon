/*
 * Minimal polynomial least-squares fit, drop-in for the part of ml-regression-polynomial we use
 * (constructor(x, y, degree) + toString(precision)). Same method as that library: normal equations
 * (A = F^T F, B = F^T y) solved with partial-pivot elimination, so results match it; it saves ~50 KB
 * (ml-matrix) in the embedded UI. Self-contained, no imports.
 */
const fmt = (n, precision) => {
  // Same rules as ml-regression-base maybeToPrecision
  const s = typeof precision === 'number' ? Math.abs(n).toPrecision(precision) : Math.abs(n).toString()
  return n < 0 ? `- ${s}` : s
}

export class PolynomialRegression {
  constructor(x, y, degree) {
    if (x.length !== y.length) throw new RangeError('x and y arrays must have the same length')
    const m = degree + 1
    // Augmented normal-equation matrix [F^T F | F^T y]
    const a = Array.from({ length: m }, (_, r) => {
      const row = new Array(m + 1).fill(0)
      for (let i = 0; i < x.length; i++) {
        for (let c = 0; c < m; c++) row[c] += x[i] ** (r + c)
        row[m] += y[i] * x[i] ** r
      }
      return row
    })
    for (let k = 0; k < m; k++) {
      let p = k
      for (let i = k + 1; i < m; i++) if (Math.abs(a[i][k]) > Math.abs(a[p][k])) p = i
      if (a[p][k] === 0) throw new Error('Matrix is singular')
      ;[a[k], a[p]] = [a[p], a[k]]
      for (let i = k + 1; i < m; i++) {
        const f = a[i][k] / a[k][k]
        for (let j = k; j <= m; j++) a[i][j] -= f * a[k][j]
      }
    }
    this.degree = degree
    this.coefficients = new Array(m)
    for (let k = m - 1; k >= 0; k--) {
      let s = a[k][m]
      for (let j = k + 1; j < m; j++) s -= a[k][j] * this.coefficients[j]
      this.coefficients[k] = s / a[k][k]
    }
  }

  // Format: "f(x) = c3 * x^3 + c2 * x^2 + c1 * x + c0" (negative terms as "- c"), as the original library
  toString(precision) {
    const last = this.coefficients.length - 1
    let fn = ''
    for (let k = 0; k <= last; k++) {
      const c = this.coefficients[k]
      let s = ''
      if (c !== 0) {
        s = k === 0 ? fmt(c, precision) : `${fmt(c, precision)} * x${k > 1 ? `^${k}` : ''}`
        if (k !== last) s = c > 0 ? ` + ${s}` : ` ${s}`
      }
      fn = s + fn
    }
    if (fn.startsWith('+')) fn = fn.slice(1)
    return `f(x) = ${fn}`
  }
}
