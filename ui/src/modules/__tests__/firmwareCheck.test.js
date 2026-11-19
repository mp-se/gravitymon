import { describe, it, expect } from 'vitest'
import { checkFirmwareFile } from '../firmwareCheck'

function image(chipId, { magic = 0xe9, length = 16, name = 'firmware.bin' } = {}) {
  const bytes = new Uint8Array(length)
  bytes[0] = magic
  if (length > 13) {
    bytes[12] = chipId & 0xff
    bytes[13] = chipId >> 8
  }
  return new File([bytes], name)
}

describe('checkFirmwareFile', () => {
  // Chip ids confirmed against the images built for each target
  it.each([
    ['ESP32', 0],
    ['ESP32S2', 2],
    ['ESP32C3', 5],
    ['ESP32S3', 9]
  ])('accepts an image for %s', async (platform, chipId) => {
    expect(await checkFirmwareFile(image(chipId), platform)).toEqual({ ok: true })
  })

  it('rejects an image for another chip', async () => {
    expect(await checkFirmwareFile(image(9), 'ESP32C3')).toEqual({
      ok: false,
      error: 'wrong_chip'
    })
  })

  it('rejects a file with the wrong magic byte', async () => {
    expect(await checkFirmwareFile(image(5, { magic: 0x00 }), 'ESP32C3')).toEqual({
      ok: false,
      error: 'not_firmware'
    })
  })

  it('rejects a file shorter than the header', async () => {
    expect(await checkFirmwareFile(image(5, { length: 8 }), 'ESP32C3')).toEqual({
      ok: false,
      error: 'not_firmware'
    })
  })

  it('only checks the magic byte on ESP8266 (no chip id in the header)', async () => {
    expect(await checkFirmwareFile(image(5), 'ESP8266')).toEqual({ ok: true })
    expect(await checkFirmwareFile(image(5, { magic: 0x01 }), 'ESP8266').then((r) => r.error)).toBe(
      'not_firmware'
    )
  })

  describe('file name', () => {
    it('accepts the name the device reported, ignoring case', async () => {
      const file = image(5, { name: 'Firmware32C3.bin' })
      expect(await checkFirmwareFile(file, 'ESP32C3', 'firmware32c3.bin')).toEqual({ ok: true })
    })

    it('rejects another board variant on the same chip', async () => {
      const file = image(5, { name: 'firmware32c3supermini.bin' })
      expect(await checkFirmwareFile(file, 'ESP32C3', 'firmware32c3.bin')).toEqual({
        ok: false,
        error: 'wrong_name'
      })
    })

    it('rejects a file renamed by the browser', async () => {
      const file = image(5, { name: 'firmware32c3 (1).bin' })
      expect((await checkFirmwareFile(file, 'ESP32C3', 'firmware32c3.bin')).error).toBe(
        'wrong_name'
      )
    })

    it('is applied on ESP8266 and unknown platforms too', async () => {
      const file = image(0, { name: 'other.bin' })
      expect((await checkFirmwareFile(file, 'ESP8266', 'firmware.bin')).error).toBe('wrong_name')
      expect((await checkFirmwareFile(file, '', 'firmware.bin')).error).toBe('wrong_name')
    })

    it('is skipped when the device did not report a name', async () => {
      expect(await checkFirmwareFile(image(5, { name: 'x.bin' }), 'ESP32C3', '')).toEqual({
        ok: true
      })
    })

    it('reports a wrong chip before a wrong name', async () => {
      const file = image(9, { name: 'other.bin' })
      expect((await checkFirmwareFile(file, 'ESP32C3', 'firmware32c3.bin')).error).toBe(
        'wrong_chip'
      )
    })
  })

  it('leaves an unknown platform to the device', async () => {
    expect(await checkFirmwareFile(image(5), '')).toEqual({ ok: true })
  })
})
