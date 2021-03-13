#ifndef _BotMesh_h_
#define _BotMesh_h_

#include <painlessMesh.h>
using namespace painlessmesh;

#include <patterns.h>
// defines for the MESH SSID etc. to use
#include <secrets.h>

/**
 * BotMesh is an extension to painlessMesh 
 * @see https://gitlab.com/painlessMesh/painlessMesh .
 */
class BotMesh : public painlessMesh
{
    MAKE_SINGLETON(BotMesh)
    
    public:
        void setup( Scheduler * defaultScheduler );

    protected:

    private:
};

#endif