<template>
  <div class="container">
    <p></p>
    <p class="h3">{{ t('cuckoo.about.title') }}</p>
    <hr />

    <p class="h4">{{ t('cuckoo.about.based_on') }}</p>
    <p class="fw-normal">
      {{ t('cuckoo.about.intro_before') }}
      <a class="link-primary" href="https://github.com/mp-se/gravitymon" target="_blank">GravityMon</a>
      {{ t('cuckoo.about.intro_after') }}
    </p>

    <p class="h4">{{ t('cuckoo.about.license_title') }}</p>
    <p class="fw-normal">{{ t('cuckoo.about.license_text') }}</p>

    <p class="h4">MIT License</p>

    <!-- The license text is kept in English in all languages -->
    <p class="fw-normal">
      Permission is hereby granted, free of charge, to any person obtaining a copy of this software
      and associated documentation files (the "Software"), to deal in the Software without
      restriction, including without limitation the rights to use, copy, modify, merge, publish,
      distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
      Software is furnished to do so, subject to the following conditions: The copyright notice and
      this permission notice shall be included in all copies or substantial portions of the
      Software. THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
      INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
      PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
      ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
      ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
      SOFTWARE.
    </p>

    <p class="fw-normal">Copyright (c) 2021-2026 Magnus Persson</p>
  </div>
</template>

<script setup>
import { useI18n } from 'vue-i18n'

const { t } = useI18n()
</script>
