import { describe, it, expect, beforeEach, vi } from 'vitest'
import { mount } from '@vue/test-utils'
import FormulaGraphFragment from '../FormulaGraphFragment.vue'
import { createTestingPinia } from '../../tests/testUtils'

describe('FormulaGraphFragment (interaction tests)', () => {
  it('mounts without error', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(FormulaGraphFragment, {
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
    const wrapper = mount(FormulaGraphFragment, {
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
    expect(wrapper.text()).toContain('graphical representation')
  })

  it('has expressions prop defined', () => {
    const pinia = createTestingPinia()
    const props = {
      expressions: {
        o1: { eval: () => 1.0 },
        o2: { eval: () => 1.0 },
        o3: { eval: () => 1.0 },
        o4: { eval: () => 1.0 }
      }
    }
    const wrapper = mount(FormulaGraphFragment, {
      props,
      global: { plugins: [pinia] }
    })
    expect(wrapper.props('expressions')).toBeDefined()
  })

  it('displays row layout', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(FormulaGraphFragment, {
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

  it('displays canvas element for graph', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(FormulaGraphFragment, {
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
    expect(wrapper.find('canvas').exists()).toBe(true)
  })

  it('has horizontal rule separator', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(FormulaGraphFragment, {
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
    expect(wrapper.find('hr').exists()).toBe(true)
  })

  it('displays heading paragraph', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(FormulaGraphFragment, {
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
    const paragraphs = wrapper.findAll('p')
    expect(paragraphs.length).toBeGreaterThan(0)
  })

  it('canvas has id attribute', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(FormulaGraphFragment, {
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
    const canvas = wrapper.find('canvas')
    expect(canvas.attributes('id')).toBeDefined()
  })
})

describe('FormulaGraphFragment (onMounted logic)', () => {
  beforeEach(() => vi.clearAllMocks())

  it('returns early and does not populate chart data when expressions is invalid', async () => {
    const pinia = createTestingPinia()
    const wrapper = mount(FormulaGraphFragment, {
      props: { expressions: null },
      global: { plugins: [pinia] }
    })
    await wrapper.vm.$nextTick()
    expect(wrapper.vm.chartDataOrder1.length).toBe(0)
  })

  it('returns early when expressions has fewer than 4 elements', async () => {
    const pinia = createTestingPinia()
    const wrapper = mount(FormulaGraphFragment, {
      props: { expressions: ['tilt*0.02', 'tilt*0.021'] },
      global: { plugins: [pinia] }
    })
    await wrapper.vm.$nextTick()
    expect(wrapper.vm.chartDataOrder1.length).toBe(0)
  })

  it('populates chartDataOrder1 when given valid 4-element expressions array', async () => {
    const pinia = createTestingPinia()
    const wrapper = mount(FormulaGraphFragment, {
      props: { expressions: ['tilt*0.02', 'tilt*0.021', 'tilt*0.022', 'tilt*0.023'] },
      global: { plugins: [pinia] }
    })
    await wrapper.vm.$nextTick()
    // evaluateFormula iterates a from 25 to 80 in steps of 5 → 11 points
    expect(wrapper.vm.chartDataOrder1.length).toBeGreaterThan(0)
  })

  it('hides dataset when expression is empty string', async () => {
    const pinia = createTestingPinia()
    const wrapper = mount(FormulaGraphFragment, {
      props: { expressions: ['', '', '', ''] },
      global: { plugins: [pinia] }
    })
    await wrapper.vm.$nextTick()
    expect(wrapper.vm.chartDataOrder1.length).toBe(0)
  })

  it('populates chartDataForm from config.formula_calculation_data', async () => {
    const { config } = await import('@/modules/pinia')
    config.formula_calculation_data = [
      { a: 45, g: 1.05 },
      { a: 60, g: 1.08 }
    ]
    const pinia = createTestingPinia()
    const wrapper = mount(FormulaGraphFragment, {
      props: { expressions: ['tilt*0.02', 'tilt*0.021', 'tilt*0.022', 'tilt*0.023'] },
      global: { plugins: [pinia] }
    })
    await wrapper.vm.$nextTick()
    expect(wrapper.vm.chartDataForm.length).toBe(2)
    // Restore
    config.formula_calculation_data = []
  })

  it('handles invalid expressions array gracefully', async () => {
    const { mount } = await import('@vue/test-utils')
    const { default: Fragment } = await import('../FormulaGraphFragment.vue')
    const { createTestingPinia } = await import('../../tests/testUtils')
    const wrapper = mount(Fragment, {
      props: { expressions: null },
      global: { plugins: [createTestingPinia()] }
    })
    expect(wrapper.exists()).toBe(true)
  })

  it('handles expressions with empty strings', async () => {
    const { mount } = await import('@vue/test-utils')
    const { default: Fragment } = await import('../FormulaGraphFragment.vue')
    const { createTestingPinia } = await import('../../tests/testUtils')
    const wrapper = mount(Fragment, {
      props: { expressions: ['', '', '', ''] },
      global: { plugins: [createTestingPinia()] }
    })
    expect(wrapper.exists()).toBe(true)
  })

  it('handles expressions with mixed empty and valid strings', async () => {
    const { mount } = await import('@vue/test-utils')
    const { default: Fragment } = await import('../FormulaGraphFragment.vue')
    const { createTestingPinia } = await import('../../tests/testUtils')
    const wrapper = mount(Fragment, {
      props: { expressions: ['x', '', 'x*2', ''] },
      global: { plugins: [createTestingPinia()] }
    })
    expect(wrapper.exists()).toBe(true)
  })

  it('mounts with valid formula array', async () => {
    const { mount } = await import('@vue/test-utils')
    const { default: Fragment } = await import('../FormulaGraphFragment.vue')
    const { createTestingPinia } = await import('../../tests/testUtils')
    const wrapper = mount(Fragment, {
      props: { expressions: ['x', 'x*2', 'x*3', 'x*4'] },
      global: { plugins: [createTestingPinia()] }
    })
    await wrapper.vm.$nextTick()
    expect(wrapper.exists()).toBe(true)
  })

  it('handles expressions shorter than 4 elements', async () => {
    const { mount } = await import('@vue/test-utils')
    const { default: Fragment } = await import('../FormulaGraphFragment.vue')
    const { createTestingPinia } = await import('../../tests/testUtils')
    const wrapper = mount(Fragment, {
      props: { expressions: ['x'] },
      global: { plugins: [createTestingPinia()] }
    })
    expect(wrapper.exists()).toBe(true)
  })
})
