#ifndef _NodeBot_hpp_
#define _NodeBot_hpp_

/**
 * 
 */
#include <Arduino.h>

// defines for the MESH SSID etc. to use
#include "secrets.h"

// a specialization of painlessMesh to implement extensions
#include "components/BotMesh.hpp"

// to control tasks
static Scheduler defaultScheduler;

// access to mesh
//static BotMesh & mesh = BotMesh::getInstance();
static BotMesh mesh;

#ifdef HELLO_WORLD  // should be defined as build flag in platformio.ini
// a simple hello world component
#include "components/BotHelloWorld.hpp"
static BotHelloWorld hello;
#endif

#ifdef HAS_WEB_SERVER  // should be defined as build flag in platformio.ini
// the web server component
#include "components/BotWebServer.hpp"
static BotWebServer webServer;
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
            Serial.println("bot setup begin");

            //ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE
            mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

            // Create mesh object with mode WIFI_AP_STA = Station and AccessPoint 
            mesh.init( MESH_SSID, MESH_PASSWORD, &defaultScheduler, MESH_PORT, WIFI_AP_STA, MESH_CHANNEL );

            //#if defined(IS_ROOT) || defined(HAS_WEB_SERVER)
            #ifdef IS_ROOT
            // as the name implies: this is the root. There should only be one!
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

            mesh.onNewConnection( &_NodeBot::newConnectionCallback );
            mesh.onChangedConnections( &_NodeBot::changedConnectionCallback );
            mesh.onNodeTimeAdjusted( &_NodeBot::nodeTimeAdjustedCallback );
            mesh.onNodeDelayReceived( &_NodeBot::nodeDelayReceivedCallback );

            #ifdef HELLO_WORLD  // should be defined as build flag in platformio.ini
            hello.setup( mesh, defaultScheduler );
            #endif
            
            #ifdef HAS_WEB_SERVER  // should be defined as build flag in platformio.ini
            // setup BotWebServer
            webServer.setup( mesh, defaultScheduler );
            #endif

            #ifdef HAS_WEB_SERVER  // should be defined as build flag in platformio.ini
            // start BotWebServer
            webServer.startWebServer();
            #endif

            Serial.println("bot setup end");
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