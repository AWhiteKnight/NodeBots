/**
 * 
 */

#include "BotFeature.h"

/**
 * 
 */
BotFeature::BotFeature()
{
    // creation
}

/**
 * 
 */
void BotFeature::setMsgRecCallback( receivedCallback_t onReceive )
{
    BotMesh::getInstance().onReceive( onReceive );
};
