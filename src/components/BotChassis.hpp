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
    #include "../driver/d1mini_motor_shield/D1Mini_Motor.h"
#endif

// namespace to keep things local
namespace _BotChassis
{
    BotMesh * pMesh;

    #define CYCLE_TIME 10UL
    #define WATCHDOG_TIMEOUT 15 // amount of cycles until autostop
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

    joystick joy1;
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
            // set the callbacks
            mesh.onReceive( &_BotChassis::receivedCallback );
            
            #ifdef IS_DIFF_DRIVE
                // add Tasks
                defaultScheduler.addTask( _BotChassis::taskSetMotorSpeeds );
                _BotChassis::taskSetMotorSpeeds.enable();
            #endif

            // remember mesh for future use
            _BotChassis::pMesh = &mesh;
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
        
        // joy1 message
        if( msg.startsWith( "{\"joy1" ) )
        {
            #ifdef IS_DIFF_DRIVE
                deserializeJson(doc, msg);
                joy1.x = doc["joy1"]["x"];
                joy1.y = doc["joy1"]["y"];
                joy1.sw = doc["joy1"]["sw"];

                float_t scaler = 1.0f;
                float_t tmp = abs( joy1.x ) + abs( joy1.y );
                if( tmp > 100.0 ) 
                scaler = 100.0f / tmp; 

                leftMotorSpeed = scaler*(joy1.x + joy1.y);
                rightMotorSpeed = scaler*(joy1.x - joy1.y);
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
    };
}

#endif