#ifndef _BotHelloWorld_hpp_
#define _BotHelloWorld_hpp_

/**
 * 
 */
#include <Arduino.h>

#include "patterns.h"

#include "BotMesh.hpp"

// task prototypes
void broadcastBotHelloWorld();
Task taskBotHelloWorld( 3000UL , TASK_FOREVER, &broadcastBotHelloWorld );

// namespace to keep calllbacks local
// implementation below
namespace BotHelloWorldCallbacks
{
    void receivedCallback( uint32_t from, String &msg );
    void newConnectionCallback( uint32_t nodeId );
    void changedConnectionCallback();
    void nodeTimeAdjustedCallback( int32_t offset );
    void nodeDelayReceivedCallback( uint32_t nodeId, int32_t delay );
}

using namespace BotHelloWorldCallbacks;
class BotHelloWorld
{
    MAKE_SINGLETON(BotHelloWorld)

    public:
        void setup()
        {
            // set the callbacks
            MESH.onReceive( &receivedCallback );
            MESH.onNewConnection( &newConnectionCallback );
            MESH.onChangedConnections( &changedConnectionCallback );
            MESH.onNodeTimeAdjusted( &nodeTimeAdjustedCallback );
            MESH.onNodeDelayReceived( &nodeDelayReceivedCallback );
            MESH.getDefaultSCheduler().addTask( taskBotHelloWorld );
            taskBotHelloWorld.enable();
        };

        void addTasks()
        {
        };

        inline void runOnce() {};


    protected:

    private:

};

void broadcastBotHelloWorld()
{
    Serial.println("called BotHelloWorld()");
    MESH.sendBroadcast( "Hello world!" );
};

// namespace to keep calllbacks local
namespace BotHelloWorldCallbacks
{
    // callbacks for mesh
    void receivedCallback( uint32_t from, String &msg )
    {
        Serial.printf( "Received from %u msg=%s\n", from, msg.c_str() );
    };

    void newConnectionCallback( uint32_t nodeId )
    {
        Serial.printf( "New Connection, nodeId = %u\n", nodeId );
        MESH.sendSingle( nodeId, "Hello, I am the new one." );
    };

    void changedConnectionCallback()
    {
        Serial.printf( "Changed connections\n" );
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