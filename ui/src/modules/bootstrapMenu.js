/*
 * Minimal Bootstrap data-API compatibility for the navigation controls used
 * by GravityMon. Keeping this local avoids bundling all Bootstrap JavaScript
 * (and Popper) into every firmware image.
 */

function setExpanded(toggle, expanded) {
  toggle.setAttribute('aria-expanded', String(expanded))
}

function closeDropdowns(except = null) {
  document.querySelectorAll('.dropdown-menu.show').forEach((menu) => {
    if (menu !== except) menu.classList.remove('show')
  })
  document.querySelectorAll('[data-bs-toggle="dropdown"][aria-expanded="true"]').forEach((toggle) => {
    const container = toggle.closest('.dropdown')
    if (!except || !container?.contains(except)) setExpanded(toggle, false)
  })
}

document.addEventListener('click', (event) => {
  const collapseToggle = event.target.closest('[data-bs-toggle="collapse"]')
  if (collapseToggle) {
    event.preventDefault()
    const selector = collapseToggle.getAttribute('data-bs-target')
    const target = selector ? document.querySelector(selector) : null
    if (!target) return

    const expanded = !target.classList.contains('show')
    target.classList.toggle('show', expanded)
    setExpanded(collapseToggle, expanded)
    return
  }

  const dropdownToggle = event.target.closest('[data-bs-toggle="dropdown"]')
  if (dropdownToggle) {
    event.preventDefault()
    const container = dropdownToggle.closest('.dropdown')
    const menu = container?.querySelector('.dropdown-menu')
    if (!menu) return

    const expanded = !menu.classList.contains('show')
    closeDropdowns(menu)
    menu.classList.toggle('show', expanded)
    setExpanded(dropdownToggle, expanded)
    return
  }

  if (!event.target.closest('.dropdown')) closeDropdowns()
})
