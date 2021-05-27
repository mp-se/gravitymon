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
#ifndef _HELPER_H
#define _HELPER_H

// Includes
#include <ArduinoLog.h>

// Sleep mode
void deepSleep(int t);

// Show build options
void printBuildOptions();

// Float to String
char* convertFloatToString( float f, char* buf, int dec = 2);
float reduceFloatPrecision( float f, int dec = 2 );

// Logging via serial
void printTimestamp(Print* _logOutput);
void printNewline(Print* _logOutput);
void printHeap();

// Classes
class SerialDebug {
    public:
        SerialDebug(const long serialSpeed = 115200L);
        static Logging* getLog() { return &Log; };
};

class BatteryVoltage {
    private:
        float batteryLevel;

    public:
        void  read();
        float getVoltage() { return batteryLevel; };
};

#if defined( COLLECT_PERFDATA )

class PerfLogging {
    private:
        struct PerfEntry {
            unsigned long start;        // millis()
            unsigned long end;          // millis() 
            unsigned long max;          // max time in ms
            const char*   key;          // measurement

            PerfEntry*    next;         // Next in the linked list

            float         mA;           // Power consumption
            float         V;            // Power consumption
        };

        PerfEntry* first = 0;
        bool measurePower = false;

        PerfEntry* find( const char* k ) {
            if( first == 0 )
                return 0;

            PerfEntry* pe = first;

            while( strcmp( k, pe->key ) != 0 ) {
                if( pe->next == 0 )
                    return 0;
                
                pe = pe->next;
            }
            return pe;
        };

        PerfEntry* add( const char* k ) {
            if( first == 0 ) {
                first = new PerfEntry();
                first->key = k;
                first->next = 0;
                first->max = 0;
                return first;
            }

            PerfEntry* pe = first;

            while( strcmp( k, pe->key ) != 0 ) {
                if( pe->next == 0 ) {
                    pe->next = new PerfEntry();
                    pe->next->key = k;
                    pe->next->max = 0;
                    pe->next->next = 0;
                    return pe->next;
                }
                
                pe = pe->next;
            }

            return pe;
        };

    public:
        void clear();
        void start( const char* key );
        void stop( const char* key );
        void print();
        void pushInflux(); 
};

extern PerfLogging myPerfLogging;

// Use these to collect performance data from various parts of the code
#define LOG_PERF_START(s)   myPerfLogging.start(s)
#define LOG_PERF_STOP(s)    myPerfLogging.stop(s)
//#define LOG_PERF_PRINT()    myPerfLogging.print()
#define LOG_PERF_PRINT()    
#define LOG_PERF_CLEAR()    myPerfLogging.clear()
#define LOG_PERF_PUSH()     myPerfLogging.pushInflux()

#else

// These will disable the performance collection function
#define LOG_PERF_START(s)   
#define LOG_PERF_STOP(s)    
#define LOG_PERF_PRINT()    
#define LOG_PERF_CLEAR()    
#define LOG_PERF_PUSH()    

#endif  // COLLECT_PERFDATA

// Global instance created
extern SerialDebug mySerial;
extern BatteryVoltage myBatteryVoltage;

#endif // _HELPER_H

// EOF