#ifndef _BotOtaServer_hpp_
#define _BotOtaServer_hpp_

/**
 * 
 */
#include <Arduino.h>

#include "BotMesh.hpp"

// This feature requires a SD-Card, so do not compile without WITH_SD_CARD set!
#ifdef HAS_OTA_SERVER 
#ifndef WITH_SD_CARD
    #error "OTA-Server needs a SD-Card and build-flag -D WITH_SD_CARD set in platformio.ini"
#endif
#endif

#define OTA_PART_SIZE 240                   // How many bytes to send per OTA data packet
#define OTA_START_DELAY 30000UL             // Start to broadcast OTA with delay
#define OTA_BROADCAST_INTERVAL 7200000UL    // broadcast updates every 120 Minutes

// namespace to keep things local
namespace _BotOtaServer
{
    BotMesh * pMesh;

    typedef struct {
        TSTRING ident;
        TSTRING hardware;
        TSTRING role;
        TSTRING extension;
    } firmware_t;

    File dir;
    File entry;

    // prototypes - implementation below
    void checkForUpdates();
    void receivedCallback( uint32_t from, String &msg );

    // Task definitions
    Task taskCheckForUpdates( OTA_BROADCAST_INTERVAL , TASK_FOREVER, &checkForUpdates );
}

class BotOtaServer
{ 
    public:
        BotOtaServer()
        {

        };

        void setup( BotMesh & mesh, Scheduler & defaultScheduler )
        {
            // remember mesh for future use
            _BotOtaServer::pMesh = &mesh;

            // set the callbacks
            mesh.onReceive( &_BotOtaServer::receivedCallback );
            
            // add Tasks
            defaultScheduler.addTask( _BotOtaServer::taskCheckForUpdates );
            _BotOtaServer::taskCheckForUpdates.enableDelayed( OTA_START_DELAY );
        };

    protected:

    private:

};

// implementation of namespace
namespace _BotOtaServer
{
    /**
     * This function parses the file name to make sure it is valid.
     * It will also get the role and hardware the firmware is targeted at.
     */
    bool isFirmware( firmware_t * fw )
    {
        TSTRING name = entry.name();
        // test format of name
        if (name.length() > 1 && name.indexOf( '_' ) != -1 &&
            name.indexOf( '_' ) != name.lastIndexOf( '_' ) &&
            name.indexOf( '.' ) != -1 )
        {
        // get components of name and fill into firmware_t
        fw->ident = name.substring( 0, name.indexOf( '_' ) );
        fw->hardware = name.substring( name.indexOf( '_' ) + 1, name.lastIndexOf( '_' ) );
        fw->role = name.substring( name.lastIndexOf( '_' ) + 1, name.indexOf( '.' ) );
        fw->extension = name.substring( name.indexOf( '.' ) + 1, name.length() );
        // check for valid firmware name
        if ( fw->ident.equals( "firmware" ) &&
            ( fw->hardware.equals( "ESP8266" ) || fw->hardware.equals( "ESP32") ) &&
            fw->extension.equals( "bin" ) )
        {
            return true;
        }
        }

        return false;
    }

    void sendFirmware(firmware_t * fw)
    {
        //This is the important bit for OTA, up to now was just getting the file. 
        //If you are using some other way to upload firmware, possibly from 
        //mqtt or something, this is what needs to be changed.
        //This function could also be changed to support OTA of multiple files
        //at the same time, potentially through using the pkg.md5 as a key in
        //a map to determine which to send

        pMesh->initOTASend(
            [&entry]( painlessmesh::plugin::ota::DataRequest pkg, char* buffer )
            {
                //fill the buffer with the requested data packet from the node.
                entry.seek( OTA_PART_SIZE * pkg.partNo );
                entry.readBytes( buffer, OTA_PART_SIZE );
                
                //The buffer can hold OTA_PART_SIZE bytes, but the last packet may
                //not be that long. Return the actual size of the packet.
                return min( (unsigned)OTA_PART_SIZE, entry.size() - ( OTA_PART_SIZE * pkg.partNo ) );
            },
            OTA_PART_SIZE
        );

        //Calculate the MD5 hash of the firmware we are trying to send. This will be used
        //to validate the firmware as well as tell if a node needs this firmware.
        MD5Builder md5;
        md5.begin();
        md5.addStream( entry, entry.size() );
        md5.calculate();

        //Make it known to the network that there is OTA firmware available.
        //This will send a message every minute for an hour letting nodes know
        //that firmware is available.
        //This returns a task that allows you to do things on disable or more,
        //like closing your files or whatever.
        pMesh->offerOTA( fw->role, fw->hardware, md5.toString(), ceil( ( (float)entry.size() ) / OTA_PART_SIZE ), false );
    }

    void checkForUpdates()
    {
        #ifdef WITH_SD_CARD
            Serial.println( "Checking for Updates ..." );
            if( SD.begin( CHIP_SELECT) )
            {
                Serial.println( "SD-Card found!" );
                // check for new updates in OtaFiles directory
                dir = SD.open( "/OtaFiles/" );
                entry = dir.openNextFile();
                while(entry)
                {
                    // skip directories
                    if ( !entry.isDirectory() )
                    {
                        firmware_t fw;
                        // remember timestamp of file
                        //fw.timestamp = dir.getCreationTime();
                        // test for firmware and get components of name
                        if( isFirmware( &fw ) )
                        {
                            Serial.printf( "OTA FIRMWARE FOUND: %s, %s\n", fw.hardware.c_str(), fw.role.c_str() );
                            sendFirmware( &fw );
                            // right now bail out after sending first update
                            return;
                        }
                    }
                    entry = dir.openNextFile();
                }
            }
        #endif
    }

    // callbacks for mesh
    void receivedCallback( uint32_t from, String &msg )
    {
        Serial.printf( "Received from %u msg=%s\n", from, msg.c_str() );
    };
}

#endif