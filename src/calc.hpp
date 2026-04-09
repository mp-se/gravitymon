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


#ifndef SRC_CALC_HPP_
#define SRC_CALC_HPP_

#if defined(GRAVITYMON)

#include <Arduino.h>

double calculateGravity(const char* formula, double angle, double tempC,
                        const char* tempFormula = 0);
double gravityTemperatureCorrectionC(double gravity, double tempC,
                                     double calTempC);

#endif  // GRAVITYMON

#endif  // SRC_CALC_HPP_

// EOF
