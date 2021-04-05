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
//#define DEBUG_ESP_HTTP_SERVER
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <LittleFS.h>

// Binary resouces
#if defined( EMBED_HTML )
INCBIN_EXTERN(IndexHtm);
INCBIN_EXTERN(DeviceHtm);
INCBIN_EXTERN(ConfigHtm);
INCBIN_EXTERN(AboutHtm);
#else
INCBIN_EXTERN(UploadHtm);
#endif

WebServer myWebServer;                  // My wrapper class fr webserver functions
ESP8266WebServer server(80);

extern bool sleepModeActive;
extern bool sleepModeAlwaysSkip;

//
// Callback from webServer when / has been accessed.
//
void webHandleDevice() {
    LOG_PERF_START("webserver-api-device");
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
    LOG_PERF_STOP("webserver-api-device");
}

//
// Callback from webServer when / has been accessed.
//
void webHandleConfig() {
    LOG_PERF_START("webserver-api-config");
#if LOG_LEVEL==6
    Log.verbose(F("WEB : webServer callback for /api/config." CR));
#endif
    DynamicJsonDocument doc(CFG_JSON_BUFSIZE);
    myConfig.createJson( doc );

    double angle = myGyro.getAngle();
    double temp  = myTempSensor.getValueCelcius();
    double gravity = calculateGravity( angle, temp );

    doc[ CFG_PARAM_ANGLE ]   = reduceFloatPrecision( angle);
    doc[ CFG_PARAM_GRAVITY ] = reduceFloatPrecision( gravityTemperatureCorrection( gravity, temp ), 4);
    doc[ CFG_PARAM_BATTERY ] = reduceFloatPrecision( myBatteryVoltage.getVoltage()); 

#if LOG_LEVEL==6
    serializeJson(doc, Serial);
    Serial.print( CR );
#endif    
    String out;
    serializeJson(doc, out);
    server.send(200, "application/json", out.c_str() );
    LOG_PERF_STOP("webserver-api-config");
}

//
// Callback from webServer when / has been accessed.
//
void webHandleUpload() {
    LOG_PERF_START("webserver-api-upload");
#if LOG_LEVEL==6
    Log.verbose(F("WEB : webServer callback for /api/upload." CR));
#endif
    Log.notice(F("WEB : webServer callback for /api/upload." CR));
    DynamicJsonDocument doc(100);

    doc[ "index" ]  = myWebServer.checkHtmlFile( WebServer::HTML_INDEX );
    doc[ "device" ] = myWebServer.checkHtmlFile( WebServer::HTML_DEVICE );
    doc[ "config" ] = myWebServer.checkHtmlFile( WebServer::HTML_CONFIG );
    doc[ "about" ]  = myWebServer.checkHtmlFile( WebServer::HTML_ABOUT );
    
#if LOG_LEVEL==6
    serializeJson(doc, Serial);
    Serial.print( CR );
#endif    
    String out;
    serializeJson(doc, out);
    server.send(200, "application/json", out.c_str() );
    LOG_PERF_STOP("webserver-api-upload");
}

//
// Callback from webServer when / has been accessed.
//
File uploadFile;

void webHandleUploadFile() {
    LOG_PERF_START("webserver-api-upload-file");
#if LOG_LEVEL==6
    Log.verbose(F("WEB : webServer callback for /api/upload/file." CR));
#endif
    HTTPUpload& upload = server.upload();
    String f = upload.filename;
    bool validFilename = false;

    if( f.equalsIgnoreCase("index.min.htm")  || f.equalsIgnoreCase("device.min.htm") ||
        f.equalsIgnoreCase("config.min.htm") || f.equalsIgnoreCase("about.min.htm")  ) {
            validFilename = true;
    }

    Log.notice(F("WEB : webServer callback for /api/upload, receiving file %s, valid=%s." CR), f.c_str(), validFilename?"yes":"no");

    if(upload.status == UPLOAD_FILE_START) {
        Log.notice(F("WEB : Start upload." CR) );
        if( validFilename )
            uploadFile = LittleFS.open( f, "w");
    } else if(upload.status == UPLOAD_FILE_WRITE) {
        Log.notice(F("WEB : Writing upload." CR) );
        if(uploadFile)
            uploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
    } else if(upload.status == UPLOAD_FILE_END){
        Log.notice(F("WEB : Finish upload." CR) );
        if(uploadFile) {                                    
            uploadFile.close();                               
            Log.notice(F("WEB : File uploaded %d bytes." CR), upload.totalSize);
        } 
        server.sendHeader("Location","/");               
        server.send(303);
    } else {
        server.send(500, "text/plain", "Couldn't create file.");
    }
    LOG_PERF_STOP("webserver-api-upload-file");
}

//
// Callback from webServer when / has been accessed.
//
void webHandleCalibrate() {
    LOG_PERF_START("webserver-api-calibrate");
    String id = server.arg( CFG_PARAM_ID );
#if LOG_LEVEL==6
    Log.verbose(F("WEB : webServer callback for /api/calibrate." CR));
#endif
    if( !id.equalsIgnoreCase( myConfig.getID() ) ) {
        Log.error(F("WEB : Wrong ID received %s, expected %s" CR), id.c_str(), myConfig.getID());
        server.send(400, "text/plain", "Invalid ID.");
        LOG_PERF_STOP("webserver-api-calibrate");
        return;
    }
    myGyro.calibrateSensor();
    server.send(200, "text/plain", "Device calibrated" );
    LOG_PERF_STOP("webserver-api-calibrate");
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
    LOG_PERF_START("webserver-api-status");
#if LOG_LEVEL==6
    Log.verbose(F("WEB : webServer callback for /api/status." CR));
#endif
    DynamicJsonDocument doc(256);

    double angle = myGyro.getAngle();
    double temp  = myTempSensor.getValueCelcius();
    double gravity = calculateGravity( angle, temp );

    doc[ CFG_PARAM_ID ]         = myConfig.getID();
    doc[ CFG_PARAM_ANGLE ]      = reduceFloatPrecision( angle);
    doc[ CFG_PARAM_GRAVITY ]    = reduceFloatPrecision( gravityTemperatureCorrection( gravity, temp ), 4);
    doc[ CFG_PARAM_TEMP_C ]     = reduceFloatPrecision( temp, 1);
    doc[ CFG_PARAM_TEMP_F ]     = reduceFloatPrecision( myTempSensor.getValueFarenheight(), 1);
    doc[ CFG_PARAM_BATTERY ]    = reduceFloatPrecision( myBatteryVoltage.getVoltage()); 
    doc[ CFG_PARAM_TEMPFORMAT ] = String( myConfig.getTempFormat() ); 
    doc[ CFG_PARAM_SLEEP_MODE ] = sleepModeAlwaysSkip; 
    doc[ CFG_PARAM_RSSI ]       = WiFi.RSSI(); 
#if LOG_LEVEL==6
    serializeJson(doc, Serial);
    Serial.print( CR );
#endif    
    String out;
    serializeJson(doc, out);
    server.send(200, "application/json", out.c_str() );
    LOG_PERF_STOP("webserver-api-status");
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
    LOG_PERF_START("webserver-api-sleepmode");
    String id = server.arg( CFG_PARAM_ID );
#if LOG_LEVEL==6
    Log.verbose(F("WEB : webServer callback for /api/status/sleepmode." CR) );
#endif
    if( !id.equalsIgnoreCase( myConfig.getID() ) ) {
        Log.error(F("WEB : Wrong ID received %s, expected %s" CR), id.c_str(), myConfig.getID());
        server.send(400, "text/plain", "Invalid ID.");
        LOG_PERF_STOP("webserver-api-sleepmode");
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
    LOG_PERF_STOP("webserver-api-sleepmode");
}

//
// Update device settings.
//
void webHandleConfigDevice() {
    LOG_PERF_START("webserver-api-config-device");
    String id = server.arg( CFG_PARAM_ID );
#if LOG_LEVEL==6
    Log.verbose(F("WEB : webServer callback for /api/config/device." CR) );
#endif
    if( !id.equalsIgnoreCase( myConfig.getID() ) ) {
        Log.error(F("WEB : Wrong ID received %s, expected %s" CR), id.c_str(), myConfig.getID());
        server.send(400, "text/plain", "Invalid ID.");
        LOG_PERF_STOP("webserver-api-config-device");
        return;
    }
#if LOG_LEVEL==6
    Log.verbose(F("WEB : mdns=%s, temp-format=%s." CR), server.arg( CFG_PARAM_MDNS ).c_str(), server.arg( CFG_PARAM_TEMPFORMAT ).c_str() );
#endif
    myConfig.setMDNS( server.arg( CFG_PARAM_MDNS ).c_str() );
    myConfig.setTempFormat( server.arg( CFG_PARAM_TEMPFORMAT ).charAt(0) );
    myConfig.setSleepInterval( server.arg( CFG_PARAM_SLEEP_INTERVAL ).c_str() );
    myConfig.saveFile();
    server.sendHeader("Location", "/config.htm#collapseOne", true);  
    server.send(302, "text/plain", "Device config updated" );
    LOG_PERF_STOP("webserver-api-config-device");
}

//
// Update push settings.
//
void webHandleConfigPush() {
    LOG_PERF_START("webserver-api-config-push");
    String id = server.arg( CFG_PARAM_ID );
#if LOG_LEVEL==6
    Log.verbose(F("WEB : webServer callback for /api/config/push." CR) );
#endif
    if( !id.equalsIgnoreCase( myConfig.getID() ) ) {
        Log.error(F("WEB : Wrong ID received %s, expected %s" CR), id.c_str(), myConfig.getID());
        server.send(400, "text/plain", "Invalid ID.");
        LOG_PERF_STOP("webserver-api-config-push");
        return;
    }
#if LOG_LEVEL==6
    Log.verbose(F("WEB : http=%s,%s, bf=%s influx2=%s, %s, %s, %s." CR), server.arg( CFG_PARAM_PUSH_HTTP ).c_str(),
                    server.arg( CFG_PARAM_PUSH_HTTP2 ).c_str(), server.arg( CFG_PARAM_PUSH_BREWFATHER ).c_str(),
                    server.arg( CFG_PARAM_PUSH_INFLUXDB2 ).c_str(), server.arg( CFG_PARAM_PUSH_INFLUXDB2_ORG ).c_str(),
                    server.arg( CFG_PARAM_PUSH_INFLUXDB2_BUCKET ).c_str(), server.arg( CFG_PARAM_PUSH_INFLUXDB2_AUTH ).c_str()
          );
#endif
    myConfig.setHttpPushUrl( server.arg( CFG_PARAM_PUSH_HTTP ).c_str() );
    myConfig.setHttpPushUrl2( server.arg( CFG_PARAM_PUSH_HTTP2 ).c_str() );
    myConfig.setBrewfatherPushUrl( server.arg( CFG_PARAM_PUSH_BREWFATHER ).c_str() );
    myConfig.setInfluxDb2PushUrl( server.arg( CFG_PARAM_PUSH_INFLUXDB2 ).c_str() );
    myConfig.setInfluxDb2PushOrg( server.arg( CFG_PARAM_PUSH_INFLUXDB2_ORG ).c_str() );
    myConfig.setInfluxDb2PushBucket( server.arg( CFG_PARAM_PUSH_INFLUXDB2_BUCKET ).c_str() );
    myConfig.setInfluxDb2PushToken( server.arg( CFG_PARAM_PUSH_INFLUXDB2_AUTH ).c_str() );
    myConfig.saveFile();
    server.sendHeader("Location", "/config.htm#collapseTwo", true);  
    server.send(302, "text/plain", "Push config updated" );
    LOG_PERF_STOP("webserver-api-config-push");
}

//
// Update gravity settings.
//
void webHandleConfigGravity() {
    LOG_PERF_START("webserver-api-config-gravity");
    String id = server.arg( CFG_PARAM_ID );
#if LOG_LEVEL==6
    Log.verbose(F("WEB : webServer callback for /api/config/gravity." CR) );
#endif
    if( !id.equalsIgnoreCase( myConfig.getID() ) ) {
        Log.error(F("WEB : Wrong ID received %s, expected %s" CR), id.c_str(), myConfig.getID());
        server.send(400, "text/plain", "Invalid ID.");
        LOG_PERF_STOP("webserver-api-config-gravity");
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
    LOG_PERF_STOP("webserver-api-config-gravity");
}

//
// Update hardware settings.
//
void webHandleConfigHardware() {
    LOG_PERF_START("webserver-api-config-hardware");
    String id = server.arg( CFG_PARAM_ID );
#if LOG_LEVEL==6
    Log.verbose(F("WEB : webServer callback for /api/config/hardware." CR) );
#endif
    if( !id.equalsIgnoreCase( myConfig.getID() ) ) {
        Log.error(F("WEB : Wrong ID received %s, expected %s" CR), id.c_str(), myConfig.getID());
        server.send(400, "text/plain", "Invalid ID.");
        LOG_PERF_STOP("webserver-api-config-hardware");
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
    LOG_PERF_STOP("webserver-api-config-hardware");
}

//
// Helper function to check if files exist on file system.
//
const char* WebServer::getHtmlFileName( HtmlFile item ) {
#if LOG_LEVEL==6
    Log.verbose(F("WEB : Looking up filename for %d." CR), item);
#endif
    switch( item ) {
        case HTML_INDEX:
        return "index.min.htm";
        case HTML_DEVICE:
        return "device.min.htm";
        case HTML_CONFIG:
        return "config.min.htm";
        case HTML_ABOUT:
        return "about.min.htm";
    }

    return "";
}

//
// Helper function to check if files exist on file system.
//
bool WebServer::checkHtmlFile( HtmlFile item ) {
    const char *fn = getHtmlFileName( item );

#if LOG_LEVEL==6
    Log.verbose(F("WEB : Checking for file %s." CR), fn );
#endif

    // TODO: We might need to add more checks here like zero file size etc. But for now we only check if the file exist.

    return LittleFS.exists( fn );
}

//
// Setup the Web Server callbacks and start it
//
bool WebServer::setupWebServer() {
    Log.notice(F("WEB : Configuring web server." CR));

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
    Log.notice( F("WEB : File system Total=%d, Used=%d." CR), fs.totalBytes, fs.usedBytes );
    Dir dir = LittleFS.openDir("/");
    while( dir.next() ) {
        Log.notice( F("WEB : File=%s, %d bytes" CR), dir.fileName().c_str(), dir.fileSize() );
    }

    // Check if the html files exist, if so serve them, else show the static upload page.
    if( checkHtmlFile( HTML_INDEX ) && checkHtmlFile( HTML_DEVICE ) && checkHtmlFile( HTML_CONFIG ) && checkHtmlFile( HTML_ABOUT ) ) {
        Log.notice(F("WEB : All html files exist, starting in normal mode." CR));

        server.serveStatic("/", LittleFS, "/index.min.htm" );
        server.serveStatic("/index.htm", LittleFS, "/index.min.htm" );
        server.serveStatic("/device.htm", LittleFS, "/device.min.htm" );
        server.serveStatic("/config.htm", LittleFS, "/config.min.htm" );
        server.serveStatic("/about.htm", LittleFS, "/about.min.htm" );

        // Also add the static upload view in case we we have issues that needs to be fixed.
        server.on("/upload.htm",[]() {
            server.send_P(200, "text/html", (const char*) gUploadHtmData, gUploadHtmSize );
        } );
    } else {
        Log.error(F("WEB : Missing html files, starting with upload UI." CR));

        server.on("/",[]() {
            server.send_P(200, "text/html", (const char*) gUploadHtmData, gUploadHtmSize );
        } );
    }
#endif

    // Dynamic content
    server.on("/api/config", HTTP_GET, webHandleConfig);              // Get config.json
    server.on("/api/device", HTTP_GET, webHandleDevice);              // Get device.json
    server.on("/api/calibrate", HTTP_POST, webHandleCalibrate);       // Run calibration routine (param id)
    server.on("/api/factory", HTTP_GET, webHandleFactoryReset);       // Reset the device
    server.on("/api/status", HTTP_GET, webHandleStatus);              // Get the status.json
    server.on("/api/clearwifi", HTTP_GET, webHandleClearWIFI);        // Clear wifi settings
    server.on("/api/upload", HTTP_GET, webHandleUpload);              // Get upload.json

    server.on("/api/upload", HTTP_POST, [](){ server.send(200); }, webHandleUploadFile);        // File upload data
    server.on("/api/status/sleepmode", HTTP_POST, webHandleStatusSleepmode);                    // Change sleep mode 
    server.on("/api/config/device", HTTP_POST, webHandleConfigDevice);                          // Change device settings
    server.on("/api/config/push", HTTP_POST, webHandleConfigPush);                              // Change push settings
    server.on("/api/config/gravity", HTTP_POST, webHandleConfigGravity);                        // Change gravity settings
    server.on("/api/config/hardware", HTTP_POST, webHandleConfigHardware);                      // Change hardware settings

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