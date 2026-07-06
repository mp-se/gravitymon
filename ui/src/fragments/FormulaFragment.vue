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
  <div class="row">
    <p></p>
    <hr />
  </div>

  <div class="row">
    <div class="form-text">Below is a list of the generated formulas.</div>
  </div>

  <div class="row">
    <div class="col-md-12" v-for="(i, index) in expressions" :key="index">
      <BsInputReadonly
        :label="'Formula order ' + index"
        v-model="expressions[index]"
        v-if="expressions[index] != ''"
      ></BsInputReadonly>
      <BsInputReadonly
        :label="'Formula order ' + index"
        v-model="rejected"
        v-if="expressions[index] == ''"
      ></BsInputReadonly>
    </div>
  </div>
</template>

<script setup>
import { ref } from 'vue'

/**
 * Hold results from regression library.
 */
const expressions = defineModel('expressions', {
  type: Array,
  default: () => []
}) // Hold results from regression library
const rejected = ref('Formula rejected')
</script>
