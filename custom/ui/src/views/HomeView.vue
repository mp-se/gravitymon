<template>
  <div class="container">
    <p></p>

    <div class="container overflow-hidden text-center">
      <div class="row gy-4">
        <div class="col-md-4">
          <CuckooCard :header="t('cuckoo.home.wifi_header')">
            <div class="cuckoo-card-body">
              <span class="text-center">{{ status.wifi_ssid }}</span>
              <router-link :class="linkClass" to="/device/wifi">{{
                t('cuckoo.home.wifi_link')
              }}</router-link>
            </div>
          </CuckooCard>
        </div>

        <div class="col-md-4">
          <CuckooCard :header="t('cuckoo.home.http_post_header')">
            <div class="cuckoo-card-body">
              <router-link :class="linkClass" to="/push/http-post">{{
                t('cuckoo.home.http_post_link')
              }}</router-link>
            </div>
          </CuckooCard>
        </div>

        <div class="col-md-4" v-if="global.feature.ble">
          <CuckooCard :header="t('cuckoo.home.bluetooth_header')">
            <div class="cuckoo-card-body">
              <router-link :class="linkClass" to="/push/bluetooth">{{
                t('cuckoo.home.bluetooth_link')
              }}</router-link>
            </div>
          </CuckooCard>
        </div>

        <div class="col-md-4">
          <CuckooCard :header="t('cuckoo.home.formula_header')">
            <div class="cuckoo-card-body">
              <router-link :class="linkClass" to="/gravity/formula">{{
                t('cuckoo.home.formula_link')
              }}</router-link>
            </div>
          </CuckooCard>
        </div>

        <div class="col-md-4">
          <CuckooCard :header="t('cuckoo.home.firmware_header')">
            <div class="cuckoo-card-body">
              <span class="text-center">{{
                t('cuckoo.home.firmware_text', { version: global.app_ver })
              }}</span>
              <router-link :class="linkClass" to="/other/firmware">{{
                t('cuckoo.home.firmware_link')
              }}</router-link>
            </div>
          </CuckooCard>
        </div>

        <div class="col-md-4">
          <CuckooCard :header="t('cuckoo.home.battery_header')">
            <div class="cuckoo-card-body">
              <span class="text-center">{{ status.battery }} V</span>
              <router-link :class="linkClass" to="/device/battery">{{
                t('cuckoo.home.battery_link')
              }}</router-link>
            </div>
          </CuckooCard>
        </div>

        <div class="col-md-4">
          <CuckooCard :header="t('cuckoo.home.device_header')">
            <div class="cuckoo-card-body">
              <span class="text-center">
                {{ status.id }}
                <button
                  type="button"
                  class="btn btn-outline-secondary btn-sm mx-2"
                  style="width: 69px; height: 24px; padding-top: 0; padding-bottom: 0"
                  @click="copyId"
                >
                  {{ copied ? t('cuckoo.home.copied') : t('cuckoo.home.copy_id') }}
                </button>
              </span>
            </div>
          </CuckooCard>
        </div>

        <div class="col-md-4">
          <CuckooCard :header="t('cuckoo.home.device_header')">
            <div class="cuckoo-card-body">
              <span class="text-center">{{ t('cuckoo.home.tools_text') }}</span>
              <router-link :class="linkClass" to="/other/system-tools">{{
                t('cuckoo.home.tools_link')
              }}</router-link>
            </div>
          </CuckooCard>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { useI18n } from 'vue-i18n'
import { global, status } from '@/modules/pinia'
import { useCopied } from '@custom/utils'
import CuckooCard from '@custom/components/CuckooCard.vue'

const { t } = useI18n()

const linkClass =
  'link-primary link-offset-2 link-underline-opacity-25 link-underline-opacity-100-hover'

const { copied, copy } = useCopied()
const copyId = () => copy(status.id)
</script>

<style scoped>
.cuckoo-card-body {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  height: 80px;
  gap: 1rem;
}
</style>
