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
#define ESP8266_DRD_USE_RTC       true
#define DRD_TIMEOUT               2
#define DRD_ADDRESS               0
#include <ESP_DoubleResetDetector.h>            
DoubleResetDetector *drd;

// Define constats for this program
#ifdef DEACTIVATE_SLEEPMODE
const int interval = 1000;                  // ms, time to wait between changes to output
bool sleepModeAlwaysSkip = true;           // Web interface can override normal behaviour
#else
const int interval = 200;                   // ms, time to wait between changes to output
bool sleepModeAlwaysSkip = false;           // Web interface can override normal behaviour
#endif
unsigned long loopMillis = 0;               // Used for main loop to run the code every _interval_
unsigned long runtimeMillis;                // Used to calculate the total time since start/wakeup 
unsigned long stableGyroMillis;             // Used to calculate the total time since last stable gyro reading 
bool          sleepModeActive = false;
bool          goToSleep = false;
int           loopCounter = 0;

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
    sleepModeActive = (volt<4.15 && (angle>85 && angle<95)) || (volt>4.15) ? false : true; 

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
    LOG_PERF_START("run-time");
    LOG_PERF_START("main-setup");
    runtimeMillis = millis();

    drd = new DoubleResetDetector(DRD_TIMEOUT, DRD_ADDRESS);
    bool dt = drd->detectDoubleReset();  
#if LOG_LEVEL==6
    Log.verbose(F("Main: Reset reason %s." CR), ESP.getResetInfo().c_str() );
#endif
    // Main startup
    Log.notice(F("Main: Started setup for %s." CR), String( ESP.getChipId(), HEX).c_str() );
    printBuildOptions();

    LOG_PERF_START("main-config-load");
    myConfig.checkFileSystem();
    myConfig.loadFile();
    LOG_PERF_STOP("main-config-load");

    // Setup watchdog
    ESP.wdtDisable();
    ESP.wdtEnable( interval*2 );

    if( dt ) {
        Log.notice(F("Main: Detected doubletap on reset. Reset reason=%s" CR), ESP.getResetReason().c_str());
    }

#ifdef SKIP_SLEEPMODE
    // If we are running in debug more we skip this part. makes is hard to debug in case of crash/watchdog reset
    dt = false;
#endif

    LOG_PERF_START("main-wifi-connect");
    myWifi.connect( dt );                   // This will return false if unable to connect to wifi, will be handled in loop()
    LOG_PERF_STOP("main-wifi-connect");

    LOG_PERF_START("main-temp-setup");
    myTempSensor.setup();
    LOG_PERF_STOP("main-temp-setup");

    //LOG_PERF_START("main-gyro-setup");    // Takes less than 5ms, so skip this measurment
    if( !myGyro.setup() )
        Log.error(F("Main: Failed to initialize the gyro." CR));
    //LOG_PERF_STOP("main-gyro-setup");

    LOG_PERF_START("main-gyro-read");
    myGyro.read();
    LOG_PERF_STOP("main-gyro-read");

    LOG_PERF_START("main-batt-read");
    myBatteryVoltage.read();
    LOG_PERF_STOP("main-batt-read");
    checkSleepMode( myGyro.getAngle(), myBatteryVoltage.getVoltage() );

    if( myWifi.isConnected() ) {
#if defined( ACTIVATE_OTA ) 
        LOG_PERF_START("main-wifi-ota");
        if( !sleepModeActive && myWifi.checkFirmwareVersion() ) {
            myWifi.updateFirmware();
        }
        LOG_PERF_STOP("main-wifi-ota");
#endif
        if( !sleepModeActive ) {
            //LOG_PERF_START("main-webserver-setup");   // Takes less than 4ms , so skip this measurment
            myWebServer.setupWebServer();
            //LOG_PERF_STOP("main-webserver-setup");
        }
    }

    LOG_PERF_STOP("main-setup");
    Log.notice(F("Main: Setup completed." CR));
    stableGyroMillis = millis();                        // Put it here so we dont include time for wifi connection
}

//
// Main loops
//
void loop() {
    drd->loop();

    if( sleepModeActive || abs( (long) (millis() - loopMillis)) > interval ) {
        float angle = 0;
        float volt = myBatteryVoltage.getVoltage();
        //float sensorTemp = 0;
        loopCounter++;
#if LOG_LEVEL==6
        Log.verbose(F("Main: Entering main loop." CR) );
#endif   
        // Process the sensor values and push data to targets.
        // ------------------------------------------------------------------------------------------------
        // If we dont get any readings we just skip this and try again the next interval.
        //
        if( myGyro.hasValue() ) {
            angle = myGyro.getAngle();              // Gyro angle

            stableGyroMillis = millis();            // Reset timer

            LOG_PERF_START("loop-temp-read");
            float temp    = myTempSensor.getTempC();  
            LOG_PERF_STOP("loop-temp-read");

            //LOG_PERF_START("loop-gravity-calc");  // Takes less than 2ms , so skip this measurment
            float gravity = calculateGravity( angle, temp );
            //LOG_PERF_STOP("loop-gravity-calc");

            //LOG_PERF_START("loop-gravity-corr");  // Takes less than 2ms , so skip this measurment
            // Use default correction temperature of 20C
            float corrGravity = gravityTemperatureCorrection( gravity, temp, myConfig.getTempFormat() ); 
            //LOG_PERF_STOP("loop-gravity-corr");

#if LOG_LEVEL==6
            Log.verbose(F("Main: Sensor values gyro angle=%F, temp=%F, gravity=%F, corr=%F." CR), angle, temp, gravity, corrGravity );
#endif   
            // Limit the printout when sleep mode is not active.
            if( loopCounter%10 == 0 || sleepModeActive ) {
                Log.notice(F("Main: angle=%F, temp=%F, gravity=%F, corrGravity=%F, batt=%F." CR), angle, temp, gravity, corrGravity ,volt );
            }

            LOG_PERF_START("loop-push");
            myPushTarget.send( angle, gravity, corrGravity, temp, (millis()-runtimeMillis)/1000, sleepModeActive );    // Force the transmission if we are going to sleep
            LOG_PERF_STOP("loop-push");

            // If we have completed the update lets go to sleep
            if( sleepModeActive )
                goToSleep = true;
        } else {
            Log.error(F("Main: No gyro value." CR) );
        }

#if LOG_LEVEL==6
        Log.verbose(F("Main: Sleep mode not active." CR) );
#endif   
        int sleepInterval = myConfig.getSleepInterval();

        // If we didnt get a wifi connection, we enter sleep for a short time to conserve battery.
        // ------------------------------------------------------------------------------------------------
        //
        if( !myWifi.isConnected() ) {           // no connection to wifi
            Log.notice(F("MAIN: No connection to wifi established, sleeping for 60s." CR) );
            sleepInterval = 60;                                                   // 60s
            goToSleep = true;            
        }

        // If the sensor is moving and we are not getting a clear reading, we enter sleep for a short time to conserve battery.
        // ------------------------------------------------------------------------------------------------
        //
        if( sleepModeActive && ((millis()-stableGyroMillis)>10000L) ) {           // 10s since last stable gyro reading
            Log.notice(F("MAIN: Unable to get a stable reading for 10s, sleeping for 60s." CR) );
            sleepInterval = 60;                                                   // 60s
            goToSleep = true;            
        }

        // Enter sleep mode if the conditions are right 
        // ------------------------------------------------------------------------------------------------
        //
        if( goToSleep && !sleepModeAlwaysSkip ) {
            Log.notice(F("MAIN: Entering deep sleep for %d s, run time %l s, battery=%F V." CR), sleepInterval, (millis()-runtimeMillis)/1000, volt );
            LittleFS.end();
            myGyro.enterSleep();
            drd->stop();
            LOG_PERF_STOP("run-time");
            LOG_PERF_PUSH();
            delay(100);
            deepSleep( sleepInterval ); 
        }

        // If we are running in normal mode we just continue 
        // ------------------------------------------------------------------------------------------------
        // Do these checks if we are running in normal mode (not sleep mode)
        //
        checkSleepMode( angle, volt );

        LOG_PERF_START("loop-gyro-read");
        myGyro.read();
        LOG_PERF_STOP("loop-gyro-read");

        //LOG_PERF_START("loop-batt-read"); // Takes less than 2ms , so skip this measurment
        myBatteryVoltage.read();
        //LOG_PERF_STOP("loop-batt-read");

        loopMillis = millis();
//#if LOG_LEVEL==6
        Log.verbose(F("Main: Heap %d kb FreeSketch %d kb." CR), ESP.getFreeHeap()/1024, ESP.getFreeSketchSpace()/1024 );
        Log.verbose(F("Main: HeapFrag %d %%." CR), ESP.getHeapFragmentation() );
//#endif    
    }

    //if( myWifi.isConnected() )
    myWebServer.loop();
}

// EOF