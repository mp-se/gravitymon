import { describe, it, expect, vi, beforeEach, afterEach } from 'vitest'
import { mount } from '@vue/test-utils'
import piniaInstance, { global as globalStore, config, status } from '@/modules/pinia'
import PushHttpPost1View from '../PushHttpPost1View.vue'
import PushHttpPost2View from '../PushHttpPost2View.vue'
import PushHttpGetView from '../PushHttpGetView.vue'
import PushInfluxdbView from '../PushInfluxdbView.vue'
import PushMqttView from '../PushMqttView.vue'

// Temperature is the third data type (next to gravity and pressure) in the common push views.
const views = [
  {
    name: 'PushHttpPost1View',
    comp: PushHttpPost1View,
    formatKey: 'http_post_format_temperature',
    callback: 'temperatureHttpFormatCallback'
  },
  {
    name: 'PushHttpPost2View',
    comp: PushHttpPost2View,
    formatKey: 'http_post2_format_temperature',
    callback: 'temperatureHttpFormatCallback'
  },
  {
    name: 'PushHttpGetView',
    comp: PushHttpGetView,
    formatKey: 'http_get_format_temperature',
    callback: 'temperatureHttpFormatCallback'
  },
  {
    name: 'PushInfluxdbView',
    comp: PushInfluxdbView,
    formatKey: 'influxdb2_format_temperature',
    callback: 'temperatureInfluxdb2FormatCallback'
  },
  {
    name: 'PushMqttView',
    comp: PushMqttView,
    formatKey: 'mqtt_format_temperature',
    callback: 'temperatureMqttFormatCallback'
  }
]

const stubs = {
  BsInputText: true,
  BsProgress: true,
  BsMessage: true,
  BsInputTextAreaFormat: true,
  BsDropdown: true,
  BsInputSwitch: true,
  BsInputNumber: true,
  BsModal: true
}

function mountView(comp) {
  return mount(comp, { global: { plugins: [piniaInstance], stubs } })
}

function setTypes(gravity, pressure, temperature) {
  globalStore.ui.enableGravity = gravity
  globalStore.ui.enablePressure = pressure
  globalStore.ui.enableTemperature = temperature
}

describe.each(views)('$name temperature support', ({ comp, formatKey, callback }) => {
  beforeEach(() => {
    vi.restoreAllMocks()
  })

  afterEach(() => {
    setTypes(true, false, false) // gravitymon defaults
  })

  it('shows only the temperature test button when temperature is the only type', async () => {
    setTypes(false, false, true)
    const wrapper = mountView(comp)
    await wrapper.vm.$nextTick()

    const labels = wrapper.findAll('button').map((b) => b.text().toLowerCase())
    expect(labels.some((l) => l.includes('temperature'))).toBe(true)
    expect(labels.some((l) => l.includes('gravity'))).toBe(false)
    expect(labels.some((l) => l.includes('pressure'))).toBe(false)
  })

  it('has no temperature test button when temperature is disabled (gravitymon default)', async () => {
    setTypes(true, false, false)
    const wrapper = mountView(comp)
    await wrapper.vm.$nextTick()

    const labels = wrapper.findAll('button').map((b) => b.text().toLowerCase())
    expect(labels.some((l) => l.includes('temperature'))).toBe(false)
    expect(labels.some((l) => l.includes('gravity'))).toBe(true)
  })

  it('shows one test button per type when all types are enabled', async () => {
    setTypes(true, true, true)
    const wrapper = mountView(comp)
    await wrapper.vm.$nextTick()

    const labels = wrapper.findAll('button').map((b) => b.text().toLowerCase())
    expect(labels.filter((l) => l.includes('gravity')).length).toBe(1)
    expect(labels.filter((l) => l.includes('pressure')).length).toBe(1)
    expect(labels.filter((l) => l.includes('temperature')).length).toBe(1)
  })

  it('has an enable switch for the temperature section only when another type is active', () => {
    const switches = (g, p, tmp) => {
      setTypes(g, p, tmp)
      return mountView(comp).findAll('bs-input-switch-stub').length
    }

    const base = switches(false, false, false)
    expect(switches(false, false, true)).toBe(base) // temperature only: no enable switch
    expect(switches(true, false, true)).toBeGreaterThan(switches(true, false, false))
    expect(switches(false, true, true)).toBeGreaterThan(switches(false, true, false))
    expect(switches(true, true, true)).toBeGreaterThan(switches(true, true, false))
  })

  it('runTestTemperature starts a push test for the temperature format', async () => {
    config.runPushTest = vi.fn(async () => {})
    config.mqtt_target = 'broker.local' // required by the MQTT view validation
    config.mqtt_port = 1883
    setTypes(false, false, true)
    const wrapper = mountView(comp)

    await wrapper.vm.runTestTemperature()
    expect(config.runPushTest).toHaveBeenCalledWith({ push_format: formatKey })
  })

  it('runTestTemperature reports an error when the push test fails', async () => {
    config.mqtt_target = 'broker.local'
    config.mqtt_port = 1883
    config.runPushTest = vi.fn().mockRejectedValueOnce(new Error('Network error'))
    globalStore.messageError = ''
    const wrapper = mountView(comp)

    await wrapper.vm.runTestTemperature()
    expect(globalStore.messageError).not.toBe('')
  })

  it('the predefined format callback decodes and stores the temperature format', async () => {
    const wrapper = mountView(comp)
    wrapper.vm[callback](encodeURIComponent('t=${temp}|'))
    expect(config[formatKey].startsWith('t=${temp}|')).toBe(true)
  })

  it('renders the temperature format preview from the status values', async () => {
    config[formatKey] = 'T=${temp}'
    status.temp = '21.50'
    const wrapper = mountView(comp)

    wrapper.vm.temperatureRenderFormat()
    expect(wrapper.vm.temperatureRender).toContain('21.50')
  })
})
