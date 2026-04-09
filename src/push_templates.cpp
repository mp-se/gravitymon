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


#include <Arduino.h>

#include <pushtarget.hpp>

const char iPressureHttpPostFormat[] PROGMEM =
    "{"
    "\"name\": \"${mdns}\", "
    "\"id\": \"${id}\", "
    "\"token\": \"${token}\", "
    "\"interval\": ${sleep-interval}, "
    "\"temperature\": ${temp}, "
    // "\"temperature1\": ${temp1}, "
    "\"temperature-unit\": \"${temp-unit}\", "
    "\"pressure\": ${pressure}, "
    // "\"pressure1\": ${pressure1}, "
    "\"pressure-unit\": \"${pressure-unit}\", "
    "\"battery\": ${battery}, "
    "\"rssi\": ${rssi}, "
    "\"run-time\": ${run-time} "
    "}";

const char iPressureHttpGetFormat[] PROGMEM =
    "?name=${mdns}"
    "&id=${id}"
    "&token=${token2}"
    "&interval=${sleep-interval}"
    "&temperature=${temp}"
    // "&temperature1=${temp1}"
    "&temperature-unit=${temp-unit}"
    "&pressure=${pressure}"
    // "&pressure1=${pressure1}"
    "&pressure-unit=${pressure-unit}"
    "&battery=${battery}"
    "&rssi=${rssi}"
    "&run-time=${run-time}";

const char iPressureInfluxDbFormat[] PROGMEM =
    "measurement,host=${mdns},"
    "device=${id},"
    "temperature-unit=${temp-unit},"
    "pressure-unit=${pressure-unit} "
    "pressure=${pressure},"
    // "pressure1=${pressure1},"
    "temp=${temp},"
    // "temp1=${temp1},"
    "battery=${battery},"
    "rssi=${rssi}\n";

const char iPressureMqttFormat[] PROGMEM =
    "pressuremon/${mdns}/temperature:${temp}|"
    // "pressuremon/${mdns}/temperature1:${temp1}|"
    "pressuremon/${mdns}/temperature-unit:${temp-unit}|"
    "pressuremon/${mdns}/battery:${battery}|"
    "pressuremon/${mdns}/pressure:${pressure}|"
    // "pressuremon/${mdns}/pressure1:${pressure1}|"
    "pressuremon/${mdns}/pressure-unit:${pressure-unit}|"
    "pressuremon/${mdns}/interval:${sleep-interval}|"
    "pressuremon/${mdns}/RSSI:${rssi}|";

// Use iSpindle format for compatibility, HTTP POST
const char iGravityHttpPostFormat[] PROGMEM =
    "{"
    "\"name\": \"${mdns}\", "
    "\"ID\": \"${id}\", "
    "\"token\": \"${token}\", "
    "\"interval\": ${sleep-interval}, "
    "\"temperature\": ${temp}, "
    "\"temp_units\": \"${temp-unit}\", "
    "\"gravity\": ${gravity}, "
    "\"velocity\": ${velocity}, "
    "\"angle\": ${angle}, "
    "\"battery\": ${battery}, "
    "\"RSSI\": ${rssi}, "
    "\"corr-gravity\": ${corr-gravity}, "
    "\"gravity-unit\": \"${gravity-unit}\", "
    "\"run-time\": ${run-time} "
    "}";

// Format for an HTTP GET
const char iGravityHttpGetFormat[] PROGMEM =
    "?name=${mdns}"
    "&id=${id}"
    "&token=${token2}"
    "&interval=${sleep-interval}"
    "&temperature=${temp}"
    "&temp-units=${temp-unit}"
    "&gravity=${gravity}"
    "&velocity=${velocity}"
    "&angle=${angle}"
    "&battery=${battery}"
    "&rssi=${rssi}"
    "&corr-gravity=${corr-gravity}"
    "&gravity-unit=${gravity-unit}"
    "&run-time=${run-time}";

const char iGravityInfluxDbFormat[] PROGMEM =
    "measurement,host=${mdns},device=${id},temp-format=${temp-unit},gravity-"
    "format=${gravity-unit} "
    "gravity=${gravity},velocity=${velocity},corr-gravity=${corr-gravity},"
    "angle=${angle},temp=${"
    "temp},battery=${battery},"
    "rssi=${rssi}\n";

const char iGravityMqttFormat[] PROGMEM =
    "ispindel/${mdns}/tilt:${angle}|"
    "ispindel/${mdns}/temperature:${temp}|"
    "ispindel/${mdns}/temp_units:${temp-unit}|"
    "ispindel/${mdns}/battery:${battery}|"
    "ispindel/${mdns}/gravity:${gravity}|"
    "ispindel/${mdns}/velocity:${velocity}|"
    "ispindel/${mdns}/interval:${sleep-interval}|"
    "ispindel/${mdns}/RSSI:${rssi}|";

// EOF
