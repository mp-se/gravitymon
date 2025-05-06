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
      for (int i = 1; i < FILTER_BUFFER_SIZE; i++) {
        _data->buffer[i - 1] = _data->buffer[i];
      }
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
    float maxVal = _data->buffer[0];

    for (int i = 0; i < getValueCount(); i++) {
      if (_data->buffer[i] > maxVal) {
        maxVal = _data->buffer[i];
      }
    }

    return maxVal;
  }

  float getValueMin() const {
    float minVal = _data->buffer[0];

    for (int i = 0; i < getValueCount(); i++) {
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
