//************************************************************
// this is a simple example that uses the painlessMesh library
// 
// This example shows how to build a mesh with named nodes
//
//************************************************************
#include "namedMesh.h"

#define   MESH_SSID       "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

Scheduler     userScheduler; // to control your personal task
namedMesh  mesh;

String nodeName = "1"; // Name needs to be unique

Task taskSendMessage( TASK_SECOND*30, TASK_FOREVER, []() {
    Serial.println("Task pop up\n");
    String msg = String("This is a message from: ") + nodeName + String(" for 2");
    String to = "2";
    mesh.sendBroadcast(msg);     
}); // start with a one second interval

Task taskSendMessageunitest( TASK_SECOND*5, TASK_FOREVER, []() {
    Serial.println("Task1 pop up\n");
    String msg = String("Surprise MOTHERFUCKER");
    uint32_t to = 3808757381;
    mesh.sendSingle(to, msg); 
     //mesh.sendBroadcast(msg); 
}); // start with a one second interval




void setup() {
  Serial.begin(115200);

 // mesh.setDebugMsgTypes(ERROR | DEBUG | CONNECTION);  // set before init() so that you can see startup messages

  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT);

  mesh.setName(nodeName); // This needs to be an unique name! 


  //WARNING TWO CALLBACKS FOR THE SAME EVENT JUST FOR TESTING
  mesh.onReceive([](uint32_t from, String &msg) {
    Serial.printf("Received message by id from: %u, %s\n", from, msg.c_str());
  });

  mesh.onReceive([](String &from, String &msg) {
    Serial.printf("Received message by name from: %s, %s\n", from.c_str(), msg.c_str());
  });

  mesh.onChangedConnections([]() {
    Serial.printf("Changed connection\n");
    Serial.println(mesh.subConnectionJson());
  });
  //Force this node as Root of the MESH
  mesh.setRoot();
  mesh.setContainsRoot();//Informs the node that a root should exist
  userScheduler.addTask(taskSendMessageunitest);
  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();
  taskSendMessageunitest.enable();
}

void loop() {
  userScheduler.execute(); // it will run mesh scheduler as well
  mesh.update();
}
