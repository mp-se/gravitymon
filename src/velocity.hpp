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


#ifndef SRC_VELOCITY_HPP_
#define SRC_VELOCITY_HPP_

#include <Arduino.h>

#include <lowpass.hpp>
#include <memory>

constexpr auto VELOCITY_PERIODS = 4;
constexpr auto VELOCITY_PERIOD_TIME = 4;  // Each period is a 4 hour window
constexpr auto VELOCITY_MIN_HOURS = 1;

// Data stored in RTC memory should be aligned to 4 bytes
struct __attribute__((packed, aligned(4))) GravityVelocityPeriodData {
  float total;
  float min;
  float max;
  int16_t count;
  int16_t _padding;  // Explicit padding for 4-byte alignment
};

struct GravityVelocityData {
  FilterData filter;
  GravityVelocityPeriodData period[VELOCITY_PERIODS];
};

class GravityVelocity {
 protected:
  GravityVelocityData* _data;
  std::unique_ptr<FilterBase> _filter;
  int16_t _samplesPerPeriod;

  int16_t getNoValues() const {
    int16_t dataPoints = 0;

    for (int i = 0; i < VELOCITY_PERIODS; i++)
      dataPoints += _data->period[i].count;

    return dataPoints;
  }

  float getFirstMax() const {
    for (int i = 0; i < VELOCITY_PERIODS; i++)
      if (_data->period[i].count > 0) return _data->period[i].max;

    return 0.0;
  }

  float lastFirstMin() const {
    for (int i = 1; i < VELOCITY_PERIODS; i++)
      if (_data->period[i].count == 0) return _data->period[i - 1].min;

    return _data->period[VELOCITY_PERIODS - 1].min;
  }

  float getFirstAve() const {
    for (int i = 0; i < VELOCITY_PERIODS; i++)
      if (_data->period[i].count > 0)
        return _data->period[i].total / _data->period[i].count;

    return 0.0;
  }

  float lastFirstAve() const {
    for (int i = 1; i < VELOCITY_PERIODS; i++)
      if (_data->period[i].count == 0)
        return _data->period[i - 1].total / _data->period[i - 1].count;

    return _data->period[VELOCITY_PERIODS - 1].total /
           _data->period[VELOCITY_PERIODS - 1].count;
  }

 public:
  explicit GravityVelocity(GravityVelocityData* data, int sleepInterval)
      : _data(data) {
    _samplesPerPeriod = sleepInterval >= (3600 * VELOCITY_PERIOD_TIME)
                            ? 1
                            : (3600 * VELOCITY_PERIOD_TIME) / sleepInterval;
    _filter.reset(new MovingAverageFilter(&data->filter));
  }

  float addValue(float value) {
    int dataPoints = getNoValues();

    // Check if there is room for more values
    if (dataPoints >= _samplesPerPeriod * VELOCITY_PERIODS) {
      memmove(&_data->period[0], &_data->period[1],
              sizeof(GravityVelocityPeriodData) * (VELOCITY_PERIODS - 1));
      _data->period[VELOCITY_PERIODS - 1] = {0};
    }

    // Add value
    float fValue = _filter->filter(value);

    for (int i = 0; i < VELOCITY_PERIODS; i++) {
      if (_data->period[i].count < _samplesPerPeriod) {
        _data->period[i].total += fValue;
        _data->period[i].count++;

        if (value < _data->period[i].min || _data->period[i].count == 1)
          _data->period[i].min = value;
        if (value > _data->period[i].max || _data->period[i].count == 1)
          _data->period[i].max = value;

        break;
      }
    }

    return fValue;
  }

  float getVelocity() const {
    int dataPoints = getNoValues();

    if (!dataPoints)  // No samples to calculate velocity
      return 0.0;

    int hours = (dataPoints * VELOCITY_PERIOD_TIME) / _samplesPerPeriod;

    if (hours <=
        VELOCITY_MIN_HOURS)  // Not enough samples to calculate velocity
      return 0.0;

    float first = getFirstAve(), last = lastFirstAve();
    float velocity = ((last - first) / hours) * 24;
    return velocity * 1000;
  }

  bool isVelocityValid() const {
    int dataPoints = getNoValues();

    if (!dataPoints)  // No samples to calculate velocity
      return false;

    int hours = (dataPoints * VELOCITY_PERIOD_TIME) / _samplesPerPeriod;

    if (hours <=
        VELOCITY_MIN_HOURS)  // Not enough samples to calculate velocity
      return false;

    return true;
  }

  void dump() const {
#if LOG_LEVEL == 6
    for (int i = 0; i < VELOCITY_PERIODS; i++) {
      float average = (_data->period[i].count > 0)
                          ? (_data->period[i].total / _data->period[i].count)
                          : 0.0f;
      Log.verbose(F("VEL : Section %d (%dh): Min: %.4f Max: %.4f, Ave: %.4f "
                    "Total: %.4f # %d (%d)" CR),
                  i, VELOCITY_PERIOD_TIME, _data->period[i].min,
                  _data->period[i].max, average, _data->period[i].total,
                  _data->period[i].count, _samplesPerPeriod);
    }
#endif
  }
};

#endif  // SRC_VELOCITY_HPP_

// EOF
