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

#include <calc.hpp>
#include <helper.hpp>

// TODO: Add more test cases to explore formula creation error conditions when
// values are out of bounds
// TODO: Add more test cases to check order 3 + 4 formula creation as well.

test(calc_createFormula1) {
  char buffer[100];
  RawFormulaData fd = {
      {0.0, 25.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
      {0.0, 1.0, 1.02, 1.033, 1.00011, 1.0, 1.0, 1.0, 1.0, 1.0}};
  int i = createFormula(fd, &buffer[0], sizeof(buffer), 2);
  assertEqual(i, ERR_FORMULA_NOTENOUGHVALUES);
}

test(calc_createFormula2) {
  char buffer[100];
  RawFormulaData fd = {
      {25.0, 30.0, 35.0, 40.0, 45.0, 50.0, 55.0, 60.0, 65.0, 70.0},
      {1.0, 1.01, 1.02, 1.03, 1.04, 1.05, 1.06, 1.07, 1.08, 1.1}};
  int i = createFormula(fd, &buffer[0], sizeof(buffer), 2);
  assertEqual(i, 0);
  assertEqual(&buffer[0], "0.00000909*tilt^2+0.00124545*tilt+0.96445455");
}

test(calc_createFormula3) {
  char buffer[100];
  RawFormulaData fd = {
      {25.0, 30.0, 35.0, 45.0, 55.0, 0.0, 0.0, 0.0, 0.0, 0.0},
      {1.0, 1.1, 1.2, 1.3, 1.4, 0.0, 0.0, 0.0, 0.0, 0.0}};
  float f = myConfig.getMaxFormulaCreationDeviation(); 
  myConfig.setMaxFormulaCreationDeviation(10); // Change threashold to pass
  int i = createFormula(fd, &buffer[0], sizeof(buffer), 3);
  myConfig.setMaxFormulaCreationDeviation(f); // Restore to default for next test
  assertEqual(i, 0);
  assertEqual(&buffer[0], "0.00001140*tilt^3+-0.00161278*tilt^2+0.08512845*tilt+-0.30122180");
}

test(calc_createFormula4) {
  char buffer[100];
  RawFormulaData fd = {
      {25.0, 30.0, 35.0, 45.0, 55.0, 0.0, 0.0, 0.0, 0.0, 0.0},
      {1.0, 1.1, 1.2, 1.3, 1.4, 1.0, 1.0, 1.0, 1.0, 1.0}};
  int i = createFormula(fd, &buffer[0], sizeof(buffer), 4);
  assertEqual(i, 0);
  assertEqual(&buffer[0], "0.00000200*tilt^4+-0.00030333*tilt^3+0.01645000*tilt^2+-0.36241667*tilt+3.73750001");
}

test(calc_createFormula5) {
  char buffer[100];
  RawFormulaData fd = {
      {25.0, 30.0, 35.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
      {1.0, 1.1, 1.2, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}};
  int i = createFormula(fd, &buffer[0], sizeof(buffer), 1);
  assertEqual(i, 0);
  assertEqual(&buffer[0], "0.02000000*tilt+0.50000000");
}

test(calc_createFormula6) {
  char buffer[100];
  RawFormulaData fd = {
      {25.0, 30.0, 35.0, 45.0, 55.0, 0.0, 0.0, 0.0, 0.0, 0.0},
      {1.0, 1.1, 1.2, 1.3, 1.4, 0.0, 0.0, 0.0, 0.0, 0.0}};
  int i = createFormula(fd, &buffer[0], sizeof(buffer), 3);
  assertEqual(i, -3);
}

/*test(calc_createFormula7) { // TODO: Find data that will cause INTERNAL_ERROR in the fitCurve library...
  char buffer[100];
  RawFormulaData fd = {
      {25.0, 30.0, 35.0, 45.0, 55.0, 0.0, 0.0, 0.0, 0.0, 0.0},
      {1.0, 1.1, 1.2, 1.3, 1.4, 0.0, 0.0, 0.0, 0.0, 0.0}};
  int i = createFormula(fd, &buffer[0], sizeof(buffer), 3);
  assertEqual(i, -1);
}*/

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

// EOF
