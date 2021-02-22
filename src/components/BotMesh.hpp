#ifndef _BotMesh_hpp_
#define _BotMesh_hpp_

//#define _TASK_OO_CALLBACKS

/**
 * 
 */
#include <Arduino.h>

#include"patterns.h"

// defines for the MESH SSID etc. to use
#include "secrets.h"

#include <painlessMesh.h>
using namespace painlessmesh;

/**
 * BotMesh is an extension to painlessMesh 
 * @see https://gitlab.com/painlessMesh/painlessMesh .
 */
class BotMesh : public painlessMesh
{
    public:

    protected:

    private:
};

#endif