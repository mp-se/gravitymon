/*
MIT License

Copyright (c) 2021 Magnus

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
#ifndef _GYRO_H
#define _GYRO_H

#define I2CDEV_IMPLEMENTATION I2CDEV_ARDUINO_WIRE
//#define I2CDEV_IMPLEMENTATION I2CDEV_BUILTIN_SBWIRE

// Includes
#include <arduino.h>
#include "MPU6050.h"
#include "config.h"

// Classes
struct RawGyroDataL {   // Used for average multiple readings
    long ax;        // Raw Acceleration
    long ay;
    long az;

    long gx;        // Raw Position
    long gy;
    long gz;

    long temp;      // Only for information (temperature of chip)
};

class GyroSensor {
    private:
        MPU6050     accelgyro;
        bool        sensorConnected = false;
        bool        validValue = false;
        double      angle = 0;
        float       sensorTemp = 0;
        RawGyroData calibrationOffset;

        void    debug();
        void    applyCalibration();
        void    dumpCalibration();
        void    readSensor(RawGyroData &raw, const int noIterations = 100, const int delayTime = 1);
        bool    isSensorMoving(RawGyroData &raw);
        double  calculateAngle(RawGyroData &raw);

    public:
        bool setup();
        bool read();
        void calibrateSensor();

        double getAngle() { return angle; };
        double getSensorTempC() { return sensorTemp; };
        bool   isConnected() { return sensorConnected; };
        bool   hasValue() { return validValue; };
        void   enterSleep();
};

// Global instance created
extern GyroSensor myGyro;

#endif // _GYRO_H

// EOF