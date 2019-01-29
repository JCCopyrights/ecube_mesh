#include "IPAddress.h"
#include "namedMesh.h"
#include "Hash.h"
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555
#define   CHANNEL         6

#define   STATION_SSID     "XXXX"
#define   STATION_PASSWORD "XXXXXXXX"

#define HOSTNAME "HTTP_BRIDGE"
String nodeName;
 
// Prototype
void receivedCallback( const uint32_t &from, const String &msg );

IPAddress getlocalIP();
namedMesh  mesh;
AsyncWebServer server(80);
IPAddress myIP(0,0,0,0);
IPAddress myAPIP(0,0,0,0);

void setup() {
  Serial.begin(115200);
  //MESH CONFIGURATION
  //mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION );  // set before init() so that you can see startup messages
  //Same Channel Must be used for MESH and WIFI
  mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT, WIFI_AP_STA, CHANNEL );
  nodeName = HOSTNAME+String(mesh.getNodeId(),DEC);
  mesh.setName(nodeName); // This needs to be an unique name!
  mesh.onReceive([](String &from, String &msg) {
    Serial.printf("Message from: %s, %s\n", from.c_str(), msg.c_str());
  });
  mesh.onChangedConnections([]() {
    Serial.printf("Changed connection\n");
    Serial.println(mesh.subConnectionJson());
  });
  mesh.setRoot();
  mesh.setContainsRoot();
  //HTTP SERVER CONFIGURATION
  mesh.stationManual(STATION_SSID, STATION_PASSWORD);
  mesh.setHostname(HOSTNAME);
  myAPIP = IPAddress(mesh.getAPIP());
  Serial.println("My AP IP is " + myAPIP.toString());
  static const String &http_code="<form>Text to Broadcast<br><input type='text' name='BROADCAST'><br><br><input type='submit' value='Submit'></form>";
  //Async webserver
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", http_code + "<br>" + mesh.subConnectionJson());
    if (request->hasArg("BROADCAST")){
      String msg = request->arg("BROADCAST");
      mesh.sendBroadcast(msg);
    }
  });
  server.begin();
}

void loop() {
  mesh.update();
  if(myIP != getlocalIP()){
    myIP = getlocalIP();
    Serial.println("My Network IP is " + myIP.toString());
  }
}
IPAddress getlocalIP() {
  return IPAddress(mesh.getStationIP());
}
