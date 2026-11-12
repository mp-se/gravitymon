import { defineConfig } from 'vitest/config'
import vue from '@vitejs/plugin-vue'
import { fileURLToPath } from 'url'
import path from 'path'

const __dirname = path.dirname(fileURLToPath(import.meta.url))

export default defineConfig({
  plugins: [vue()],
  resolve: { alias: { '@': path.resolve(__dirname, 'src') } },
  test: {
    globals: true,
    environment: 'jsdom',
    setupFiles: 'src/tests/setup.js',
    // vitest 5 defaults to true; pinia.test.js reads mock calls recorded at module load
    clearMocks: false,
    coverage: {
      provider: 'istanbul',
      include: ['src/**/*.{js,vue}'],
      exclude: [
        'src/tests/**',
        'src/**/__tests__/**',
        'src/main.js'
      ],
      reporter: ['text', 'lcov', 'html'],
      reportsDirectory: './coverage'
    }
  }
})
