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
Task taskBotHelloWorld( 60000UL , TASK_FOREVER, &broadcastBotHelloWorld );

// namespace to keep callbacks local
// implementation below
namespace BotHelloWorldCallbacks
{
    void receivedCallback( uint32_t from, String &msg );
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
            
            // add Tasks
            MESH.getDefaultSCheduler().addTask( taskBotHelloWorld );
            taskBotHelloWorld.enable();
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

// namespace to keep callbacks local
namespace BotHelloWorldCallbacks
{
    // callbacks for mesh
    void receivedCallback( uint32_t from, String &msg )
    {
        Serial.printf( "Received from %u msg=%s\n", from, msg.c_str() );
    };
}

#endif