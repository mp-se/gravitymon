import { describe, it, expect } from 'vitest'
import { mount } from '@vue/test-utils'
import LineChart from '../LineChart.vue'

const ds = (over = {}) => ({
  label: 'A',
  color: 'blue',
  points: [
    { x: 30, y: 1.0 },
    { x: 60, y: 1.1 }
  ],
  ...over
})

describe('LineChart', () => {
  it('renders one polyline and one circle per point', () => {
    const w = mount(LineChart, { props: { datasets: [ds()] } })
    expect(w.findAll('polyline').length).toBe(1)
    expect(w.findAll('circle').length).toBe(2)
  })

  it('renders axis titles and legend labels', () => {
    const w = mount(LineChart, { props: { datasets: [ds()], xTitle: 'Angle', yTitle: 'Gravity' } })
    expect(w.text()).toContain('Angle')
    expect(w.text()).toContain('Gravity')
    expect(w.find('.line-chart-legend').text()).toBe('A')
  })

  it('skips hidden datasets', () => {
    const w = mount(LineChart, { props: { datasets: [ds(), ds({ label: 'B', hidden: true })] } })
    expect(w.findAll('polyline').length).toBe(1)
  })

  it('toggles a dataset when its legend entry is clicked', async () => {
    const w = mount(LineChart, { props: { datasets: [ds()] } })
    await w.find('.line-chart-legend').trigger('click')
    expect(w.findAll('polyline').length).toBe(0)
    await w.find('.line-chart-legend').trigger('click')
    expect(w.findAll('polyline').length).toBe(1)
  })

  it('handles no data and a single point without throwing', () => {
    expect(() => mount(LineChart, { props: { datasets: [] } })).not.toThrow()
    const w = mount(LineChart, { props: { datasets: [ds({ points: [{ x: 5, y: 5 }] })] } })
    expect(w.findAll('circle').length).toBe(1)
  })

  it('extends the axis down to the suggested minimum', () => {
    const w = mount(LineChart, { props: { datasets: [ds()], xMin: 0 } })
    const labels = w.findAll('text').map((t) => t.text())
    expect(labels).toContain('0')
  })

  it('shows a tooltip for the nearest point on hover', async () => {
    const w = mount(LineChart, { props: { datasets: [ds()] } })
    const svg = w.find('svg')
    svg.element.getBoundingClientRect = () => ({ left: 0, width: 600 })
    svg.element.dispatchEvent(new MouseEvent('pointermove', { clientX: 590 }))
    await w.vm.$nextTick()
    expect(w.text()).toContain('A: 60')
    svg.element.dispatchEvent(new MouseEvent('pointerleave'))
    await w.vm.$nextTick()
    expect(w.text()).not.toContain('A: 60')
  })
})
