#include "namedMesh.h"

#define   MESH_SSID       "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555
#define   CHANNEL         6

Scheduler             userScheduler; 
namedMesh             mesh;
SimpleList<uint32_t>  nodes;

String nodeName; // Name needs to be unique
int angulo=180;
int face=0;
bool ledstate=LOW;

const long A = 1000;
const int B = 15;
const int Rc = 10;
const int LDRPin = A0;
int V;
int ilum;

Task taskSendMessage( TASK_SECOND*40, TASK_FOREVER, []() {
    String msg = String("Message from: ") + nodeName + String(" for all Nodes");
    //mesh.sendSingle(to, msg); 
     mesh.sendBroadcast(msg); 
});

long task_period=TASK_SECOND;
Task taskRGB( TASK_SECOND, TASK_FOREVER, []() {
  V = analogRead (LDRPin);
  ilum = ((long)V*A*10)/((long)B*Rc*(1024-V));
  String msg = String("LDR_") + String(ilum,DEC);
  //mesh.sendSingle(to, msg); 
  mesh.sendBroadcast(msg); 
  
  switch (face){
    case 1:
      analogWrite (D2, angulo*0.71);//(255/360)
      analogWrite (D5, 0);
      analogWrite (D6, 0);
      break;
    case 2:
      analogWrite (D5, angulo*0.71);
      analogWrite (D2, 0);
      analogWrite (D6, 0);
      break;
    case 3:
      analogWrite (D6, angulo*0.71);
      analogWrite (D2, 0);
      analogWrite (D5, 0);
      break;
    case 4:
      if(angulo<=120){
        analogWrite (D2, -2.125*angulo+255.0);//(-255/120)
        analogWrite (D5, angulo/120.0*255.0); 
        analogWrite (D6, 0);}
      else if (angulo>120&&angulo<=240){
        analogWrite (D2, 0);
        analogWrite (D5, -2.125*(angulo-120.0)+255.0);
        analogWrite (D6, (angulo-120.0)/120.0*255.0);}
      else{
        analogWrite (D5, 0);
        analogWrite (D6, -2.125*(angulo-240.0)+255.0);
        analogWrite (D2, (angulo-240.0)/120.0*255.0);}
      break;
    case 5:
      digitalWrite (D2, ledstate);
      digitalWrite (D5, ledstate);
      digitalWrite (D6, ledstate);
      ledstate=!ledstate;
      task_period=TASK_SECOND*angulo/180.0;
      taskRGB.setInterval(task_period);
      break;
    case 6:
      analogWrite (D5, 5000.0/(ilum+1));
      analogWrite (D6, 5000.0/(ilum+1));
      analogWrite (D2, 5000.0/(ilum+1));
      break;
    default:  
      break;
  }

  if(face==5&&ledstate)//Allows sync of blink
    taskRGB.delay(2*task_period -(mesh.getNodeTime() % (2*task_period*1000))/1000);
  else
    taskRGB.delay(task_period -(mesh.getNodeTime() % (task_period*1000))/1000);
}); // start with a one second interval

void setup() {
  Serial.begin(115200);
  //mesh.setDebugMsgTypes(ERROR | DEBUG | CONNECTION);  // set before init() so that you can see startup messages
  pinMode (D2,OUTPUT);
  pinMode (D5,OUTPUT);
  pinMode (D6,OUTPUT);
  pinMode (A0,INPUT);
  digitalWrite (D2, LOW);
  digitalWrite (D5, LOW);
  digitalWrite (D6, LOW);
  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT,WIFI_AP_STA,CHANNEL);
  nodeName = "LDRNode_"+String(mesh.getNodeId(),DEC);
  mesh.setName(nodeName); // This needs to be an unique name! 
  //Callback for incoming messages
  mesh.onReceive([](String &from, String &msg) {
    Serial.printf("Message from: %s, %s\n", from.c_str(), msg.c_str());
    String header= msg.substring(0, 3);//Decoding of messages
    String number= msg.substring(4);
    int i_dec = number.toInt();
    //Serial.printf("Header:%s, Int:%s, interpreted as: %i\n", header.c_str(), number.c_str(),i_dec);
    if (header=="FAC")
      face=i_dec;
    else if (header=="ANG")
        angulo=i_dec;
  });
  //Callback for Changed Mesh
  mesh.onChangedConnections([]() {
    Serial.printf("Changed connection\n");
    //For every Change in Mesh recalculate delays
   /* nodes = mesh.getNodeList();
    SimpleList<uint32_t>::iterator node = nodes.begin();
    while (node != nodes.end()) {
      mesh.startDelayMeas(*node);
      node++;}*/
  });
  mesh.setContainsRoot();//Informs the node that a root should exist
  userScheduler.addTask(taskSendMessage);
  userScheduler.addTask(taskRGB);
  taskSendMessage.enable();
  taskRGB.enable();
}
void loop() {
  userScheduler.execute(); // it will run mesh scheduler as well
  mesh.update();
}
