#ifndef _BotWebServer_hpp_
#define _BotWebServer_hpp_

/**
 * 
 */
#include <Arduino.h>

#include "BotMesh.hpp"

#include "IPAddress.h"

#ifdef ESP32
    #include <AsyncTCP.h>
#elif defined( ESP8266 )
    #include "Hash.h"
    #include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>

const String corsHeader = "Access-Control-Allow-Origin";
const String corsValue = "*";
const String defaultMimeType = "text/html";

static AsyncWebServer server( 80 );

class BotWebServer
{
    public:
        static BotWebServer & getInstance()
        {
            // Guaranteed to be destroyed, instantiated on first use.
            static BotWebServer instance;
            return instance;
        };

        void setup()
        {
            String hostname = "";
            // implement the bridge
            BotMesh::getInstance().stationManual( WLAN_SSID, WLAN_PASSWORD );
            // set the hostname depending on build_flags
            #ifdef UNIQUE_HOSTNAME
                hostname = UNIQUE_HOSTNAME;
            #elif defined(HOSTNAME)
                hostname = HOSTNAME;
                hostname += BotMesh::getInstance().getNodeId();
            #else
                hostname += BotMesh::getInstance().getNodeId();
            #endif
            BotMesh::getInstance().setHostname( hostname.c_str() );
            Serial.println( "My Hostname is " + hostname );
            Serial.println( "My AP IP is " + BotMesh::getInstance().getAPIP().toString() );
            Serial.println( "My IP is " + BotMesh::getInstance().getStationIP().toString() );

            // async webserver
            // webserver routes
            // api call to mesh structure, this is possible without sd-card
            server.on(
                "/api/getMeshStructure",
                HTTP_GET,
                [](AsyncWebServerRequest *request )
                {
                    AsyncWebServerResponse * response = request->beginResponse
                    (
                        200,
                        defaultMimeType,
                        BotMesh::getInstance().subConnectionJson().c_str()
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
        // Constructor (the {} brackets) are needed here).
        BotWebServer() {};

    // Make sure these are inaccessible(especially from outside), 
    // otherwise, we may accidentally get copies of the singleton appearing.
#if (__cplusplus >= 201103L)
    public:
        // Scott Meyers mentions in his Effective Modern
        // C++ book, that deleted functions should generally
        // be public as it results in better error messages
        // due to the compilers behavior to check accessibility
        // before deleted status    public:
        BotWebServer(BotWebServer const&)    = delete;
        void operator=(BotWebServer const&)  = delete;
#else
    private:
        BotWebServer(BotWebServer const&);   // Don't Implement
        void operator=(BotWebServer const&); // Don't implement
#endif
};

#endif