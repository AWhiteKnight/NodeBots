/**
 * 
 */

#include "BotMesh.h"

#include <logging.h>

// namespace to keep callbacks local
// implementation below
namespace _BotMesh
{
    void newConnectionCallback( uint32_t nodeId );
    void changedConnectionCallback();
    void nodeTimeAdjustedCallback( int32_t offset );
    void nodeDelayReceivedCallback( uint32_t nodeId, int32_t delay );
    void receivedCallback( uint32_t from, String &msg );
}

void BotMesh::setup( Scheduler * defaultScheduler ) 
{
        // set before init() so that you can see startup messages
    // ERROR | STARTUP | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE
    #ifdef SERIAL_DEBUG
        BotMesh::getInstance().setDebugMsgTypes( ERROR );
    #endif

    // Create mesh object with mode WIFI_AP_STA = Station and AccessPoint 
    BotMesh::getInstance().init( MESH_SSID, MESH_PASSWORD, defaultScheduler, MESH_PORT, WIFI_AP_STA, MESH_CHANNEL );

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

    BotMesh::getInstance().onNewConnection( &_BotMesh::newConnectionCallback );
    BotMesh::getInstance().onChangedConnections( &_BotMesh::changedConnectionCallback );
    BotMesh::getInstance().onNodeTimeAdjusted( &_BotMesh::nodeTimeAdjustedCallback );
    BotMesh::getInstance().onNodeDelayReceived( &_BotMesh::nodeDelayReceivedCallback );

    BotMesh::getInstance().onReceive( &_BotMesh::receivedCallback );
}


// namespace implementation
namespace _BotMesh
{
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

    void receivedCallback( uint32_t from, String &msg )
    {
        SERIAL_PRINT( "Received from " );
        SERIAL_PRINT( from );
        SERIAL_PRINT( "msg=" );
        SERIAL_PRINTLN( msg.c_str() );
    };
}
