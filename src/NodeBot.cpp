/**
 * 
 */
#include <Arduino.h>
#include <logging.h>

// defines for the MESH SSID etc. to use
#include <secrets.h>

#include "NodeBot.h"
// a specialization of painlessMesh to implement extensions
#include "BotMesh.h"

// to control tasks
static Scheduler defaultScheduler;

// rtc stuff
#ifdef WITH_RTC_PCF8523
    #ifndef WITH_RTC
        #define WITH_RTC
    #endif
    #include "RTClib.h"
    static RTC_PCF8523 rtc;
#endif
#ifdef WITH_RTC_DS1307
    #ifndef WITH_RTC
        #define WITH_RTC
    #endif
    #include "RTClib.h"
    static RTC_DS1307 rtc;
#endif


#ifdef WITH_SD_CARD
    #include <FS.h>
    #include <SD.h>
    #include <SPI.h>
#endif

#ifdef HELLO_WORLD
    // a simple hello world feature
    #include "features/BotHelloWorld.hpp"
    static BotHelloWorld hello;
#endif

#ifdef HAS_WEB_SERVER
    #ifndef HAS_INTERNET_ACCESS
        #define HAS_INTERNET_ACCESS
    #endif
    // the web server feature
    #include "features/BotWebServer.hpp"
    static BotWebServer webServer;
#endif

#ifdef HAS_CHASSIS
    // the chassis feature
    #include "features/BotChassis.hpp"
    static BotChassis chassis;
#endif

#ifdef HAS_CONTROL
    // the control feature
    #include "features/BotControl.hpp"
    static BotControl control;
#endif

#ifdef HAS_INTERNET_ACCESS
    static const char * ntpServer = "pool.ntp.org";
#endif

static timezone tz = { 3600, 3600 };
#ifdef WITH_RTC
    static timeval tv = { 0, 0 };
#endif
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

    // Task definitions
    #ifdef SERIAL_DEBUG
        void serialPrint();
        Task taskSerialPrint( 1000UL * 60UL * 10UL, TASK_FOREVER, &serialPrint );
    #endif
    #ifdef HAS_INTERNET_ACCESS
        void getNtpTime();
        Task taskGetNtpTime( 1000UL * 3600UL * 24UL * 5UL, TASK_FOREVER, &getNtpTime );
    #endif
}

void NodeBot::setup()
{
    SERIAL_PRINTLN("bot setup begin");

    // set timezone values
    configTime(tz.tz_minuteswest, tz.tz_dsttime, nullptr );

    #ifdef WITH_RTC
        rtc.begin();
        // adjust time from compiletime if not already set
        #ifdef WITH_RTC_PCF8523
            if ( !rtc.initialized() || rtc.lostPower() )
        #elif defined(WITH_RTC_DS1307)
            if ( !rtc.isrunning() )
        #else
            #error "unknown RTC"
        #endif
        {
            rtc.adjust( DateTime( F( __DATE__ ), F( __TIME__ ) ) );
            SERIAL_PRINTLN( "RTC Time adjusted" );
            #ifdef WITH_RTC_PCF8523
                rtc.start();
            #endif
        }
        #ifdef SERIAL_DEBUG
            _NodeBot::serialPrintRtcDateTime();
        #endif
        // set system time from rtc
        tv.tv_sec = rtc.now().unixtime();
        settimeofday( &tv, &tz );
    #endif
    #ifdef SERIAL_DEBUG
        _NodeBot::serialPrintDateTime();
    #endif
    // set before init() so that you can see startup messages
    // ERROR | STARTUP | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE
    #ifdef SERIAL_DEBUG
        BotMesh::getInstance().setDebugMsgTypes( ERROR );
    #endif

    // Create mesh object with mode WIFI_AP_STA = Station and AccessPoint 
    BotMesh::getInstance().init( MESH_SSID, MESH_PASSWORD, &defaultScheduler, MESH_PORT, WIFI_AP_STA, MESH_CHANNEL );

    // as the name implies: this will be a root. There should only be one in your mesh!
    #ifdef IS_MESH_ROOT
        BotMesh::getInstance().setRoot( true );
    #endif

    // A node should ideally know the mesh contains a root
    // If no root is present, restructuring might slow down, but still should work
    // So call this on all nodes regardless we will have a root or not.
    BotMesh::getInstance().setContainsRoot();

    // enable OTA if a role is defined (should be done as build flag)
    #ifdef OTA_ROLE
        BotMesh::getInstance().initOTAReceive( OTA_ROLE );
    #endif

    BotMesh::getInstance().onNewConnection( &_NodeBot::newConnectionCallback );
    BotMesh::getInstance().onChangedConnections( &_NodeBot::changedConnectionCallback );
    BotMesh::getInstance().onNodeTimeAdjusted( &_NodeBot::nodeTimeAdjustedCallback );
    BotMesh::getInstance().onNodeDelayReceived( &_NodeBot::nodeDelayReceivedCallback );

    #ifdef HELLO_WORLD
        hello.setup( defaultScheduler );
    #endif
    
    #ifdef HAS_WEB_SERVER
        webServer.setup( defaultScheduler );
    #endif

    #ifdef HAS_CHASSIS
        chassis.setup( defaultScheduler );
    #endif

    #ifdef HAS_CONTROL
        control.setup( defaultScheduler );
    #endif

    // add Tasks
    #ifdef SERIAL_DEBUG
        defaultScheduler.addTask( _NodeBot::taskSerialPrint );
        _NodeBot::taskSerialPrint.enableDelayed( 30000UL );
    #endif
    #ifdef HAS_INTERNET_ACCESS
        defaultScheduler.addTask( _NodeBot::taskGetNtpTime );
        _NodeBot::taskGetNtpTime.enableDelayed( 90000UL );
    #endif

    SERIAL_PRINTLN( "MESH IP is " + BotMesh::getInstance().getAPIP().toString() );

    SERIAL_PRINTLN("bot setup end");
};

void NodeBot::update()
{
    // this will run the schedulers as well
    BotMesh::getInstance().update();
};

// namespace implementation
namespace _NodeBot
{
    #ifdef SERIAL_DEBUG
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
    #endif

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

    #ifdef SERIAL_DEBUG
        void serialPrintDateTime()
        {
            // set system time from rtc
            #ifdef WITH_RTC
                Serial.println( rtc.now().unixtime() );
                tv.tv_sec = rtc.now().unixtime();
                settimeofday( &tv, &tz );
            #endif
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

        void serialPrint()
        {
            #ifdef WITH_RTC
                serialPrintRtcDateTime();
            #endif
            serialPrintDateTime();
        }
    #endif

    #ifdef HAS_INTERNET_ACCESS
        void getNtpTime()
        {
            SERIAL_PRINTLN( "NTP-Request" );
            // update time from ntp server
            configTime(tz.tz_minuteswest, tz.tz_dsttime, ntpServer);
            #ifdef WITH_RTC
                rtc.adjust( DateTime( time( nullptr ) ) );
                #ifdef SERIAL_DEBUG
                    serialPrintRtcDateTime();
                #endif
            #endif
            #ifdef SERIAL_DEBUG
                serialPrintDateTime();
            #endif
        }
    #endif

    void newConnectionCallback( uint32_t nodeId )
    {
        SERIAL_PRINT( "New Connection, nodeId = " );
        SERIAL_PRINTLN( nodeId );
    };

    void changedConnectionCallback()
    {
        SERIAL_PRINTLN( "Changed Connections" );
    };

    void nodeTimeAdjustedCallback( int32_t offset )
    {
        SERIAL_PRINT( "Adjusted time " );
        SERIAL_PRINT( BotMesh::getInstance().getNodeTime() )
        SERIAL_PRINT( " Offset = " );
        SERIAL_PRINTLN( offset ) ;
    };

    void nodeDelayReceivedCallback( uint32_t nodeId, int32_t delay )
    {
        SERIAL_PRINT( "Delay from node: " );
        SERIAL_PRINT( nodeId )
        SERIAL_PRINT( " delay =  " );
        SERIAL_PRINTLN( delay ) ;
    };
}
