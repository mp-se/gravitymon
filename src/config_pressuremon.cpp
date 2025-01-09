/*
MIT License

Copyright (c) 2022-2025 Magnus

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
#if defined(PRESSUREMON)

#include <config_pressuremon.hpp>

PressuremonConfig::PressuremonConfig(String baseMDNS, String fileName)
    : BrewingConfig(baseMDNS, fileName) {}

void PressuremonConfig::createJson(JsonObject& doc) {
  BrewingConfig::createJson(doc);

  doc[CONFIG_BATTERY_SAVING] = this->isBatterySaving();
}

void PressuremonConfig::parseJson(JsonObject& doc) {
  BrewingConfig::parseJson(doc);

  if (!doc[CONFIG_BATTERY_SAVING].isNull())
    setBatterySaving(doc[CONFIG_BATTERY_SAVING].as<bool>());
}

#endif  // PRESSUREMON

// EOF
