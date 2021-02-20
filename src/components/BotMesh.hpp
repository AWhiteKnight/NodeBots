#ifndef _BotMesh_hpp_
#define _BotMesh_hpp_

//#define _TASK_OO_CALLBACKS

/**
 * 
 */
#include <Arduino.h>

// defines for the MESH SSID etc. to use
#include "secrets.h"

#include <painlessMesh.h>
using namespace painlessmesh;

/**
 * extMesh is an extension to painlessMesh 
 * @see https://gitlab.com/painlessMesh/painlessMesh .
 */
class BotMesh : public painlessMesh
{
    public:
        inline static BotMesh & getInstance()
        {
            static BotMesh instance;    // Guaranteed to be destroyed.
                                        // Instantiated on first use.
            return instance;
        }

        void setup()
        {
            //ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE
            setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

            // Create mesh object with mode WIFI_AP_STA = Station and AccessPoint 
            init( MESH_SSID, MESH_PASSWORD, &defaultScheduler, MESH_PORT, WIFI_AP_STA, MESH_CHANNEL );
        };

        inline void runOnce()
        {
            // this will also run the defaultScheduler
            update();
        };

        Scheduler & getDefaultSCheduler()
        {
            return defaultScheduler;
        }

    protected:

    private:
        Scheduler defaultScheduler;     // to control tasks

        // Constructor (the {} brackets) are needed here).
        BotMesh() {};

    // Make sure these are inaccessible(especially from outside), 
    // otherwise, we may accidentally get copies of the singleton appearing.
#if (__cplusplus >= 201103L)
    public:
        // Scott Meyers mentions in his Effective Modern
        // C++ book, that deleted functions should generally
        // be public as it results in better error messages
        // due to the compilers behavior to check accessibility
        // before deleted status    public:
        BotMesh(BotMesh const&)         = delete;
        void operator=(BotMesh const&)  = delete;
#else
    private:
        BotMesh(BotMesh const&);        // Don't Implement
        void operator=(BotMesh const&); // Don't implement
#endif
};

#endif