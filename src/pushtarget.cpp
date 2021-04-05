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
    unsigned long interval = myConfig.getSleepInterval()*1000;

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

    if( myConfig.isBrewfatherActive() ) {
        LOG_PERF_START("push-brewfather");
        sendBrewfather( angle,  gravity, temp );
        LOG_PERF_STOP("push-brewfather");
    }

    if( myConfig.isHttpActive() ) {
        LOG_PERF_START("push-http");
        sendHttp( myConfig.getHttpPushUrl(), angle, gravity, temp, runTime );
        LOG_PERF_STOP("push-http");
    }

    if( myConfig.isHttpActive2() ) {
        LOG_PERF_START("push-http2");
        sendHttp( myConfig.getHttpPushUrl2(), angle, gravity, temp, runTime );
        LOG_PERF_STOP("push-http2");
    }

    if( myConfig.isInfluxDb2Active() ) {
        LOG_PERF_START("push-influxdb2");
        sendInfluxDb2( angle, gravity, temp, runTime );
        LOG_PERF_STOP("push-influxdb2");
    }
}

//
// Send to influx db v2
//
void PushTarget::sendInfluxDb2(float angle, float gravity, float temp, float runTime) {
    Log.notice(F("PUSH: Sending values to influxdb2 angle=%F, gravity=%F, temp=%F." CR), angle, gravity, temp );

    WiFiClient client;
    HTTPClient http;
    String serverPath = String(myConfig.getInfluxDb2PushUrl()) + "/api/v2/write?org=" + 
                        String(myConfig.getInfluxDb2PushOrg()) + "&bucket=" + 
                        String(myConfig.getInfluxDb2PushBucket());

    http.begin( client, serverPath);

    // Create body for influxdb2
    char buf[1024];
    sprintf( &buf[0], "gravity,host=%s,device=%s,format=%s value=%.4f\n"
                    "angle,host=%s,device=%s value=%.2f\n"
                    "temp,host=%s,device=%s,format=%c value=%.1f\n"
                    "battery,host=%s,device=%s value=%.2f\n"
                    "rssi,host=%s,device=%s value=%d\n",
                    // TODO: Add support for plato format
                    myConfig.getMDNS(), myConfig.getID(), "SG", gravity,
                    myConfig.getMDNS(), myConfig.getID(), angle,
                    myConfig.getMDNS(), myConfig.getID(), myConfig.getTempFormat(), temp,
                    myConfig.getMDNS(), myConfig.getID(), myBatteryVoltage.getVoltage(),
                    myConfig.getMDNS(), myConfig.getID(),  WiFi.RSSI() );

#if LOG_LEVEL==6
    Log.verbose(F("PUSH: url %s." CR), serverPath.c_str());
    Log.verbose(F("PUSH: data %s." CR), &buf[0] );
#endif

    // Send HTTP POST request
    String auth = "Token " + String( myConfig.getInfluxDb2PushToken() );
    http.addHeader(F("Authorization"), auth.c_str() );
    int httpResponseCode = http.POST(&buf[0]);

    if (httpResponseCode==204) {
        Log.notice(F("PUSH: InfluxDB2 push successful, response=%d" CR), httpResponseCode);
    } else {
        Log.error(F("PUSH: InfluxDB2 push failed, response=%d" CR), httpResponseCode);
    }

    http.end();

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
    doc["temp_unit"]     = String( myConfig.getTempFormat() ); 
    doc["battery"]       = reduceFloatPrecision( myBatteryVoltage.getVoltage(), 2 ); 
    // TODO: Add support for plato format
    doc["gravity"]       = reduceFloatPrecision( gravity, 4 );
    doc["gravity_unit"]  = myConfig.isGravitySG()?"G":"P";

    WiFiClient client;
    HTTPClient http;
    String serverPath = myConfig.getBrewfatherPushUrl();

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
        Log.notice(F("PUSH: Brewfather push successful, response=%d" CR), httpResponseCode);
    } else {
        Log.error(F("PUSH: Brewfather push failed, response=%d" CR), httpResponseCode);
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
    doc["name"]          = myConfig.getMDNS();
    doc["ID"]            = myConfig.getID();
    doc["token"]         = "gravmon";
    doc["interval"]      = myConfig.getSleepInterval();
    doc["temperature"]   = reduceFloatPrecision( temp, 1 );
    doc["temp-units"]    = String( myConfig.getTempFormat() ); 
    // TODO: Add support for plato format
    doc["gravity"]       = reduceFloatPrecision( gravity, 4 );
    doc["angle"]         = reduceFloatPrecision( angle, 2);
    doc["battery"]       = reduceFloatPrecision( myBatteryVoltage.getVoltage(), 2 );
    doc["rssi"]          = WiFi.RSSI(); 

    // Some additional information
    doc["gravity-units"] = "SG";
    doc["run-time"]      = reduceFloatPrecision( runTime, 2 );

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
        Log.notice(F("PUSH: HTTP push successful, response=%d" CR), httpResponseCode);
    } else {
        Log.error(F("PUSH: HTTP push failed, response=%d" CR), httpResponseCode);
    }

    http.end();
}

// EOF 