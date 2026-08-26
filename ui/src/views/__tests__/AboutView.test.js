import { shallowMount } from '@vue/test-utils'
import AboutView from '../AboutView.vue'
import { createTestingPinia } from '../../tests/testUtils'

describe('AboutView (smoke)', () => {
  it('mounts without error', () => {
    const pinia = createTestingPinia()
    const wrapper = shallowMount(AboutView, { global: { plugins: [pinia] } })
    expect(wrapper.exists()).toBe(true)
  })
})
