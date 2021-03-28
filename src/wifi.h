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
#ifndef _WIFI_H
#define _WIFI_H

// Include 
#include <ESP8266WiFi.h>

// classes
class Wifi {
    private:
        // WIFI
        bool      connectedFlag = false;

        // OTA
        bool newFirmware = false;
        bool parseFirmwareVersionString( int (&num)[3], const char *version );
        void downloadFile(const char *fname);

    public:
        // WIFI
        bool   connect( bool showPortal = false );
        bool   disconnect();
        bool   isConnected() { return connectedFlag; };
        String getIPAddress() { return WiFi.localIP().toString(); };

        // OTA
        bool updateFirmware();
        bool checkFirmwareVersion();
};

// Global instance created
extern Wifi myWifi;

#endif // _WIFI_H

// EOF