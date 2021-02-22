
#ifndef _STD_MSGS_H
#define _STD_MSGS_H

#include <stdint.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float_t x;
    float_t y;
    float_t z;
} vec3_t;

typedef struct { 
    float_t linear;
    float_t angular;
} velCmd2D;

typedef struct { 
    float_t linear;
    vec3_t angular;
} velCmd2_5D;

typedef struct { 
    vec3_t linear;
    vec3_t angular;
} velCmd3D;

typedef struct {
    int8_t x;
    int8_t y;
    bool sw;
} joystick;

#ifdef __cplusplus
}
#endif

#endif /* _STD_MSGS_H */
