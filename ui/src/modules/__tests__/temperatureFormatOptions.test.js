import { describe, it, expect } from 'vitest'
import {
  temperatureHttpPostFormatOptions,
  temperatureHttpGetFormatOptions,
  temperatureInfluxdb2FormatOptions,
  temperatureMqttFormatOptions
} from '@/modules/temperatureFormatOptions'

describe('temperatureFormatOptions', () => {
  it('exports temperatureHttpPostFormatOptions', () => {
    expect(temperatureHttpPostFormatOptions).toBeTruthy()
    expect(Array.isArray(temperatureHttpPostFormatOptions.value)).toBe(true)
    expect(temperatureHttpPostFormatOptions.value.length).toBeGreaterThan(0)
  })

  it('temperatureHttpPostFormatOptions contains Empty placeholder', () => {
    const labels = temperatureHttpPostFormatOptions.value.map((opt) => opt.label)
    expect(labels).toContain('Empty')
  })

  it('exports temperatureHttpGetFormatOptions', () => {
    expect(temperatureHttpGetFormatOptions).toBeTruthy()
    expect(Array.isArray(temperatureHttpGetFormatOptions.value)).toBe(true)
    expect(temperatureHttpGetFormatOptions.value.length).toBeGreaterThan(0)
  })

  it('temperatureHttpGetFormatOptions contains Empty placeholder', () => {
    const labels = temperatureHttpGetFormatOptions.value.map((opt) => opt.label)
    expect(labels).toContain('Empty')
  })

  it('exports temperatureInfluxdb2FormatOptions', () => {
    expect(temperatureInfluxdb2FormatOptions).toBeTruthy()
    expect(Array.isArray(temperatureInfluxdb2FormatOptions.value)).toBe(true)
    expect(temperatureInfluxdb2FormatOptions.value.length).toBeGreaterThan(0)
  })

  it('exports temperatureMqttFormatOptions', () => {
    expect(temperatureMqttFormatOptions).toBeTruthy()
    expect(Array.isArray(temperatureMqttFormatOptions.value)).toBe(true)
    expect(temperatureMqttFormatOptions.value.length).toBeGreaterThan(0)
  })

  it('temperatureMqttFormatOptions contains Empty placeholder', () => {
    const labels = temperatureMqttFormatOptions.value.map((opt) => opt.label)
    expect(labels).toContain('Empty')
  })

  it('all options have label and value properties', () => {
    const allOptions = [
      ...temperatureHttpPostFormatOptions.value,
      ...temperatureHttpGetFormatOptions.value,
      ...temperatureInfluxdb2FormatOptions.value,
      ...temperatureMqttFormatOptions.value
    ]
    for (const opt of allOptions) {
      expect(opt).toHaveProperty('label')
      expect(opt).toHaveProperty('value')
      expect(typeof opt.label).toBe('string')
      expect(typeof opt.value).toBe('string')
    }
  })
})
