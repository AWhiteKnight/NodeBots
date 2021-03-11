#ifndef _logging_h_
#define _logging_h_
/**
 * Some macros helping to create serial output for debugging only
 */

#ifdef SERIAL_DEBUG
    #define SERIAL_PRINT(msg); Serial.print( msg );
    #define SERIAL_PRINTLN(msg); Serial.println( msg );

#else
    #define SERIAL_PRINT(msg)
    #define SERIAL_PRINTLN(msg)
#endif

#endif