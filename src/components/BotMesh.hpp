#ifndef _BotMesh_hpp_
#define _BotMesh_hpp_

//#define _TASK_OO_CALLBACKS

/**
 * 
 */
#include <Arduino.h>

#include"patterns.h"

// defines for the MESH SSID etc. to use
#include "secrets.h"

#include <painlessMesh.h>
using namespace painlessmesh;

// macro to shorten lines
#define MESH BotMesh::getInstance()

// namespace to keep callbacks local
// implementation below
namespace BotMeshCallbacks
{
    void newConnectionCallback( uint32_t nodeId );
    void changedConnectionCallback();
    void nodeTimeAdjustedCallback( int32_t offset );
    void nodeDelayReceivedCallback( uint32_t nodeId, int32_t delay );
}

/**
 * BotMesh is an extension to painlessMesh 
 * @see https://gitlab.com/painlessMesh/painlessMesh .
 */
using namespace BotMeshCallbacks;
class BotMesh : public painlessMesh
{
    MAKE_SINGLETON(BotMesh)

    public:
        void setup()
        {
            //ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE
            setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

            // Create mesh object with mode WIFI_AP_STA = Station and AccessPoint 
            MESH.init( MESH_SSID, MESH_PASSWORD, &defaultScheduler, MESH_PORT, WIFI_AP_STA, MESH_CHANNEL );

            //#if defined(IS_ROOT) || defined(HAS_WEB_SERVER)
            #ifdef IS_ROOT
            // as the name implies: this is the root. There should only be one!
            MESH.setRoot( true );
            #endif

            // A node should ideally know the mesh contains a root
            // If no root is present, restructuring might slow down, but still should work
            // So call this on all nodes
            MESH.setContainsRoot();

            // enable OTA if a role is defined (should be done as build flag)
            #ifdef OTA_ROLE
            MESH.initOTAReceive( OTA_ROLE );
            #endif

            MESH.onNewConnection( &newConnectionCallback );
            MESH.onChangedConnections( &changedConnectionCallback );
            //MESH.onNodeTimeAdjusted( &nodeTimeAdjustedCallback );
            //MESH.onNodeDelayReceived( &nodeDelayReceivedCallback );
        };

        using painlessMesh::update;
        inline void update()
        {
            // this will also run the defaultScheduler
            return painlessMesh::update();
        };

        Scheduler & getDefaultSCheduler()
        {
            return defaultScheduler;
        };

    protected:

    private:
        Scheduler defaultScheduler;     // to control tasks

};

// namespace to keep callbacks local
namespace BotMeshCallbacks
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
        Serial.printf( "Adjusted time %u. Offset = %d\n", MESH.getNodeTime(), offset) ;
    };

    void nodeDelayReceivedCallback( uint32_t nodeId, int32_t delay )
    {
        Serial.printf( "Delay from node:%u delay = %d\n", nodeId, delay );
    };
}

#endif