/*
MIT License

Copyright (c) 2021-2024 Magnus

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
#ifndef SRC_UTILS_HPP_
#define SRC_UTILS_HPP_

float convertCtoF(float c);
float convertFtoC(float f);
float convertKGtoLBS(float kg);
float convertLBStoKG(float lbs);
float convertCLtoUSOZ(float cl);
float convertCLtoUKOZ(float cl);
float convertUSOZtoCL(float usoz);
float convertUKOZtoCL(float ukoz);
double convertToPlato(double sg);
double convertToSG(double plato);

float reduceFloatPrecision(float f, int dec);
char* convertFloatToString(float f, char* buf, int dec = 2);

void tcp_cleanup();
void deepSleep(int t);

void printHeap(String prefix);

void forcedReset();

void checkResetReason();

String urldecode(String str);
String urlencode(String str);

#endif  // SRC_UTILS_HPP_

// EOF
