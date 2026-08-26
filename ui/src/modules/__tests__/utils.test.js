import { describe, it, expect } from 'vitest'
import {
  isGyroCalibrated,
  httpHeaderOptions,
  httpPostUrlOptions,
  httpGetUrlOptions
} from '@/modules/utils'
import { config } from '@/modules/pinia'

describe('utils.isGyroCalibrated', () => {
  it('returns false when calibration data sums to zero', () => {
    config.gyro_calibration_data = { ax: 0, ay: 0, az: 0, gx: 0, gy: 0, gz: 0 }
    expect(isGyroCalibrated()).toBe(false)
  })

  it('returns true when at least one calibration value is non-zero', () => {
    config.gyro_calibration_data = { ax: 1, ay: 0, az: 0, gx: 0, gy: 0, gz: 0 }
    expect(isGyroCalibrated()).toBe(true)
  })

  it('returns true when multiple calibration values are non-zero', () => {
    config.gyro_calibration_data = { ax: 10, ay: 5, az: -3, gx: 2, gy: 1, gz: 8 }
    expect(isGyroCalibrated()).toBe(true)
  })

  it('returns true when at least gyro values are calibrated', () => {
    config.gyro_calibration_data = { ax: 0, ay: 0, az: 0, gx: 1, gy: 0, gz: 0 }
    expect(isGyroCalibrated()).toBe(true)
  })
})

describe('utils - exported options/constants', () => {
  it('exports httpHeaderOptions as a ref-like object', () => {
    expect(httpHeaderOptions).toBeTruthy()
    expect(httpHeaderOptions.value).toBeTruthy()
    expect(Array.isArray(httpHeaderOptions.value)).toBe(true)
  })

  it('httpHeaderOptions contains expected items', () => {
    const labels = httpHeaderOptions.value.map((opt) => opt.label)
    expect(labels).toContain('JSON data')
    expect(labels).toContain('Authorization')
    expect(labels).toContain('User agent')
  })

  it('exports httpPostUrlOptions', () => {
    expect(httpPostUrlOptions).toBeTruthy()
    expect(httpPostUrlOptions.value).toBeTruthy()
    expect(Array.isArray(httpPostUrlOptions.value)).toBe(true)
  })

  it('httpPostUrlOptions contains known services', () => {
    const labels = httpPostUrlOptions.value.map((opt) => opt.label)
    expect(labels.some((l) => l.includes('Brewfather'))).toBe(true)
    expect(labels.some((l) => l.includes('UBI'))).toBe(true)
  })

  it('exports httpGetUrlOptions', () => {
    expect(httpGetUrlOptions).toBeTruthy()
    expect(httpGetUrlOptions.value).toBeTruthy()
  })
})
