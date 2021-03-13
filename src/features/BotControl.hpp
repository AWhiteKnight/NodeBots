#ifndef _BotControl_hpp_
#define _BotControl_hpp_

/**
 * 
 */
#include "../BotFeature.h"

#include <ArduinoJson.h>

#include "std_msgs.h"

// pin usages
#ifdef ESP32
    #define DEAD_MAN_LED 18
    #define DEAD_MAN_SW 19
    #define JOY1_X  32
    #define JOY1_Y  33
    #define JOY1_SW 14    // joy_sw is a pulldown!
#elif defined(ESP8266)
    #define DEAD_MAN_LED D8
    #define DEAD_MAN_SW D9

#endif

// namespace to keep things local
namespace _BotControl
{
    static uint32_t drive_node = DRIVE_NODE;   // set with build flag!!
    static rc3D_t rc3D;

    static int64_t sum_x;
    static int32_t center_x;
    static int64_t sum_y;
    static int32_t center_y;
    static uint32_t loop_counter;

    StaticJsonDocument<240> doc;  // message size < 250 which is esp-now conform

    // prototypes - implementation below
    void calibrateSticks();
    void collectValues();
    void sendValues();

    // Task definitions
    #define COLLECT_INTERVAL   10UL     // collect values with 10 ms interval
    #define VALUE_INTERVAL    100UL     // send values with 100 ms interval
    Task taskCollectValues( COLLECT_INTERVAL , TASK_FOREVER, &collectValues );
    Task taskSendValues( VALUE_INTERVAL , TASK_FOREVER, &sendValues );
}

class BotControl : public BotFeature
{ 
    public:
        BotControl()
        {

        };

        void setup( Scheduler & defaultScheduler )
        {
            #ifdef ESP32
                // configure pins
                pinMode( JOY1_SW, INPUT );
                pinMode( DEAD_MAN_SW, INPUT );
                pinMode( DEAD_MAN_LED, OUTPUT );
            #elif defined(ESP8266)

            #endif

            // add Tasks
            defaultScheduler.addTask( _BotControl::taskCollectValues );
            _BotControl::taskCollectValues.enable();
            defaultScheduler.addTask( _BotControl::taskSendValues );
            _BotControl::taskSendValues.enable();

            // calibrate sticks
            _BotControl::calibrateSticks();
        };

    protected:

    private:

};

// implementation of namespace
namespace _BotControl
{
    void calibrateSticks()
    {
        // set initial values
        loop_counter = 0;
        sum_x = 0;
        sum_y = 0;
        uint16_t cnt = 100;
        // read some values
        for( uint16_t i = 0; i < cnt; i++ )
        {
            #ifdef ESP32
                // sum up sensor values
                sum_x += analogRead( JOY1_X );
                sum_y += analogRead( JOY1_Y );
            #elif defined(ESP8266)

            #endif
        }
        
        // calculate center values
        center_x = sum_x / cnt;
        center_y = sum_y / cnt;
        
        // (re)set initial values
        loop_counter = 0;
        sum_x = 0;
        sum_y = 0;
    };

    void collectValues()
    {
        // sum up sensor values
        loop_counter++;
        #ifdef ESP32
            sum_x += analogRead( JOY1_X );
            sum_y += analogRead( JOY1_Y );
        #elif defined(ESP8266)

        #endif
    }

    void sendValues()
    {
        // only if dead man switch is presssed
        if( digitalRead( DEAD_MAN_SW ) > 0 )
        {
            // switch on signaling LED
            digitalWrite( DEAD_MAN_LED, HIGH );

            // scale values of axis
            int32_t x = 1 + ( center_x - ( sum_x / loop_counter ) ) / 19;
            int32_t y = 1 + ( center_y - ( sum_y / loop_counter ) ) / 19;
            
            // limit x
            if( x < -100 )
                rc3D[0] = -100;
            else if ( x > 100 )
                rc3D[0] = 100;
            else
                rc3D[0] = x;
            
            //limit y
            if( y < -100 )
                rc3D[1] = -100;
            else if ( y > 100 )
                rc3D[1] = 100;
            else
                rc3D[1] = y;

            SERIAL_PRINT( "rc3D: " );
            SERIAL_PRINT( rc3D[0] );
            SERIAL_PRINT( "," );
            SERIAL_PRINT( rc3D[1] );
            SERIAL_PRINT( "," );
            SERIAL_PRINT( rc3D[2] );
            SERIAL_PRINT( "," );
            SERIAL_PRINTLN( rc3D[3] );

            // create JSON
            String target(drive_node);
            doc["tgt"] = target;
            doc["rc3D"][0] = rc3D[0]; 
            doc["rc3D"][1] = rc3D[1]; 
            doc["rc3D"][2] = rc3D[2]; 
            doc["rc3D"][3] = rc3D[3]; 
            
            // send JSON 
            String msg;
            serializeJson( doc, msg );
            //Serial.println( msg );
            BotMesh::getInstance().sendSingle( drive_node, msg );
        } 
        else
        {
            digitalWrite( DEAD_MAN_LED, LOW );
        }

        // reset values
        loop_counter = 0;
        sum_x = 0;
        sum_y = 0;
    }
}

#endif