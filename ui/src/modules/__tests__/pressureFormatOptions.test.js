import { describe, it, expect } from 'vitest'
import {
  pressureHttpPostFormatOptions,
  pressureHttpGetFormatOptions,
  pressureInfluxdb2FormatOptions,
  pressureMqttFormatOptions
} from '@/modules/pressureFormatOptions'

describe('pressureFormatOptions', () => {
  it('exports pressureHttpPostFormatOptions', () => {
    expect(pressureHttpPostFormatOptions).toBeTruthy()
    expect(Array.isArray(pressureHttpPostFormatOptions.value)).toBe(true)
    expect(pressureHttpPostFormatOptions.value.length).toBeGreaterThan(0)
  })

  it('pressureHttpPostFormatOptions contains Empty placeholder', () => {
    const labels = pressureHttpPostFormatOptions.value.map((opt) => opt.label)
    expect(labels).toContain('Empty')
  })

  it('exports pressureHttpGetFormatOptions', () => {
    expect(pressureHttpGetFormatOptions).toBeTruthy()
    expect(Array.isArray(pressureHttpGetFormatOptions.value)).toBe(true)
    expect(pressureHttpGetFormatOptions.value.length).toBeGreaterThan(0)
  })

  it('pressureHttpGetFormatOptions contains Empty placeholder', () => {
    const labels = pressureHttpGetFormatOptions.value.map((opt) => opt.label)
    expect(labels).toContain('Empty')
  })

  it('exports pressureInfluxdb2FormatOptions', () => {
    expect(pressureInfluxdb2FormatOptions).toBeTruthy()
    expect(Array.isArray(pressureInfluxdb2FormatOptions.value)).toBe(true)
    expect(pressureInfluxdb2FormatOptions.value.length).toBeGreaterThan(0)
  })

  it('exports pressureMqttFormatOptions', () => {
    expect(pressureMqttFormatOptions).toBeTruthy()
    expect(Array.isArray(pressureMqttFormatOptions.value)).toBe(true)
    expect(pressureMqttFormatOptions.value.length).toBeGreaterThan(0)
  })

  it('pressureMqttFormatOptions contains Empty placeholder', () => {
    const labels = pressureMqttFormatOptions.value.map((opt) => opt.label)
    expect(labels).toContain('Empty')
  })

  it('all options have label and value properties', () => {
    const allOptions = [
      ...pressureHttpPostFormatOptions.value,
      ...pressureHttpGetFormatOptions.value,
      ...pressureInfluxdb2FormatOptions.value,
      ...pressureMqttFormatOptions.value
    ]
    for (const opt of allOptions) {
      expect(opt).toHaveProperty('label')
      expect(opt).toHaveProperty('value')
      expect(typeof opt.label).toBe('string')
      expect(typeof opt.value).toBe('string')
    }
  })
})
