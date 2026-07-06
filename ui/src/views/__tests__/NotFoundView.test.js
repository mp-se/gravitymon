import { mount } from '@vue/test-utils'
import NotFoundView from '../NotFoundView.vue'
import { createTestingPinia } from '../../tests/testUtils'

describe('NotFoundView (smoke)', () => {
  it('mounts without error', () => {
    const pinia = createTestingPinia()
    const wrapper = mount(NotFoundView, { global: { plugins: [pinia] } })
    expect(wrapper.exists()).toBe(true)
  })

  it('renders with router for branch coverage of $route.path', async () => {
    const { createRouter, createMemoryHistory } = await import('vue-router')
    const pinia = createTestingPinia()
    const router = createRouter({
      history: createMemoryHistory(),
      routes: [{ path: '/:pathMatch(.*)*', component: NotFoundView }]
    })
    await router.push('/notfound/page')
    await router.isReady()
    const wrapper = mount(NotFoundView, {
      global: {
        plugins: [pinia, router],
        stubs: {
          BsMessage: {
            template: '<div><slot /></div>'
          }
        }
      }
    })
    expect(wrapper.exists()).toBe(true)
    expect(wrapper.text()).toContain('/notfound/page')
  })
})
