import { ref } from 'vue'

// Copy to the clipboard. The device is served over http, where the async
// clipboard API is not available, so the old select-and-copy way is used.
export function copyToClipboard(text) {
  const input = document.createElement('input')
  input.style.position = 'fixed'
  input.style.top = '-10000px'
  input.style.zIndex = '-999'
  document.body.appendChild(input)
  input.value = text
  input.focus()
  input.select()
  const result = document.execCommand('copy')
  document.body.removeChild(input)
  return result
}

// "(14 min 50 sec)" shown after a sleep interval label
export function intervalLabel(seconds) {
  const s = Number(seconds) || 0
  return ' (' + Math.floor(s / 60) + ' min ' + (s % 60) + ' s)'
}

// Copy button state: `copied` is true for a short while after a successful copy
export function useCopied() {
  const copied = ref(false)

  const copy = (text) => {
    if (copyToClipboard(text)) {
      copied.value = true
      setTimeout(() => {
        copied.value = false
      }, 1500)
    }
  }

  return { copied, copy }
}
