/**
 *  Basic painlessMesh based Bot, doing nothing
 */
#include <Arduino.h>

/* Mesh stuff ---------------------------------------------------------------------- */
#include "painlessMesh.h"

/* defines for the MESH SSID etc to use */
#include "secrets.h"

Scheduler userScheduler; // to control tasks
painlessMesh mesh;

void helloWorld() ; // Prototype so PlatformIO doesn't complain
Task taskHelloWorld( 60000UL , TASK_FOREVER, &helloWorld );

// Available callbacks of painlessMesh
void receivedCallback( uint32_t from, String &msg )
{
  Serial.printf( "Received from %u msg=%s\n", from, msg.c_str() );
}

void newConnectionCallback( uint32_t nodeId )
{
  Serial.printf( "New Connection, nodeId = %u\n", nodeId );
  mesh.sendSingle( nodeId, "Hello, I am the new one." );
}

void changedConnectionCallback()
{
  Serial.printf( "Changed connections\n" );
}

void nodeTimeAdjustedCallback( int32_t offset )
{
  Serial.printf( "Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset) ;
}

void nodeDelayReceivedCallback( uint32_t nodeId, int32_t delay )
{
  Serial.printf( "Delay from node:%u delay = %d\n", nodeId, delay );
}

void setup() {
  // serial speed is defined in platformio.ini as build-flag
  Serial.begin( SERIAL_SPEED );

  //ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

  // Create mesh object with mode WIFI_AP_STA = Station and AccessPoint 
  mesh.init( MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT, WIFI_AP_STA, MESH_CHANNEL );

  // set the callback(s)
  mesh.onReceive( &receivedCallback );
  mesh.onNewConnection( &newConnectionCallback );
  mesh.onChangedConnections( &changedConnectionCallback );
  mesh.onNodeTimeAdjusted( &nodeTimeAdjustedCallback );
  mesh.onNodeDelayReceived( &nodeDelayReceivedCallback );

  // add the task(s)
  userScheduler.addTask( taskHelloWorld );
  taskHelloWorld.enable();

}

void loop() {
  // this will run the user scheduler as well
  mesh.update();
}

void helloWorld()
{
  Serial.println("called helloWorld()");
  mesh.sendBroadcast( "Hello world!" );
}