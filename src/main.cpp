/**
 *  Wrapper to the Arduino framwork
 */
#include <Arduino.h>

#include "NodeBot.h"

void setup() {
    // use default serial speed of boards to see boot messages
    // take care that monitor_speed in platformio.ini is set accordingly
    #ifdef ESP32
        Serial.begin( 115200 );
    #elif defined(ESP8266)
        Serial.begin( 76800 );
    #else
        #error "unknown board"
    #endif
    // setup of the bot
    NodeBot::getInstance().setup();
}

void loop() {
    NodeBot::getInstance().update();
}
