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
#include "helper.h"
#include "gyro.h"
#include "config.h"
#include "wifi.h"
#include "webserver.h"
#include "calc.h"
#include "tempsensor.h"
#include "pushtarget.h"
#include <LittleFS.h>

// Settings for double reset detector.
#define ESP_MRD_USE_LITTLEFS    true
#define ESP_MRD_USE_SPIFFS      false
#define ESP_MRD_USE_EEPROM      false
#define MRD_TIMES               3
#define MRD_TIMEOUT             10
#define MRD_ADDRESS             0
//#define MULTIRESETDETECTOR_DEBUG true 
#include <ESP_MultiResetDetector.h>            
MultiResetDetector *mrd;

// Define constats for this program
#if LOG_LEVEL==6 
const int interval = 1000;                  // ms, time to wait between changes to output
bool sleepModeAlwaysSkip = true;           // Web interface can override normal behaviour
#else
const int interval = 100;                   // ms, time to wait between changes to output
bool sleepModeAlwaysSkip = false;           // Web interface can override normal behaviour
#endif
unsigned long lastMillis = 0;
unsigned long startMillis;
bool sleepModeActive = false;

//
// Check if we should be in sleep mode
//
void checkSleepMode( float angle, float volt ) {

#if defined( SKIP_SLEEPMODE )
    sleepModeActive = false;
    Log.verbose(F("MAIN: Skipping sleep mode (SKIP_SLEEPMODE is defined)." CR) );
    return;
#endif

    const RawGyroData &g = myConfig.getGyroCalibration();

    // Will not enter sleep mode if: no calibration data 
    if( g.ax==0 && g.ay==0  && g.az==0  && g.gx==0  && g.gy==0 && g.gz==0 ) {
        Log.notice(F("MAIN: Missing calibration data, so forcing webserver to be active." CR) );
        sleepModeAlwaysSkip = true;
    }

    if( sleepModeAlwaysSkip ) {
        Log.notice(F("MAIN: Sleep mode disabled from web interface." CR) );
        sleepModeActive = false;
        return;
    }

    // Will not enter sleep mode if: charger is connected 
    sleepModeActive = volt<4.15 ? true : false; 

    // sleep mode active when flat
    //sleepModeActive = ( angle<85 && angle>5 ) ? true : false; 
#if LOG_LEVEL==6
    Log.verbose(F("MAIN: Deep sleep mode %s (angle=%F volt=%F)." CR), sleepModeActive ? "true":"false", angle, volt );
#endif
}

//
// Setup
//
void setup() {
    startMillis = millis();
    mrd = new MultiResetDetector(MRD_TIMEOUT, MRD_ADDRESS);
    bool dt = mrd->detectMultiReset();  

#if LOG_LEVEL==6
    Log.verbose(F("Main: Reset reason %s." CR), ESP.getResetInfo().c_str() );
#endif
    // Main startup
    Log.notice(F("Main: Started setup for %s." CR), String( ESP.getChipId(), HEX).c_str() );
    printBuildOptions();

    Log.notice(F("Main: Loading configuration." CR));
    myConfig.checkFileSystem();
    myConfig.loadFile();

    // Setup watchdog
    ESP.wdtDisable();
    ESP.wdtEnable( interval*2 );

    myTempSensor.setup();

    // Setup Gyro
    if( !myGyro.setup() )
        Log.error(F("Main: Failed to initialize the gyro." CR));

    if( dt ) 
        Log.notice(F("Main: Detected doubletap on reset." CR));

    Log.notice(F("Main: Connecting to wifi." CR));
    myWifi.connect( dt );
    Log.notice(F("Main: WIFI connected." CR));

    myGyro.read();
    myBatteryVoltage.read();
    checkSleepMode( myGyro.getAngle(), myBatteryVoltage.getVoltage() );

    if( myWifi.isConnected() ) {
        Log.notice(F("Main: Connected to wifi ip=%s." CR), myWifi.getIPAddress().c_str() );

#if defined( ACTIVATE_OTA ) 
        if( !sleepModeActive && myWifi.checkFirmwareVersion() ) {
            myWifi.updateFirmware();
        }
#endif
        if( !sleepModeActive )
            if( myWebServer.setupWebServer() )
                Log.notice(F("Main: Webserver is running." CR) );
    }
}

//
// Main loops
//
void loop() {
    mrd->loop();

    if( sleepModeActive || abs(millis() - lastMillis) > interval ) {
        float angle = 90;
        float volt = myBatteryVoltage.getVoltage();
#if LOG_LEVEL==6
        Log.verbose(F("Main: Entering main loop." CR) );
#endif   
        // If we dont get any readings we just skip this and try again the next interval.
        if( myGyro.hasValue() ) {
            angle = myGyro.getAngle();
            float temp    = myTempSensor.getValueCelcius();     // The code is build around using C for temp. 
            float gravity = calculateGravity( angle, temp );
#if LOG_LEVEL==6
            Log.verbose(F("Main: Sensor values gyro=%F, temp=%F, gravity=%F." CR), angle, temp, gravity );
#endif   
            if( myConfig.isGravityTempAdj() ) {
                gravity = gravityTemperatureCorrection( gravity, temp);       // Use default correction temperature of 20C
#if LOG_LEVEL==6
                Log.verbose(F("Main: Temp adjusted gravity=%F." CR), gravity );
#endif   
            }

            Log.notice(F("Main: Gyro angle=%F, temp=%F, gravity=%F, batt=%F." CR), angle, temp, gravity, volt );

#if defined( ACTIVATE_PUSH )
            unsigned long runTime = millis() - startMillis;

            myPushTarget.send( angle, gravity, temp, runTime/1000, sleepModeActive );    // Force the transmission if we are going to sleep
#endif
        } else {
            Log.error(F("Main: No gyro value." CR) );
        }

        if( sleepModeActive ) {
            unsigned long runTime = millis() - startMillis;

            // Enter sleep mode...
            Log.notice(F("MAIN: Entering deep sleep, run time %l s." CR), runTime/1000 );
            LittleFS.end();
            myGyro.enterSleep();
            mrd->stop();
            delay(100);
            deepSleep( myConfig.getPushInterval() ); 
        }

#if LOG_LEVEL==6
        Log.verbose(F("Main: Sleep mode not active." CR) );
#endif   
        // Do these checks if we are running in normal mode (not sleep mode)
        checkSleepMode( angle, volt );
        myGyro.read();
        myBatteryVoltage.read();
        lastMillis = millis();
#if LOG_LEVEL==6
        Log.verbose(F("Main: Heap %d kb FreeSketch %d kb." CR), ESP.getFreeHeap()/1024, ESP.getFreeSketchSpace()/1024 );
        Log.verbose(F("Main: HeapFrag %d %%." CR), ESP.getHeapFragmentation() );
#endif    
    }

    myWebServer.loop();
}

// EOF