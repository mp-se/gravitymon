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
    <p></p>
    <p class="h2">Gravity - Formula</p>
    <hr />

    <BsMessage v-if="config.gravity_formula === ''" dismissable="true" message="" alert="warning">
      You need to enter a formula in order to report gravity
    </BsMessage>

    <div
      class="col-md-12"
      v-if="
        status.self_check.gyro_calibration &&
        status.self_check.gyro_connected &&
        status.wifi_setup == false
      "
    >
      <div class="alert alert-info">
        <div class="row">
          <div class="col-md-6">
            <label class="form-label fs-6 fw-bold">Angle:</label>&nbsp;
            <label class="form-label fs-6">{{ angle.last }}</label
            >&nbsp; <label class="form-label fs-6 fw-bold">Average angle:</label>&nbsp;
            <label class="form-label fs-6">{{ angle.average }} ({{ angle.count }})</label>&nbsp;

            <button
              @click="clearAverage"
              type="button"
              class="btn btn-outline-primary btn-sm"
              style="font-size: 0.7rem"
            >
              Clear
            </button>
          </div>
        </div>
      </div>
    </div>

    <form @submit.prevent="save" class="needs-validation" novalidate>
      <div class="row">
        <div class="col-md-10">
          <BsInputText
            v-model="config.gravity_formula"
            maxlength="200"
            label="Gravity formula"
            help="Formula used to convert angle to gravity. If created outside Gravitymon the formula needs to be created for Specific Gravity!"
            :badge="badge.gravityFormulaBadge()"
            :disabled="global.disabled"
          >
          </BsInputText>
        </div>

        <div class="col-md-2">
          <BsDropdown
            label="Formulas"
            button="Formula"
            :options="formulaOptions"
            :callback="formulaSelectCallback"
            :disabled="formulaOptions.length == 0"
          />
        </div>

        <div class="col-md-12">
          <label class="form-label fw-bold">Data for gravity calculation (Angle and Gravity)</label>
        </div>

        <template v-for="(data, index) in config.formula_calculation_data" :key="index">
          <div class="col-md-6">
            <div class="input-group has-validation">
              <span class="input-group-text">{{ index + 1 }}</span>
              <input
                v-model="config.formula_calculation_data[index].a"
                class="form-control w-2"
                type="number"
                min="0"
                max="90"
                step=".001"
                :disabled="global.disabled"
              />
              <span class="input-group-text">{{ '°' }}</span>
            </div>
          </div>
          <div class="col-md-6">
            <div class="input-group has-validation">
              <span class="input-group-text">{{ index + 1 }}</span>
              <input
                v-model="config.formula_calculation_data[index].g"
                class="form-control"
                type="number"
                min="0"
                max="30"
                step=".0001"
                :disabled="global.disabled"
              />
              <span class="input-group-text">{{ config.gravity_unit == 'G' ? 'SG' : 'P' }}</span>
            </div>
          </div>
        </template>

        <div class="form-text">
          Enter the data that is used to create a new formula. The most optimal formula will be
          selected and also validated towards these values.
        </div>

        <div class="col-md-6">
          <BsInputNumber
            v-model="config.formula_max_deviation"
            label="Max allowed deviation"
            min="0"
            max="10"
            step=".0001"
            width="4"
            help="When validating the derived formula this is the maximum accepted deviation for the supplied values, use graph below to visually check where there are deviations"
            :disabled="global.disabled"
          ></BsInputNumber>
        </div>

        <div class="col-md-6">
          <BsInputNumber
            v-model="noDecimals"
            label="Number of decimals in formula"
            min="1"
            max="10"
            step="1"
            width="4"
            help="How many decimals to try to limit in the formula"
            :disabled="global.disabled"
          ></BsInputNumber>
        </div>
      </div>

      <div class="row gy-2">
        <div class="col-md-12">
          <p></p>
          <button
            type="submit"
            class="btn btn-primary w-2"
            :disabled="global.disabled || !global.configChanged"
          >
            <span
              class="spinner-border spinner-border-sm"
              role="status"
              aria-hidden="true"
              v-show="global.disabled"
            ></span>
            &nbsp;Save</button
          >&nbsp;
          <button
            @click.prevent="createFormula"
            type="button"
            class="btn btn-primary w-2"
            :disabled="global.disabled"
          >
            Create formula</button
          >&nbsp;
          <button
            v-if="global.ui.enableCalibrationRegistration"
            @click.prevent="openRegisterModal"
            type="button"
            class="btn btn-secondary w-2"
            :disabled="global.disabled || status.wifi_setup == true"
          >
            Report calibration data</button
          >&nbsp;
          <button
            v-if="!hasFormulaCalculationData"
            @click.prevent="generateFormulaCalculationData"
            type="button"
            class="btn btn-secondary w-2"
            :disabled="global.disabled || status.wifi_setup == true"
          >
            Generate datapoints based on formula</button
          >&nbsp;
        </div>
      </div>

      <div class="row" v-if="expressions != null">
        <BsInputRadio
          v-model="formulaOutput"
          :options="formulaOutputOptions"
          label="Output format"
          :disabled="global.disabled"
        ></BsInputRadio>
      </div>
    </form>

    <GravityGraphFragment v-if="renderComponent && formulaOutput == 0"></GravityGraphFragment>
    <FormulaFragment
      v-if="renderComponent && expressions != null && formulaOutput == 1"
      :expressions="expressions"
    ></FormulaFragment>
    <FormulaTableFragment
      v-if="renderComponent && expressions != null && formulaOutput == 2"
      :expressions="expressions"
    ></FormulaTableFragment>
    <FormulaGraphFragment
      v-if="renderComponent && expressions != null && formulaOutput == 3"
      :expressions="expressions"
    ></FormulaGraphFragment>

    <RegisterCalibrationFragment v-if="showRegisterModal" @close="closeRegisterModal" />
  </div>
</template>

<script setup>
import { nextTick, ref, onBeforeMount, computed } from 'vue'
import { validateCurrentForm, roundVal } from '@mp-se/espframework-ui-components'
import { global, status, config } from '@/modules/pinia'
import GravityGraphFragment from '@/fragments/GravityGraphFragment.vue'
import { logDebug } from '@mp-se/espframework-ui-components'
import * as badge from '@/modules/badge'
import FormulaFragment from '@/fragments/FormulaFragment.vue'
import FormulaGraphFragment from '@/fragments/FormulaGraphFragment.vue'
import FormulaTableFragment from '@/fragments/FormulaTableFragment.vue'
import { PolynomialRegression } from 'ml-regression-polynomial'
import { validateFormula, calculate } from '@/modules/formula'
import { gravityToSG } from '@mp-se/espframework-ui-components'
import { useTimers } from '@mp-se/espframework-ui-components'

const showRegisterModal = ref(false)

const { createInterval } = useTimers()
const polling = ref(null)
const angle = ref({ last: 0, average: 0, sum: 0, count: 0 })

const expressions = ref(null)
const noDecimals = ref(8)
const formulaOptions = ref([])
const renderComponent = ref(true)
const formulaOutput = ref(0)
const formulaOutputOptions = ref([
  { label: 'Current', value: 0 },
  { label: 'Formula', value: 1 },
  { label: 'Table', value: 2 },
  { label: 'Graph', value: 3 }
])

const hasFormulaCalculationData = computed(() => {
  return config.formula_calculation_data.some((d) => d.a !== 0)
})

function openRegisterModal() {
  showRegisterModal.value = true
}

function closeRegisterModal() {
  showRegisterModal.value = false
}

function clearAverage() {
  angle.value.last = 0
  angle.value.sum = 0
  angle.value.count = 0
  angle.value.average = 0
}

async function refresh() {
  const result = await status.getGyro()
  if (result.success) {
    if (result.data.angle !== 0) {
      angle.value.last = Math.round((parseFloat(result.data.angle) * 100) / 100).toFixed(2)
      angle.value.sum += parseFloat(result.data.angle)
      angle.value.count++
      angle.value.average = (Math.round((angle.value.sum / angle.value.count) * 100) / 100).toFixed(
        2
      )
    }
  }
}

onBeforeMount(async () => {
  await refresh()
  polling.value = createInterval(refresh, 2000)
})

const formulaSelectCallback = (opt) => {
  config.gravity_formula = opt
}

const createFormula = () => {
  if (!validateCurrentForm()) return

  logDebug('GravityFormulaView.createFormula()')
  expressions.value = null
  formulaOptions.value = []
  formulaOutput.value = 3

  var x = [],
    y = [],
    res = { 1: '', 2: '', 3: '', 4: '' }

  for (let i = 0; i < config.formula_calculation_data.length; i++) {
    x.push(config.formula_calculation_data[i].a)
    y.push(
      config.gravity_unit == 'P'
        ? gravityToSG(config.formula_calculation_data[i].g)
        : config.formula_calculation_data[i].g
    )
  }

  for (var i = 1; i < 5; i++) {
    const regression = new PolynomialRegression(x, y, i)

    let f = regression.toString(noDecimals.value)

    f = f.replaceAll(' ', '')
    f = f.replaceAll('f(x)=', '')
    f = f.replaceAll('x', 'tilt')

    if (validateFormula(f)) {
      res[i] = f
      formulaOptions.value.push({ value: f, label: 'Formula Order ' + i })
    } else {
      res[i] = ''
    }
  }

  expressions.value = Object.values(res)
  forceRerender()
}

const forceRerender = async () => {
  renderComponent.value = false
  await nextTick()
  renderComponent.value = true
}

const generateFormulaCalculationData = () => {
  if (!config.gravity_formula) {
    global.messageWarning = 'No formula defined. Please enter a formula first.'
    return
  }

  const startAngle = 20
  const angleIncrement = 5
  const numberOfPositions = 14

  for (let i = 0; i < numberOfPositions; i++) {
    const angle = startAngle + i * angleIncrement
    const gravityValue = calculate(config.gravity_formula, angle)

    config.formula_calculation_data[i].a = angle
    config.formula_calculation_data[i].g = roundVal(gravityValue, 4)
  }

  global.messageSuccess = `Generated ${numberOfPositions} data points for formula validation`
}

const save = async () => {
  if (!validateCurrentForm()) return

  await config.saveAll()
  forceRerender()
}
</script>
