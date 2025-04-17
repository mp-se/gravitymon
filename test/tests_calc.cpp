/*
MIT License

Copyright (c) 2022-2024 Magnus

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
#include <AUnit.h>

#if defined(GRAVITYMON)

#include <calc.hpp>
#include <helper.hpp>

test(calc_calculateGravity1) {
  const char* formula = "0.00000909*tilt^2+0.00124545*tilt+0.96445455";
  double g = calculateGravity(30, 20, formula);
  float v1 = reduceFloatPrecision(g, 2);
  float v2 = 1.01;
  assertEqual(v1, v2);
}

test(calc_calculateGravity2) {
  const char* formula = "0.00000909*tilt2^2+0.00124545*tilt+0.96445455";
  double g = calculateGravity(30, 20, formula);
  double g2 = 0.0;
  assertEqual(g, g2);
}

test(calc_gravityTemperatureCorrectionC) {
  double g = gravityTemperatureCorrectionC(1.02, 45.0, 20.0);
  float v1 = reduceFloatPrecision(g, 2);
  float v2 = 1.03;
  assertEqual(v1, v2);
}

#endif // GRAVITYMON

// EOF
