#ifndef _BotChassis_hpp_
#define _BotChassis_hpp_

/**
 * 
 */
#include <Arduino.h>
#include <Wire.h>

#include "BotMesh.hpp"
#include "std_msgs.h"

#ifdef WITH_D1MINI_MOTOR_SHIELD
    #include "../component/d1mini/D1Mini_Motor_Shield.h"
#endif

// namespace to keep things local
namespace _BotChassis
{
    BotMesh * pMesh;

    #define CYCLE_TIME 20UL
    #define WATCHDOG_TIMEOUT 11 // amount of cycles until autostop
    uint16_t watchdog = WATCHDOG_TIMEOUT; 
    
    #ifdef WITH_D1MINI_MOTOR_SHIELD
        //Motor shield I2C Address: 0x30
        #define SHIELD_ADDRESS 0x30
    #endif

    #ifdef IS_DIFF_DRIVE
        float_t leftMotorSpeed  = 0;
        float_t rightMotorSpeed = 0;

        // prototypes - implementation below
        void setMotorSpeeds() ; // Prototype so PlatformIO doesn't complain
        // Task definitions
        Task taskSetMotorSpeeds( CYCLE_TIME , TASK_FOREVER, &setMotorSpeeds );

        #ifdef WITH_D1MINI_MOTOR_SHIELD
            // PWM frequency: 1000Hz(1kHz)
            Motor rightMotor( SHIELD_ADDRESS, _MOTOR_A, 1000 );   //Motor A
            Motor leftMotor( SHIELD_ADDRESS, _MOTOR_B, 1000 );    //Motor B
        #endif
    #endif

    void receivedCallback( uint32_t from, String &msg );

    rc3D_t rc3D;
    StaticJsonDocument<240> doc;  // message size < 250 which is esp-now conform
}

class BotChassis
{ 
    public:
        BotChassis()
        {

        };

        void setup( BotMesh & mesh, Scheduler & defaultScheduler )
        {
            // remember mesh for future use
            _BotChassis::pMesh = &mesh;

            // set the callbacks
            mesh.onReceive( &_BotChassis::receivedCallback );
            
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
            // debugging output
            //Serial.printf( "Setting speeds: left=%d right=%d\n", leftMotorSpeed, rightMotorSpeed );

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
        // debugging output
        //Serial.printf( "Received from %u msg=%s\n", from, msg.c_str() );
        
        deserializeJson(doc, msg);

        // message for me?
        String target = doc["tgt"];
        String me(pMesh->getNodeId());
        if( target == me || target == "broadcast" )
        {
            // rc3D message?
            if(doc.containsKey( "rc3D" ))
            {
                #ifdef IS_DIFF_DRIVE
                    rc3D[0] = doc["rc3D"][0];
                    rc3D[1] = doc["rc3D"][1];
                    // use only half of speed for turning
                    rc3D[1] = rc3D[1] / 2;

                    float_t tmp = ( abs( rc3D[0] ) + abs( rc3D[1] ) );
                    float_t scaler = ( tmp > 100.0 ) ? ( 100.0f / tmp ) : 1.0f;

                    leftMotorSpeed  = scaler * ( rc3D[0] + rc3D[1] );
                    rightMotorSpeed = scaler * ( rc3D[0] - rc3D[1] );
                #endif

                // reset watchdog
                watchdog = WATCHDOG_TIMEOUT;
                // debugging output
                //Serial.printf( "=> left: %f, right: %f\n", leftMotorSpeed, rightMotorSpeed );
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