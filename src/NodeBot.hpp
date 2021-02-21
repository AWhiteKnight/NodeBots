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
            MESH.setup();
            #ifdef HELLO_WORLD  // should be defined as build flag in platformio.ini
            BotHelloWorld::getInstance().setup();
            #endif
            #ifdef HAS_WEB_SERVER  // should be defined as build flag in platformio.ini
            BotWebServer::getInstance().setup();
            #endif
        };

        void runOnce()
        {
            // this will run the user scheduler as well
            BotMesh::getInstance().runOnce();
            //#ifdef HELLO_WORLD  // should be defined as build flag in platformio.ini
            //HelloWorld::getInstance().runOnce();
            //#endif
        };

    protected:

    private:

};

#endif