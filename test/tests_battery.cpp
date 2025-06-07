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
#include <Arduino.h>
#include <battery.hpp>

test(batt_percentage) {
    assertEqual(getBatteryPercentage(4.2, BatteryType::LithiumIon), 100.0);
    assertEqual(getBatteryPercentage(4.1, BatteryType::LithiumIon), 92.0);
    assertEqual(getBatteryPercentage(4.0, BatteryType::LithiumIon), 83.0);
    assertEqual(getBatteryPercentage(3.9, BatteryType::LithiumIon), 75.0);
    assertEqual(getBatteryPercentage(3.8, BatteryType::LithiumIon), 67.0);
    assertEqual(getBatteryPercentage(3.7, BatteryType::LithiumIon), 58.0);
    assertEqual(getBatteryPercentage(3.6, BatteryType::LithiumIon), 50.0);
    assertEqual(getBatteryPercentage(3.5, BatteryType::LithiumIon), 42.0);
    assertEqual(getBatteryPercentage(3.4, BatteryType::LithiumIon), 33.0);
    assertEqual(getBatteryPercentage(3.3, BatteryType::LithiumIon), 25.0);
    assertEqual(getBatteryPercentage(3.2, BatteryType::LithiumIon), 17.0);
    assertEqual(getBatteryPercentage(3.1, BatteryType::LithiumIon), 8.0);
    assertEqual(getBatteryPercentage(3.0, BatteryType::LithiumIon), 0.0);
}

test(batt_percentage2) {
    assertEqual(getBatteryPercentage(4.2, BatteryType::LiPo), 100.0);
    assertEqual(getBatteryPercentage(4.1, BatteryType::LiPo), 89.0);
    assertEqual(getBatteryPercentage(4.0, BatteryType::LiPo), 78.0);
    assertEqual(getBatteryPercentage(3.9, BatteryType::LiPo), 67.0);
    assertEqual(getBatteryPercentage(3.8, BatteryType::LiPo), 56.0);
    assertEqual(getBatteryPercentage(3.7, BatteryType::LiPo), 44.0);
    assertEqual(getBatteryPercentage(3.6, BatteryType::LiPo), 33.0);
    assertEqual(getBatteryPercentage(3.5, BatteryType::LiPo), 22.0);
    assertEqual(getBatteryPercentage(3.4, BatteryType::LiPo), 11.0);
    assertEqual(getBatteryPercentage(3.3, BatteryType::LiPo), 0.0);
}

// EOF
