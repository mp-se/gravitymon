/*
MIT License

Copyright (c) 2021 Magnus

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
#include "calc.h"
#include "helper.h"
#include "config.h"
#include "tinyexpr.h"
#include "tempsensor.h"

//#define LOG_LEVEL 5

//
// Calculates gravity according to supplied formula, compatible with iSpindle/Fermentrack formula  
//
double calculateGravity( double angle, double temp ) {
    const char* formula = myConfig.getGravityFormula();
#if LOG_LEVEL==6
    Log.verbose(F("CALC: Calculating gravity for angle %F, temp %F." CR), angle, temp);
    Log.verbose(F("CALC: Formula %s." CR), formula);
#endif

    if( strlen(formula) == 0 ) 
        return 0.0;

    // Store variable names and pointers.
    te_variable vars[] = {{"tilt", &angle}, {"temp", &temp}};

    int err;
    // Compile the expression with variables.
    te_expr *expr = te_compile(formula, vars, 2, &err);

    if (expr) {
        double g = te_eval(expr); 
        te_free(expr);

#if LOG_LEVEL==6
        Log.verbose(F("CALC: Calculated gravity is %F." CR), g);
#endif
        return g;
    } 

    Log.error(F("CALC: Failed to parse expression %d." CR), err);
    return 0;
}

//
// Do a standard gravity temperature correction. This is a simple way to adjust for differnt worth temperatures
//
double gravityTemperatureCorrection( double gravity, double temp, double calTemp) {
#if LOG_LEVEL==6
    Log.verbose(F("CALC: Adjusting gravity based on temperature, gravity %F, temp %F, calTemp %F." CR), gravity, temp, calTemp);
#endif

    double tempF = convertCtoF( temp );
    double calTempF = convertCtoF(calTemp); 
    const char* formula = "gravity*((1.00130346-0.000134722124*temp+0.00000204052596*temp^2-0.00000000232820948*temp^3)/(1.00130346-0.000134722124*cal+0.00000204052596*cal^2-0.00000000232820948*cal^3))";

    // Store variable names and pointers.
    te_variable vars[] = {{"gravity", &gravity}, {"temp", &tempF}, {"cal", &calTempF}};

    int err;
    // Compile the expression with variables.
    te_expr *expr = te_compile(formula, vars, 3, &err);

    if (expr) {
        double g = te_eval(expr); 
        te_free(expr);

#if LOG_LEVEL==6
        Log.verbose(F("CALC: Corrected gravity is %F." CR), g);
#endif
        return g;
    } 

    Log.error(F("CALC: Failed to parse expression %d, no correction has been made." CR), err);
    return gravity;
}

// EOF 