/*
MIT License

Copyright (c) 2025 Magnus

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

#if defined(PRESSUREMON)
#include <pressure.hpp>
#include <config.hpp>
#include <Wire.h>

test(pressure_connectSensor) {
  myConfig.setPressureSensorType(SensorXidibeiXDB401_I2C_KPa_200, 0);
  Wire.pins(PIN_SDA, PIN_SCL);
  Wire.begin();

  myPressureSensor[0].setup(0, &Wire);
  assertEqual(myPressureSensor[0].isActive(), true);
  assertEqual(myPressureSensor[0].read(), true);
}
#endif // PRESSUREMON

// EOF
