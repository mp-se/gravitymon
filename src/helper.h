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
// Use these to collect performance data from various parts of the code
#define LOG_PERF_START(s)   myPerfLogging.start(s)
#define LOG_PERF_STOP(s)    myPerfLogging.stop(s)
#define LOG_PERF_PRINT()    myPerfLogging.print()
#define LOG_PERF_CLEAR()    myPerfLogging.clear()
#else
// These will disable the performance collection
#define LOG_PERF_START(s)   
#define LOG_PERF_STOP(s)    
#define LOG_PERF_PRINT()    
#define LOG_PERF_CLEAR()    
#endif

class PerfLogging {
    private:
        struct PerfEntry {
            unsigned long start;        // millis()
            unsigned long end;          // millis() 
            unsigned long max;          // max time in ms
            const char*   key;          // measurement
            PerfEntry*    next;         //
        };

        PerfEntry* first = 0;

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
        //
        // Clear the current cache
        //
        void clear() { 
            if( first == 0 )
                return;

            PerfEntry* pe = first;

            do {
                PerfEntry* p = pe;
                pe = pe->next;
                delete p;
            } while( pe != 0 );

            first = 0;
        }

        //
        // Start measuring this performance point
        //
        void start( const char* key ) { 
            PerfEntry* pe = add( key );
            pe->start = millis(); 
        }

        //
        // Finalize measuring of this performance point
        //
        void stop( const char* key ) { 
            PerfEntry* pe = find( key );

            if( pe != 0 ) {
                pe->end = millis(); 

                unsigned long t = pe->end - pe->start;
                
                if( t > pe->max )
                    pe->max = t;
            }
        }

        //
        // Print the collected performance data
        //
        void print() { 
            PerfEntry* pe = first;

            while( pe != 0 ) {
                //Log.notice( F("PERF: %s=%l ms (%l, %l)" CR), pe->key, (pe->end - pe->start), pe->start, pe->end );                
                Log.notice( F("PERF: %s %lms" CR), pe->key, pe->max );                
                pe = pe->next;
            }
        }
};

// Global instance created
extern SerialDebug mySerial;
extern PerfLogging myPerfLogging;
extern BatteryVoltage myBatteryVoltage;

#endif // _HELPER_H

// EOF