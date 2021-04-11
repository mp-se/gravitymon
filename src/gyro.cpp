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
#include "gyro.h"
#include "helper.h"

GyroSensor myGyro;

#define SENSOR_MOVING_THREASHOLD 500
#define SENSOR_READ_COUNT        50
#define SENSOR_READ_DELAY        3150     // us, empirical, to hold sampling to 200 Hz

//#define GYRO_SHOW_MINMAX                // Will calculate the min/max values when doing calibration 
//#define GYRO_CALIBRATE_STARTUP          // Will  calibrate sensor at startup

//
// Initialize the sensor chip.
//
bool GyroSensor::setup() {
#if LOG_LEVEL==6
    Log.verbose(F("GYRO: Setting up hardware." CR));
#endif
    Wire.begin(D3, D4);
    Wire.setClock(400000);              // 400kHz I2C clock. Comment this line if having compilation difficulties
    accelgyro.initialize();
    
    if( !accelgyro.testConnection() ) {
        Log.error(F("GYRO: Failed to connect to MPU6050 (gyro)." CR));
        sensorConnected = false;
    } else {

        Log.notice(F("GYRO: Connected to MPU6050 (gyro)." CR));
        sensorConnected = true;

        // Configure the sensor
        accelgyro.setTempSensorEnabled(true);
        accelgyro.setFullScaleAccelRange(MPU6050_ACCEL_FS_2);
        accelgyro.setFullScaleGyroRange(MPU6050_GYRO_FS_250);
        accelgyro.setDLPFMode(MPU6050_DLPF_BW_5);
        accelgyro.setRate(17);
        
        // For now we run the calibration at start.
    #if defined ( GYRO_CALIBRATE_STARTUP )
        calibrateSensor();
    #endif

        // Once we have calibration values stored we just apply them from the config.
        calibrationOffset = myConfig.getGyroCalibration();
        applyCalibration();
    }

    return sensorConnected;
}

//
// Set sensor in sleep mode to conserve battery
//
void GyroSensor::enterSleep() {
#if LOG_LEVEL==6
    Log.verbose(F("GYRO: Setting up hardware." CR));
#endif
    accelgyro.setSleepEnabled( true );
}

//
// Do a number of reads to get a more stable value.
//
void GyroSensor::readSensor(RawGyroData &raw, const int noIterations, const int delayTime) {
    RawGyroDataL average = { 0, 0, 0, 0, 0, 0 };

#if LOG_LEVEL==6
    Log.verbose(F("GYRO: Reading sensor with %d iterations %d us delay." CR), noIterations, delayTime );
#endif

    // Set some initial values
#if defined( GYRO_SHOW_MINMAX )
    RawGyroData min, max; 
    //accelgyro.getRotation( &min.gx, &min.gy, &min.gz );
    accelgyro.getAcceleration( &min.ax, &min.ay, &min.az );
    min.temp = accelgyro.getTemperature();
    max = min;
#endif
    for(int cnt = 0; cnt < noIterations ; cnt ++) {
        accelgyro.getRotation( &raw.gx, &raw.gy, &raw.gz );
        accelgyro.getAcceleration( &raw.ax, &raw.ay, &raw.az );
        raw.temp = accelgyro.getTemperature();

        average.ax += raw.ax; 
        average.ay += raw.ay; 
        average.az += raw.az; 
        average.gx += raw.gx; 
        average.gy += raw.gy; 
        average.gz += raw.gz; 
        average.temp += raw.temp; 

        // Log what the minium value is
#if defined( GYRO_SHOW_MINMAX )
        if( raw.ax < min.ax ) min.ax = raw.ax;
        if( raw.ay < min.ay ) min.ay = raw.ay;
        if( raw.az < min.az ) min.az = raw.az;
        if( raw.gx < min.gx ) min.gx = raw.gx;
        if( raw.gy < min.gy ) min.gy = raw.gy;
        if( raw.gz < min.gz ) min.gz = raw.gz;
        if( raw.temp < min.temp ) min.temp = raw.temp;

        // Log what the maximum value is
        if( raw.ax > max.ax ) max.ax = raw.ax;
        if( raw.ay > max.ay ) max.ay = raw.ay;
        if( raw.az > max.az ) max.az = raw.az;
        if( raw.gx > max.gx ) max.gx = raw.gx;
        if( raw.gy > max.gy ) max.gy = raw.gy;
        if( raw.gz > max.gz ) max.gz = raw.gz;
        if( raw.temp > max.temp ) max.temp = raw.temp;
#endif

        delayMicroseconds( delayTime );
    }

    raw.ax = average.ax/noIterations;
    raw.ay = average.ay/noIterations;
    raw.az = average.az/noIterations;
    raw.gx = average.gx/noIterations;
    raw.gy = average.gy/noIterations;
    raw.gz = average.gz/noIterations;
    raw.temp = average.temp/noIterations;

#if LOG_LEVEL==6
    #if defined( GYRO_SHOW_MINMAX )
    Log.verbose(F("GYRO: Min    \t%d\t%d\t%d\t%d\t%d\t%d\t%d." CR), min.ax, min.ay, min.az, min.gx, min.gy, min.gz, min.temp );
    Log.verbose(F("GYRO: Max    \t%d\t%d\t%d\t%d\t%d\t%d\t%d." CR), max.ax, max.ay, max.az, max.gx, max.gy, max.gz, max.temp );
    #endif
    Log.verbose(F("GYRO: Average\t%d\t%d\t%d\t%d\t%d\t%d\t%d." CR), raw.ax, raw.ay, raw.az, raw.gx, raw.gy, raw.gz, raw.temp );
    //Log.verbose(F("GYRO: Result \t%d\t%d\t%d\t%d\t%d\t%d." CR), average.ax/noIterations, average.ay/noIterations, average.az/noIterations, 
    //                                                           average.gx/noIterations, average.gy/noIterations, average.gz/noIterations );
#endif
}

//
// Calcuate the angles (tilt)
//
double GyroSensor::calculateAngle(RawGyroData &raw) {
#if LOG_LEVEL==6
    Log.verbose(F("GYRO: Calculating the angle." CR) );
#endif

    // Source: https://www.nxp.com/docs/en/application-note/AN3461.pdf
    double v = (acos( raw.ay / sqrt( raw.ax*raw.ax + raw.ay*raw.ay + raw.az*raw.az ) ) *180.0 / PI);
    //Log.notice(F("GYRO: angle = %F." CR), v );
    //double v = (acos( raw.az / sqrt( raw.ax*raw.ax + raw.ay*raw.ay + raw.az*raw.az ) ) *180.0 / PI);
    //Log.notice(F("GYRO: angle = %F." CR), v );

#if LOG_LEVEL==6
    Log.verbose(F("GYRO: angle = %F." CR), v );
#endif
    return v;
}

//
// Check if the values are high that indicate that the sensor is moving.
//
bool GyroSensor::isSensorMoving(RawGyroData &raw) {
#if LOG_LEVEL==6
    Log.verbose(F("GYRO: Checking for sensor movement." CR) );
#endif

    int x = abs(raw.gx), y = abs(raw.gy), z = abs(raw.gz);

    if( x>SENSOR_MOVING_THREASHOLD || y>SENSOR_MOVING_THREASHOLD || z>SENSOR_MOVING_THREASHOLD ) {
        Log.notice(F("GYRO: Movement detected (%d)\t%d\t%d\t%d." CR), SENSOR_MOVING_THREASHOLD, x, y, z);
        return true;
    }

    return false;
}

//
// Read the tilt angle from the gyro.
//
bool GyroSensor::read() {
#if LOG_LEVEL==6
    Log.verbose(F("GYRO: Getting new gyro position." CR) );
#endif

    RawGyroData raw;
    readSensor( raw, SENSOR_READ_COUNT, SENSOR_READ_DELAY );

    // If the sensor is unstable we return false to signal we dont have valid value
    if( isSensorMoving(raw) ) {
        Log.notice(F("GYRO: Sensor is moving." CR) );
        validValue = false;
    } else {
        validValue = true;
        angle = calculateAngle( raw );
        //Log.notice(F("GYRO: Calculated angle %F" CR), angle );
    }

    sensorTemp = ((float) raw.temp) / 340 + 36.53;

    // The first read value is close to the DS18 value according to my tests, if more reads are 
    // done then the gyro temp will increase to much
    if( initialSensorTemp == INVALID_TEMPERATURE )
        initialSensorTemp = sensorTemp;

    return validValue;
}

//
// Dump the stored calibration values.
//
void GyroSensor::dumpCalibration() {
#if LOG_LEVEL==6
    Log.verbose(F("GYRO: Accel offset\t%d\t%d\t%d" CR), calibrationOffset.ax, calibrationOffset.ay, calibrationOffset.az );
    Log.verbose(F("GYRO: Gyro offset \t%d\t%d\t%d" CR), calibrationOffset.gx, calibrationOffset.gy, calibrationOffset.gz );
#endif
}

//
// Update the sensor with out calculated offsets.
//
void GyroSensor::applyCalibration() {
#if LOG_LEVEL==6
    Log.verbose(F("GYRO: Applying calibration offsets to sensor." CR) );
#endif
    if( ( calibrationOffset.ax + calibrationOffset.ay + calibrationOffset.az + calibrationOffset.gx + calibrationOffset.gy + calibrationOffset.gz ) == 0 ) {
        Log.error(F("GYRO: No valid calibraion values exist, aborting." CR) );
        return;
    }

    accelgyro.setXAccelOffset( calibrationOffset.ax );
    accelgyro.setYAccelOffset( calibrationOffset.ay );
    accelgyro.setZAccelOffset( calibrationOffset.az );
    accelgyro.setXGyroOffset( calibrationOffset.gx );
    accelgyro.setYGyroOffset( calibrationOffset.gy );
    accelgyro.setZGyroOffset( calibrationOffset.gz );
}

//
// Calculate the offsets for calibration.
//
void GyroSensor::calibrateSensor() {
#if LOG_LEVEL==6
    Log.verbose(F("GYRO: Calibrating sensor" CR) );
#endif
    //accelgyro.PrintActiveOffsets();
    //Serial.print( CR );

    accelgyro.setDLPFMode(MPU6050_DLPF_BW_5);
    accelgyro.CalibrateAccel(6);                    // 6 = 600 readings
    accelgyro.CalibrateGyro(6);

    accelgyro.PrintActiveOffsets();
    Serial.print( CR );

    calibrationOffset.ax = accelgyro.getXAccelOffset();
    calibrationOffset.ay = accelgyro.getYAccelOffset();
    calibrationOffset.az = accelgyro.getZAccelOffset();
    calibrationOffset.gx = accelgyro.getXGyroOffset();
    calibrationOffset.gy = accelgyro.getYGyroOffset();
    calibrationOffset.gz = accelgyro.getZGyroOffset();

    // Save the calibrated values
    myConfig.setGyroCalibration( calibrationOffset );
    myConfig.saveFile();
}

//
// Calibrate the device.
//
void GyroSensor::debug() {
#if LOG_LEVEL==6
    Log.verbose(F("GYRO: Debug - Clock src   %d." CR), accelgyro.getClockSource() );
    Log.verbose(F("GYRO: Debug - Device ID   %d." CR), accelgyro.getDeviceID() );
    Log.verbose(F("GYRO: Debug - DHPF Mode   %d." CR), accelgyro.getDHPFMode() );
    Log.verbose(F("GYRO: Debug - DMP on      %s." CR), accelgyro.getDMPEnabled()?"on":"off" );
    Log.verbose(F("GYRO: Debug - Acc range   %d." CR), accelgyro.getFullScaleAccelRange() );
    Log.verbose(F("GYRO: Debug - Gyr range   %d." CR), accelgyro.getFullScaleGyroRange() );
    Log.verbose(F("GYRO: Debug - Int         %s." CR), accelgyro.getIntEnabled()?"on":"off" );
    Log.verbose(F("GYRO: Debug - Clock       %d." CR), accelgyro.getMasterClockSpeed() );
    Log.verbose(F("GYRO: Debug - Rate        %d." CR), accelgyro.getRate() );
    Log.verbose(F("GYRO: Debug - Gyro range  %d." CR), accelgyro.getFullScaleGyroRange() );
//  Log.verbose(F("GYRO: Debug - I2C bypass %s." CR), accelgyro.getI2CBypassEnabled()?"on":"off" );
//  Log.verbose(F("GYRO: Debug - I2C master %s." CR), accelgyro.getI2CMasterModeEnabled()?"on":"off" );
    Log.verbose(F("GYRO: Debug - Acc FactX   %d." CR), accelgyro.getAccelXSelfTestFactoryTrim() );
    Log.verbose(F("GYRO: Debug - Acc FactY   %d." CR), accelgyro.getAccelYSelfTestFactoryTrim() );
    Log.verbose(F("GYRO: Debug - Acc FactZ   %d." CR), accelgyro.getAccelZSelfTestFactoryTrim() );
    Log.verbose(F("GYRO: Debug - Gyr FactX   %d." CR), accelgyro.getGyroXSelfTestFactoryTrim() );
    Log.verbose(F("GYRO: Debug - Gyr FactY   %d." CR), accelgyro.getGyroYSelfTestFactoryTrim() );
    Log.verbose(F("GYRO: Debug - Gyr FactZ   %d." CR), accelgyro.getGyroZSelfTestFactoryTrim() );

    switch( accelgyro.getFullScaleAccelRange() ) {
        case 0:
            Log.verbose(F("GYRO: Debug - Accel range +/- 2g." CR));
        break;
        case 1:
            Log.verbose(F("GYRO: Debug - Accel range +/- 4g." CR));
        break;
        case 2:
            Log.verbose(F("GYRO: Debug - Accel range +/- 8g." CR));
        break;
        case 3:
            Log.verbose(F("GYRO: Debug - Accel range +/- 16g." CR));
        break; 
    }

    Log.verbose(F("GYRO: Debug - Acc OffX    %d\t%d." CR), accelgyro.getXAccelOffset(), calibrationOffset.az );
    Log.verbose(F("GYRO: Debug - Acc OffY    %d\t%d." CR), accelgyro.getYAccelOffset(), calibrationOffset.ay );
    Log.verbose(F("GYRO: Debug - Acc OffZ    %d\t%d." CR), accelgyro.getZAccelOffset(), calibrationOffset.az );
    Log.verbose(F("GYRO: Debug - Gyr OffX    %d\t%d." CR), accelgyro.getXGyroOffset(), calibrationOffset.gx );
    Log.verbose(F("GYRO: Debug - Gyr OffY    %d\t%d." CR), accelgyro.getYGyroOffset(), calibrationOffset.gy );
    Log.verbose(F("GYRO: Debug - Gyr OffZ    %d\t%d." CR), accelgyro.getZGyroOffset(), calibrationOffset.gz );
#endif
}

// EOF