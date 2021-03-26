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
#ifndef _CONFIG_H
#define _CONFIG_H

// Includes
#include "helper.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <stdlib.h>

// defintions
#define CFG_JSON_BUFSIZE    1000

#define CFG_APPNAME         "GravityMon "       // Name of firmware
#define CFG_FILENAME        "/gravitymon.json"  // Name of config file

#define WIFI_DEFAULT_SSID   "GravityMon"        // Name of created SSID
#define WIFI_DEFAULT_PWD    "password"          // Password for created SSID
#define WIFI_MDNS           "gravitymon"        // Prefix for MDNS name
#define WIFI_PORTAL_TIMEOUT 120                 // Number of seconds until the config portal is closed

// These are used in API + Savefile
#define CFG_PARAM_ID                 "id"
#define CFG_PARAM_MDNS               "mdns"                           // Device name
#define CFG_PARAM_OTA                "ota-url"                        // Base URL for OTA
#define CFG_PARAM_PUSH_BREWFATHER    "brewfather-push"                // URL (brewfather format)
#define CFG_PARAM_PUSH_HTTP          "http-push"                      // URL (iSpindle format)
#define CFG_PARAM_PUSH_INTERVAL      "push-interval"                  // Time between push 
#define CFG_PARAM_TEMPFORMAT         "temp-format"                    // C or F
#define CFG_PARAM_VOLTAGEFACTOR      "voltage-factor"                 // Factor to calculate the battery voltage
#define CFG_PARAM_GRAVITY_FORMULA    "gravity-formula"                // Formula for calculating gravity
#define CFG_PARAM_GRAVITY_TEMP_ADJ   "gravity-temp-adjustment"        // True/False. Adjust gravity for temperature
#define CFG_PARAM_TEMP_ADJ           "temp-adjustment-value"          // Correction value for temp sensor
#define CFG_PARAM_GYRO_CALIBRATION   "gyro-calibration-data"          // READ ONLY

// These are used in API's
#define CFG_PARAM_APP_NAME           "app-name"
#define CFG_PARAM_APP_VER            "app-ver"
#define CFG_PARAM_ANGLE              "angle"
#define CFG_PARAM_GRAVITY            "gravity"
#define CFG_PARAM_TEMP_C             "temp-c"
#define CFG_PARAM_TEMP_F             "temp-f"
#define CFG_PARAM_BATTERY            "battery"
#define CFG_PARAM_SLEEP_MODE         "sleep-mode"
#define CFG_PARAM_RSSI               "rssi"

// Used for holding sensordata or sensoroffsets
struct RawGyroData { 
    int16_t ax;     // Raw Acceleration
    int16_t ay;
    int16_t az;

    int16_t gx;     // Raw Position
    int16_t gy;
    int16_t gz;

    int16_t temp;   // Only for information (temperature of chip)
};

// Main configuration class
class Config {
    private:
        bool saveNeeded;

        // Device configuration
        char  id[10];
        char  mDNS[30];
        char  otaURL[200];
        char  tempFormat;                      // C, F
        float voltageFactor;
        float tempSensorAdj;                   // This value will be added to the read sensor value
 
        // Push target settings
        char brewfatherPushTarget[200];
        char httpPushTarget[200];               
        int  pushInterval;                

        // Gravity and temperature calculations
        char gravityFormula[200];
        bool gravityTempAdj;                  // true, false      
        char gravityFormat;                   // G, P

        // Gyro calibration data
        RawGyroData gyroCalibration;          // Holds the gyro calibration constants (6 * int16_t)

        void debug();
        void formatFileSystem();

    public:
        Config();
        const char*  getID() { return &id[0]; };

        const char*  getMDNS() { return &mDNS[0]; }
        void         setMDNS( const char* s ) { strncpy( &mDNS[0], s, sizeof(mDNS)-1); saveNeeded = true; }

        const char*  getOtaURL() { return &otaURL[0]; }
        void         setOtaURL( const char* s ) { strncpy( &otaURL[0], s, sizeof(otaURL)-1); saveNeeded = true; }
        bool         isOtaActive() { return strlen(&otaURL[0])>0?true:false; }

        const char*  getBrewfatherPushTarget() { return &brewfatherPushTarget[0]; }
        void         setBrewfatherPushTarget( const char* s ) { strncpy(&brewfatherPushTarget[0], s, sizeof(brewfatherPushTarget)-1); saveNeeded = true; }
        bool         isBrewfatherActive() { return strlen(&brewfatherPushTarget[0])>0?true:false; }

        const char*  getHttpPushTarget() { return &httpPushTarget[0]; }
        void         setHttpPushTarget( const char* s ) { strncpy(&httpPushTarget[0], s, sizeof(httpPushTarget)-1); saveNeeded = true; }
        bool         isHttpActive() { return strlen(&httpPushTarget[0])>0?true:false; }

        int          getPushInterval() { return pushInterval; }
        void         setPushInterval( int v ) { pushInterval = v; saveNeeded = true; }
        void         setPushInterval( const char* s ) { pushInterval = atoi(s); saveNeeded = true; }

        char         getTempFormat() { return tempFormat; }
        void         setTempFormat( char c ) { tempFormat = c; saveNeeded = true; }
        bool         isTempC() { return tempFormat=='C'?false:true; };
        bool         isTempF() { return tempFormat=='F'?false:true; };

        float        getVoltageFactor() { return voltageFactor; }
        void         setVoltageFactor( float f ) { voltageFactor = f; saveNeeded = true; }
        void         setVoltageFactor( const char* s ) { voltageFactor = atof(s); saveNeeded = true; }

        float        getTempSensorAdj() { return tempSensorAdj; }
        void         setTempSensorAdj( float f ) { tempSensorAdj = f; saveNeeded = true; }
        void         setTempSensorAdj( const char* s ) { tempSensorAdj = atof(s); saveNeeded = true; }

        const char*  getGravityFormula() { return &gravityFormula[0]; }
        void         setGravityFormula( const char* s ) { strncpy(&gravityFormula[0], s, sizeof(gravityFormula)-1); saveNeeded = true; }

        bool         isGravityTempAdj() { return gravityTempAdj; }
        void         setGravityTempAdj( bool b ) { gravityTempAdj = b; saveNeeded = true; }

        char         getGravityFormat() { return gravityFormat; }
        void         setGravityFormat( char c ) { gravityFormat = c; saveNeeded = true; }
        bool         isGravitySG() { return gravityFormat=='G'?false:true; };
        bool         isGravityPlato() { return gravityFormat=='P'?false:true; };

        const RawGyroData& getGyroCalibration() { return gyroCalibration; }
        void               setGyroCalibration( const RawGyroData &r ) { gyroCalibration = r; saveNeeded = true; }

        // IO functions
        void createJson(DynamicJsonDocument& doc);
        bool saveFile();
        bool loadFile();
        void checkFileSystem();
        bool isSaveNeeded() { return saveNeeded; };
        void setSaveNeeded() { saveNeeded = true; };
};

// Global instance created
extern Config myConfig;

#endif // _CONFIG_H

// EOF