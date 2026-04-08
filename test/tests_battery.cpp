/*
  Copyright (c) 2021-2026 Magnus
  https://github.com/mp-se/gravitymon

  Licensed under the GNU General Public License v3.0 (GPL v3) for open source use,
  or a Commercial License for proprietary use. See LICENSE and LICENSE_COMMERCIAL.
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
