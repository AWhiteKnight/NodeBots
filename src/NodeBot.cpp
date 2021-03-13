/**
 * 
 */
#include <Arduino.h>
#include <logging.h>

#include "NodeBot.h"

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

// namespace to keep callbacks local
// implementation below
namespace _NodeBot
{
    void receivedCallback( uint32_t from, String &msg );
}

void NodeBot::setup( Scheduler * defaultScheduler )
{
    // setup globel callback for messages
    BotMesh::getInstance().onReceive( &_NodeBot::receivedCallback );

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

    SERIAL_PRINTLN( "MESH IP is " + BotMesh::getInstance().getAPIP().toString() );
};

// namespace implementation
namespace _NodeBot
{
    void receivedCallback( uint32_t from, String &msg )
    {
        SERIAL_PRINT( "Received from " );
        SERIAL_PRINT( from );
        SERIAL_PRINT( "msg=" );
        SERIAL_PRINTLN( msg.c_str() );
    };
}
