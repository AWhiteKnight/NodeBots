/**
 *  Wrapper to the Arduino framwork
 */

#include <Arduino.h>

#include "NodeBot.hpp"
NodeBot & bot = NodeBot::getInstance();

void setup() {
  // serial speed is defined in platformio.ini as build-flag
  Serial.begin( SERIAL_SPEED );
  // setup of the bot
  bot.setup();
}

void loop() {
  bot.runOnce();
}
