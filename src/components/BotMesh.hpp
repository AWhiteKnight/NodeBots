#ifndef _BotMesh_hpp_
#define _BotMesh_hpp_

//#define _TASK_OO_CALLBACKS

/**
 * 
 */
#include <Arduino.h>

#include"patterns.h"

// defines for the MESH SSID etc. to use
#include "secrets.h"

#include <painlessMesh.h>
using namespace painlessmesh;

// macro to shorten lines
#define MESH BotMesh::getInstance()

/**
 * BotMesh is an extension to painlessMesh 
 * @see https://gitlab.com/painlessMesh/painlessMesh .
 */
class BotMesh : public painlessMesh
{
    MAKE_SINGLETON(BotMesh)

    public:
        void setup()
        {
            //ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE
            setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

            // Create mesh object with mode WIFI_AP_STA = Station and AccessPoint 
            init( MESH_SSID, MESH_PASSWORD, &defaultScheduler, MESH_PORT, WIFI_AP_STA, MESH_CHANNEL );

            // enable OTA if a role is defined (should be done as build flag)
            #ifdef OTA_ROLE
            MESH.initOTAReceive( OTA_ROLE );
            #endif
        };

        inline void runOnce()
        {
            // this will also run the defaultScheduler
            update();
        };

        Scheduler & getDefaultSCheduler()
        {
            return defaultScheduler;
        };

    protected:

    private:
        Scheduler defaultScheduler;     // to control tasks

};

#endif