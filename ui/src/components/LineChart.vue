<!--
  Small dependency-free x/y line chart (inline SVG), replacing Chart.js to save firmware flash.
  Self-contained on purpose (only depends on vue) so it can move to espframework-ui-components.

  Props:
    datasets   [{ label, color, points: [{x, y}], hidden? }]   hidden datasets are not drawn and shown struck-through in the legend
    xTitle / yTitle   axis titles
    xMin / yMin       "suggested min": the axis starts at min(data min, suggested min), like Chart.js suggestedMin
    grace             fraction of the range added as padding on each axis (default 0.05)
  Legend entries toggle a dataset (click). Hovering/touching shows the nearest point.
-->
<template>
  <div class="line-chart">
    <div class="d-flex flex-wrap justify-content-center gap-3 mb-1 small">
      <span
        v-for="(d, i) in datasets"
        :key="i"
        class="line-chart-legend"
        :class="{ 'text-decoration-line-through opacity-50': isOff(i) }"
        role="button"
        tabindex="0"
        @click="toggle(i)"
        @keydown.enter="toggle(i)"
      >
        <span class="line-chart-swatch" :style="{ background: d.color }"></span>{{ d.label }}
      </span>
    </div>
    <svg
      :viewBox="`0 0 ${W} ${H}`"
      class="w-100"
      @pointermove="onMove"
      @pointerleave="hover = null"
    >
      <g stroke="currentColor" fill="none">
        <g v-for="t in xTicks" :key="'x' + t">
          <line :x1="sx(t)" :x2="sx(t)" :y1="T" :y2="H - B" opacity="0.12" />
          <text :x="sx(t)" :y="H - B + 14" fill="currentColor" stroke="none" text-anchor="middle">
            {{ fmt(t, xDec) }}
          </text>
        </g>
        <g v-for="t in yTicks" :key="'y' + t">
          <line :x1="L" :x2="W - R" :y1="sy(t)" :y2="sy(t)" opacity="0.12" />
          <text :x="L - 6" :y="sy(t) + 4" fill="currentColor" stroke="none" text-anchor="end">
            {{ fmt(t, yDec) }}
          </text>
        </g>
        <path :d="`M${L},${T}V${H - B}H${W - R}`" opacity="0.5" />
      </g>
      <text :x="(L + W - R) / 2" :y="H - 4" fill="currentColor" text-anchor="middle">
        {{ xTitle }}
      </text>
      <text
        :transform="`translate(12 ${(T + H - B) / 2}) rotate(-90)`"
        fill="currentColor"
        text-anchor="middle"
      >
        {{ yTitle }}
      </text>
      <template v-for="(d, i) in datasets" :key="'d' + i">
        <g v-if="!isOff(i)" :stroke="d.color" :fill="d.color">
          <polyline
            :points="d.points.map((p) => sx(p.x) + ',' + sy(p.y)).join(' ')"
            fill="none"
            stroke-width="2"
          />
          <circle v-for="(p, j) in d.points" :key="j" :cx="sx(p.x)" :cy="sy(p.y)" r="3" stroke="none" />
        </g>
      </template>
      <g v-if="hover" pointer-events="none">
        <circle :cx="sx(hover.p.x)" :cy="sy(hover.p.y)" r="5" fill="none" stroke="currentColor" />
        <rect :x="tipX" :y="tipY" :width="tipW" height="18" rx="3" fill="#000" opacity="0.8" />
        <text :x="tipX + 6" :y="tipY + 13" fill="#fff">{{ tipText }}</text>
      </g>
    </svg>
  </div>
</template>

<script setup>
import { ref, computed, reactive } from 'vue'

const props = defineProps({
  datasets: { type: Array, default: () => [] },
  xTitle: { type: String, default: '' },
  yTitle: { type: String, default: '' },
  xMin: { type: Number, default: undefined },
  yMin: { type: Number, default: undefined },
  grace: { type: Number, default: 0.05 }
})

const W = 600
const H = 320
const L = 56
const R = 12
const T = 8
const B = 40

const off = reactive({})
const hover = ref(null)
// User toggle wins over the dataset's own 'hidden' flag
const isOff = (i) => off[i] ?? !!props.datasets[i].hidden
const toggle = (i) => (off[i] = !isOff(i))

const visible = computed(() =>
  props.datasets.flatMap((d, i) => (isOff(i) ? [] : d.points.map((p) => ({ ...p, d }))))
)

// Axis domain: data range (+ suggested min) padded by 'grace'
function domain(key, suggestedMin) {
  const v = visible.value.map((p) => p[key])
  let lo = v.length ? Math.min(...v) : 0
  let hi = v.length ? Math.max(...v) : 1
  if (suggestedMin !== undefined) lo = Math.min(lo, suggestedMin)
  if (hi === lo) hi = lo + 1
  const g = (hi - lo) * props.grace
  return [lo - g, hi + g]
}
const xd = computed(() => domain('x', props.xMin))
const yd = computed(() => domain('y', props.yMin))

const sx = (v) => L + ((v - xd.value[0]) / (xd.value[1] - xd.value[0])) * (W - L - R)
const sy = (v) => H - B - ((v - yd.value[0]) / (yd.value[1] - yd.value[0])) * (H - T - B)

// "Nice" tick positions (1, 2, 5 x 10^n)
function ticks([lo, hi], count) {
  const raw = (hi - lo) / count
  const mag = 10 ** Math.floor(Math.log10(raw))
  const f = raw / mag
  const step = (f < 1.5 ? 1 : f < 3 ? 2 : f < 7 ? 5 : 10) * mag
  const out = []
  for (let t = Math.ceil(lo / step) * step; t <= hi + step * 1e-9; t += step) out.push(+t.toPrecision(12))
  return { out, dec: Math.min(6, Math.max(0, -Math.floor(Math.log10(step) + 1e-9))) }
}
const xt = computed(() => ticks(xd.value, 8))
const yt = computed(() => ticks(yd.value, 6))
const xTicks = computed(() => xt.value.out)
const yTicks = computed(() => yt.value.out)
const xDec = computed(() => xt.value.dec)
const yDec = computed(() => yt.value.dec)
const fmt = (v, dec) => v.toFixed(dec)

function onMove(e) {
  const r = e.currentTarget.getBoundingClientRect()
  const mx = ((e.clientX - r.left) / r.width) * W
  let best = null
  let bd = Infinity
  for (const p of visible.value) {
    const dist = Math.abs(sx(p.x) - mx)
    if (dist < bd) {
      bd = dist
      best = { p, d: p.d }
    }
  }
  hover.value = best
}

const tipText = computed(() =>
  hover.value
    ? `${hover.value.d.label}: ${fmt(hover.value.p.x, xDec.value + 1)}, ${fmt(hover.value.p.y, yDec.value + 1)}`
    : ''
)
const tipW = computed(() => tipText.value.length * 6.2 + 12)
const tipX = computed(() => Math.min(Math.max(sx(hover.value.p.x) + 8, L), W - R - tipW.value))
const tipY = computed(() => Math.max(sy(hover.value.p.y) - 26, T))
</script>

<style scoped>
.line-chart svg {
  font-size: 11px;
  touch-action: pan-y;
}
.line-chart-legend {
  cursor: pointer;
  user-select: none;
}
.line-chart-swatch {
  display: inline-block;
  width: 2.2em;
  height: 0.7em;
  margin-right: 0.4em;
}
</style>
