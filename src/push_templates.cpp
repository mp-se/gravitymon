/*
MIT License

Copyright (c) 2025-2026 Magnus

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
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
