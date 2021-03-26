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
//#define DEBUG_ESP_HTTP_SERVER
#include "webserver.h"
#include "config.h"
#include "helper.h"
#include "gyro.h"
#include "calc.h"
#include "tempsensor.h"
#include <ArduinoJson.h>
#include <incbin.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <LittleFS.h>

// Binary resouces
#if defined( EMBED_HTML )
INCBIN_EXTERN(IndexHtm);
INCBIN_EXTERN(DeviceHtm);
INCBIN_EXTERN(ConfigHtm);
INCBIN_EXTERN(AboutHtm);
#endif

WebServer myWebServer;
ESP8266WebServer server(80);

extern bool sleepModeActive;
extern bool sleepModeAlwaysSkip;

//
// Callback from webServer when / has been accessed.
//
void webHandleDevice() {
#if LOG_LEVEL==6
    Log.verbose(F("WEB : webServer callback for /api/config." CR));
#endif
    DynamicJsonDocument doc(100);
    doc[ CFG_PARAM_ID ]       = myConfig.getID();
    doc[ CFG_PARAM_APP_NAME ] = CFG_APPNAME;
    doc[ CFG_PARAM_APP_VER ]  = CFG_APPVER;
    doc[ CFG_PARAM_MDNS ]     = myConfig.getMDNS();
#if LOG_LEVEL==6
    serializeJson(doc, Serial);
    Serial.print( CR );
#endif    
    String out;
    serializeJson(doc, out);
    server.send(200, "application/json", out.c_str() );
}

//
// Callback from webServer when / has been accessed.
//
void webHandleConfig() {
#if LOG_LEVEL==6
    Log.verbose(F("WEB : webServer callback for /api/config." CR));
#endif
    DynamicJsonDocument doc(CFG_JSON_BUFSIZE);
    myConfig.createJson( doc );

    double angle = myGyro.getAngle();
    double temp  = myTempSensor.getValueCelcius();
    double gravity = calculateGravity( angle, temp );

    doc[ CFG_PARAM_ANGLE ]              = reduceFloatPrecision( angle);
    doc[ CFG_PARAM_GRAVITY ]            = reduceFloatPrecision( gravityTemperatureCorrection( gravity, temp ), 4);
    doc[ CFG_PARAM_BATTERY ]            = reduceFloatPrecision( myBatteryVoltage.getVoltage()); 

#if LOG_LEVEL==6
    serializeJson(doc, Serial);
    Serial.print( CR );
#endif    
    String out;
    serializeJson(doc, out);
    server.send(200, "application/json", out.c_str() );
}

//
// Callback from webServer when / has been accessed.
//
void webHandleCalibrate() {
    String id = server.arg( CFG_PARAM_ID );
#if LOG_LEVEL==6
    Log.verbose(F("WEB : webServer callback for /api/calibrate." CR));
#endif
    if( !id.equalsIgnoreCase( myConfig.getID() ) ) {
        Log.error(F("WEB : Wrong ID received %s, expected %s" CR), id.c_str(), myConfig.getID());
        server.send(400, "text/plain", "Invalid ID.");
        return;
    }
    myGyro.calibrateSensor();
    server.send(200, "text/plain", "Device calibrated" );
}

//
// Callback from webServer when / has been accessed.
//
void webHandleFactoryReset() {
    String id = server.arg( CFG_PARAM_ID );
#if LOG_LEVEL==6
    Log.verbose(F("WEB : webServer callback for /api/factory." CR));
#endif
    if( !id.compareTo( myConfig.getID() ) ) {
        server.send(200, "text/plain", "Doing reset...");
        LittleFS.remove(CFG_FILENAME);
        LittleFS.end();
        delay(500);
        ESP.reset();
    } else {
        server.send(400, "text/plain", "Unknown ID.");
    }
}

//
// Callback from webServer when / has been accessed.
//
void webHandleStatus() {
#if LOG_LEVEL==6
    Log.verbose(F("WEB : webServer callback for /api/status." CR));
#endif
    DynamicJsonDocument doc(256);

    double angle = myGyro.getAngle();
    double temp  = myTempSensor.getValueCelcius();
    double gravity = calculateGravity( angle, temp );

    doc[ CFG_PARAM_ID ]                 = myConfig.getID();
    doc[ CFG_PARAM_ANGLE ]              = reduceFloatPrecision( angle);
    doc[ CFG_PARAM_GRAVITY ]            = reduceFloatPrecision( gravityTemperatureCorrection( gravity, temp ), 4);
    doc[ CFG_PARAM_TEMP_C ]             = reduceFloatPrecision( temp, 1);
    doc[ CFG_PARAM_TEMP_F ]             = reduceFloatPrecision( myTempSensor.getValueFarenheight(), 1);
    doc[ CFG_PARAM_BATTERY ]            = reduceFloatPrecision( myBatteryVoltage.getVoltage()); 
    doc[ CFG_PARAM_TEMPFORMAT ]         = String( myConfig.getTempFormat() ); 
    doc[ CFG_PARAM_SLEEP_MODE ]         = sleepModeAlwaysSkip; 
    doc[ CFG_PARAM_RSSI ]               = WiFi.RSSI(); 
#if LOG_LEVEL==6
    serializeJson(doc, Serial);
    Serial.print( CR );
#endif    
    String out;
    serializeJson(doc, out);
    server.send(200, "application/json", out.c_str() );
}

//
// Callback from webServer when / has been accessed.
//
void webHandleClearWIFI() {
    String id    = server.arg( CFG_PARAM_ID );
#if LOG_LEVEL==6
    Log.verbose(F("WEB : webServer callback for /api/clearwifi." CR));
#endif
    if( !id.compareTo( myConfig.getID() ) ) {
        server.send(200, "text/plain", "Clearing WIFI credentials and doing reset...");
        delay(1000);
        WiFi.disconnect();  // Clear credentials
        ESP.reset();
    } else {
        server.send(400, "text/plain", "Unknown ID.");
    }
}

//
// Used to force the device to never sleep.
//
void webHandleStatusSleepmode() {
    String id = server.arg( CFG_PARAM_ID );
#if LOG_LEVEL==6
    Log.verbose(F("WEB : webServer callback for /api/status/sleepmode." CR) );
#endif
    if( !id.equalsIgnoreCase( myConfig.getID() ) ) {
        Log.error(F("WEB : Wrong ID received %s, expected %s" CR), id.c_str(), myConfig.getID());
        server.send(400, "text/plain", "Invalid ID.");
        return;
    }
#if LOG_LEVEL==6
    Log.verbose(F("WEB : sleep-mode=%s." CR), server.arg( CFG_PARAM_SLEEP_MODE ).c_str() );
#endif
    if( server.arg( CFG_PARAM_SLEEP_MODE ).equalsIgnoreCase( "true" ) )
        sleepModeAlwaysSkip = true;
    else
        sleepModeAlwaysSkip = false;
    server.send(200, "text/plain", "Sleep mode updated" );    
}

//
// Update device settings.
//
void webHandleConfigDevice() {
    String id = server.arg( CFG_PARAM_ID );
#if LOG_LEVEL==6
    Log.verbose(F("WEB : webServer callback for /api/config/device." CR) );
#endif
    if( !id.equalsIgnoreCase( myConfig.getID() ) ) {
        Log.error(F("WEB : Wrong ID received %s, expected %s" CR), id.c_str(), myConfig.getID());
        server.send(400, "text/plain", "Invalid ID.");
        return;
    }
#if LOG_LEVEL==6
    Log.verbose(F("WEB : mdns=%s, temp-format=%s." CR), server.arg( CFG_PARAM_MDNS ).c_str(), server.arg( CFG_PARAM_TEMPFORMAT ).c_str() );
#endif
    myConfig.setMDNS( server.arg( CFG_PARAM_MDNS ).c_str() );
    myConfig.setTempFormat( server.arg( CFG_PARAM_TEMPFORMAT ).charAt(0) );
    myConfig.saveFile();
    server.sendHeader("Location", "/config.htm#collapseOne", true);  
    server.send(302, "text/plain", "Device config updated" );
}

//
// Update push settings.
//
void webHandleConfigPush() {
    String id = server.arg( CFG_PARAM_ID );
#if LOG_LEVEL==6
    Log.verbose(F("WEB : webServer callback for /api/config/push." CR) );
#endif
    if( !id.equalsIgnoreCase( myConfig.getID() ) ) {
        Log.error(F("WEB : Wrong ID received %s, expected %s" CR), id.c_str(), myConfig.getID());
        server.send(400, "text/plain", "Invalid ID.");
        return;
    }
#if LOG_LEVEL==6
    Log.verbose(F("WEB : http=%s, bf=%s interval=%s." CR), server.arg( CFG_PARAM_PUSH_HTTP ).c_str(), server.arg( CFG_PARAM_PUSH_BREWFATHER ).c_str(),  server.arg( CFG_PARAM_PUSH_INTERVAL ).c_str() );
#endif
    myConfig.setHttpPushTarget( server.arg( CFG_PARAM_PUSH_HTTP ).c_str() );
    myConfig.setBrewfatherPushTarget( server.arg( CFG_PARAM_PUSH_BREWFATHER ).c_str() );
    myConfig.setPushInterval( server.arg( CFG_PARAM_PUSH_INTERVAL ).c_str() );
    myConfig.saveFile();
    server.sendHeader("Location", "/config.htm#collapseTwo", true);  
    server.send(302, "text/plain", "Push config updated" );
}

//
// Update gravity settings.
//
void webHandleConfigGravity() {
    String id = server.arg( CFG_PARAM_ID );
#if LOG_LEVEL==6
    Log.verbose(F("WEB : webServer callback for /api/config/gravity." CR) );
#endif
    if( !id.equalsIgnoreCase( myConfig.getID() ) ) {
        Log.error(F("WEB : Wrong ID received %s, expected %s" CR), id.c_str(), myConfig.getID());
        server.send(400, "text/plain", "Invalid ID.");
        return;
    }
#if LOG_LEVEL==6
    Log.verbose(F("WEB : formula=%s, temp-corr=%s." CR), server.arg( CFG_PARAM_GRAVITY_FORMULA ).c_str(), server.arg( CFG_PARAM_GRAVITY_TEMP_ADJ ).c_str() );
#endif
    myConfig.setGravityFormula( server.arg( CFG_PARAM_GRAVITY_FORMULA ).c_str() );
    myConfig.setGravityTempAdj( server.arg( CFG_PARAM_GRAVITY_TEMP_ADJ ).equalsIgnoreCase( "on" ) ? true:false); 
    myConfig.saveFile();
    server.sendHeader("Location", "/config.htm#collapseThree", true);  
    server.send(302, "text/plain", "Gravity config updated" );
}

//
// Update hardware settings.
//
void webHandleConfigHardware() {
    String id = server.arg( CFG_PARAM_ID );
#if LOG_LEVEL==6
    Log.verbose(F("WEB : webServer callback for /api/config/hardware." CR) );
#endif
    if( !id.equalsIgnoreCase( myConfig.getID() ) ) {
        Log.error(F("WEB : Wrong ID received %s, expected %s" CR), id.c_str(), myConfig.getID());
        server.send(400, "text/plain", "Invalid ID.");
        return;
    }
#if LOG_LEVEL==6
    Log.verbose(F("WEB : vf=%s, tempadj=%s, ota=%s." CR), server.arg( CFG_PARAM_VOLTAGEFACTOR ).c_str(), server.arg( CFG_PARAM_TEMP_ADJ ).c_str(), server.arg( CFG_PARAM_OTA ).c_str() );
#endif
    myConfig.setVoltageFactor( server.arg( CFG_PARAM_VOLTAGEFACTOR ).toFloat() );
    myConfig.setTempSensorAdj( server.arg( CFG_PARAM_TEMP_ADJ ).toFloat() ); 
    myConfig.setOtaURL( server.arg( CFG_PARAM_OTA ).c_str() ); 
    myConfig.saveFile();
    server.sendHeader("Location", "/config.htm#collapseFour", true);  
    server.send(302, "text/plain", "Hardware config updated" );
}

//
// Setup the Web Server callbacks and start it
//
bool WebServer::setupWebServer() {
#if LOG_LEVEL==6
    Log.verbose(F("WEB : Setting up web server." CR));
#endif

    Log.notice(F("WEB : Web server setup started." CR));

    MDNS.begin( myConfig.getMDNS() );
    MDNS.addService("http", "tcp", 80);

    // Static content
#if defined( EMBED_HTML )
    server.on("/",[]() {
        server.send_P(200, "text/html", (const char*) gIndexHtmData, gIndexHtmSize );
    } );
    server.on("/index.htm",[]() {
        server.send_P(200, "text/html", (const char*) gIndexHtmData, gIndexHtmSize );
    } );
    server.on("/device.htm",[]() {
        server.send_P(200, "text/html", (const char*) gDeviceHtmData, gDeviceHtmSize );
    } );
    server.on("/config.htm",[]() {
        server.send_P(200, "text/html", (const char*) gConfigHtmData, gConfigHtmSize );
    } );
    server.on("/about.htm",[]() {
        server.send_P(200, "text/html", (const char*) gAboutHtmData, gAboutHtmSize );
    } );
#else
    // Show files in the filessytem at startup
    FSInfo fs;
    LittleFS.info(fs);
    Log.notice( F("File system: Total=%d, Used=%d." CR), fs.totalBytes, fs.usedBytes );
    Dir dir = LittleFS.openDir("/");
    while( dir.next() ) {
        Log.notice( F("File: %s, %d bytes" CR), dir.fileName().c_str(), dir.fileSize() );
    }

    server.serveStatic("/", LittleFS, "/index.htm" );
    server.serveStatic("/index.htm", LittleFS, "/index.htm" );
    server.serveStatic("/device.htm", LittleFS, "/device.htm" );
    server.serveStatic("/config.htm", LittleFS, "/config.htm" );
    server.serveStatic("/about.htm", LittleFS, "/about.htm" );
#endif

    // Dynamic content
    server.on("/api/config", webHandleConfig);              // Get config.json
    server.on("/api/device", webHandleDevice);              // Get device.json
    server.on("/api/calibrate", webHandleCalibrate);        // Run calibration routine (param id)
    server.on("/api/factory", webHandleFactoryReset);                // Reset the device
    server.on("/api/status", webHandleStatus);              // Get the status.json
    server.on("/api/clearwifi", webHandleClearWIFI);        // Clear wifi settings

    server.on("/api/status/sleepmode", webHandleStatusSleepmode); 
    server.on("/api/config/device", webHandleConfigDevice); 
    server.on("/api/config/push", webHandleConfigPush); 
    server.on("/api/config/gravity", webHandleConfigGravity); 
    server.on("/api/config/hardware", webHandleConfigHardware); 

    server.onNotFound( []() {
        Log.error(F("WEB : URL not found %s received." CR), server.uri().c_str());
        server.send(404, "text/plain", F("URL not found") );
    } );

    server.begin();
    Log.notice(F("WEB : Web server started." CR));
    return true;
}

//
// called from main loop
//
void WebServer::loop() {
    // Dont put serial debug output in this call
    server.handleClient();
    MDNS.update();
}

// EOF 