/*
 * GravityMon
 * Copyright (c) 2021-2026 Magnus
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Alternatively, this software may be used under the terms of a
 * commercial license. See LICENSE_COMMERCIAL for details.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#ifndef SRC_HWTEST_HPP_
#define SRC_HWTEST_HPP_

#if defined(GRAVITYMON) && defined(RUN_HARDWARE_TEST)

// Hardware test used to validate hardware builds (build with RUN_HARDWARE_TEST).
// Runs without wifi, prints a quality report on the serial console and then
// repeats the report until the device is reset. This function never returns.
//
// Optional build flag: HWTEST_CLASSIFY_SENSOR=1 also identifies the DS18B20
// clone family. That uses undocumented commands which can change the
// calibration of clone sensors, so it should not be used on customer sensors.
void runHardwareTest();

#endif  // GRAVITYMON && RUN_HARDWARE_TEST

#endif  // SRC_HWTEST_HPP_

// EOF
