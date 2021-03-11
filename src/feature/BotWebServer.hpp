#ifndef _BotWebServer_hpp_
#define _BotWebServer_hpp_

/**
 * 
 */
#include <Arduino.h>

#include "BotMesh.hpp"

#include "IPAddress.h"

#ifdef ESP8266
    #include "Hash.h"
    #include <ESPAsyncTCP.h>
#else
    #include <AsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>

static const String corsHeader = "Access-Control-Allow-Origin";
static const String corsValue = "*";
static const String defaultMimeType = "text/html";
static const String plainTextMimeType = "text/plain";

static AsyncWebServer server( 80 );

// namespace to keep callbacks local
// implementation below
namespace _BotWebServer
{
    static BotMesh * pMesh;
    static StaticJsonDocument<240> recDoc;    // message size < 250 which is esp-now conform

    // prototypes - implementation below
    bool handlePost( AsyncWebServerRequest *request, uint8_t *datas );
    #ifdef WITH_SD_CARD
        bool sendFromSdCard( AsyncWebServerRequest *request, String path );
    #endif
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
            // remember mesh for future use
            _BotWebServer::pMesh = &mesh;

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
            Serial.println( "Hostname is " + hostname );

            setRoutes();

            // fire up web server
            server.begin();

            // add Tasks
            defaultScheduler.addTask( _BotWebServer::taskCheckIP );
            _BotWebServer::taskCheckIP.enableDelayed(60000UL);
        }


    protected:

    private:
        // webserver routes from specialized to general!
        void setRoutes()
        {
            // api call to mesh structure, possible without sd-card
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
                    response->addHeader( corsHeader, corsValue );
                    request->send( response );
                }
            );

            // api call to post message, possible without sd-card
            server.onRequestBody
            (
                [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
                {
                    if (request->url() == "/api/postMessage")
                    {
                        if ( !_BotWebServer::handlePost(request, data) ) 
                            request->send(400, plainTextMimeType, "false");
                        else
                            request->send(200, plainTextMimeType, "true");
                    }
                }
            );

            // serve any file from the sd-card
            #ifdef WITH_SD_CARD
                server.on
                (
                    "/*",
                    HTTP_GET,
                    []( AsyncWebServerRequest *request )
                    {
                        if( SD.begin( CHIP_SELECT) )
                        {
                            _BotWebServer::sendFromSdCard( request, request->url() );
                        }
                        else
                        {
                            request->send(404, defaultMimeType, "Not found");
                        }
                    }
                );
            #endif
        };
};

// implementation of namespace
namespace _BotWebServer
{
    // handler for web server
    // TODO: This crashes when multiple clients send messages!
    bool handlePost(AsyncWebServerRequest *request, uint8_t *datas) {
        //Serial.printf("[REQUEST]\t%s\r\n", (const char *)datas);

        // forward message to target(s)
        deserializeJson(recDoc, (const char*)datas);
        String target = recDoc["tgt"];

        String msg;
        serializeJson( recDoc, msg );
        if(target == "broadcast")
        {
            pMesh->sendBroadcast( msg );
            return true;
        }
        else if(target != "")
        {
            uint32_t tgt = recDoc["tgt"];
            pMesh->sendSingle( tgt, msg );
            return true;
        }

        Serial.println( "Error in /api/postMessage" );
        Serial.println( (const char*)datas );
        return false;
    }

    #ifdef WITH_SD_CARD
        bool sendFromSdCard( AsyncWebServerRequest *request, String path )
        {
            String dataType = defaultMimeType;
            if(path.equals("/"))
            {
                path += "index.html";
            } 
            else
            {
                if(path.endsWith(".ico")) dataType = "image/ico";
                else if( path.endsWith( ".css" ) ) dataType = "text/css";
                else if( path.endsWith( ".js" ) ) dataType = "application/javascript";
                else if( path.endsWith( ".jpg" ) ) dataType = "image/jpeg";
                else if( path.endsWith( ".txt" ) ) dataType = plainTextMimeType;
                else if( path.endsWith( ".json" ) ) dataType = "application/json";
            }

            File dataFile = SD.open(path.c_str());
            if (!dataFile)
            {
                Serial.println("File not found");
                request->send(404, defaultMimeType, "Not found");
                return false;
            }

            AsyncWebServerResponse * response = request->beginResponse( dataFile, path, dataType );
            request->send(response);

            return true;
        }
    #endif

    // callback for scheduler
    void checkIP()
    {
        if(pMesh->getStationIP())
        {
            Serial.println( "STA IP is " + pMesh->getStationIP().toString() );
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