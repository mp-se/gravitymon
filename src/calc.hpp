/*
MIT License

Copyright (c) 2021-22 Magnus

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
#ifndef SRC_CALC_HPP_
#define SRC_CALC_HPP_

#include <config.hpp>

#define ERR_FORMULA_NOTENOUGHVALUES -1
#define ERR_FORMULA_INTERNAL -2
#define ERR_FORMULA_UNABLETOFFIND -3

double calculateGravity(double angle, double tempC,
                        const char *tempFormula = 0);
double gravityTemperatureCorrectionC(double gravity, double tempC,
                                     double calTempC);
int createFormula(RawFormulaData &fd, char *formulaBuffer,
                  int formulaBufferSize, int order);

#endif  // SRC_CALC_HPP_

// EOF
