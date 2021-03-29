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
#include "tempsensor.h"
#include "helper.h"
#include "config.h"
#include <onewire.h>
#include <DallasTemperature.h>
#include <Wire.h>

//
// Conversion between C and F
//
float convertCtoF( float t ) { 
    return (t * 1.8 ) + 32.0; 
}

OneWire myOneWire(D6);
DallasTemperature mySensors(&myOneWire);
TempSensor myTempSensor;

#define TEMPERATURE_PRECISION 9

//
// Setup temp sensors
//
void TempSensor::setup() {

#if defined( SIMULATE_TEMP )
    hasSensors = true;
    return;
#endif

    if( mySensors.getDeviceCount() )
        return;

#if LOG_LEVEL==6
    Log.verbose(F("TSEN: Looking for temp sensors." CR));
#endif
    mySensors.begin();

    if( mySensors.getDeviceCount() ) {
        Log.notice(F("TSEN: Found %d sensors." CR), mySensors.getDeviceCount());
        mySensors.setResolution(TEMPERATURE_PRECISION);
    }

    float t = myConfig.getTempSensorAdj();

    // Set the temp sensor adjustment values
    if( myConfig.isTempC() ) {
        tempSensorAdjF = t * 1.8;     // Convert the adjustment value to C
        tempSensorAdjC = t;
    } else {
        tempSensorAdjF = t;
        tempSensorAdjC = t * 0.556;   // Convert the adjustent value to F
    }

#if LOG_LEVEL==6
    Log.verbose(F("TSEN: Adjustment values for temp sensor %F C, %F F." CR), tempSensorAdjC, tempSensorAdjF );
#endif
}

//
// Retrieving value from sensor
//
float TempSensor::getValue() {
    float c = 0;

#if defined( SIMULATE_TEMP )
    return 21;
#endif

    // Read the sensors
    mySensors.requestTemperatures();

    if( mySensors.getDeviceCount() >= 1) {
        c = mySensors.getTempCByIndex(0);

#if LOG_LEVEL==6
        Log.verbose(F("TSEN: Reciving temp value for sensor %F C." CR), c);
#endif
        hasSensor = true;
    }

    return c;
}

// EOF 