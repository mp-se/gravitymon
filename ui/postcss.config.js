import purgecss from '@fullhuman/postcss-purgecss'
import cssnano from 'cssnano'

export default {
  plugins: [
    purgecss.default({
      content: [
        './index.html',
        './src/**/*.{vue,js,ts,jsx,tsx}',
        './src/**/*.html'
      ],
      safelist: {
        standard: [
        // Bootstrap Layout
        /^container(-fluid)?$/,
        /^row$/,
        /^col(-.*)?$/,
        /^g[xy]?-\d+$/,
        
        // Bootstrap Components
        /^btn(-.*)?$/,
        /^badge(-.*)?$/,
  /^spinner(-.*)?$/,
  'spinner-border',
  'spinner-border-sm',
  'visually-hidden',
        /^form(-.*)?$/,
        /^input(-.*)?$/,
        /^select(-.*)?$/,
        /^textarea(-.*)?$/,
        /^card(-.*)?$/,
        /^nav(-.*)?$/,
        /^navbar(-.*)?$/,
        /^dropdown(-.*)?$/,
        /^modal(-.*)?$/,
        /^alert(-.*)?$/,
        /^progress(-.*)?$/,
        /^table(-.*)?$/,
        
        // Bootstrap Utilities
        /^d(-.*)?$/,
        /^p[xytblr]?-\d+$/,
        /^m[xytblr]?-\d+$/,
        /^text(-.*)?$/,
        /^bg(-.*)?$/,
        /^border(-.*)?$/,
        /^w(-.*)?$/,
        /^h(-.*)?$/,
        /^position(-.*)?$/,
        /^top-\d+$/,
        /^start-\d+$/,
        /^end-\d+$/,
        /^bottom-\d+$/,
        /^justify(-.*)?$/,
        /^align(-.*)?$/,
        /^flex(-.*)?$/,
        /^float(-.*)?$/,
        /^rounded(-.*)?$/,
        /^shadow(-.*)?$/,
        /^opacity(-.*)?$/,
        /^overflow(-.*)?$/,
        /^z(-.*)?$/,
        
        // Typography
        /^h[1-6]$/,
        /^display-\d+$/,
        /^fs(-.*)?$/,
        /^fw(-.*)?$/,
        /^lh(-.*)?$/,
        /^font(-.*)?$/,
        /^lead$/,
        /^mark$/,
        /^small$/,
        
        // Interactive states
        'active',
        'disabled',
        'show',
        'hide',
        'fade',
        'collapse',
        'collapsed',
        'collapsing',
        
        // Dark mode support - Bootstrap 5.3+ data-bs-theme attribute handling
        // Keep all CSS that responds to [data-bs-theme="dark"]
        /.*\[data-bs-theme.*\].*/,
        // Keep dark mode utility classes
        /.*-dark$/,
        'table-dark',
        'navbar-dark',
        'bg-dark',
        'text-light',
        'border-dark',
        'btn-dark',
        // Keep light mode utility classes for completeness
        'table-light', 
        'navbar-light',
        'bg-light',
        'text-dark',
        'border-light',
        'btn-light',
        
        // Form validation
        'needs-validation',
        'was-validated',
        'is-valid',
        'is-invalid',
        'valid-feedback',
        'invalid-feedback',
        
        // Link utilities
        /^link(-.*)?$/,
        
        // Additional classes found in codebase
        'btn-close',
        'vr', // Vertical rule
        'align-center',
        'align-items-center', 
        /^mx-.*$/,
        /^my-.*$/,
        'overflow-hidden',
        'btn-outline-info',
        'btn-sm',
        'navbar-toggler',
        'navbar-toggler-icon',
        'navbar-brand',
        'navbar-collapse',
        'navbar-nav',
        'nav-item',
        'nav-link',
        'dropdown-toggle',
        'dropdown-menu',
        'dropdown-item',
        /^h-\d+$/,
        /^alert-dismissible$/,
        
        // Dynamically generated classes from components
        /^bg-.*-subtle$/,
        /^text-bg-.*$/
        ],
        keyframes: [
          'spinner-border',
          'spinner-grow'
        ]
      },
      // Standard extraction to catch more classes
      defaultExtractor: content => content.match(/[\w-/:]+(?<!:)/g) || [],
      // Remove unused CSS variables and keyframes
      variables: true,
      keyframes: true
    }),
    // Additional CSS optimization
    cssnano({
      preset: ['default', {
        discardComments: { removeAll: true },
        normalizeWhitespace: true,
        mergeLonghand: true,
        mergeRules: true,
        minifySelectors: true,
        reduceTransforms: true
      }]
    })
  ]
}