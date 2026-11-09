<!--
  GravityMon
  Copyright (c) 2021-2026 Magnus

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Alternatively, this software may be used under the terms of a
  commercial license. See LICENSE_COMMERCIAL for details.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
-->
<template>
  <div class="container">
    <div class="alert alert-info alert-dismissible fade show" role="alert">
      <strong>{{ t('fragment_brewgraph_promo.title') }}</strong>
      {{ t('fragment_brewgraph_promo.intro') }}
      <a class="alert-link" :href="url" target="_blank" rel="noopener" @click="emit('close')">
        {{ t('fragment_brewgraph_promo.visit') }}
      </a>
      <button
        type="button"
        class="btn-close"
        :aria-label="t('fragment_brewgraph_promo.close')"
        @click="emit('close')"
      ></button>
    </div>
  </div>
</template>

<script setup>
import { useI18n } from 'vue-i18n'

const { t } = useI18n()
const emit = defineEmits(['close'])
const url = 'https://www.brewgraph.com'
</script>
