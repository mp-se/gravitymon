import { describe, it, expect, beforeEach } from 'vitest'
import { applyTemplate } from '@/modules/formatTemplate'
import { global } from '@/modules/pinia'

describe('formatTemplate.applyTemplate', () => {
  beforeEach(() => {
    global.app_ver = '3.0.0'
    global.app_build = '42'
    // Enable feature flags for tests
    global.ui = {
      enableGravity: true,
      enablePressure: false,
      enableHttpPostTcpMode: true
    }
  })

  it('replaces temperature placeholders', () => {
    const status = { temp: 20, angle: 0, rssi: 0, gravity: 1.0, battery: 0 }
    const cfg = {
      mdns: 'test',
      id: '1',
      sleep_interval: 60,
      token: '',
      token2: '',
      temp_unit: 'C',
      gravity_unit: 'G'
    }

    const template = '{"temp": ${temp}}'
    const result = applyTemplate(status, cfg, template)

    expect(result).toContain('"temp": 20')
  })

  it('converts temperature C to F', () => {
    const status = { temp: 20, angle: 0, rssi: 0, gravity: 1.0, battery: 0 }
    const cfg = {
      mdns: 'test',
      id: '1',
      sleep_interval: 60,
      token: '',
      token2: '',
      temp_unit: 'C',
      gravity_unit: 'G'
    }

    const template = '{"temp_f": ${temp-f}}'
    const result = applyTemplate(status, cfg, template)

    expect(result).toContain('"temp_f": 68')
  })

  it('handles gravity unit G (SG)', () => {
    const status = { temp: 20, angle: 0, rssi: 0, gravity: 1.05, battery: 0 }
    const cfg = {
      mdns: 'test',
      id: '1',
      sleep_interval: 60,
      token: '',
      token2: '',
      temp_unit: 'C',
      gravity_unit: 'G'
    }

    const template = '{"gravity": ${gravity}}'
    const result = applyTemplate(status, cfg, template)

    expect(result).toContain('"gravity": 1.05')
  })

  it('handles gravity unit P (Plato)', () => {
    const status = { temp: 20, angle: 0, rssi: 0, gravity: 5.0, battery: 0 }
    const cfg = {
      mdns: 'test',
      id: '1',
      sleep_interval: 60,
      token: '',
      token2: '',
      temp_unit: 'C',
      gravity_unit: 'P'
    }

    const template = '{"gravity": ${gravity}}'
    const result = applyTemplate(status, cfg, template)

    expect(result).toContain('"gravity": 5')
  })

  it('replaces config placeholders', () => {
    const status = { temp: 20, angle: 10, rssi: -50, gravity: 1.05, battery: 3.7 }
    const cfg = {
      mdns: 'myhost',
      id: '42',
      sleep_interval: 120,
      token: 'secret',
      token2: 'secret2',
      temp_unit: 'C',
      gravity_unit: 'G'
    }

    const template = '{"mdns": "${mdns}", "id": "${id}", "token": "${token}"}'
    const result = applyTemplate(status, cfg, template)

    expect(result).toContain('"mdns": "myhost"')
    expect(result).toContain('"id": "42"')
    expect(result).toContain('"token": "secret"')
  })

  it('replaces angle/tilt placeholders', () => {
    const status = { temp: 20, angle: 25.5, rssi: -50, gravity: 1.05, battery: 3.7 }
    const cfg = {
      mdns: 'test',
      id: '1',
      sleep_interval: 60,
      token: '',
      token2: '',
      temp_unit: 'C',
      gravity_unit: 'G'
    }

    const template = '{"angle": ${angle}, "tilt": ${tilt}}'
    const result = applyTemplate(status, cfg, template)

    expect(result).toContain('"angle": 25.5')
    expect(result).toContain('"tilt": 25.5')
  })

  it('replaces battery and RSSI placeholders', () => {
    const status = { temp: 20, angle: 10, rssi: -45, gravity: 1.05, battery: 3.85 }
    const cfg = {
      mdns: 'test',
      id: '1',
      sleep_interval: 60,
      token: '',
      token2: '',
      temp_unit: 'C',
      gravity_unit: 'G'
    }

    const template = '{"battery": ${battery}, "rssi": ${rssi}}'
    const result = applyTemplate(status, cfg, template)

    expect(result).toContain('"battery": 3.85')
    expect(result).toContain('"rssi": -45')
  })

  it('replaces app metadata placeholders', () => {
    const status = { temp: 20, angle: 0, rssi: 0, gravity: 1.0, battery: 0 }
    const cfg = {
      mdns: 'test',
      id: '1',
      sleep_interval: 60,
      token: '',
      token2: '',
      temp_unit: 'C',
      gravity_unit: 'G'
    }

    const template = '{"version": "${app-ver}", "build": "${app-build}"}'
    const result = applyTemplate(status, cfg, template)

    expect(result).toContain('"version": "3.0.0"')
    expect(result).toContain('"build": "42"')
  })

  it('returns formatted JSON when template is valid JSON', () => {
    const status = { temp: 20, angle: 0, rssi: 0, gravity: 1.0, battery: 0 }
    const cfg = {
      mdns: 'test',
      id: '1',
      sleep_interval: 60,
      token: '',
      token2: '',
      temp_unit: 'C',
      gravity_unit: 'G'
    }

    const template = '{"temp":${temp}}'
    const result = applyTemplate(status, cfg, template)

    expect(() => JSON.parse(result)).not.toThrow()
  })

  it('returns string when template is not valid JSON', () => {
    const status = { temp: 20, angle: 0, rssi: 0, gravity: 1.0, battery: 0 }
    const cfg = {
      mdns: 'test',
      id: '1',
      sleep_interval: 60,
      token: '',
      token2: '',
      temp_unit: 'C',
      gravity_unit: 'G'
    }

    const template = 'not json: ${temp}'
    const result = applyTemplate(status, cfg, template)

    expect(typeof result).toBe('string')
    expect(result).toContain('not json: 20')
  })

  it('computes tempToC when device unit is F', () => {
    const status = { temp: 68, angle: 0, rssi: 0, gravity: 1.0, battery: 0 }
    const cfg = {
      mdns: 'test',
      id: '1',
      sleep_interval: 60,
      token: '',
      token2: '',
      temp_unit: 'F',
      gravity_unit: 'G'
    }
    const result = applyTemplate(status, cfg, '${temp-c}')
    expect(result).toBe('20')
  })
})
