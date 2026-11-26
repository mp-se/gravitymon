import { describe, it, expect, beforeEach, vi } from 'vitest'
import { mount, flushPromises } from '@vue/test-utils'
import GyroAxisCalibrationFragment from '../GyroAxisCalibrationFragment.vue'
import { config, status } from '@/modules/pinia'

const ZERO = { nx: 0, ny: 0, nz: 0, bx: 0, by: 0, bz: 0 }

// Device on a table, sensor mounted 2.5 degrees off around X with a bias
const reading = (rollDeg, flipped = false) => {
  const r = (rollDeg * Math.PI) / 180
  const m = (2.5 * Math.PI) / 180
  const slope = ((flipped ? -1 : 1) * Math.PI) / 180
  const g = [Math.sin(r) * Math.cos(slope), Math.sin(slope), Math.cos(r) * Math.cos(slope)]
  return {
    angle: 0,
    valid: true,
    ax: (g[0] + 0.03).toFixed(5),
    ay: (g[1] * Math.cos(m) - g[2] * Math.sin(m) - 0.02).toFixed(5),
    az: (g[1] * Math.sin(m) + g[2] * Math.cos(m) + 0.05).toFixed(5)
  }
}

const mountFragment = () =>
  mount(GyroAxisCalibrationFragment, { props: { readings: 2, readingDelay: 0 } })

const captureAt = async (wrapper, roll, button = 0, flipped = false) => {
  status.getGyro = vi.fn(async () => ({ success: true, data: reading(roll, flipped) }))
  await wrapper.findAll('button.btn-secondary')[button].trigger('click')
  await flushPromises()
  await new Promise((r) => setTimeout(r, 5))
  await flushPromises()
}

describe('GyroAxisCalibrationFragment', () => {
  beforeEach(() => {
    config.gyro_axis_calibration = { ...ZERO }
    config.gyro_swap_xy = false
    config.saveAll = vi.fn(async () => true)
  })

  it('mounts and shows that no calibration is active', () => {
    const wrapper = mountFragment()
    expect(wrapper.text()).toContain('Axis calibration')
    expect(wrapper.text()).toContain('No axis calibration')
    expect(wrapper.find('table').exists()).toBe(false)
  })

  it('cannot save before enough positions are captured', async () => {
    const wrapper = mountFragment()
    await captureAt(wrapper, 0)
    await captureAt(wrapper, 90)
    expect(wrapper.findAll('tbody tr')).toHaveLength(2)
    expect(wrapper.find('button.btn-primary').attributes('disabled')).toBeDefined()
  })

  it('captures positions, fits and saves the calibration', async () => {
    const wrapper = mountFragment()
    for (const roll of [0, 90, 180, 270]) await captureAt(wrapper, roll)

    expect(wrapper.findAll('tbody tr')).toHaveLength(4)
    expect(wrapper.text()).toContain('Sensor misalignment')

    const save = wrapper.find('button.btn-primary')
    expect(save.attributes('disabled')).toBeUndefined()
    await save.trigger('click')
    await flushPromises()

    expect(config.saveAll).toHaveBeenCalled()
    const c = config.gyro_axis_calibration
    expect(c.ny).toBeGreaterThan(0.99)
    expect(Math.hypot(c.nx, c.ny, c.nz)).toBeCloseTo(1, 4)
    // The simulated table slopes one degree which ends up in the bias along
    // the axis when there are no end for end positions.
    expect(c.bx).toBeCloseTo(0.03, 3)
    expect(c.bz).toBeCloseTo(0.05, 2)
  })

  it('uses end for end positions to remove the table slope', async () => {
    const wrapper = mountFragment()
    for (const roll of [0, 90, 180, 270]) await captureAt(wrapper, roll)
    for (const roll of [0, 120, 240]) await captureAt(wrapper, roll, 1, true)

    expect(wrapper.findAll('tbody tr')).toHaveLength(7)
    expect(wrapper.text()).toContain('Table slope')

    await wrapper.find('button.btn-primary').trigger('click')
    await flushPromises()
    expect(config.gyro_axis_calibration.by).toBeCloseTo(-0.02, 3)
  })

  it('rejects a reading when the device is moving', async () => {
    const wrapper = mountFragment()
    status.getGyro = vi.fn(async () => ({
      success: true,
      data: { ...reading(0), valid: false }
    }))
    await wrapper.findAll('button.btn-secondary')[0].trigger('click')
    await flushPromises()

    expect(wrapper.find('table').exists()).toBe(false)
    expect(wrapper.text()).toContain('The device is moving')
  })

  it('reports firmware without support for axis calibration', async () => {
    const wrapper = mountFragment()
    status.getGyro = vi.fn(async () => ({ success: true, data: { angle: 45 } }))
    await wrapper.findAll('button.btn-secondary')[0].trigger('click')
    await flushPromises()

    expect(wrapper.find('table').exists()).toBe(false)
    expect(wrapper.text()).toContain('Failed to read the gyro')
  })

  it('removes a single position', async () => {
    const wrapper = mountFragment()
    await captureAt(wrapper, 0)
    await captureAt(wrapper, 90)
    await wrapper.find('tbody button').trigger('click')
    expect(wrapper.findAll('tbody tr')).toHaveLength(1)
  })
})
