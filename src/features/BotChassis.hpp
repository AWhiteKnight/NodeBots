#ifndef _BotChassis_hpp_
#define _BotChassis_hpp_

/**
 * 
 */
#include "../BotFeature.h"

#include <ArduinoJson.h>
#include <Wire.h>

#include "std_msgs.h"

#ifdef WITH_D1MINI_MOTOR_SHIELD
    #include "../components/d1mini/D1Mini_Motor_Shield.h"
#endif

// namespace to keep things local
namespace _BotChassis
{
    static rc3D_t rc3D;
    StaticJsonDocument<240> doc;  // message size < 250 which is esp-now conform

    #define CYCLE_TIME 20UL
    #define FREQUENCY 1000
    #define WATCHDOG_TIMEOUT 11 // amount of cycles until autostop
    static uint16_t watchdog = WATCHDOG_TIMEOUT; 
    
    #ifdef WITH_D1MINI_MOTOR_SHIELD
        // Motor shield I2C Address: 0x30
        #define SHIELD_ADDRESS 0x30
    #endif

    #ifdef IS_DIFF_DRIVE
        static float_t leftMotorSpeed  = 0;
        static float_t rightMotorSpeed = 0;

        // prototypes - implementation below
        void setMotorSpeeds() ; // Prototype so PlatformIO doesn't complain
        // Task definitions
        Task taskSetMotorSpeeds( CYCLE_TIME , TASK_FOREVER, &setMotorSpeeds );

        #ifdef WITH_D1MINI_MOTOR_SHIELD
            // PWM frequency: 1000Hz(1kHz)
            Motor rightMotor( SHIELD_ADDRESS, _MOTOR_A, FREQUENCY );   //Motor A
            Motor leftMotor( SHIELD_ADDRESS, _MOTOR_B, FREQUENCY );    //Motor B
        #endif
    #endif

    void receivedCallback( uint32_t from, String &msg );
}

class BotChassis : public BotFeature
{ 
    public:
        BotChassis()
        {

        };

        void setup( Scheduler & defaultScheduler )
        {
            // set the callbacks
            BotMesh::getInstance().onReceive( &_BotChassis::receivedCallback );
            
            #ifdef IS_DIFF_DRIVE
                // add Tasks
                defaultScheduler.addTask( _BotChassis::taskSetMotorSpeeds );
                _BotChassis::taskSetMotorSpeeds.enable();
            #endif
        };

    protected:

    private:

};

// implementation of namespace
namespace _BotChassis
{
    #ifdef IS_DIFF_DRIVE
        void setMotorSpeeds()
        {
            // got new command within timeout
            if( watchdog > 0 )
            {
                #ifdef WITH_D1MINI_MOTOR_SHIELD
                    if( leftMotorSpeed >= 0.0 )
                        leftMotor.setmotor( _CW, leftMotorSpeed );
                    else
                        leftMotor.setmotor( _CCW, -leftMotorSpeed );

                    if( rightMotorSpeed >= 0.0 )
                        rightMotor.setmotor( _CW, rightMotorSpeed );
                    else
                        rightMotor.setmotor( _CCW, -rightMotorSpeed );
                #endif

                // count down watchdog timer
                watchdog--;
            }
            // didn't get a command for the timeout period
            else
            {
                #ifdef WITH_D1MINI_MOTOR_SHIELD
                    leftMotor.setmotor( _CW, 0.0 );
                    rightMotor.setmotor( _CW, 0.0 );
                #endif
            }
        }
    #endif

    // callbacks for mesh
    void receivedCallback( uint32_t from, String &msg )
    {
        deserializeJson(doc, msg);

        // message for me?
        String target = doc["tgt"];
        String me(BotMesh::getInstance().getNodeId());
        if( target == me || target == "broadcast" )
        {
            // rc3D message?
            if(doc.containsKey( "rc3D" ))
            {
                rc3D[0] = doc["rc3D"][0];
                rc3D[1] = doc["rc3D"][1];
                rc3D[2] = doc["rc3D"][2];
                rc3D[3] = doc["rc3D"][3];
                #ifdef IS_DIFF_DRIVE
                    // use only half of speed for turning
                    rc3D[1] = rc3D[1] / 2;

                    float_t tmp = ( abs( rc3D[0] ) + abs( rc3D[1] ) );
                    float_t scaler = ( tmp > 100.0 ) ? ( 100.0f / tmp ) : 1.0f;

                    leftMotorSpeed  = scaler * ( rc3D[0] + rc3D[1] );
                    rightMotorSpeed = scaler * ( rc3D[0] - rc3D[1] );

                    SERIAL_PRINT( "=> left: " );
                    SERIAL_PRINT( leftMotorSpeed );
                    SERIAL_PRINT( " right: " );
                    SERIAL_PRINTLN( rightMotorSpeed );
                #endif

                // reset watchdog
                watchdog = WATCHDOG_TIMEOUT;
            }
            // any other message 
            else
            {
                ;
            }
        }
    };
}

#endif