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

#ifdef WITH_RTC_PCF8523 // should be defined as build flag in platformio.ini
    #include "RTClib.h"
    RTC_PCF8523 rtc;
#endif

#ifdef WITH_SD_CARD // should be defined as build flag in platformio.ini

#endif

#ifdef HELLO_WORLD  // should be defined as build flag in platformio.ini
    // a simple hello world feature
    #include "feature/BotHelloWorld.hpp"
    static BotHelloWorld hello;
#endif

#ifdef HAS_WEB_SERVER  // should be defined as build flag in platformio.ini
    // the web server feature
    #include "feature/BotWebServer.hpp"
    static BotWebServer webServer;
#endif

#ifdef HAS_CHASSIS  // should be defined as build flag in platformio.ini
    // the chassis feature
    #include "feature/BotChassis.hpp"
    static BotChassis chassis;
#endif

#ifdef HAS_CONTROL  // should be defined as build flag in platformio.ini
    // the control feature
    #include "feature/BotControl.hpp"
    static BotControl control;
#endif

// namespace to keep callbacks local
// implementation below
namespace _NodeBot
{
    void newConnectionCallback( uint32_t nodeId );
    void changedConnectionCallback();
    void nodeTimeAdjustedCallback( int32_t offset );
    void nodeDelayReceivedCallback( uint32_t nodeId, int32_t delay );
}

class NodeBot
{ 
    public:
        NodeBot()
        {

        };

        void setup() {
            //Serial.println("bot setup begin");

            //ERROR | STARTUP | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE
            mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

            // Create mesh object with mode WIFI_AP_STA = Station and AccessPoint 
            mesh.init( MESH_SSID, MESH_PASSWORD, &defaultScheduler, MESH_PORT, WIFI_AP_STA, MESH_CHANNEL );

            // as the name implies: this will be a root. There should only be one!
            #ifdef IS_MESH_ROOT
                mesh.setRoot( true );
            #endif

            // A node should ideally know the mesh contains a root
            // If no root is present, restructuring might slow down, but still should work
            // So call this on all nodes
            mesh.setContainsRoot();

            // enable OTA if a role is defined (should be done as build flag)
            #ifdef OTA_ROLE
                mesh.initOTAReceive( OTA_ROLE );
            #endif

            #ifdef WITH_RTC
                rtc.begin();
                // adjust time from compiletime if not already set
                if ( !rtc.initialized() || rtc.lostPower() )
                {
                    rtc.adjust( DateTime( F( __DATE__ ), F( __TIME__ ) ) );
                    Serial.println( "Time adjusted" );
                }
                rtc.start();
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

            #ifdef HAS_WEB_SERVER
                webServer.startWebServer();
            #endif

            #ifdef WITH_RTC
                DateTime now = rtc.now();
                Serial.print(" since 2000 = ");
                Serial.print(now.unixtime());
                Serial.print("s = ");
                Serial.print(now.unixtime() / 86400L);
                Serial.println("d");
            #endif
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