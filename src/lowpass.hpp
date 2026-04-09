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


#ifndef SRC_LOWPASS_HPP_
#define SRC_LOWPASS_HPP_

#include <Arduino.h>

constexpr auto FILTER_BUFFER_SIZE = 5;

struct FilterData {
  // Data stored in RTC memory should be aligned to 4 bytes
  float buffer[FILTER_BUFFER_SIZE];
  int32_t bufferCount;
};

class FilterBase {
 protected:
  FilterData* _data;

  void addValue(float newValue) {
    // Rotate the buffer if needed
    if (_data->bufferCount >= FILTER_BUFFER_SIZE) {
      memmove(&_data->buffer[0], &_data->buffer[1],
              sizeof(float) * (FILTER_BUFFER_SIZE - 1));
      _data->bufferCount--;
    }

    _data->buffer[_data->bufferCount] = newValue;
    _data->bufferCount++;
  }

  float getValueSum() const {
    float sum = 0.0;

    for (int i = 0; i < getValueCount(); i++) {
      sum += _data->buffer[i];
    }

    return sum;
  }
  float getValueCount() const { return _data->bufferCount; }

 public:
  explicit FilterBase(FilterData* data) : _data(data) {}
  virtual ~FilterBase() = default;
  virtual float filter(float newValue) = 0;
};

class MovingAverageFilter : public FilterBase {
 public:
  explicit MovingAverageFilter(FilterData* data) : FilterBase(data) {}

  float filter(float newValue) {
    addValue(newValue);
    return getValueSum() / getValueCount();
  }
};

class TrimmedMovingAverageFilter : public FilterBase {
 private:
  float getValueMax() const {
    int count = getValueCount();
    if (count == 0) return 0.0;  // Handle empty buffer

    float maxVal = _data->buffer[0];
    for (int i = 1; i < count;
         i++) {  // Start from 1 since we already have buffer[0]
      if (_data->buffer[i] > maxVal) {
        maxVal = _data->buffer[i];
      }
    }

    return maxVal;
  }

  float getValueMin() const {
    int count = getValueCount();
    if (count == 0) return 0.0;  // Handle empty buffer

    float minVal = _data->buffer[0];
    for (int i = 1; i < count;
         i++) {  // Start from 1 since we already have buffer[0]
      if (_data->buffer[i] < minVal) {
        minVal = _data->buffer[i];
      }
    }

    return minVal;
  }

 public:
  explicit TrimmedMovingAverageFilter(FilterData* data) : FilterBase(data) {}

  float filter(float newValue) {
    addValue(newValue);

    // Ensure we have enough samples to exclude lowest min/max
    if (getValueCount() < 5) {
      return getValueSum() / getValueCount();
    }

    // Calculate the trimmed average (excluding min and max)
    float minVal = getValueMin();
    float maxVal = getValueMax();
    bool minExcluded = false;
    bool maxExcluded = false;
    float sum = 0.0;

    for (int i = 0; i < getValueCount(); i++) {
      if (!minExcluded && _data->buffer[i] == minVal) {
        minExcluded = true;
        continue;
      }
      if (!maxExcluded && _data->buffer[i] == maxVal) {
        maxExcluded = true;
        continue;
      }

      sum += _data->buffer[i];
    }

    // Exclude min and max from the count
    return sum / (getValueCount() - 2);
  }
};

#endif  // SRC_LOWPASS_HPP_

// EOF
