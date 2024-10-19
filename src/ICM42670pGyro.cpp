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
#include <ICM42670pGyro.hpp>
#include <I2Cdev.h>
#include <log.hpp>
#include <main.hpp>

ICM42670pGyro mpuGyro;

#define ICM42670_PRIMARY_ADDRESS 0x68
#define ICM42670_SECONDARY_ADDRESS 0x69
#define ICM42670_WHOAMI_REGISTER 0x75
#define ICM42670_WHOAMI_VALUE 0x67
#define ICM42670_FLUSH_REGISTER 0x75
#define ICM42670_FLUSH_VALUE 0x67
#define ICM42670_PWR_MGMT0_REGISTER 0x75

bool ICM42670pGyro::isOnline()
{
  if (I2Cdev::readByte(ICM42670_PRIMARY_ADDRESS, ICM42670_WHOAMI_REGISTER, buffer) == 0 && buffer[0] == ICM42670_WHOAMI_VALUE)
  {
    addr = ICM42670_PRIMARY_ADDRESS;
    return true;
  }
  if (I2Cdev::readByte(ICM42670_SECONDARY_ADDRESS, ICM42670_WHOAMI_REGISTER, buffer) == 0 && buffer[0] == ICM42670_WHOAMI_VALUE)
  {
    addr = ICM42670_SECONDARY_ADDRESS;
    return true;
  }
  return false;
}

bool ICM42670pGyro::setup()
{
  // if (I2Cdev::readByte(addr, ICM42670_PWR_MGMT0_REGISTER, buffer)==0 && buffer[0] == 0x)
  // {
  // }
  // else
  // {
  //   I2Cdev::writeByte(addr, ICM42670_FLUSH_REGISTER, ICM42670_FLUSH_VALUE)
  // }
  // GYRO=LN ACCEL=LN
  if (!I2Cdev::writeByte(addr, ICM42670_PWR_MGMT0_REGISTER, 0x0F))
  {
    return false;
  }
  delayMicroseconds(200); // mandatory per datasheet
  if (
      // GYRO=250dps GYRO=800Hz
      !I2Cdev::writeByte(addr, 0x20, 0x66)
      // ACCEL=2g ACCEL=800Hz
      | !I2Cdev::writeByte(addr, 0x21, 0x66)
      // TEMP=4HzBW
      // | !I2Cdev::writeByte(addr, 0x22, 0x70)
      // GYRO=16HzBW
      | !I2Cdev::writeByte(addr, 0x23, 0x37)
      // ACCEL=16HzBW
      | !I2Cdev::writeByte(addr, 0x24, 0x47))
  {
    return false;
  }

  _sensorConnected = true;
  _calibrationOffset = myConfig.getGyroCalibration();
  applyCalibration();
  return true;
}

void ICM42670pGyro::enterSleep()
{
  I2Cdev::writeByte(addr, ICM42670_PWR_MGMT0_REGISTER, 0x00);
}

void ICM42670pGyro::readSensor(RawGyroData &raw, const int noIterations,
                               const int delayTime)
{
  RawGyroDataL average = {0, 0, 0, 0, 0, 0};

  for (int cnt = 0; cnt < noIterations; cnt++)
  {
    // INT_STATUS_DRDY
    while (I2Cdev::readByte(addr, 0x36, buffer) == 0 && buffer[0] == 0)
    {
    }

    I2Cdev::readBytes(addr, 0x09, 14, buffer);
    raw.temp = (((int16_t)buffer[0]) << 8) | buffer[1];
    raw.ax = (((int16_t)buffer[2]) << 8) | buffer[3];
    raw.ay = (((int16_t)buffer[4]) << 8) | buffer[5];
    raw.az = (((int16_t)buffer[6]) << 8) | buffer[7];
    raw.gx = (((int16_t)buffer[8]) << 8) | buffer[9];
    raw.gy = (((int16_t)buffer[10]) << 8) | buffer[11];
    raw.gz = (((int16_t)buffer[12]) << 8) | buffer[13];

    average.ax += raw.ax;
    average.ay += raw.ay;
    average.az += raw.az;
    average.gx += raw.gx;
    average.gy += raw.gy;
    average.gz += raw.gz;
    average.temp += raw.temp;
  }

  raw.ax = average.ax / noIterations;
  raw.ay = average.ay / noIterations;
  raw.az = average.az / noIterations;
  raw.gx = average.gx / noIterations;
  raw.gy = average.gy / noIterations;
  raw.gz = average.gz / noIterations;
  raw.temp = average.temp / noIterations;
}

void ICM42670pGyro::applyCalibration()
{
  //don't for now, these should be properly factory calibrated, any slight error during calibration will introduce more error
}

void ICM42670pGyro::calibrateSensor()
{
  //don't for now, these should be properly factory calibrated, any slight error during calibration will introduce more error
}

void ICM42670pGyro::debug()
{
  // todo
}

String ICM42670pGyro::getGyroFamily()
{
  return "ICM42670-p";
}

// EOF
