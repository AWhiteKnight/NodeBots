#ifndef _config_h_
#define _config_h_

/**
 * Configurtion File
 */

#ifdef ESP32
    #define CHIP_SELECT 33
#elif defined(ESP8266)
    #define CHIP_SELECT D8
#else
    #error "unknown board"
#endif

#endif