#ifndef _BotWebServer_hpp_
#define _BotWebServer_hpp_

/**
 * 
 */
#include <Arduino.h>

#include "patterns.h"

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
    MAKE_SINGLETON(BotWebServer)

    public:
        void setup()
        {
            String hostname = "";
            // implement the bridge
            MESH.stationManual( WLAN_SSID, WLAN_PASSWORD );
            // set the hostname depending on build_flags
            #ifdef UNIQUE_HOSTNAME
                hostname = UNIQUE_HOSTNAME;
            #elif defined(HOSTNAME)
                hostname = HOSTNAME;
                hostname += MESH.getNodeId();
            #else
                hostname += BotMesh::getInstance().getNodeId();
            #endif
            MESH.setHostname( hostname.c_str() );
            Serial.println( "My Hostname is " + hostname );
            Serial.println( "My AP IP is " + MESH.getAPIP().toString() );
            Serial.println( "My IP is " + MESH.getStationIP().toString() );

            // webserver routes from specialized to general!
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
                        MESH.subConnectionJson().c_str()
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

#endif