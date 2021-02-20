#ifndef _HelloWorld_hpp_
#define _HelloWorld_hpp_

/**
 * 
 */
#include <Arduino.h>

#include "BotMesh.hpp"

// prototypes
void helloWorld();
void receivedCallback( uint32_t from, String &msg );
void newConnectionCallback( uint32_t nodeId );
void changedConnectionCallback();
void nodeTimeAdjustedCallback( int32_t offset );
void nodeDelayReceivedCallback( uint32_t nodeId, int32_t delay );

Task taskHelloWorld( 3000UL , TASK_FOREVER, &helloWorld );

class HelloWorld
{
    public:
        static HelloWorld & getInstance()
        {
            static HelloWorld instance; // Guaranteed to be destroyed.
                                        // Instantiated on first use.
            return instance;
        }

        void setup()
        {
            // set the callbacks
            BotMesh::getInstance().onReceive( &receivedCallback );
            BotMesh::getInstance().onNewConnection( &newConnectionCallback );
            BotMesh::getInstance().onChangedConnections( &changedConnectionCallback );
            BotMesh::getInstance().onNodeTimeAdjusted( &nodeTimeAdjustedCallback );
            BotMesh::getInstance().onNodeDelayReceived( &nodeDelayReceivedCallback );
            BotMesh::getInstance().getDefaultSCheduler().addTask( taskHelloWorld );
            taskHelloWorld.enable();
        };

        void addTasks()
        {
        };

        inline void runOnce() {};


    protected:

    private:
        // Constructor (the {} brackets) are needed here).
        HelloWorld() {};

    // Make sure these are inaccessible(especially from outside), 
    // otherwise, we may accidentally get copies of the singleton appearing.
#if (__cplusplus >= 201103L)
    public:
        // Scott Meyers mentions in his Effective Modern
        // C++ book, that deleted functions should generally
        // be public as it results in better error messages
        // due to the compilers behavior to check accessibility
        // before deleted status    public:
        HelloWorld(HelloWorld const&)      = delete;
        void operator=(HelloWorld const&)  = delete;
#else
    private:
        HelloWorld(HelloWorld const&);     // Don't Implement
        void operator=(HelloWorld const&); // Don't implement
#endif
};

void helloWorld()
{
    Serial.println("called helloWorld()");
    BotMesh::getInstance().sendBroadcast( "Hello world!" );
};

// callbacks for mesh
void receivedCallback( uint32_t from, String &msg )
{
    Serial.printf( "Received from %u msg=%s\n", from, msg.c_str() );
};

void newConnectionCallback( uint32_t nodeId )
{
    Serial.printf( "New Connection, nodeId = %u\n", nodeId );
    BotMesh::getInstance().sendSingle( nodeId, "Hello, I am the new one." );
};

void changedConnectionCallback()
{
    Serial.printf( "Changed connections\n" );
};

void nodeTimeAdjustedCallback( int32_t offset )
{
    Serial.printf( "Adjusted time %u. Offset = %d\n", BotMesh::getInstance().getNodeTime(), offset) ;
};

void nodeDelayReceivedCallback( uint32_t nodeId, int32_t delay )
{
    Serial.printf( "Delay from node:%u delay = %d\n", nodeId, delay );
};


#endif