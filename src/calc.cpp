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
#include <curveFitting.h>
#include <tinyexpr.h>

#include <calc.hpp>
#include <main.hpp>

//
// Use values to derive a formula
//
int createFormula(RawFormulaData &fd, char *formulaBuffer,
                  int formulaBufferSize, int order) {
  int noAngles = 0;
  RawFormulaData fd2;

  // Check how many valid values we have got and make sure we have a full series.
  for (int i = 0; i < FORMULA_DATA_SIZE; i++) {
    if (fd.a[i]) {
      fd2.a[noAngles] = fd.a[i];
      fd2.g[noAngles] = fd.g[i];
      noAngles++;
    }
  }

#if LOG_LEVEL == 6 && !defined(CALC_DISABLE_LOGGING)
  Log.verbose(
      F("CALC: Trying to create formula using order = %d, found %d angles" CR),
      order, noAngles);
#endif

  if (noAngles <3) {
    ErrorFileLog errLog;
    errLog.addEntry(F("CALC: Not enough values for deriving formula"));
    return ERR_FORMULA_NOTENOUGHVALUES;
  } else {
    double coeffs[order + 1];
    int ret = fitCurve(order, noAngles, fd2.a, fd2.g,
                       sizeof(coeffs) / sizeof(double), coeffs);

    // Returned value is 0 if no error
    if (ret == 0) {
#if LOG_LEVEL == 6 && !defined(CALC_DISABLE_LOGGING)
      Log.verbose(F("CALC: Finshied processing data points, order = %d." CR), order);
#endif

      // Print the formula based on 'order'
      if (order == 4) {
        snprintf(formulaBuffer, formulaBufferSize,
                 "%.8f*tilt^4+%.8f*tilt^3+%.8f*tilt^2+%.8f*tilt+%.8f",
                 coeffs[0], coeffs[1], coeffs[2], coeffs[3], coeffs[4]);
      } else if (order == 3) {
        snprintf(formulaBuffer, formulaBufferSize,
                 "%.8f*tilt^3+%.8f*tilt^2+%.8f*tilt+%.8f", coeffs[0], coeffs[1],
                 coeffs[2], coeffs[3]);
      } else if (order == 2) {
        snprintf(formulaBuffer, formulaBufferSize, "%.8f*tilt^2+%.8f*tilt+%.8f",
                 coeffs[0], coeffs[1], coeffs[2]);
      } else {  // order == 1
        snprintf(formulaBuffer, formulaBufferSize, "%.8f*tilt+%.8f", coeffs[0],
                 coeffs[1]);
      }

#if LOG_LEVEL == 6 && !defined(CALC_DISABLE_LOGGING)
      Log.verbose(F("CALC: Formula: %s" CR), formulaBuffer);
#endif

      bool valid = true;

      for (int i = 0; i < 5; i++) {
        if (fd.a[i] == 0 && valid) break;

        double g = calculateGravity(fd.a[i], 0, formulaBuffer);
        double dev = (g - fd.g[i]) < 0 ? (fd.g[i] - g) : (g - fd.g[i]);

        // If the deviation is more than 2 degress we mark it as failed.
        if (dev * 1000 > myAdvancedConfig.getMaxFormulaCreationDeviation()) {
          char s[20];
          snprintf(&s[0], sizeof(s), "%.8f", dev);
          Log.error(F("CALC: Deviation to large: %s" CR), &s[0]);
          valid = false;
        }
      }

      if (!valid) {
        ErrorFileLog errLog;
        errLog.addEntry(
            F("CALC: Error validating created formula. Deviation to large, "
              "formula rejected."));
        return ERR_FORMULA_UNABLETOFFIND;
      }

      Log.info(F("CALC: Found formula '%s'." CR), formulaBuffer);
      return 0;
    }
  }

  ErrorFileLog errLog;
  errLog.addEntry(F("CALC: Internal error finding formula."));
  return ERR_FORMULA_INTERNAL;
}

//
// Calculates gravity according to supplied formula, compatible with
// iSpindle/Fermentrack formula
//
double calculateGravity(double angle, double temp, const char *tempFormula) {
  const char *formula = myConfig.getGravityFormula();

  if (tempFormula != 0) {
#if LOG_LEVEL == 6 && !defined(CALC_DISABLE_LOGGING)
    Log.verbose(F("CALC: Using temporary formula." CR));
#endif
    formula = tempFormula;
  }

#if LOG_LEVEL == 6 && !defined(CALC_DISABLE_LOGGING)
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

#if LOG_LEVEL == 6 && !defined(CALC_DISABLE_LOGGING)
    char s[20];
    snprintf(&s[0], sizeof(s), "%.8f", g);
    Log.verbose(F("CALC: Calculated gravity is %s." CR), &s[0]);
#endif
    return g;
  }

  ErrorFileLog errLog;
  errLog.addEntry("CALC: Failed to parse gravity expression " + String(err));
  return 0;
}

//
// Do a standard gravity temperature correction. This is a simple way to adjust
// for differnt worth temperatures. This function uses C as temperature.
//
// Source: https://homebrewacademy.com/hydrometer-temperature-correction/
//
double gravityTemperatureCorrectionC(double gravity, double tempC,
                                     double calTempC) {
#if LOG_LEVEL == 6 && !defined(CALC_DISABLE_LOGGING)
  Log.verbose(F("CALC: Adjusting gravity based on temperature, gravity %F, "
                "temp %F, calTemp %F." CR),
              gravity, tempC, calTempC);
#endif
  // float tempF = convertCtoF(tempC);
  // float calTempF = convertCtoF(calTempC);

  const char *formula =
      "gravity*((1.00130346-0.000134722124*temp+0.00000204052596*temp^2-0."
      "00000000232820948*temp^3)/"
      "(1.00130346-0.000134722124*cal+0.00000204052596*cal^2-0."
      "00000000232820948*cal^3))";

  // Store variable names and pointers.
  te_variable vars[] = {
      {"gravity", &gravity}, {"temp", &tempC}, {"cal", &calTempC}};

  int err;
  // Compile the expression with variables.
  te_expr *expr = te_compile(formula, vars, 3, &err);

  if (expr) {
    double g = te_eval(expr);
    te_free(expr);

#if LOG_LEVEL == 6 && !defined(CALC_DISABLE_LOGGING)
    char s[10];
    snprintf(&s[0], sizeof(s), "%.8f", g);
    Log.verbose(F("CALC: Corrected gravity is %s." CR), &s[0]);
#endif
    return g;
  }

  ErrorFileLog errLog;
  errLog.addEntry(
      "CALC: Failed to parse expression for gravity temperature correction " +
      String(err));
  return gravity;
}

// EOF
