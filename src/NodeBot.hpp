#ifndef _NodeBot_hpp_
#define _NodeBot_hpp_

/**
 * 
 */
#include <Arduino.h>

// a specialization of painlessMesh to implement extensions
#include "components/BotMesh.hpp"

#ifdef HELLO_WORLD  // should be defined as build flag in platformio.ini
// a simple hello world component
#include "components/HelloWorld.hpp"
#endif

#ifdef HAS_WEB_SERVER  // should be defined as build flag in platformio.ini
// the web server component
#include "components/BotWebServer.hpp"
#endif

class NodeBot
{
    public:
        static NodeBot & getInstance()
        {
            static NodeBot instance;    // Guaranteed to be destroyed, instantiated on first use.
            return instance;
        };

        void setup() {
            BotMesh::getInstance().setup();
            #ifdef HELLO_WORLD  // should be defined as build flag in platformio.ini
            HelloWorld::getInstance().setup();
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
        // Constructor (the {} brackets) are needed here).
        NodeBot() {};

    // Make sure these are inaccessible(especially from outside), 
    // otherwise, we may accidentally get copies of the singleton appearing.
#if (__cplusplus >= 201103L)
    public:
        // Scott Meyers mentions in his Effective Modern
        // C++ book, that deleted functions should generally
        // be public as it results in better error messages
        // due to the compilers behavior to check accessibility
        // before deleted status    public:
        NodeBot(NodeBot const&)         = delete;
        void operator=(NodeBot const&)  = delete;
#else
    private:
        NodeBot(NodeBot const&);        // Don't Implement
        void operator=(NodeBot const&); // Don't implement
#endif
};

#endif