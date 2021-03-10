#ifndef _NodeBot_hpp_
#define _NodeBot_hpp_

/**
 * 
 */
#include <Arduino.h>

// defines for the MESH SSID etc. to use
#include "secrets.h"

// a specialization of painlessMesh to implement extensions
#include "feature/BotMesh.hpp"
static BotMesh mesh;

// to control tasks
static Scheduler defaultScheduler;

// rtc stuff
#ifdef WITH_RTC_PCF8523
    #ifndef WITH_RTC
        #define WITH_RTC
    #endif
    #include "RTClib.h"
    RTC_PCF8523 rtc;
#endif
#ifdef WITH_RTC_DS1307
    #ifndef WITH_RTC
        #define WITH_RTC
    #endif
    #include "RTClib.h"
    RTC_DS1307 rtc;
#endif


#ifdef WITH_SD_CARD
    #include <FS.h>
    #include <SD.h>
    #include <SPI.h>
#endif

#ifdef HELLO_WORLD
    // a simple hello world feature
    #include "feature/BotHelloWorld.hpp"
    static BotHelloWorld hello;
#endif

#ifdef HAS_WEB_SERVER
    #ifndef HAS_INTERNET_ACCESS
        #define HAS_INTERNET_ACCESS
    #endif
    // the web server feature
    #include "feature/BotWebServer.hpp"
    static BotWebServer webServer;
#endif

#ifdef HAS_CHASSIS
    // the chassis feature
    #include "feature/BotChassis.hpp"
    static BotChassis chassis;
#endif

#ifdef HAS_CONTROL
    // the control feature
    #include "feature/BotControl.hpp"
    static BotControl control;
#endif

#ifdef HAS_INTERNET_ACCESS
    const char * ntpServer = "pool.ntp.org";
#endif

timezone tz = { 3600,    0 };
timeval  tv = {    0,    0 };

// namespace to keep callbacks local
// implementation below
namespace _NodeBot
{
    void serialPrintDateTime();
    #ifdef WITH_RTC
        void serialPrintRtcDateTime();
    #endif
    void newConnectionCallback( uint32_t nodeId );
    void changedConnectionCallback();
    void nodeTimeAdjustedCallback( int32_t offset );
    void nodeDelayReceivedCallback( uint32_t nodeId, int32_t delay );

    #ifdef HAS_INTERNET_ACCESS
        void getNtpTime();
        // Task definitions
        Task taskGetNtpTime( 1000UL * 3600UL * 24UL * 10UL , TASK_FOREVER, &getNtpTime );
    #endif
}

class NodeBot
{ 
    public:
        NodeBot()
        {

        };

        void setup()
        {
            //Serial.println("bot setup begin");

            #ifdef WITH_RTC
                rtc.begin();
                // adjust time from compiletime if not already set
                #ifdef WITH_RTC_PCF8523
                    if ( !rtc.initialized() || rtc.lostPower() )
                    {
                        rtc.adjust( DateTime( F( __DATE__ ), F( __TIME__ ) ) );
                        Serial.println( "RTC Time adjusted" );
                        rtc.start();
                    }
                #elif defined(WITH_RTC_DS1307)
                    if ( !rtc.isrunning() )
                    {
                        rtc.adjust( DateTime( F( __DATE__ ), F( __TIME__ ) ) );
                        Serial.println( "RTC Time adjusted" );
                    }
                #else
                    #error "unknown RTC"
                #endif

                _NodeBot::serialPrintRtcDateTime();

                // set system time from rtc
                tv.tv_sec = rtc.now().unixtime();
                settimeofday( &tv, &tz );
            #endif
            
            configTime( tz.tz_minuteswest, tz.tz_dsttime, (const char *)NULL );
            _NodeBot::serialPrintDateTime();

            // set before init() so that you can see startup messages
            // ERROR | STARTUP | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE
            mesh.setDebugMsgTypes( ERROR );

            // Create mesh object with mode WIFI_AP_STA = Station and AccessPoint 
            mesh.init( MESH_SSID, MESH_PASSWORD, &defaultScheduler, MESH_PORT, WIFI_AP_STA, MESH_CHANNEL );

            // as the name implies: this will be a root. There should only be one!
            #ifdef IS_MESH_ROOT
                mesh.setRoot( true );
            #endif

            // A node should ideally know the mesh contains a root
            // If no root is present, restructuring might slow down, but still should work
            // So call this on all nodes regardless we will have a root or not.
            mesh.setContainsRoot();

            // enable OTA if a role is defined (should be done as build flag)
            #ifdef OTA_ROLE
                mesh.initOTAReceive( OTA_ROLE );
            #endif

            mesh.onNewConnection( &_NodeBot::newConnectionCallback );
            mesh.onChangedConnections( &_NodeBot::changedConnectionCallback );
            mesh.onNodeTimeAdjusted( &_NodeBot::nodeTimeAdjustedCallback );
            mesh.onNodeDelayReceived( &_NodeBot::nodeDelayReceivedCallback );

            #ifdef HELLO_WORLD
                hello.setup( mesh, defaultScheduler );
            #endif
            
            #ifdef HAS_WEB_SERVER
                webServer.setup( mesh, defaultScheduler );
            #endif

            #ifdef HAS_CHASSIS
                chassis.setup( mesh, defaultScheduler );
            #endif

            #ifdef HAS_CONTROL
                control.setup( mesh, defaultScheduler );
            #endif

            // add Tasks
            #ifdef HAS_INTERNET_ACCESS
                defaultScheduler.addTask( _NodeBot::taskGetNtpTime );
                _NodeBot::taskGetNtpTime.enableDelayed( 60000UL );
            #endif

            Serial.println( "MESH IP is " + mesh.getAPIP().toString() );

            //Serial.println("bot setup end");
        };

        void update()
        {
            // this will run the schedulers as well
            mesh.update();
        };

    protected:

    private:
};

// namespace to keep callbacks local
namespace _NodeBot
{
    size_t _print( struct tm * timeinfo, const char * format )
    {
        char buf[64];
        size_t written = strftime( buf, 63, format, timeinfo );
        if(written == 0)  return written;
        return Serial.print(buf);
    }

    size_t _println( struct tm * timeinfo, const char * format )
    {
        size_t n = _print( timeinfo, format );
        n += Serial.println();
        return n;
    }

    bool _getLocalTime( struct tm * info, uint32_t ms = 5000 )
    {
        uint32_t start = millis();
        time_t now;
        while((millis()-start) <= ms)
        {
            time(&now);
            localtime_r(&now, info);
            if(info->tm_year > (2016 - 1900)) return true;
            delay(10);
        }
        return false;
    }

    void serialPrintDateTime()
    {
        struct tm timeinfo;
        _getLocalTime( &timeinfo );
        _println( &timeinfo, "%A, %B %d %Y %H:%M:%S" );
    }

    #ifdef WITH_RTC
        void serialPrintRtcDateTime()
        {
            Serial.print("RTC: ");

            DateTime now = rtc.now();
        
            Serial.print(now.year(), DEC);
            Serial.print('-');
            Serial.print(now.month(), DEC);
            Serial.print('-');
            Serial.print(now.day(), DEC);
            Serial.print(' ');
            Serial.print(now.hour(), DEC);
            Serial.print(':');
            Serial.print(now.minute(), DEC);
            Serial.print(':');
            Serial.print(now.second(), DEC);
            Serial.println();
        }
    #endif

    #ifdef HAS_INTERNET_ACCESS
        void getNtpTime()
        {
            // update time from ntp server
            configTime(tz.tz_minuteswest, tz.tz_dsttime, ntpServer);
            #ifdef WITH_RTC
                rtc.adjust( DateTime( time( nullptr ) ) );
                serialPrintRtcDateTime();
            #endif
            serialPrintDateTime();
        }
    #endif

    void newConnectionCallback( uint32_t nodeId )
    {
        Serial.printf( "New Connection, nodeId = %u\n", nodeId );
    };

    void changedConnectionCallback()
    {
        Serial.printf( "Changed Connections\n" );
    };

    void nodeTimeAdjustedCallback( int32_t offset )
    {
        Serial.printf( "Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset) ;
    };

    void nodeDelayReceivedCallback( uint32_t nodeId, int32_t delay )
    {
        Serial.printf( "Delay from node:%u delay = %d\n", nodeId, delay );
    };
}

#endif