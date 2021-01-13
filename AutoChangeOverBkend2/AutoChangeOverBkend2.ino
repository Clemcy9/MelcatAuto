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
bool messengerNode3(int x ){
  int y;
  radio.openWritingPipe(addresses[2]);//node3
  radio.openReadingPipe(1,addresses[3]);
  radio.stopListening();
  Serial.println(F("Messenger2 sending.."));
  if (!radio.write( &x, sizeof(x) ))
  {
       Serial.println(F("messenger2 failed"));
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
    radio.read( &y, sizeof(y) );
//    check if message sent
    if(y == x){
      Serial.println("messenger2 sent");
      return true;
    }else{
      return false;
      Serial.println("messenger2 not delivered");
    }
  }
  delay(1000);
 
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
//      autoFunction(1);
      break;
    }
    case 1202:{
      //auto off
      //autoFunction(0);
      break;
    }//iu
    case 1300:{
      values.gen =1100;
//       changeOver(1);
      // bool startEngine();
      break;
    }
    case 1301:{
      values.gen =1101;
      // stopEngine()
//       changeOver(2);   
      break;
    }
    case 9900:{
      //ref for gen on
      startEngine();
      break;
    }
    case 9901:{
      //ref for gen on
      stopEngine();
      break;
    }
    default:{
      Serial.println("no new message");
    }
  }
}

/*
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
        delay(2000);
        if(checkNepa()){
//          stopEngine();
          changeOver(2); // to nepa
//          state =2;
           Serial.println("case0: state1 checkNepa()");
          delay(300);
        }
      }else if(state ==2){
        delay(2000);
        if(!checkNepa() && checkStartEngine()){
          changeOver(1);
//          state = 1;
          Serial.println("case0: state2 checkStartEngine()");
        }else{
          changeOver(2);
            Serial.println("case0: state2 checkStartEngine() else");
//          state = 2;
        }
      }
      break;
    }
    case 1: {
      Serial.println("autoFn case1");
      if(state ==1){
        if(checkNepa() ==1){
//          stopEngine();
          changeOver(2); // to nepa
//          stopEngine();
//          state =2;
          Serial.println("case1: state2 checkNepa()");
          delay(300);
        }
      }else if(state ==2){
        if(!checkNepa() && !checkStartEngine()){
//          startEngine();
          changeOver(1);
//          state = 1;
           Serial.println(" case1 \n state2!checkNepa && !checkStartEngine()");
        }else if(checkNepa()){
          changeOver(2);
//          state = 2;
           Serial.println("case1: state2 checkNepa()");
        }
      }
      break;
    }
   }
}
//for backend1
*/
int RL1 = 10; //relay pins N/B relay is active low
int RL2 = 9;
int startState =1;
bool startEngine(){
  Serial.println("Starting gen: deactivated");
  if (startState==1 || startState % 2 ==0)
  {
    Serial.println("Starting gen: activated");
    int kickTimes =0;
    Serial.println("Starting Engine");
    digitalWrite(RL1,LOW); //turns key to on position
    delay(2000);
    crank: digitalWrite(RL2,LOW); //crank engine
    kickTimes++;
    delay(3000);
    digitalWrite(RL2,HIGH);
    /*
    if(!checkStartEngine()){
      if(kickTimes<3){
        goto crank;
      }else if(kickTimes>3){
        Serial.println("Unable to start Engine");
        return false;
      }
    }else if(checkStartEngine()){
      Serial.println("Engine Started");
      return true;
    }
    */
    startState = 3;
  }  
}

bool stopEngine(){
  Serial.println("stopping gen: deactivated");
  if(startState ==3 || startState ==1){
    Serial.println("stopping gen: activated");
    digitalWrite(RL1,HIGH); //turn key to off position
    /*
    if(!checkStartEngine()){
      Serial.println("Engine has been turned off");
      return true;
    }else{
      return false;
    }
    */
    startState =2;
  }
}
/*
void manualOn(){
  //
  Serial.println("manual On mode");
  // startEngine();//message for backend2
  while(!messengerTx(9900)){
    radio.stopListening();
  }
  changeOver(1);
}
void manualOff(){
  Serial.println("manual Off mode");
  // stopEngine();
  while(!messengerTx(9901)){
    radio.stopListening();
  }
  changeOver(2);
} //not implemented
*/
void setup() {
  // put your setup code here, to run once:
  
  //set to nepa by initially
  /*
  pinMode(RL3,OUTPUT); //for changeover
  pinMode(RL4,OUTPUT);
  changeOver(2); //change over to nepa
  */
  pinMode(RL1,OUTPUT); //for starter relay
  pinMode(RL2,OUTPUT);
  digitalWrite(RL1,HIGH); //TURN OFF
  digitalWrite(RL2,HIGH);
  
  Serial.begin(115200);
  radio.begin();
  radio.setPALevel(RF24_PA_HIGH);
//  if(radioNumber){
    radio.openWritingPipe(addresses[3]);
    radio.openReadingPipe(1,addresses[2]);
//  }else{
//    radio.openWritingPipe(addresses[0]);
//    radio.openReadingPipe(1,addresses[1]);
//  }
  // Start the radio listening for data
  radio.startListening();
}
void loop() {
  // put your main code here, to run repeatedly:

    response = messengerRx();

  delay(300);
  Serial.print("response is: ");
  Serial.println(response);
  decoder(response);
  delay(300);
  

}
