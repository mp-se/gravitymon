/*
  Copyright (c) 2021-2026 Magnus
  https://github.com/mp-se/gravitymon

  Licensed under the GNU General Public License v3.0 (GPL v3) for open source use,
  or a Commercial License for proprietary use. See LICENSE and LICENSE_COMMERCIAL.
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
