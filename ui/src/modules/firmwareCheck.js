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
 *
 */

const IMAGE_MAGIC = 0xe9
const HEADER_LENGTH = 16

// Platform names as reported by /api/status (upper case in the global store)
const CHIP_IDS = {
  ESP32: 0x0000,
  ESP32S2: 0x0002,
  ESP32C3: 0x0005,
  ESP32S3: 0x0009
}

// Checks the header of a firmware image in the browser, so a wrong file is
// rejected before the whole file is sent to the device.
//
// Image header (esp-idf / esptool): byte 0 is always 0xE9, bytes 12-13 hold
// the chip id (ESP32 family only). Must match firmware_check.hpp in
// espframework, which repeats the check on the device.

function readHeader(file) {
  return new Promise((resolve, reject) => {
    const reader = new FileReader()
    reader.onload = () => resolve(new Uint8Array(reader.result))
    reader.onerror = () => reject(reader.error)
    reader.readAsArrayBuffer(file.slice(0, HEADER_LENGTH))
  })
}

// Returns { ok: true } or
// { ok: false, error: 'not_firmware' | 'wrong_chip' | 'wrong_name' }
//
// expectedName is the firmware file name the device reports in its status. It
// separates board variants that share a chip (the chip id cannot). Compared
// without regard to case, and skipped when the device did not report one.
export async function checkFirmwareFile(file, platform, expectedName = '') {
  const header = await readHeader(file)

  if (header.length < HEADER_LENGTH || header[0] !== IMAGE_MAGIC) {
    return { ok: false, error: 'not_firmware' }
  }

  // ESP8266 images have no chip id, and an unknown platform is left to the
  // device to decide.
  const expectedChip = CHIP_IDS[platform]

  if (expectedChip !== undefined) {
    const chipId = header[12] | (header[13] << 8)
    if (chipId !== expectedChip) return { ok: false, error: 'wrong_chip' }
  }

  if (expectedName && file.name.toLowerCase() !== expectedName.toLowerCase()) {
    return { ok: false, error: 'wrong_name' }
  }

  return { ok: true }
}
