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
