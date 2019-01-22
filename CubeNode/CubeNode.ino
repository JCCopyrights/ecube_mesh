#include <Arduino.h> 
#include "ecube_led.h"
#include "pitches.h"
#include "ecube_buzzer.h"
#include "ecube_ssd1306.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "I2Cdev.h"
#include "MPU6050.h"
#include "namedMesh.h"

#define   MESH_SSID       "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555
#define   CHANNEL         6

Scheduler             userScheduler; 
namedMesh             mesh;
SimpleList<uint32_t>  nodes;

String nodeName; // Name needs to be unique

//I2C Chain
void scan_i2c();
byte i2c_address[5];
unsigned int nDevices=0;

Adafruit_SSD1306 ssd_display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

MPU6050 sensor(0x68);

float ang_disp;                               //Rotation in Axis
float ang_x_prev, ang_y_prev, ang_z_prev;     //Previous Rotation for Integration
unsigned char cube_face=0, prev_cube_face=0;  //Standing Cube Face
long tiempo_prev=0;                           //Actual Time of execution
float dt;                                     //Integration Step

   
Task taskSendMessage( TASK_SECOND/2, TASK_FOREVER, []() {
  String msg = String("ANG_")+ String((unsigned int)(abs(900*ang_disp)+1), DEC);
  mesh.sendBroadcast(msg); 
});

Task taskMPU( TASK_SECOND/50, TASK_FOREVER, []() {
  //ANGLE CALCULATION
  int16_t gx, gy, gz;
  sensor.getRotation(&gx, &gy, &gz);
  dt = (millis()-tiempo_prev)/1000.0;
  tiempo_prev=millis();
  float ang_x, ang_y, ang_z;
  ang_x = (gx/131)*dt/1000.0 + ang_x_prev;
  ang_y = (gy/131)*dt/1000.0 + ang_y_prev;
  ang_z = (gz/131)*dt/1000.0 + ang_z_prev;
  ang_x_prev=ang_x;
  ang_y_prev=ang_y;
  ang_z_prev=ang_z;

  //ACCELERATION CALCULATION
  int16_t ax, ay, az; 
  float ax_m_s2,ay_m_s2,az_m_s2;
  sensor.getAcceleration(&ax, &ay, &az);
  ax_m_s2 = ax * (9.81/16384.0);
  ay_m_s2 = ay * (9.81/16384.0);
  az_m_s2 = az * (9.81/16384.0);
  
  if(az_m_s2>8){
    cube_face=1;
    ang_disp=ang_z;}
  else if (az_m_s2<-8){
    cube_face=2;
    ang_disp=ang_z;}
  else if (ax_m_s2>8){
    cube_face=5;
    ang_disp=ang_x;}
  else if (ax_m_s2<-8){
    cube_face=6;
    ang_disp=ang_x;}
  else if (ay_m_s2<-8){
    cube_face=3;
    ang_disp=ang_y;}
  else if (ay_m_s2>8){
    cube_face=4;
    ang_disp=ang_y;}
  else{
    cube_face=0;
    ang_disp=0;}

  if(prev_cube_face!=cube_face){//Cube has moved
    ang_x=0; ang_y=0; ang_z=0;
    ang_x_prev=0; ang_y_prev=0; ang_z_prev=0;
    String msg = String("FAC_")+ String(cube_face, DEC);
    mesh.sendBroadcast(msg); 
   // msg = String("ANG_")+ String(1, DEC);
   // mesh.sendBroadcast(msg); 
  }
  prev_cube_face=cube_face;
  //TODO: Reset Giriscope values when changin the active face
  ssd_display.clearDisplay();
  ssd_display.setTextSize(1.9);   // Normal 1:1 pixel scale
  ssd_display.setTextColor(WHITE);// Draw white text
  ssd_display.setCursor(0,0);     // Start at top-left corner
  ssd_display.print("ax_m_s2: \t");
  ssd_display.println(ax_m_s2);
  ssd_display.print("ay_m_s2: \t");
  ssd_display.println(ay_m_s2);
  ssd_display.print("az_m_s2: \t");
  ssd_display.println(az_m_s2);
  ssd_display.print("eCube Face: \t");
  ssd_display.println(cube_face);
  ssd_display.print("Angulo X: \t");
  ssd_display.println(ang_x*900);
  ssd_display.print("Angulo Y: \t");
  ssd_display.println(ang_y*900);
  ssd_display.print("Angulo Z: \t");
  ssd_display.println(ang_z*900);
  ssd_display.display(); 
  turn_led(LEDINT, LOW);
}); // start with a one second interval

void setup() {
  //PLATFORM INITIALIZATION
  Serial.begin(115200);
  initialize_led();
  //initialize_buzzer();
  Wire.begin(); 
  scan_i2c();
  initialize_ssd1306(ssd_display);
  sensor.initialize(); 
  if (sensor.testConnection()) 
    Serial.println("Accelerometer Detected");
  else 
    Serial.println("Error in Accelerometer");
  turn_led(LEDINT, LOW);
  turn_led(LED0, LOW);
  turn_led(LED1, LOW);
  turn_led(LED2, LOW);
  
  //MESH CONFIGURATION
  //mesh.setDebugMsgTypes(ERROR | DEBUG | CONNECTION);  // set before init() so that you can see startup messages
  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT,WIFI_AP_STA,CHANNEL);
  nodeName = "CubeNode_"+String(mesh.getNodeId(),DEC);
  mesh.setName(nodeName); // This needs to be an unique name! 
  //Callback for incoming messages
  mesh.onReceive([](String &from, String &msg) {
    Serial.printf("Message from: %s, %s\n", from.c_str(), msg.c_str());
  });
  //Callback for Changed Mesh
  mesh.onChangedConnections([]() {
    Serial.printf("Changed connection\n");
    String msg = String("FAC_")+ String(cube_face, DEC);
    mesh.sendBroadcast(msg); 
  });
  mesh.setContainsRoot();//Informs the node that a root should exist
  userScheduler.addTask(taskSendMessage);
  userScheduler.addTask(taskMPU);
  taskSendMessage.enable();
  taskMPU.enable();
}
void loop() {
  userScheduler.execute(); // it will run mesh scheduler as well
  mesh.update();
}

void scan_i2c(){
  byte error, address;
  Serial.println("Scanning...");
  nDevices=0;
  for(address = 1; address < 127; address++ )
  {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0)
    {
      Serial.print("I2C Device in ADDRESS 0x");
      if (address<16)
        Serial.print("0");
      i2c_address[nDevices]=address;
      Serial.print(address,HEX);
      Serial.println("  !");
      nDevices++;
    }
    else if (error==4)
    {
      Serial.print("Unknown error in ADDRESS 0x");
      if (address<16)
        Serial.print("0");
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0)
    Serial.println("No I2C Device connected\n");
  else
    Serial.println("Done\n");
  delay(1000);    
}
