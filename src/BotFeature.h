#ifndef _BotFeature_h_
#define _BotFeature_h_

#include "BotMesh.h"

/**
 * BotFeature is implementing a base class for all features 
 */
class BotFeature
{
    public:
        /**
         * 
         */
        BotFeature();
        /**
         * 
         */
        void setMsgRecCallback( receivedCallback_t onReceive );

    protected:

    private:

};

#endif