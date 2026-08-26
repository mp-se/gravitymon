import { describe, it, expect } from 'vitest'

describe('pinia.js', () => {
  describe('module structure', () => {
    it('exports Pinia instance as default', async () => {
      const module = await import('@/modules/pinia')
      expect(module.default).toBeDefined()
    })

    it('exports config store', async () => {
      const module = await import('@/modules/pinia')
      expect(module.config).toBeDefined()
    })

    it('exports global store', async () => {
      const module = await import('@/modules/pinia')
      expect(module.global).toBeDefined()
    })

    it('exports status store', async () => {
      const module = await import('@/modules/pinia')
      expect(module.status).toBeDefined()
    })

    it('exports saveConfigState function', async () => {
      const module = await import('@/modules/pinia')
      expect(module.saveConfigState).toBeDefined()
      expect(typeof module.saveConfigState).toBe('function')
    })

    it('exports getConfigChanges function', async () => {
      const module = await import('@/modules/pinia')
      expect(module.getConfigChanges).toBeDefined()
      expect(typeof module.getConfigChanges).toBe('function')
    })
  })

  describe('saveConfigState and getConfigChanges', () => {
    it('getConfigChanges returns empty object before saveConfigState is called', async () => {
      const module = await import('@/modules/pinia')
      // Reset to fresh state by replacing $state with empty object
      module.config.$state = {}
      // Call getConfigChanges without having saved first — returns {} (no compare base)
      const changes = module.getConfigChanges()
      expect(changes).toEqual({})
    })

    it('saveConfigState captures current config state', async () => {
      const module = await import('@/modules/pinia')
      module.config.$state = { id: 'device-1', mdns: 'gravitymon' }
      module.saveConfigState()
      // After saving, getConfigChanges should return {} (no diff yet)
      const changes = module.getConfigChanges()
      expect(changes).toEqual({})
    })

    it('getConfigChanges detects modified fields after save', async () => {
      const module = await import('@/modules/pinia')
      module.config.$state = { id: 'original', mdns: 'gravitymon' }
      module.saveConfigState()
      // Mutate the state
      module.config.$state.id = 'changed'
      const changes = module.getConfigChanges()
      expect(changes.id).toBe('changed')
      expect(changes.mdns).toBeUndefined() // mdns unchanged
    })

    it('getConfigChanges detects nested object changes', async () => {
      const module = await import('@/modules/pinia')
      module.config.$state = { settings: { temp_unit: 'C' } }
      module.saveConfigState()
      module.config.$state.settings = { temp_unit: 'F' }
      const changes = module.getConfigChanges()
      expect(changes.settings.temp_unit).toBe('F')
    })

    it('saveConfigState resets global.configChanged to false', async () => {
      const module = await import('@/modules/pinia')
      module.global.configChanged = true
      module.config.$state = { id: 'test' }
      module.saveConfigState()
      expect(module.global.configChanged).toBe(false)
    })

    it('deepClone handles arrays in state', async () => {
      const module = await import('@/modules/pinia')
      const arr = [1, 2, 3]
      module.config.$state = { data: arr }
      module.saveConfigState()
      // Mutate the array after save
      arr.push(4)
      // The saved clone should not reflect the mutation
      const changes = module.getConfigChanges()
      // data changed (arr now has 4 elements)
      expect(changes.data).toBeDefined()
    })

    it('deepClone handles null values in state', async () => {
      const module = await import('@/modules/pinia')
      module.config.$state = { formula: null }
      module.saveConfigState()
      const changes = module.getConfigChanges()
      expect(changes).toEqual({})
    })

    it('deepEqual returns true via structural path for nested equal objects', async () => {
      const module = await import('@/modules/pinia')
      // Nested object — deepClone creates a different reference, then deepEqual
      // traverses all keys and returns true via the for-loop completion path
      module.config.$state = { settings: { temp: 'C', gravity: 1.05 } }
      module.saveConfigState()
      // Not mutating — getConfigChanges should find no differences
      const changes = module.getConfigChanges()
      expect(changes).toEqual({})
    })
  })

  describe('$subscribe callback', () => {
    it('early returns when global.initialized is false', async () => {
      const module = await import('@/modules/pinia')
      module.global.initialized = false
      module.global.configChanged = false
      // The callback was registered when pinia.js was first loaded
      const callback = module.config.$subscribe.mock.calls[0][0]
      callback()
      expect(module.global.configChanged).toBe(false)
    })

    it('sets configChanged to false when no changes exist after save', async () => {
      const module = await import('@/modules/pinia')
      module.global.initialized = true
      module.config.$state = { id: 'stable' }
      module.saveConfigState()
      module.global.configChanged = true
      const callback = module.config.$subscribe.mock.calls[0][0]
      callback()
      expect(module.global.configChanged).toBe(false)
    })

    it('sets configChanged to true when state changes after save', async () => {
      const module = await import('@/modules/pinia')
      module.global.initialized = true
      module.config.$state = { id: 'original' }
      module.saveConfigState()
      module.config.$state = { id: 'mutated' }
      module.global.configChanged = false
      const callback = module.config.$subscribe.mock.calls[0][0]
      callback()
      expect(module.global.configChanged).toBe(true)
    })
  })
})
