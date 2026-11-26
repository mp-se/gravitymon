/*
 * Minimal Bootstrap data-API compatibility for the navigation controls and
 * confirm modals used by GravityMon. Keeping this local avoids bundling all Bootstrap JavaScript
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

function showModal(modal) {
  modal.style.display = 'block'
  modal.removeAttribute('aria-hidden')
  modal.setAttribute('aria-modal', 'true')
  modal.classList.add('show')
  document.body.classList.add('modal-open')
  const backdrop = document.createElement('div')
  backdrop.className = 'modal-backdrop fade show'
  backdrop.dataset.modalBackdrop = modal.id
  document.body.appendChild(backdrop)
}

function hideModal(modal) {
  modal.classList.remove('show')
  modal.style.display = 'none'
  modal.setAttribute('aria-hidden', 'true')
  modal.removeAttribute('aria-modal')
  document.querySelectorAll('[data-modal-backdrop]').forEach((b) => b.remove())
  document.body.classList.remove('modal-open')
}

// Tooltips: the input components put their help text in data-bs-title. Show it
// as a native browser tooltip instead of bundling Bootstrap's tooltip/Popper.
document.addEventListener('mouseover', (event) => {
  const el = event.target.closest('[data-bs-title]')
  if (el && el.getAttribute('title') !== el.dataset.bsTitle) el.title = el.dataset.bsTitle
})

document.addEventListener('click', (event) => {
  const modalToggle = event.target.closest('[data-bs-toggle="modal"]')
  if (modalToggle) {
    event.preventDefault()
    const selector = modalToggle.getAttribute('data-bs-target')
    const modal = selector ? document.querySelector(selector) : null
    if (modal) showModal(modal)
    return
  }

  if (event.target.closest('[data-bs-dismiss="modal"]')) {
    const modal = event.target.closest('.modal')
    if (modal) hideModal(modal)
    return
  }

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
