/*
  Copyright (c) 2021-2026 Magnus
  https://github.com/mp-se/gravitymon

  Licensed under the GNU General Public License v3.0 (GPL v3) for open source use,
  or a Commercial License for proprietary use. See LICENSE and LICENSE_COMMERCIAL.
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
