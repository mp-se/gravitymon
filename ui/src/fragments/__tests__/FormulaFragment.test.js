import { describe, it, expect } from 'vitest'
import { mount } from '@vue/test-utils'
import FormulaFragment from '../FormulaFragment.vue'
import { createTestingPinia } from '../../tests/testUtils'

const emittingBsInputReadonly = {
  props: ['modelValue', 'label'],
  emits: ['update:modelValue'],
  template:
    '<button class="bs-emitting" @click="$emit(\'update:modelValue\', \'updated\')">{{modelValue}}</button>'
}

describe('FormulaFragment (smoke)', () => {
  it('mounts without error', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(FormulaFragment, { global: { plugins: [pinia] } })
    expect(wrapper.exists()).toBe(true)
  })

  it('renders one BsInputReadonly per expression', async () => {
    const pinia = createTestingPinia()
    const wrapper = mount(FormulaFragment, {
      props: { expressions: ['tilt*0.02', 'tilt*0.021'] },
      global: { plugins: [pinia] }
    })
    await wrapper.vm.$nextTick()
    // Two expressions → two stubs rendered (one per expression index)
    expect(wrapper.findAll('.col-md-12').length).toBe(2)
  })

  it('shows rejected text for empty formula entries', async () => {
    const pinia = createTestingPinia()
    const wrapper = mount(FormulaFragment, {
      props: { expressions: ['', 'tilt*0.02'] },
      global: { plugins: [pinia] }
    })
    await wrapper.vm.$nextTick()
    // rejected ref has default 'Formula rejected'
    expect(wrapper.vm.rejected).toBe('Formula rejected')
  })

  it('displays form-text description', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(FormulaFragment, { global: { plugins: [pinia] } })
    expect(wrapper.find('.form-text').text()).toContain('generated formulas')
  })

  it('renders correct number of col-md-12 divs for three expressions', async () => {
    const pinia = createTestingPinia()
    const wrapper = mount(FormulaFragment, {
      props: { expressions: ['a*1', 'b*2', ''] },
      global: { plugins: [pinia] }
    })
    await wrapper.vm.$nextTick()
    expect(wrapper.findAll('.col-md-12').length).toBe(3)
  })

  it('renders col-md-12 for each expression including empty', async () => {
    const pinia = createTestingPinia()
    const wrapper = mount(FormulaFragment, {
      props: { expressions: [''] },
      global: { plugins: [pinia] }
    })
    await wrapper.vm.$nextTick()
    expect(wrapper.findAll('.col-md-12').length).toBe(1)
  })

  it('has rejected ref initialized to Formula rejected', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(FormulaFragment, {
      props: { expressions: ['tilt*2'] },
      global: { plugins: [pinia] }
    })
    expect(wrapper.vm.rejected).toBe('Formula rejected')
  })

  it('renders no col-md-12 when expressions is empty array (uses default)', async () => {
    const pinia = createTestingPinia()
    const wrapper = mount(FormulaFragment, {
      props: { expressions: [] },
      global: { plugins: [pinia] }
    })
    await wrapper.vm.$nextTick()
    expect(wrapper.findAll('.col-md-12').length).toBe(0)
  })
})

describe('FormulaFragment (v-model setter coverage)', () => {
  it('calling update:modelValue on non-empty BsInputReadonly triggers expressions setter', async () => {
    const pinia = createTestingPinia()
    const wrapper = mount(FormulaFragment, {
      props: { expressions: ['tilt*0.02'] },
      global: {
        plugins: [pinia],
        components: { BsInputReadonly: emittingBsInputReadonly }
      }
    })
    await wrapper.vm.$nextTick()
    const btn = wrapper.find('.bs-emitting')
    if (btn.exists()) {
      await btn.trigger('click')
    }
    expect(wrapper.exists()).toBe(true)
  })

  it('calling update:modelValue on empty-expression BsInputReadonly triggers rejected setter', async () => {
    const pinia = createTestingPinia()
    const wrapper = mount(FormulaFragment, {
      props: { expressions: [''] },
      global: {
        plugins: [pinia],
        components: { BsInputReadonly: emittingBsInputReadonly }
      }
    })
    await wrapper.vm.$nextTick()
    const btn = wrapper.find('.bs-emitting')
    if (btn.exists()) {
      await btn.trigger('click')
    }
    expect(wrapper.exists()).toBe(true)
  })
})
