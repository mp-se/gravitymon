/*
 * GravityMon
 * Copyright (c) 2021-2026 Magnus
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Alternatively, this software may be used under the terms of a
 * commercial license. See LICENSE_COMMERCIAL for details.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
import { ref } from 'vue'

export const gravityHttpPostFormatOptions = ref([
  {
    label: 'GravityMon (null measurements)',
    value: encodeURIComponent(
      '{"name":"${mdns}","ID":"${id}","token":"${token}","interval":${sleep-interval},"temperature":${temp-null},"temp_units":"${temp-unit}","gravity":${gravity-null},"tilt":${tilt-null},"battery":${battery},"RSSI":${rssi},"corr-gravity":${corr-gravity-null},"gravity-unit":"${gravity-unit}","run-time":${run-time}}'
    )
  },
  {
    label: 'GravityMon',
    value: encodeURIComponent(
      '{"name":"${mdns}","ID":"${id}","token":"${token}","interval":${sleep-interval},"temperature":${temp},"temp_units":"${temp-unit}","gravity":${gravity},"angle":${angle},"battery":${battery},"RSSI":${rssi},"corr-gravity":${corr-gravity},"gravity-unit":"${gravity-unit}","run-time":${run-time}}'
    )
  },
  {
    label: 'GravityMon v2',
    value: encodeURIComponent('{"name":"${mdns}","ID":"${id}","token":"${token}","interval":${sleep-interval},"temperature":${temp},"temp_units":"${temp-unit}","gravity":${gravity},"velocity":${velocity},"angle":${angle},"battery":${battery},"RSSI":${rssi},"corr-gravity":${corr-gravity},"gravity-unit":"${gravity-unit}","run-time":${run-time}}')
  },
  {
    label: 'iSpindle',
    value: encodeURIComponent('{"name":"${mdns}","ID":"${id}","token":"${token}","interval":${sleep-interval},"temperature":${temp},"temp_units":"${temp-unit}","gravity":${gravity},"angle":${angle},"battery":${battery},"RSSI":${rssi}}')
  },
  {
    label: 'BrewFatherCustom',
    value: encodeURIComponent('{"name":"${mdns}","temp":${temp},"aux_temp":0,"ext_temp":0,"temp_unit":"${temp-unit}","gravity":${gravity},"gravity_unit":"${gravity-unit}","pressure":0,"pressure_unit":"PSI","ph":0,"bpm":0,"comment":"","beer":"","battery":${battery}}')
  },
  {
    label: 'UBIDots',
    value: encodeURIComponent('{"temperature":${temp},"gravity":${gravity},"angle":${angle},"battery":${battery},"rssi":${rssi}}')
  }
])

export const gravityHttpGetFormatOptions = ref([
  {
    label: 'GravityMon',
    value: encodeURIComponent('?name=${mdns}&id=${id}&token=${token2}&interval=${sleep-interval}&temperature=${temp}&temp-units=${temp-unit}&gravity=${gravity}&angle=${angle}&battery=${battery}&rssi=${rssi}&corr-gravity=${corr-gravity}&gravity-unit=${gravity-unit}&run-time=${run-time}')
  },
  {
    label: 'GravityMon v2',
    value: encodeURIComponent('?name=${mdns}&id=${id}&token=${token2}&interval=${sleep-interval}&temperature=${temp}&temp-units=${temp-unit}&gravity=${gravity}&velocity=${velocity}&angle=${angle}&battery=${battery}&rssi=${rssi}&corr-gravity=${corr-gravity}&gravity-unit=${gravity-unit}&run-time=${run-time}')
  }
])

export const gravityInfluxdb2FormatOptions = ref([
  {
    label: 'GravityMon',
    value: encodeURIComponent('measurement,host=${mdns},device=${id},temp-format=${temp-unit},gravity-format=${gravity-unit} gravity=${gravity},corr-gravity=${corr-gravity},angle=${angle},temp=${temp},battery=${battery},rssi=${rssi}\n')
  },
  {
    label: 'GravityMon v2',
    value: encodeURIComponent('measurement,host=${mdns},device=${id},temp-format=${temp-unit},gravity-format=${gravity-unit} gravity=${gravity},velocity=${velocity},corr-gravity=${corr-gravity},angle=${angle},temp=${temp},battery=${battery},rssi=${rssi}\n')
  }
])

export const gravityMqttFormatOptions = ref([
  {
    label: 'Gravitymon / iSpindel',
    value: encodeURIComponent('ispindel/${mdns}/tilt:${angle}|ispindel/${mdns}/temperature:${temp}|ispindel/${mdns}/temp_units:${temp-unit}|ispindel/${mdns}/battery:${battery}|ispindel/${mdns}/gravity:${gravity}|ispindel/${mdns}/interval:${sleep-interval}|ispindel/${mdns}/RSSI:${rssi}|')
  },
  {
    label: 'Gravitymon v2 / iSpindel',
    value: encodeURIComponent('ispindel/${mdns}/tilt:${angle}|ispindel/${mdns}/temperature:${temp}|ispindel/${mdns}/temp_units:${temp-unit}|ispindel/${mdns}/battery:${battery}|ispindel/${mdns}/gravity:${gravity}|ispindel/${mdns}/velocity:${velocity}|ispindel/${mdns}/interval:${sleep-interval}|ispindel/${mdns}/RSSI:${rssi}|')
  },
  {
    label: 'HomeAssistant',
    value: encodeURIComponent('gravmon/${mdns}/temperature:${temp}|gravmon/${mdns}/gravity:${gravity}|gravmon/${mdns}/rssi:${rssi}|gravmon/${mdns}/tilt:${tilt}|gravmon/${mdns}/battery:${battery}|')
  },
  {
    label: 'HomeAssistant 2',
    value: encodeURIComponent('gravmon/${mdns}/temperature:${temp}|gravmon/${mdns}/gravity:${gravity}|gravmon/${mdns}/rssi:${rssi}|gravmon/${mdns}/tilt:${tilt}|gravmon/${mdns}/battery:${battery}|homeassistant/sensor/gravmon_${id}/temperature/config:{"dev":{"name":"${mdns}","mdl":"gravmon","sw":"${app-ver}","ids":"${id}"},"uniq_id":"${id}_temp","name":"temperature","dev_cla":"temperature","unit_of_meas":"°${temp-unit}","stat_t":"gravmon/${mdns}/temperature"}|homeassistant/sensor/gravmon_${id}/gravity/config:{"dev":{"name":"${mdns}","mdl":"gravmon","sw":"${app-ver}","ids":"${id}"},"uniq_id":"${id}_grav","name":"gravity","unit_of_meas":" ${gravity-unit}","stat_t":"gravmon/${mdns}/gravity"}|homeassistant/sensor/gravmon_${id}/rssi/config:{"dev":{"name":"${mdns}","mdl":"gravmon","sw":"${app-ver}","ids":"${id}"},"uniq_id":"${id}_rssi","name":"rssi","dev_cla":"signal_strength","unit_of_meas":"dBm","stat_t":"gravmon/${mdns}/rssi"}|homeassistant/sensor/gravmon_${id}/tilt/config:{"dev":{"name":"${mdns}","mdl":"gravmon","sw":"${app-ver}","ids":"${id}"},"uniq_id":"${id}_tilt","name":"tilt","stat_t":"gravmon/${mdns}/tilt"}|homeassistant/sensor/gravmon_${id}/battery/config:{"dev":{"name":"${mdns}","mdl":"gravmon","sw":"${app-ver}","ids":"${id}"},"uniq_id":"${id}_batt","name":"battery","dev_cla":"voltage","unit_of_meas":"V","stat_t":"gravmon/${mdns}/battery"}|')
  },
  {
    label: 'Brewblox',
    value: encodeURIComponent('brewcast/history:{"key":"${mdns}","data":{"Temperature[degC]": ${temp-c},"Temperature[degF]": ${temp-f},"Battery[V]":${battery},"Tilt[deg]":${angle},"Rssi[dBm]":${rssi},"SG":${gravity-sg},"Plato":${gravity-plato}}}|')
  }
])
