/*
RGB LED Node for 12v common anode rgb strip,
Also has a standard on/off switch to control an extra set of garden lights 
PWM pins (3, 5, 6, 9, 10, or 11).
*/

// Enable debug prints to serial monitor
#define MY_DEBUG 
//#define MY_NODE_ID 200  // I use 200 for my test nodes I will remove this line when it is installed
// Enable and select radio type attached
#define MY_RADIO_NRF24
 
#include <SPI.h>
#include <MySensors.h>
#define RGB_CHILD_ID 1  // the rbg device includes a dimmer and on/off switch as well
//#define CHILD_ID_LIGHT_B 2

MyMessage rgb_msg(RGB_CHILD_ID,V_RGBW);
MyMessage status_msg(RGB_CHILD_ID,V_STATUS);
MyMessage dimmer_msg(RGB_CHILD_ID, V_DIMMER );

 //const int controlPinA = 7;  //used to turn of rgb Power pin
 //const int controlPinB = 8;  //used to switch off other garden lights
 const int redPin = 3;     //pin used to switch the red mosfet
 const int greenPin = 5;  //pin used to switch the green mosfet
 const int bluePin = 6;  //pin used to switch the blue mosfet
 long RGB_values[3] = {0,0,0};  //array used to hold the three rgb colours
 int dimSet = 0;                 // holder for the current dimmer setting
// int dimHolder = 0;             

 
void setup()
{
  //pinMode(controlPinA, OUTPUT);    //|
  //pinMode(controlPinB, OUTPUT);    //|
  pinMode(redPin, OUTPUT);         //| setup pins as outputs
  pinMode(greenPin, OUTPUT);       //| 
  pinMode(bluePin, OUTPUT);        //|
  setColor(0,0,0);                 // make sure lights are off when first booted
  send(rgb_msg.set("ff0000ff"));
  send(status_msg.set(0));
  send( dimmer_msg.set(0) );
}

void presentation()  {
  // Present sketch (name, version)
  sendSketchInfo("RGB Node", "1.0");        
       
  // Register sensors (id, type, description, ack back) 
   present(RGB_CHILD_ID, S_RGB_LIGHT,"RGB Control");
   //present(CHILD_ID_LIGHT_B, S_LIGHT,"Firepit Lights");
}
 
void loop()
{
  //nothing needed in here
}

/*-----------------Start of functions---------------------*/


/*----------- function to display LED colour as well as set brightness------------*/
void setColor(int red, int green, int blue){  
if (dimSet != 0){
 analogWrite(redPin, round(red*dimSet/100));          //|
 analogWrite(greenPin, round(green*dimSet/100));      //| change the three rgb colours and adjust brightness
 analogWrite(bluePin, round(blue*dimSet/100));        //|
}
else{                                                // if dimmer setting is 0 then set all colours to 0
  analogWrite(redPin, dimSet);
  analogWrite(greenPin, dimSet);
  analogWrite(bluePin, dimSet);
}  
#ifdef MY_DEBUG            //Print debug info if enabled
  Serial.print("RED = ");
  Serial.println(red);
  Serial.print("BLUE = ");
  Serial.println(blue);
  Serial.print("GREEN = ");
  Serial.println(green);
 #endif
}



void receive(const MyMessage &message) {

  if (message.type == V_STATUS) {
    
    Serial.print("Turning light ");
    Serial.println(message.getInt());
    send(status_msg.set(message.getInt()));
  }
  
 if (message.type==V_RGB) {             //check for RGB message type
      /*   process the RGB hex code    */
    send(rgb_msg.set(message.getLong()));
    String rgbHexString = message.getString();                      //Load the hex color code into a string
    long number = (long) strtol( &rgbHexString[0], NULL, 16);
    RGB_values[0] = number >> 16;
    RGB_values[1] = number >> 8 & 0xFF;
    RGB_values[2] = number & 0xFF;
    setColor(RGB_values[0],RGB_values[1],RGB_values[2]);            // call the setColor function to update the LED's
    //send(rgb_msg.set(rgbHexString));
  #ifdef MY_DEBUG                                                   //Print debug info if enabled
     Serial.print("Message is  " );
     Serial.println(message.getString());
     Serial.print("HEX RGB " );
     Serial.println(rgbHexString);
     Serial.print("long number " );
     Serial.println(number);
  #endif
 }
     
 if (message.type==V_DIMMER) {                                        //check for dimmer message type
    dimSet = message.getInt();                                        // Load the dimmer setting into dimSet
    send(dimmer_msg.set(message.getInt()));
    setColor(RGB_values[0],RGB_values[1],RGB_values[2]);              // call the setColor function to update the LED's. new dimmer setting
    #ifdef MY_DEBUG                                                   //Print debug info if enabled
     Serial.print("dimmer setting " );
     Serial.println(dimSet);
    #endif
 }
      
 if (message.type==V_LIGHT) {                                     //check for incoming  light switch message type
    switch (message.sensor) {
      case 1:                                                     //message is for sensor 1 (RGB switch)
        //digitalWrite(controlPinA, message.getBool() ? 1 : 0);
        break;
      case 2:                                                     //message is for sensor 2 (garden lights)
        //digitalWrite(controlPinB, message.getBool() ? 1 : 0);
        break;
     }
    #ifdef MY_DEBUG
      Serial.print("Incoming change for sensor:");
      Serial.println(message.sensor);
      Serial.print("Light switch setting " );
      Serial.println(message.getBool());
    #endif
    }
 }




