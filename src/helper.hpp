/*
MIT License

Copyright (c) 2021-2023 Magnus

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
#ifndef SRC_HELPER_HPP_
#define SRC_HELPER_HPP_

#include <battery.hpp>
#include <history.hpp>
#include <led.hpp>
#include <main.hpp>
#include <perf.hpp>
#include <serialdbg.hpp>

#define ERR_FILENAME "/error.log"
#define ERR_FILENAME2 "/error2.log"
#define ERR_FILEMAXSIZE 4000

#define RUNTIME_FILENAME "/runtime.log"

// tcp cleanup
void tcp_cleanup();

// forced reset as alternative to reset()
void forcedReset();

// Error logging
void writeErrorLog(const char* format, ...);
void checkResetReason();

// Sleep mode
void deepSleep(int t);

// Force config mode
bool checkPinConnected();

// Show build options
void printBuildOptions();
void detectChipRevision();
void runGpioHardwareTests();

// Data conversion
double convertToPlato(double sg);
double convertToSG(double plato);
float convertCtoF(float c);
float convertFtoC(float f);

// url encode/decode
String urldecode(String str);
String urlencode(String str);

// Float to String
char* convertFloatToString(float f, char* buf, int dec = 2);
float reduceFloatPrecision(float f, int dec = 2);

// Logging via serial
void printHeap(String prefix = "HELP");

#endif  // SRC_HELPER_HPP_

// EOF
