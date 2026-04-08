/*
  Copyright (c) 2021-2026 Magnus
  https://github.com/mp-se/gravitymon

  Licensed under the GNU General Public License v3.0 (GPL v3) for open source use,
  or a Commercial License for proprietary use. See LICENSE and LICENSE_COMMERCIAL.
*/


#include <AUnit.h>

#include <calc.hpp>
#include <utils.hpp>
#include <helper.hpp>

test(calc_calculateGravity1) {
  const char* formula = "0.00000909*tilt^2+0.00124545*tilt+0.96445455";
  double g = calculateGravity(formula, 30, 20);
  float v1 = reduceFloatPrecision(g, 2);
  float v2 = 1.01;
  assertEqual(v1, v2);
}

test(calc_calculateGravity2) {
  const char* formula = "0.00000909*tilt2^2+0.00124545*tilt+0.96445455";
  double g = calculateGravity(formula, 30, 20);
  double g2 = 0.0;
  assertEqual(g, g2);
}

test(calc_gravityTemperatureCorrectionC) {
  double g = gravityTemperatureCorrectionC(1.02, 45.0, 20.0);
  float v1 = reduceFloatPrecision(g, 2);
  float v2 = 1.03;
  assertEqual(v1, v2);
}

// EOF
