// Build config for the CuckooTilt UI.
//
// Reuses the standard UI config (defines, compression, minification) and only
// changes the entry/root, the module aliases, the CSS purge setup and the
// output folder. Run it from this folder (the standard config reads
// ./package.json and .env relative to the working dir, and the CSS purge
// looks for ./src relative to it):
//
//   cd custom/ui && npx vite build
//
// Dependencies are not installed here, node_modules is a link to ../../ui/node_modules.
import { fileURLToPath, URL } from 'node:url'
import { defineConfig, mergeConfig } from 'vite'
import baseConfig from '../../ui/vite.config.js'

const here = (p) => fileURLToPath(new URL(p, import.meta.url))

export default defineConfig((env) => {
  const config = mergeConfig(baseConfig(env), {
    root: here('.'),
    // Same .env files as the standard UI (VITE_APP_HOST for the dev server etc.)
    envDir: here('../../ui'),
    // Use the standard CSS purge/minify setup (ui/postcss.config.js)
    css: { postcss: here('../../ui') },
    server: { fs: { allow: [here('../..')] } },
    build: {
      outDir: here('./dist'),
      emptyOutDir: true,
      // The standard config drops property reads that have no result used
      // (propertyReadSideEffects: false). vue-i18n tracks the active language by
      // reading the locale refs in exactly that way, so with it dropped only the
      // components that read config.locale themselves were redrawn when the
      // language was changed, and the pages kept the old language.
      rolldownOptions: { treeshake: { propertyReadSideEffects: true } }
    }
  })

  // Replace the aliases (an array, so the order decides which one wins):
  //  - the standard i18n module is replaced by the custom one, so the full
  //    standard English file is not bundled and the shared modules get the
  //    embedded languages
  //  - '@' is the standard UI source (stores, shared views, locales)
  //  - '@custom' is this folder
  config.resolve.alias = [
    { find: '@/modules/i18n', replacement: here('./src/i18n.js') },
    { find: '@custom', replacement: here('./src') },
    { find: '@', replacement: here('../../ui/src') }
  ]
  return config
})
