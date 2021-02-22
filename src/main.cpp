/**
 *  Wrapper to the Arduino framwork
 */

#include <Arduino.h>

#include "NodeBot.hpp"
static NodeBot bot;

void setup() {
  // serial speed is defined in platformio.ini as build-flag
  Serial.begin( SERIAL_SPEED );
  // setup of the bot
  bot.setup();
}

void loop() {
  bot.update();
}
