#ifndef _NodeBot_h_
#define _NodeBot_h_

/**
 * 
 */
#include <patterns.h>

class NodeBot
{
    MAKE_SINGLETON(NodeBot)

    public:
        void setup();
        void update();

    protected:

    private:
};

#endif