#include <SPIFFS.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include "ESPAsyncWebServer.h"

//function declarations
void timer(int startHr,int stopHr, int startMin, int stopMin);



// ds3231 time lib
#include "RTClib.h"
#include <Wire.h>
RTC_DS3231 rtc;
// variables for time trigering power tabs
String startTimevar, stopTimevar;
String x,xx,yy,y;
int xHr,xMin, yHr,yMin;
int counter =1; //used to prevent frequent calling of timer function
//DateTime now = rtc.now();

void timer(int startHr,int stopHr, int startMin, int stopMin)
{
  Serial.println("calling timer: not activated");
  Serial.print("StartHour:");
  Serial.println(startHr);
  DateTime now1 = rtc.now();
  Serial.print("timer hour time is:");
  Serial.println(now1.hour());
  bool timeStatus;
    if((now1.hour()>=startHr) && (now1.hour()<=stopHr)){
      if(now1.hour() == stopHr){
        timeStatus = (now1.minute() <= stopMin);
      }else{
        timeStatus = (now1.minute() >= startMin);
      }
      Serial.println(" ");
      Serial.print("counter value is: ");
      Serial.println(counter);
      Serial.print("timeStatus value is: ");
      Serial.print(timeStatus);
      //minutes loop
      switch(timeStatus){
        case true:{
          //only run if counter divisible by 2
          if(counter % 2 ==0 || counter ==1 ){
            if(messengerTx(1201)){
              counter = 3;
              Serial.println("calling timer:activated all");
            }
          }
          break;
        }
        case false:{
//          Serial.println("calling timer: deactivated");
          if(counter % 3 ==0 || counter == 1){
            Serial.println("calling timer: deactivated");
            if(messengerTx(1202)){
              counter =2;
            }
          }
          break;
        }
      }
    }
}
// nrf24 libs
#include <SPI.h>
#include "RF24.h"
//nrf24 configs and methods
byte addresses[][6] = {"1Node","2Node"};
RF24 radio(12,5);//for ESP32 use 12,5 for nano use 7,8
bool radioNumber = 0; 
struct dataStruct{
  int text;
};
// using struct to compress all variables instead of only web variables
struct test{
  int gen;
  bool light;
  bool pump;
} values;

bool messengerTx(int hi)
{
  dataStruct give, take;
//convert string to char array. N/B radio.write can't send strings
  give.text = hi;
  radio.stopListening(); 
  Serial.println(F("Now sending"));
  if (!radio.write( &give, sizeof(give) ))
  {
       Serial.println(F("message failed"));
  }
  radio.startListening();
  
  unsigned long started_waiting_at = micros(); 
  boolean timeout = false; 
  while ( ! radio.available() ){ 
    if (micros() - started_waiting_at > 200000 ){ 
      timeout = true;
      break;     
    }
  }
  if(!timeout){
    radio.read( &take, sizeof(take) );
//    check if message sent
    if(take.text == hi){
      Serial.println("message delivered");
      //also call decoder to update states
      decoder(take.text);
      return true;
    }else{
      return false;
      Serial.println("message not delivered");
    }
  }
  delay(1000);
}
int messengerRx(){
  dataStruct take;
  radio.startListening();
    if( radio.available()){                                                  // Variable for the received timestamp
      while (radio.available()) {                          // While there is data ready
        radio.read( &take, sizeof(take) );             // Get the payload
      }
      radio.stopListening();                               // First, stop listening so we can talk  
      radio.write(&take,sizeof(take));  
   }return take.text;
}
void decoder(int data){
  switch(data){
    case 1100:{
      values.gen =1100;
      break;
    }
    case 1101:{
      values.gen = 1101;
      break;
    }//iu
    case 1102:{
      values.gen = 1102;
      break;
    }
    case 1201:{
      //auto on
      // autoFunction(1);
      break;
    }
    case 1202:{
      //auto off
      // autoFunction(2);
      break;
    }//iu
    case 1103:{
      values.gen = 1103;
      break;
    }
    case 1300 :{
      values.gen =1100;
      //changeOver(1);
      // bool startEngine();
      break;
    }
    case 1301:{
      values.gen =1101;
      // stopEngine()
      //changeOver(2)  ;   
      break;
    }
    case 9900:{
      //ref for gen on
      values.gen =1100;
      
      /* not implemented here
      while(!messengerTx(7700)); 
      */
      break;
    }
    case 9901:{
      //ref for gen off
      values.gen =1101;
      /* not implemented here
      while(!messengerTx(7701)); 
      */
      break;
    }
    default:{
      Serial.println("no new message");
    }
  }
}
//network credentials
const char* apn = "MelcatAuto";
const char* pswd = "123456789";

//web credentials
const char* username = "admin";
const char* password = "admin";

//web server port
AsyncWebServer server(80);

//web variables
const int gen = 2;
const int light = 4;
const int pump = 13;

//web placeholders
String genstate, lightstate,pumpstate;

//function to get control states
String GenState(){
  if(values.gen ==1100){
     genstate = "ON";
  }
  else if(values.gen == 1101){
    genstate = "OFF";
  }
  else if(values.gen == 1102){
    genstate ="NEPA ON";
  }
  else if(values.gen ==1103)
  {
    genstate = "GEN ERR";
  }
  
  return genstate;
}
String LightState(){
  if(digitalRead(light)){
      lightstate = "ON";
  }
  else{
    lightstate = "OFF";
  }
  return lightstate;
}
String PumpState(){
  if(digitalRead(pump)){
    pumpstate = "ON";
  }
  else{
    pumpstate = "OFF";
  }
  Serial.println("this is the value of pumpstate: ");
  Serial.println(pumpstate);
  return pumpstate;    
}

String processor (const String& var){
  Serial.println(var);
  if(var == "Power"){
    return GenState();
   }
  else if(var == "Light"){
    return LightState();
  }
  else if(var == "Pump"){
    return PumpState();
  }
  return String();
}

void setup(){
  Serial.begin(115200);
  Serial.println("To God be the Glory");
  //initializing Wifi access point
  WiFi.softAP(apn,pswd);
  IPAddress IP = WiFi.softAPIP();
  Serial.println("Ap ip address is:");
  Serial.println(IP);
  //ds3231 time configs
  #ifndef ESP8266
    while (!Serial); // wait for serial port to connect. Needed for native USB
  #endif
  // config ds3231 timer
//  rtc.adjust(DateTime(2020,1,10,9,51,20));
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    abort();
  }
  
//  DateTime now = rtc.now();
  //initializing nrf24 radio
  radio.begin();
  radio.setPALevel(RF24_PA_HIGH);
  if(radioNumber){
    radio.openWritingPipe(addresses[1]);
    radio.openReadingPipe(1,addresses[0]);
  }else{
    radio.openWritingPipe(addresses[0]);
    radio.openReadingPipe(1,addresses[1]);
  }
  radio.startListening();
  
         
  //initializing pinmodes for power =12, light=13 and pump=14
  for(int i = 12; i<=14; i++){
    pinMode(i, OUTPUT);
  }
  pinMode(2, OUTPUT);

  //initializing SPIFFS
  if(!SPIFFS.begin(true)){
    Serial.println("could not start SPIFFS");
  }
  //Route for images
  server.on("/Logo.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/Logo.png", "image/png");
  });
  server.on("/appstore.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/appstore.png", "image/png");
  });
  server.on("/playstore.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/playstore.png", "image/png");
  });
  server.on("/1.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/1.png", "image/png");
  });
  server.on("/2.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/2.png", "image/png");
  });
  server.on("/3.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/3.png", "image/png");
  });
  server.on("/4.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/4.png", "image/png");
  });
  server.on("/5.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/5.png", "image/png");
  });
  
   // Routes to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });
//  server.on("/style1.css", HTTP_GET, [](AsyncWebServerRequest *request){
//    request->send(SPIFFS, "/style1.css", "text/css");
//  });
//  server.on("/bootstrap.min.css", HTTP_GET, [](AsyncWebServerRequest *request){
//    request->send(SPIFFS, "/bootstrap.min.css", "text/css");
//  });
//  server.on("/bootstrap-grid.min.css", HTTP_GET, [](AsyncWebServerRequest *request){
//    request->send(SPIFFS, "/bootstrap-grid.min.css", "text/css");
//  });
  
  //Routes for javaScript files
  server.on("/main.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/main.js", "text/js");
  });
  server.on("/app.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/app.js", "text/js");
  });
  server.on("/sw.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/sw.js", "text/js");
  });
  server.on("/manifest.json", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/manifest.json", "text/json");
  });
      
  //Routes for web pages
  server.serveStatic("/control",SPIFFS,"/control.html").setAuthentication(username, password);
  
   server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
     request->send(SPIFFS, "/index.html", String(), false, processor);
   });
    server.on("/fallback", HTTP_GET, [](AsyncWebServerRequest *request){
     request->send(SPIFFS, "/fallback.html", String(), false, processor);
   });
   server.on("/sw", HTTP_GET, [](AsyncWebServerRequest *request){
     request->send(SPIFFS, "/sw.js", String(), false, processor);
   });

//  **This contains methods for functions for custom login page
  server.on("/Loginsetup", HTTP_POST, [](AsyncWebServerRequest *request){
    if(request->hasParam("user", true)){
      AsyncWebParameter* p = request->getParam("user", true); //get login details from web form with name user
      AsyncWebParameter* q = request->getParam("pass", true); //get login details from web form with name pass
      Serial.println("this is username:"); 
      Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
      Serial.println("this is password");
      Serial.printf("POST[%s]: %s\n", q->name().c_str(), q->value().c_str()); 
      String p1= p->value().c_str();
      String q1= q->value().c_str();
      if((p1.equalsIgnoreCase(username))&&(q1.equalsIgnoreCase(password))){
        request->send(SPIFFS, "/control.html", String(), false, processor);
      }else{
        request->send(404);
        }
      }
   });

   // route for auto function
   server.on("/auto", HTTP_POST, [](AsyncWebServerRequest *request){
    if(request->hasParam("Start", true)){
      AsyncWebParameter* p = request->getParam("Start", true); //get login details from web form with name user
      AsyncWebParameter* q = request->getParam("Stop", true); //get login details from web form with name pass
      startTimevar= p->value().c_str();
      stopTimevar= q->value().c_str();
      request->send(200);
      DateTime now = rtc.now();
//      String x,xx,yy,y;
//      int xHr,xMin, yHr,yMin;
      x = startTimevar.substring(0,2);
      xHr = x.toInt();
      xx = startTimevar.substring(3,5);
      xMin = xx.toInt();
      y = stopTimevar.substring(0,2);
      yHr = y.toInt();
      yy = stopTimevar.substring(3,5);
      yMin = yy.toInt();
//      Serial.println("this is startTime real:"); 
//      Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
      Serial.println(" ");
      Serial.print("this is StartTime convert");
      Serial.println(xHr); 
      Serial.print("current time is:");
      Serial.println(now.hour()); 
//      timer(xHr,yHr,xMin,yMin); //calling from here didnt work as web does not support while or delay function
      }
   });
  
//  server.on("/control", HTTP_GET, [](AsyncWebServerRequest *request){
//    Serial.println("the control is now here");
//    request->send(SPIFFS, "/control.html", String(), false, processor);
//   });

//*Functions that control non-custom(default) login page
  server.on("/control", HTTP_GET, [](AsyncWebServerRequest *request){
    if(!request->authenticate(username, password))
      return request->requestAuthentication();
//     request->redirect("/control");
    request->send(SPIFFS, "/control.html", String(), false, processor);
  }); 
//  .setAuthentication("user","pass") only works with server.servestatic()


  //routes for generator
  server.on("/onGen", HTTP_GET, [](AsyncWebServerRequest *request){
    messengerTx(1300);
    counter =1;
    digitalWrite(gen,HIGH); // will send messenger to turn on instead of this
    request->send(200, "text/plain", GenState().c_str());
    // request->send(SPIFFS, "/control.html", String(), false, processor);
  });

  server.on("/offGen", HTTP_GET, [](AsyncWebServerRequest *request){
    messengerTx(1301);
    counter =1;
    digitalWrite(gen,LOW);
    request->send(200, "text/plain", GenState().c_str());
    // request->send(SPIFFS, "/control.html", String(), false, processor);
  });
  //update status
  server.on("/Gupdate", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", GenState().c_str());
  });

  //routes for Lighting
  server.on("/onLight", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(light,HIGH);
    request->send(200, "text/plain", LightState().c_str());
    // request->send(SPIFFS, "/control.html", String(), false, processor);
  });

  server.on("/offLight", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(light,LOW);
    request->send(200, "text/plain", LightState().c_str());
    // request->send(SPIFFS, "/control.html", String(), false, processor);
  });

  //routes for water pump
  server.on("/onPump", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(pump,HIGH);
    request->send(200, "text/plain", PumpState().c_str());
    // request->send(SPIFFS, "/control.html", String(), false, processor);
  });

  server.on("/offPump", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(pump,LOW);
    request->send(200, "text/plain", PumpState().c_str());
    // request->send(SPIFFS, "/control.html", String(), false, processor);
  });
  
  server.begin();
}
void loop(){
  DateTime now2 = rtc.now();
  Serial.println(" ");
  Serial.print("rtc current time is :");
  Serial.println(now2.hour(), DEC);
    Serial.print(':');
    Serial.print(now2.minute(), DEC);
    Serial.print(':');
    Serial.print(now2.second(), DEC);
    delay(3000);
    Serial.println();

  timer(xHr,yHr,xMin,yMin);
  decoder(messengerRx());
}
