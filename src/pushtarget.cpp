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
#include "pushtarget.h"
#include "config.h"

PushTarget myPushTarget;

//
// Send the pressure value
//
void PushTarget::send(float angle, float gravity, float temp, float runTime, bool force ) {
    unsigned long timePassed = abs( millis() - ms );
    unsigned long interval = myConfig.getPushInterval()*1000;

    if( ( timePassed < interval ) && !force) {
#if LOG_LEVEL==6
      Log.verbose(F("PUSH: Timer has not expired %l vs %l." CR), timePassed, interval );
#endif
        return;
    }

#if LOG_LEVEL==6
    Log.verbose(F("PUSH: Sending data." CR) );
#endif
    ms = millis();

    if( myConfig.isBrewfatherActive() )
       sendBrewfather( angle,  gravity, temp );

    if( myConfig.isHttpActive() )
       sendHttp( myConfig.getHttpPushTarget(), angle, gravity, temp, runTime );

    if( myConfig.isHttpActive2() )
       sendHttp( myConfig.getHttpPushTarget2(), angle, gravity, temp, runTime );
}

//
// Send data to brewfather
//
void PushTarget::sendBrewfather(float angle, float gravity, float temp ) {
    Log.notice(F("PUSH: Sending values to brewfather angle=%F, gravity=%F, temp=%F." CR), angle, gravity, temp );

    DynamicJsonDocument doc(300);
    //
    //  {
    //  "name": "YourDeviceName", // Required field, this will be the ID in Brewfather
    //  "temp": 20.32,
    //  "aux_temp": 15.61, // Fridge Temp
    //  "ext_temp": 6.51, // Room Temp
    //  "temp_unit": "C", // C, F, K
    //  "gravity": 1.042,
    //  "gravity_unit": "G", // G, P
    //  "pressure": 10,
    //  "pressure_unit": "PSI", // PSI, BAR, KPA
    //  "ph": 4.12,
    //  "bpm": 123, // Bubbles Per Minute
    //  "comment": "Hello World",
    //  "beer": "Pale Ale"
    //  "battery": 4.98
    //  }
    //
    doc["name"]          = myConfig.getMDNS();
    doc["temp"]          = reduceFloatPrecision( temp, 1);
  //doc["aux_temp"]      = 0;
  //doc["ext_temp"]      = 0;
    doc["temp_unit"]     = String( myConfig.getTempFormat() ); 
  //doc["pressure"]      = ; 
  //doc["pressure_unit"] = ;  
    doc["battery"]       = reduceFloatPrecision( myBatteryVoltage.getVoltage(), 2 ); 
    doc["gravity"]       = reduceFloatPrecision( gravity, 4 );
    doc["gravity_unit"]  = myConfig.isGravitySG()?"G":"P";
  //doc["ph"]            = 0;
  //doc["bpm"]           = 0;
  //doc["comment"]       = "";
  //doc["beer"]          = "";

    WiFiClient client;
    HTTPClient http;
    String serverPath = myConfig.getBrewfatherPushTarget();

    // Your Domain name with URL path or IP address with path
    http.begin( client, serverPath);
    String json;
    serializeJson(doc, json);
#if LOG_LEVEL==6
    Log.verbose(F("PUSH: url %s." CR), serverPath.c_str());
    Log.verbose(F("PUSH: json %s." CR), json.c_str());
#endif

    // Send HTTP POST request
    http.addHeader(F("Content-Type"), F("application/json") );
    int httpResponseCode = http.POST(json);

    if (httpResponseCode==200) {
        Log.notice(F("PUSH: HTTP Response code %d" CR), httpResponseCode);
    } else {
        Log.error(F("PUSH: HTTP Response code %d" CR), httpResponseCode);
    }

    http.end();
}

//
// Send data to http target
//
void PushTarget::sendHttp( String serverPath, float angle, float gravity, float temp, float runTime ) {
    Log.notice(F("PUSH: Sending values to http angle=%F, gravity=%F, temp=%F." CR), angle, gravity, temp );

    DynamicJsonDocument doc(256);

    // Use iSpindle format for compatibility
    doc["name"]        = myConfig.getMDNS();
    doc["ID"]          = myConfig.getID();
    doc["token"]       = "gravmon";
    doc["interval"]    = myConfig.getPushInterval();
    doc["temperature"] = reduceFloatPrecision( temp, 1 );
    doc["temp-units"]  = String( myConfig.getTempFormat() ); 
    doc["gravity"]     = reduceFloatPrecision( gravity, 4 );
    doc["angle"]       = reduceFloatPrecision( angle, 2);
    doc["battery"]     = reduceFloatPrecision( myBatteryVoltage.getVoltage(), 2 );
    doc["rssi"]        = WiFi.RSSI(); 

    // Some additional information
    doc["run-time"]  = reduceFloatPrecision( runTime, 2 );

    WiFiClient client;
    HTTPClient http;

    // Your Domain name with URL path or IP address with path
    http.begin( client, serverPath);
    String json;
    serializeJson(doc, json);
#if LOG_LEVEL==6
    Log.verbose(F("PUSH: url %s." CR), serverPath.c_str());
    Log.verbose(F("PUSH: json %s." CR), json.c_str());
#endif

    // Send HTTP POST request
    http.addHeader(F("Content-Type"), F("application/json") );
    int httpResponseCode = http.POST(json);
    
    if (httpResponseCode==200) {
        Log.notice(F("PUSH: HTTP Response code %d" CR), httpResponseCode);
    } else {
        Log.error(F("PUSH: HTTP Response code %d" CR), httpResponseCode);
    }

    http.end();
}

// EOF 