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
    <div class="form-text">
      {{ t('fragment_gravity_graph.intro') }}
    </div>
  </div>

  <div class="row">
    <LineChart id="gravityChart" :datasets="datasets" x-title="Angle" y-title="Gravity" :x-min="25" :y-min="1.0" />
  </div>
</template>

<script setup>
import { ref, computed, onMounted } from 'vue'
import { useI18n } from 'vue-i18n'
import { config } from '@/modules/pinia'
import { evaluateFormula } from '@/modules/formula'
import LineChart from '@/components/LineChart.vue'

const { t } = useI18n()

const chartDataForm = ref([])
const chartDataCalc = ref([])

const dataSetChart = ref({
  datasets: [
    {
      label: 'Data for gravity calculation',
      color: 'blue',
      points: chartDataForm.value
    },
    {
      label: 'Data based on current formula',
      color: 'green',
      points: chartDataCalc.value
    }
  ]
})

const datasets = computed(() => dataSetChart.value.datasets)

onMounted(() => {
  evaluateFormula(config.gravity_formula).forEach((p) => {
    chartDataCalc.value.push(p)
  })

  let data = config.formula_calculation_data
  data.sort((a, b) => (a.a == 0 ? 100 : a.a - b.a))

  for (let i = 0; i < data.length; i++) {
    if (data[i].a > 0)
      chartDataForm.value.push({
        x: data[i].a,
        y: data[i].g
      })
  }
})
</script>

<style></style>
