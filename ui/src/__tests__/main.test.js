describe('main.js import', () => {
  it('imports main.js after creating #app element', async () => {
    // Ensure mount target exists before importing main
    const root = document.createElement('div')
    root.id = 'app'
    document.body.appendChild(root)

    // dynamic import so mount runs after DOM setup
    await import('@/main.js')
    expect(document.getElementById('app')).not.toBeNull()
  })
})
