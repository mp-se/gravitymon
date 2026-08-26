import { describe, it, expect, beforeEach, vi } from 'vitest'
import { mount } from '@vue/test-utils'
import FormulaTableFragment from '../FormulaTableFragment.vue'
import { createTestingPinia } from '../../tests/testUtils'

describe('FormulaTableFragment (interaction tests)', () => {
  it('mounts without error', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(FormulaTableFragment, {
      props: {
        expressions: {
          o1: { eval: () => 1.0 },
          o2: { eval: () => 1.0 },
          o3: { eval: () => 1.0 },
          o4: { eval: () => 1.0 }
        }
      },
      global: { plugins: [pinia] }
    })
    expect(wrapper.exists()).toBe(true)
  })

  it('displays heading text', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(FormulaTableFragment, {
      props: {
        expressions: {
          o1: { eval: () => 1.0 },
          o2: { eval: () => 1.0 },
          o3: { eval: () => 1.0 },
          o4: { eval: () => 1.0 }
        }
      },
      global: { plugins: [pinia] }
    })
    expect(wrapper.text()).toContain('calculated values')
  })

  it('displays table element', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(FormulaTableFragment, {
      props: {
        expressions: {
          o1: { eval: () => 1.0 },
          o2: { eval: () => 1.0 },
          o3: { eval: () => 1.0 },
          o4: { eval: () => 1.0 }
        }
      },
      global: { plugins: [pinia] }
    })
    expect(wrapper.find('table').exists()).toBe(true)
  })

  it('table has proper CSS class', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(FormulaTableFragment, {
      props: {
        expressions: {
          o1: { eval: () => 1.0 },
          o2: { eval: () => 1.0 },
          o3: { eval: () => 1.0 },
          o4: { eval: () => 1.0 }
        }
      },
      global: { plugins: [pinia] }
    })
    const table = wrapper.find('table')
    expect(table.classes()).toContain('table')
    expect(table.classes()).toContain('table-striped')
  })

  it('displays table headers', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(FormulaTableFragment, {
      props: {
        expressions: {
          o1: { eval: () => 1.0 },
          o2: { eval: () => 1.0 },
          o3: { eval: () => 1.0 },
          o4: { eval: () => 1.0 }
        }
      },
      global: { plugins: [pinia] }
    })
    const headers = wrapper.findAll('th')
    expect(headers.length).toBeGreaterThan(0)
  })

  it('has expressions prop', () => {
    const pinia = createTestingPinia()
    const props = {
      expressions: {
        o1: { eval: () => 1.0 },
        o2: { eval: () => 1.0 },
        o3: { eval: () => 1.0 },
        o4: { eval: () => 1.0 }
      }
    }
    const wrapper = mount(FormulaTableFragment, {
      props,
      global: { plugins: [pinia] }
    })
    expect(wrapper.props('expressions')).toBeDefined()
  })

  it('displays row layout', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(FormulaTableFragment, {
      props: {
        expressions: {
          o1: { eval: () => 1.0 },
          o2: { eval: () => 1.0 },
          o3: { eval: () => 1.0 },
          o4: { eval: () => 1.0 }
        }
      },
      global: { plugins: [pinia] }
    })
    expect(wrapper.findAll('.row').length).toBeGreaterThan(0)
  })

  it('displays form text', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(FormulaTableFragment, {
      props: {
        expressions: {
          o1: { eval: () => 1.0 },
          o2: { eval: () => 1.0 },
          o3: { eval: () => 1.0 },
          o4: { eval: () => 1.0 }
        }
      },
      global: { plugins: [pinia] }
    })
    expect(wrapper.find('.form-text').exists()).toBe(true)
  })

  it('table has thead element', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(FormulaTableFragment, {
      props: {
        expressions: {
          o1: { eval: () => 1.0 },
          o2: { eval: () => 1.0 },
          o3: { eval: () => 1.0 },
          o4: { eval: () => 1.0 }
        }
      },
      global: { plugins: [pinia] }
    })
    const table = wrapper.find('table')
    expect(table.find('thead').exists()).toBe(true)
  })

  it('table has tbody element', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(FormulaTableFragment, {
      props: {
        expressions: {
          o1: { eval: () => 1.0 },
          o2: { eval: () => 1.0 },
          o3: { eval: () => 1.0 },
          o4: { eval: () => 1.0 }
        }
      },
      global: { plugins: [pinia] }
    })
    const table = wrapper.find('table')
    expect(table.find('tbody').exists()).toBe(true)
  })
})

describe('FormulaTableFragment (onMounted logic)', () => {
  beforeEach(() => vi.clearAllMocks())

  it('logs error and renders empty table when expressions is not an array', async () => {
    const consoleErrorSpy = vi.spyOn(console, 'error').mockImplementation(() => {})
    const pinia = createTestingPinia()
    const wrapper = mount(FormulaTableFragment, {
      props: { expressions: null },
      global: { plugins: [pinia] }
    })
    await wrapper.vm.$nextTick()
    expect(wrapper.findAll('tbody tr').length).toBe(0)
    consoleErrorSpy.mockRestore()
  })

  it('logs error and renders empty table when expressions array has fewer than 4 elements', async () => {
    const consoleErrorSpy = vi.spyOn(console, 'error').mockImplementation(() => {})
    const pinia = createTestingPinia()
    const wrapper = mount(FormulaTableFragment, {
      props: { expressions: ['tilt', 'tilt'] },
      global: { plugins: [pinia] }
    })
    await wrapper.vm.$nextTick()
    expect(wrapper.findAll('tbody tr').length).toBe(0)
    consoleErrorSpy.mockRestore()
  })

  it('populates table rows when given valid expressions array and calculation data', async () => {
    const { config } = await import('@/modules/pinia')
    config.formula_calculation_data = [
      { a: 45, g: 1.05 },
      { a: 60, g: 1.08 }
    ]
    const pinia = createTestingPinia()
    const wrapper = mount(FormulaTableFragment, {
      props: { expressions: ['tilt*0.02', 'tilt*0.021', 'tilt*0.022', 'tilt*0.023'] },
      global: { plugins: [pinia] }
    })
    await wrapper.vm.$nextTick()
    const rows = wrapper.findAll('tbody tr')
    expect(rows.length).toBe(2)
    // Restore
    config.formula_calculation_data = []
  })

  it('skips entries with angle <= 0', async () => {
    const { config } = await import('@/modules/pinia')
    config.formula_calculation_data = [
      { a: 0, g: 1.0 },
      { a: -5, g: 1.0 },
      { a: 45, g: 1.05 }
    ]
    const pinia = createTestingPinia()
    const wrapper = mount(FormulaTableFragment, {
      props: { expressions: ['tilt*0.02', 'tilt*0.021', 'tilt*0.022', 'tilt*0.023'] },
      global: { plugins: [pinia] }
    })
    await wrapper.vm.$nextTick()
    // Only the entry with a=45 should be included
    expect(wrapper.findAll('tbody tr').length).toBe(1)
    // Restore
    config.formula_calculation_data = []
  })
})
