/*
  Copyright (c) 2021-2026 Magnus
  https://github.com/mp-se/gravitymon

  Licensed under the GNU General Public License v3.0 (GPL v3) for open source use,
  or a Commercial License for proprietary use. See LICENSE and LICENSE_COMMERCIAL.
*/


#ifndef SRC_HELPER_HPP_
#define SRC_HELPER_HPP_

#include <Arduino.h>

// Force config mode
bool checkPinConnected(int pin1, int pin2);

// Pin charging mode
bool checkPinCharging(int pin);

// Show build options
void printBuildOptions();
void detectChipRevision();

float convertPsiPressureToBar(float psi);
float convertPsiPressureToKPa(float psi);
float convertPaPressureToPsi(float pa);
float convertPaPressureToBar(float pa);

#endif  // SRC_HELPER_HPP_

// EOF
