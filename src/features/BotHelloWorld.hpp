#ifndef _BotHelloWorld_hpp_
#define _BotHelloWorld_hpp_

/**
 * 
 */
#include "../BotFeature.h"

// namespace to keep things local
namespace _BotHelloWorld
{
    // prototypes - implementation below
    void broadcastBotHelloWorld();

    // Task definitions
    Task taskBotHelloWorld( 60000UL , TASK_FOREVER, &broadcastBotHelloWorld );
}

class BotHelloWorld : public BotFeature
{ 
    public:
        BotHelloWorld()
        {

        };

        void setup( Scheduler * defaultScheduler )
        {
            // add Tasks
            defaultScheduler->addTask( _BotHelloWorld::taskBotHelloWorld );
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
        SERIAL_PRINTLN( "called BotHelloWorld()" );
        BotMesh::getInstance().sendBroadcast( "Hello world!" );
    }
}

#endif