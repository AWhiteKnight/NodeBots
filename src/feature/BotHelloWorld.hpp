#ifndef _BotHelloWorld_hpp_
#define _BotHelloWorld_hpp_

/**
 * 
 */
#include <Arduino.h>

#include "BotMesh.hpp"

// namespace to keep things local
namespace _BotHelloWorld
{
    BotMesh * pMesh;

    // prototypes - implementation below
    void broadcastBotHelloWorld();
    void receivedCallback( uint32_t from, String &msg );

    // Task definitions
    Task taskBotHelloWorld( 60000UL , TASK_FOREVER, &broadcastBotHelloWorld );
}

class BotHelloWorld
{ 
    public:
        BotHelloWorld()
        {

        };

        void setup( BotMesh & mesh, Scheduler & defaultScheduler )
        {
            // remember mesh for future use
            _BotHelloWorld::pMesh = &mesh;

            // set the callbacks
            mesh.onReceive( &_BotHelloWorld::receivedCallback );
            
            // add Tasks
            defaultScheduler.addTask( _BotHelloWorld::taskBotHelloWorld );
            _BotHelloWorld::taskBotHelloWorld.enable();
        };

    protected:

    private:

};

// implementation of namespace
namespace _BotHelloWorld
{
    // callback for scheduler
    void broadcastBotHelloWorld()
    {
        Serial.println("called BotHelloWorld()");
        pMesh->sendBroadcast( "Hello world!" );
    };

    // callbacks for mesh
    void receivedCallback( uint32_t from, String &msg )
    {
        Serial.printf( "Received from %u msg=%s\n", from, msg.c_str() );
    };
}

#endif