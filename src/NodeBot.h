#ifndef _NodeBot_h_
#define _NodeBot_h_

#include <patterns.h>

// a specialization of painlessMesh to implement extensions
#include "BotMesh.h"

/**
 * 
 */
class NodeBot
{
    MAKE_SINGLETON(NodeBot)

    public:
        void setup( Scheduler * defaultScheduler );

    protected:

    private:
};

#endif