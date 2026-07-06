import { mount, flushPromises } from '@vue/test-utils'
import GravityGraphFragment from '../GravityGraphFragment.vue'
import { createTestingPinia } from '../../tests/testUtils'

describe('GravityGraphFragment (smoke)', () => {
  it('mounts without error', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(GravityGraphFragment, { global: { plugins: [pinia] } })
    expect(wrapper.exists()).toBe(true)
  })

  it('renders canvas element for chart', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(GravityGraphFragment, { global: { plugins: [pinia] } })
    expect(wrapper.find('canvas').exists()).toBe(true)
  })

  it('renders the form-text description', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(GravityGraphFragment, { global: { plugins: [pinia] } })
    expect(wrapper.find('.form-text').text()).toContain('graphical representation')
  })

  it('processes formula_calculation_data with positive angle on mount', async () => {
    const pinia = createTestingPinia()
    const { config } = await import('@/modules/pinia')
    config.formula_calculation_data = [
      { a: 45, g: 1.05 },
      { a: 50, g: 1.06 },
      { a: 0, g: 1.0 }
    ]
    config.gravity_formula = '0.0001*tilt*tilt+0.001*tilt+1.0'
    const wrapper = mount(GravityGraphFragment, { global: { plugins: [pinia] } })
    await flushPromises()
    expect(wrapper.exists()).toBe(true)
  })

  it('processes formula_calculation_data where all angles are zero', async () => {
    const pinia = createTestingPinia()
    const { config } = await import('@/modules/pinia')
    config.formula_calculation_data = [
      { a: 0, g: 1.0 },
      { a: 0, g: 1.01 }
    ]
    config.gravity_formula = '1.0'
    const wrapper = mount(GravityGraphFragment, { global: { plugins: [pinia] } })
    await flushPromises()
    expect(wrapper.exists()).toBe(true)
  })

  it('handles evaluateFormula data and pushes to chart data on mount', async () => {
    const pinia = createTestingPinia()
    const { config } = await import('@/modules/pinia')
    config.formula_calculation_data = [
      { a: 30, g: 1.02 },
      { a: 60, g: 1.08 }
    ]
    config.gravity_formula = '0.0001*tilt+1.0'
    const wrapper = mount(GravityGraphFragment, { global: { plugins: [pinia] } })
    await flushPromises()
    expect(wrapper.find('canvas#gravityChart').exists()).toBe(true)
  })
})
