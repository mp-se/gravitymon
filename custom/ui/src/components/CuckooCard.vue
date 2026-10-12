<template>
  <div class="card">
    <div class="card-header bg-primary-subtle">{{ header }}</div>
    <div class="card-body">
      <slot></slot>
    </div>
  </div>
</template>

<script setup>
// Card used on the home page, same markup as the manufacturer's BsCardSimple.
defineProps({
  header: { type: String, default: '' }
})
</script>
