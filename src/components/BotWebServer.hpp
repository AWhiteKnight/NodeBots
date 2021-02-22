#ifndef _BotWebServer_hpp_
#define _BotWebServer_hpp_

/**
 * 
 */
#include <Arduino.h>

#include "patterns.h"

#include "BotMesh.hpp"

#include "IPAddress.h"

#ifdef ESP8266
    #include "Hash.h"
    #include <ESPAsyncTCP.h>
#else
    #include <AsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>


const String corsHeader = "Access-Control-Allow-Origin";
const String corsValue = "*";
const String defaultMimeType = "text/html";

static AsyncWebServer server( 80 );

// namespace to keep callbacks local
// implementation below
namespace _BotWebServer
{
    BotMesh * pMesh;   

    // prototypes - implementation below
    void checkIP();
    void receivedCallback( uint32_t from, String &msg );

    // Task definitions
    Task taskCheckIP( 60000UL , TASK_FOREVER, &checkIP );
}


class BotWebServer
{
    public:
        BotWebServer()
        {

        };

        void setup( BotMesh & mesh, Scheduler & defaultScheduler )
        {
            // set the callbacks
            mesh.onReceive( &_BotWebServer::receivedCallback );

            // implement the bridge
            mesh.stationManual( WLAN_SSID, WLAN_PASSWORD );

            String hostname = "";
            // set the hostname depending on build_flags
            #ifdef UNIQUE_HOSTNAME
                hostname = UNIQUE_HOSTNAME;
            #elif defined(HOSTNAME)
                hostname = HOSTNAME;
                hostname += MESH.getNodeId();
            #else
                hostname += BotMesh::getInstance().getNodeId();
            #endif
            mesh.setHostname( hostname.c_str() );
            Serial.println( "My Hostname is " + hostname );
            Serial.println( "My AP IP is " + mesh.getAPIP().toString() );

            // add Tasks
            defaultScheduler.addTask( _BotWebServer::taskCheckIP );
            _BotWebServer::taskCheckIP.enableDelayed(60000UL);

            // remember mesh for future use
            _BotWebServer::pMesh = &mesh;
        }

        void startWebServer()
        {
            Serial.println( "starting WebServer" );
            // webserver routes from specialized to general!
            // api call to mesh structure, this is possible without sd-card
            server.on
            (
                "/api/getMeshStructure",
                HTTP_GET,
                [](AsyncWebServerRequest *request )
                {
                    AsyncWebServerResponse * response = request->beginResponse
                    (
                        200,
                        defaultMimeType,
                        _BotWebServer::pMesh->subConnectionJson().c_str()
                    );
                    response->addHeader(corsHeader, corsValue);
                    request->send(response);
                }
            );
            // start web server
            server.begin();
        };

    protected:

    private:

};

// implementation of namespace
namespace _BotWebServer
{
    // callback for scheduler
    void checkIP()
    {
        if(pMesh->getStationIP())
        {
            Serial.println( "WebServer IP is " + pMesh->getStationIP().toString() );
            taskCheckIP.disable();
            //taskCheckIP.remove();
        }
    };

    // callbacks for mesh
    void receivedCallback( uint32_t from, String &msg )
    {
        Serial.printf( "Received from %u msg=%s\n", from, msg.c_str() );
    };
}

#endif