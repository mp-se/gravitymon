/*
MIT License

Copyright (c) 2021-2025 Magnus

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
#if defined(GRAVITYMON)

#include <tinyexpr.h>

#include <calc.hpp>
#include <config_gravitymon.hpp>
#include <cstdio>
#include <log.hpp>
#include <utils.hpp>

double calculateGravity(const char *formula, double angle, double temp,
                        const char *tempFormula) {
  if (tempFormula != 0) {
#if LOG_LEVEL == 6
    Log.verbose(F("CALC: Using temporary formula." CR));
#endif
    formula = tempFormula;
  }

#if LOG_LEVEL == 6
  Log.verbose(F("CALC: Calculating gravity for angle %F, temp %F." CR), angle,
              temp);
  Log.verbose(F("CALC: Formula %s." CR), formula);
#endif

  if (strlen(formula) == 0) return 0.0;

  // Store variable names and pointers.
  te_variable vars[] = {{"tilt", &angle}, {"temp", &temp}};

  int err;
  // Compile the expression with variables.
  te_expr *expr = te_compile(formula, vars, 2, &err);

  if (expr) {
    double g = te_eval(expr);
    te_free(expr);

#if LOG_LEVEL == 6
    char s[20];
    snprintf(&s[0], sizeof(s), "%.8f", g);
    Log.verbose(F("CALC: Calculated gravity is %s." CR), &s[0]);
#endif
    return g;
  }

  writeErrorLog("CALC: Failed to parse gravity expression %d", err);
  return 0;
}

// Do a standard gravity temperature correction. This is a simple way to adjust
// for differnt worth temperatures. This function uses C as temperature.
//
// Source: https://homebrewacademy.com/hydrometer-temperature-correction/
double gravityTemperatureCorrectionC(double gravitySG, double tempC,
                                     double calTempC) {
#if LOG_LEVEL == 6
  Log.verbose(F("CALC: Adjusting gravity based on temperature, gravity %F, "
                "temp %F, calTemp %F." CR),
              gravitySG, tempC, calTempC);
#endif
  double tempF = convertCtoF(tempC);
  double calTempF = convertCtoF(calTempC);
  const char *formula =
      "gravity*((1.00130346-0.000134722124*temp+0.00000204052596*temp^2-0."
      "00000000232820948*temp^3)/"
      "(1.00130346-0.000134722124*cal+0.00000204052596*cal^2-0."
      "00000000232820948*cal^3))";

  // Store variable names and pointers.
  te_variable vars[] = {
      {"gravity", &gravitySG}, {"temp", &tempF}, {"cal", &calTempF}};

  int err;
  // Compile the expression with variables.
  te_expr *expr = te_compile(formula, vars, 3, &err);

  if (expr) {
    double g = te_eval(expr);
    te_free(expr);

#if LOG_LEVEL == 6
    char s[80];
    snprintf(&s[0], sizeof(s), "Corrected gravity=%.8f, input gravity=%.8f", g,
             gravitySG);
    Log.verbose(F("CALC: %s." CR), &s[0]);
#endif
    return g;
  }

  writeErrorLog(
      "CALC: Failed to parse expression for gravity temperature correction %d",
      err);
  return gravitySG;
}

#endif  // GRAVITYMON

// EOF
