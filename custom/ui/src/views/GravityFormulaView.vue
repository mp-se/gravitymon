<template>
  <div class="container">
    <p></p>
    <p class="h3">{{ t('cuckoo.formula.title') }}</p>
    <hr />

    <BsMessage
      v-if="config.gravity_formula === ''"
      :dismissable="true"
      :message="t('gravity_formula.need_formula')"
      alert="warning"
    >
    </BsMessage>

    <form @submit.prevent="save" class="needs-validation" novalidate>
      <div class="col-md-10">
        <BsInputText
          v-model="config.gravity_formula"
          maxlength="200"
          :label="t('gravity_formula.formula_label')"
          :help="t('cuckoo.formula.help')"
          :badge="badge.gravityFormulaBadge()"
          :disabled="global.disabled"
        >
        </BsInputText>
      </div>

      <div class="col-md-12 mt-2">
        <button
          type="submit"
          class="btn btn-primary"
          :disabled="global.disabled || !global.configChanged"
        >
          <span
            class="spinner-border spinner-border-sm"
            role="status"
            aria-hidden="true"
            :hidden="!global.disabled"
          ></span
          >{{ t('gravity_formula.save') }}
        </button>
        <button
          type="button"
          class="btn btn-secondary mx-2"
          :disabled="global.disabled"
          @click="useDefaultFormula"
        >
          {{ t('cuckoo.formula.use_default') }}
        </button>
      </div>
    </form>
  </div>
</template>

<script setup>
import { useI18n } from 'vue-i18n'
import { validateCurrentForm } from '@mp-se/espframework-ui-components'
import { config, global } from '@/modules/pinia'
import * as badge from '@/modules/badge'

const { t } = useI18n()

// Same formula as the firmware default (CuckooConfig)
const DEFAULT_FORMULA =
  '0.979742642057399+0.0005883931405567239*tilt+0.000008876047085889598*tilt*tilt'

const save = async () => {
  if (!validateCurrentForm()) return

  await config.saveAll()
}

const useDefaultFormula = () => {
  config.gravity_formula = DEFAULT_FORMULA
}
</script>
