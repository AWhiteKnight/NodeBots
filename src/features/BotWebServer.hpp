#ifndef _BotWebServer_hpp_
#define _BotWebServer_hpp_

#include "../config.h"
#include "../BotFeature.h"

#include <ArduinoJson.h>
#include "IPAddress.h"

#ifdef WITH_SD_CARD
    #include <FS.h>
    #include <SD.h>
    #include <SPI.h>
#endif

#ifdef ESP8266
    #include <Hash.h>
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
    static StaticJsonDocument<240> recDoc;    // message size < 250 which is esp-now conform

    // prototypes - implementation below
    bool handlePost( AsyncWebServerRequest *request, uint8_t *datas );
    #ifdef WITH_SD_CARD
        bool sendFromSdCard( AsyncWebServerRequest *request, String path );
    #endif
    void checkIP();

    // Task definitions
    Task taskCheckIP( 60000UL , TASK_FOREVER, &checkIP );
}


class BotWebServer : public BotFeature
{
    public:
        BotWebServer()
        {

        };

        void setup( Scheduler * defaultScheduler )
        {
            // implement the bridge
            BotMesh::getInstance().stationManual( WLAN_SSID, WLAN_PASSWORD );

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
            BotMesh::getInstance().setHostname( hostname.c_str() );
            SERIAL_PRINT( "Hostname is " );
            SERIAL_PRINTLN( hostname );

            setRoutes();

            // fire up web server
            server.begin();

            // add Tasks
            defaultScheduler->addTask( _BotWebServer::taskCheckIP );
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
                        BotMesh::getInstance().subConnectionJson().c_str()
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
        SERIAL_PRINT( " REQUEST: ");
        SERIAL_PRINTLN( (const char *)datas );

        // forward message to target(s)
        deserializeJson(recDoc, (const char*)datas);
        String target = recDoc["tgt"];

        String msg;
        serializeJson( recDoc, msg );
        if(target == "broadcast")
        {
            BotMesh::getInstance().sendBroadcast( msg );
            return true;
        }
        else if(target != "")
        {
            uint32_t tgt = recDoc["tgt"];
            BotMesh::getInstance().sendSingle( tgt, msg );
            return true;
        }

        SERIAL_PRINTLN( "Error in /api/postMessage:" );
        SERIAL_PRINTLN( (const char*)datas );
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
                SERIAL_PRINTLN( "File not found" );
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
        if(BotMesh::getInstance().getStationIP())
        {
            SERIAL_PRINT( "STA IP is " );
            SERIAL_PRINTLN( BotMesh::getInstance().getStationIP().toString() );
            taskCheckIP.disable();
            //taskCheckIP.remove();
        }
    }
}

#endif