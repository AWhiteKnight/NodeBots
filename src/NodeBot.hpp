#ifndef _NodeBot_hpp_
#define _NodeBot_hpp_

/**
 * 
 */
#include <Arduino.h>

#include "patterns.h"

// a specialization of painlessMesh to implement extensions
#include "components/BotMesh.hpp"

#ifdef HELLO_WORLD  // should be defined as build flag in platformio.ini
// a simple hello world component
#include "components/BotHelloWorld.hpp"
#endif

#ifdef HAS_WEB_SERVER  // should be defined as build flag in platformio.ini
// the web server component
#include "components/BotWebServer.hpp"
#endif

class NodeBot
{
    MAKE_SINGLETON(NodeBot)

    public:
        void setup() {
            Serial.println("setup begin");
            MESH.setup();
            #ifdef HELLO_WORLD  // should be defined as build flag in platformio.ini
            BotHelloWorld::getInstance().setup();
            #endif
            #ifdef HAS_WEB_SERVER  // should be defined as build flag in platformio.ini
            // setup BotWebServr
            BotWebServer::getInstance().setup();
            #endif

            #ifdef HAS_WEB_SERVER  // should be defined as build flag in platformio.ini
            // start BotWebServer
            BotWebServer::getInstance().startWebServer();
            #endif

            Serial.println("setup end");
        };

        void update()
        {
            // this will run the user scheduler as well
            MESH.update();
        };

    protected:

    private:

};

#endif