// every function that is to be implemented by backend2 is rebroadcasted to the 
// frontend and sent from there in another messsage
#include <SPI.h>
#include "RF24.h"
byte addresses[][6] = {"1Node","2Node","3Node","4Node"};
bool radioNumber = 1;
RF24 radio(7,8);
//global variables
int response =  0;
struct dataStruct{
  int text;
};
struct test{
  int gen;
  bool light;
  bool pump;
} values;

//messenger to node 3
int y;
bool messengerNode3(int x ){
  radio.openWritingPipe(addresses[2]);//node3
  radio.openReadingPipe(1,addresses[3]);
  radio.stopListening();
  Serial.println(F("Messenger2 sending.."));
  if (!radio.write( &x, sizeof(x) ))
  {
     Serial.println(F("messenger2 failed"));
     if(radioNumber){
        radio.openWritingPipe(addresses[1]);
        radio.openReadingPipe(1,addresses[0]);
      }else{
        radio.openWritingPipe(addresses[0]);
        radio.openReadingPipe(1,addresses[1]);
      }
      delay(1000);
      radio.startListening();
      return false;
  }else{
    if(radioNumber){
      radio.openWritingPipe(addresses[1]);
      radio.openReadingPipe(1,addresses[0]);
    }else{
      radio.openWritingPipe(addresses[0]);
      radio.openReadingPipe(1,addresses[1]);
    }
    radio.startListening();
    delay(1000);
    return true;
  }

  Serial.println("got here1");
   
}

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
      return true;
    }else{
      return false;
      Serial.println("message not delivered");
    }
  }
  delay(1000);
}

unsigned int messengerRx(){
  dataStruct take;
  int backUp = take.text;
  radio.startListening();
    if( radio.available()){                                                  // Variable for the received timestamp
      while (radio.available()) {                          // While there is data ready
        radio.read( &take, sizeof(take) );             // Get the payload
      }
      radio.stopListening();                               // First, stop listening so we can talk  
      radio.write(&take,sizeof(take));  
   }return take.text;

}
void decoder(unsigned int data){
    switch(data){
    case 1100:{
      values.gen =1100;
      break;
    }
    case 1101:{
      values.gen = 1101;
      break;
    }
    case 1102:{
      values.gen = 1102;
      break;
    }
    case 1103:{
      values.gen = 1103;
      break;
    }
    case 1201:{
      //auto on
      autoFunction(1);
      break;
    }
    case 1202:{
      //auto off
      autoFunction(0);
      break;
    }//iu
    case 1300:{
      values.gen =1100;
      if(messengerNode3(9900)){
        // startEngine()
       changeOver(1);
      }
      break;
    }
    case 1301:{
      values.gen =1101;
      if(messengerNode3(9901)){
        // stopEngine()
        changeOver(2);
      } 
      break;
    }
    /*
    case 9900:{
      //ref for gen on
      while(!messengerTx(7700))
      {
        radio.stopListening(); 
      } 
      break;
    }
    case 9901:{
      //ref for gen on
      while(!messengerTx(7701)){
        radio.stopListening(); 
      }
      break;
    } //for backend2 
    */
    default:{
      Serial.println("no new message");
    }
  }
}

// hardware controls functions and variables
// current sensor relay variables
int RL1 = 10;
int RL2 = 9;
bool checkStartEngine(){
  digitalWrite(RL1,HIGH);
  digitalWrite(RL2,HIGH);
  delay(3000);
//  Serial.print("gen sensor val: ");
//  Serial.println(digitalRead(A1));
  if(digitalRead(A1)){
    return true;
  }else{
    return false;
  }
  delay(200);
}
bool checkNepa(){
  digitalWrite(RL1,LOW);
  digitalWrite(RL2,LOW);
  delay(3000);
//  Serial.print("nepa sensor val: ");
//  Serial.println(digitalRead(A1));
  if(digitalRead(A1)){
    return true;
  }else{
    return false;
  }
  delay(200);
}
//changeOver relay variables
int RL3 = 5;
int RL4 = 6;
int state; // variable for current runnig phase, nepa 2 or gen 1
//updated changeOver configs N/B these relays are active low
int changeOver(int source){
  if(source==1){
    Serial.println("Changing over to Gen");
    digitalWrite(RL3,LOW); //normally closed state of the relay
    digitalWrite(RL4,HIGH);
    state = 1;
    return 1;
  }else if(source == 2){
    Serial.println("Changing over to Nepa");
    digitalWrite(RL3,HIGH);
    digitalWrite(RL4,LOW);
    state = 2;
    return 2;
  }
}
void autoFunction(int x){
  Serial.println("autoFn called");
   switch(x){
    case 0:{
      Serial.println("autoFn case0");
      if(state==1){
        delay(300);
        if(checkNepa()){
          messengerNode3(9901); //stopEngine();
          changeOver(2); // to nepa
          Serial.println("case0: state1 checkNepa()");
          delay(300);
        }
      }else if(state ==2){
        delay(300);
        if(!checkNepa() && checkStartEngine()){
          changeOver(1);
          Serial.println("case0: state2 checkStartEngine()");
        }else{
          changeOver(2);
          Serial.println("case0: state2 checkStartEngine() else");
        }
      }
      break;
    }
    case 1: {
      Serial.println("autoFn case1");
      if(state ==1){
        if(checkNepa() ==1){
          changeOver(2); // to nepa
          messengerNode3(9901); //stopEngine();
          Serial.println("case1: state2 checkNepa()");
          delay(300);
        }
      }else if(state ==2){
        if(!checkNepa() && !checkStartEngine()){
          messengerNode3(9900); //startEngine();
          changeOver(1);
           Serial.println(" case1 \n state2!checkNepa && !checkStartEngine()");
        }else if(checkNepa()){
          changeOver(2);
           Serial.println("case1: state2 checkNepa()");
        }
      }
      break;
    }
   }
}
//void manualOn(){
//  //
//  Serial.println("manual On mode");
//  // startEngine();//message for backend2
//  while(!messengerTx(9900)){
//    radio.stopListening();
//  }
//  changeOver(1);
//}
//void manualOff(){
//  Serial.println("manual Off mode");
//  // stopEngine();
//  while(!messengerTx(9901)){
//    radio.stopListening();
//  }
//  changeOver(2);
//}
void setup() {
  // put your setup code here, to run once:
  
  //set to nepa by initially
  pinMode(RL3,OUTPUT); //for changeover
  pinMode(RL4,OUTPUT);
  changeOver(2); //change over to nepa

  pinMode(RL1,OUTPUT); //for current sensor
  pinMode(RL2,OUTPUT);
  
  Serial.begin(115200);
  radio.begin();
  radio.setPALevel(RF24_PA_HIGH);
  if(radioNumber){
    radio.openWritingPipe(addresses[1]);
    radio.openReadingPipe(1,addresses[0]);
  }else{
    radio.openWritingPipe(addresses[0]);
    radio.openReadingPipe(1,addresses[1]);
  }
  // Start the radio listening for data
  radio.startListening();
  autoFunction(0);
}
void loop() {
  // put your main code here, to run repeatedly:
//  if(messengerNode3()
  response = messengerRx();
  delay(300);
  Serial.print("response is: ");
  Serial.println(response);
  decoder(response);
  delay(300);
//  messengerNode3(9901);
//  delay(300);
}
