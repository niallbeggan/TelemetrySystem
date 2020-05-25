// This Arduino program sends dummy CAN Bus messages for all 17 signals.
// It is the final CAN bus dummy signals code for this FYP.

// Written by Niall Beggan 31/04/2020
// Last Updated: 7/5/2020 - changed all values to big endian


//####################################################
// Include libraries

#include <SPI.h>          //SPI library is used to talk to the CAN controller
#include <mcp_can.h>      //CAN bus shield library


//####################################################
// Set up CAN SPI communication

MCP_CAN CAN(10);          //Set SPI chip select to pin 10

void setup() {  
  //tries to initialize, if failed --> it will loop here for ever
  START_INIT:
  if(CAN_OK == CAN.begin(CAN_100KBPS)) {      //setting CAN baud rate to 500Kbps
    //NOP
  }
  else {
      delay(100);
      goto START_INIT;
  }
}


//####################################################
// Initialise all variables.

bool eStop1 = 1;  
float bmsTemp2 = 0;
float carSpeed3 = 0;
float bmsVoltage4 = 0;
float bmsCurrent5 = 0;
float tsPowerkW6 = 0;
float leftMotorVoltage7 = 0;
float rightMotorVoltage8 = 0;
float leftMotorCurrent9 = 0;
float rightMotorCurrent10 = 0;
float steeringInput11 = 0;
float acceleratorPedalPosition12 = 0;

// Further signals not yet on Raimonds CANBus ID List
float brakePedalPosition13 = 0;
float suspensionsFrontLeft14 = 0;
float suspensionsFrontRight15 = 0;
float suspensionsRearLeft16 = 0;
float suspensionsRearRight17 = 0;

// These variables are just for the generateDummyValues
int count = 0;
long UTCtimeStamp = 50000;
int UTC_millis = 0;


//####################################################
// Send loop.

void loop()
{ 
  generateDummyValues(); // Makes up some half realistic values

  // Send dummy CAN bus messages
  sendCAN_Message_with_timestamp(eStop1, 1);
  sendCAN_Message_with_timestamp(bmsTemp2, 2);
  sendCAN_Message_with_timestamp(carSpeed3, 3);
  sendCAN_Message_with_timestamp(bmsVoltage4, 4);
  sendCAN_Message_with_timestamp(bmsCurrent5, 5);
  sendCAN_Message_with_timestamp(tsPowerkW6, 6);
  sendCAN_Message_with_timestamp(leftMotorVoltage7, 7);
  sendCAN_Message_with_timestamp(rightMotorVoltage8, 8);
  sendCAN_Message_with_timestamp(leftMotorCurrent9, 9);
  sendCAN_Message_with_timestamp(rightMotorCurrent10, 10);
  sendCAN_Message_with_timestamp(steeringInput11, 11);
  sendCAN_Message_with_timestamp(acceleratorPedalPosition12, 12);
  
  // NOT ON CAN BUS ID LIST YET
  sendCAN_Message_with_timestamp(brakePedalPosition13, 13);
  sendCAN_Message_with_timestamp(suspensionsFrontLeft14, 14);
  sendCAN_Message_with_timestamp(suspensionsFrontRight15, 15);
  sendCAN_Message_with_timestamp(suspensionsRearLeft16, 16);
  sendCAN_Message_with_timestamp(suspensionsRearRight17, 17);

  // Small delay
  delay(166);
}


//####################################################
// Send CAN function that includes timestamp & delay.

void sendCAN_Message_with_timestamp(float signalValue, int CAN_ID) { // This function takes signed 2 byte value (-3276.8 up to 3276.7), adds a dummy timestamp and sends it onto the bus!

  // Initialise 8 byte message array
  unsigned char message[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  
  // Split signal into two bytes and add to message
  short oneDecimalPlaceValue = round(signalValue * 10); // Send with one decimal place. Currently only sending whole integers anyway.
  byte small = (byte) (oneDecimalPlaceValue & 0xFF);
  byte big = (byte) ((oneDecimalPlaceValue >> 8) & 0xFF);
  message[0] = small;
  message[1] = big;

  // Split UTCtimestamp into 4 bytes and add it to the message
  for (int i = 0; i < 4; i++) {
    message[i+2] = ((UTCtimeStamp >> (i*8)) & 0xFF); // Extract the right-most byte of the shifted variable
  }

  // Split the timestamp milliseconds into two bytes and add it to the message
  oneDecimalPlaceValue = UTC_millis;
  small = (byte) (oneDecimalPlaceValue & 0xFF);
  big = (byte) ((oneDecimalPlaceValue >> 8) & 0xFF);
  message[6] = small;
  message[7] = big;

  // Send message!
  CAN.sendMsgBuf(CAN_ID, 0, 8, message);
  delay(2); // Delay between sends because something is really slow.. ? If you dont, messages get dropped
}


//####################################################
// Generate dummy data

void generateDummyValues() { // Generates dummy data for example & sys testing
  if(count == 1200) { // 1200 samples, 5 a second, therefore roughly 4 minute loop
    count = 0;
  }
  count++;
  
  //####################################################
  // Timestamp
  UTC_millis += 200;
  if(UTC_millis == 1000){
    UTC_millis = 0;
  }
  
  //####################################################
  // eStop
  if(count > 30) {
    eStop1 = 1;
  }
  
  //####################################################
  // BMS temp - dummy signal generation
  bmsTemp2 = round( 30*sqrt(sqrt(sqrt(count))) + (bmsCurrent5/60) - 10);

  //####################################################
  // Speed - dummy signal generation (quite long)
  if(count < 52) { // Speed is zero until count is 52
    carSpeed3 = 0;
  }
  else if((count > 51) && (count < 150)) { // This brings speed up from 0-100
    carSpeed3 = count-50;
  }
  else if(count < 230) { // down from 100 - 20
    carSpeed3 = 250-count;
  }
  else if((count < 300) && (count > 229)) { // stay at 20
    carSpeed3 = 20;
  }
  else if(count < 360) { // 20 up to 80
    carSpeed3 = count-280;
  }
  else if(count < 440) { // 80 down to 1
    carSpeed3 = 440 - count;
  }
  else if(count < 540) { // 1 up to 99
    carSpeed3 = count - 440;
  }
  else if(count < 700) { // stay at 100
    carSpeed3 = 100;
  }
  else if(count < 800) { // 100 down to 0
    carSpeed3 = 800 - count;
  }
  else{
    carSpeed3 = 0;
  }

  //####################################################
  // BMS voltage - dummy signal generation
  bmsVoltage4 = (76 - sqrt(count/2)) - (bmsCurrent5/60);
  
  //####################################################
  // BMS current - dummy signal generation
  bmsCurrent5 = carSpeed3*3;
  
  //####################################################
  // Left motor voltage - dummy signal generation
  leftMotorVoltage7 = bmsVoltage4 - 3;
  
  //####################################################
  // Right motor voltage - dummy signal generation
  rightMotorVoltage8 = bmsVoltage4 - 3;
  
  //####################################################
  // Left motor current - dummy signal generation
  leftMotorCurrent9 = 50*sin((count%100)/(100/(2*PI)));
  
  //####################################################
  // Right motor current - dummy signal generation
  rightMotorCurrent10 = 0;

  //####################################################
  // Accelerator & Brake pedal position (0 - 100%) - dummy signal generation
  acceleratorPedalPosition12 = carSpeed3;
  //
  if(acceleratorPedalPosition12 == 0) {
    brakePedalPosition13 = 100;
  }
  else {
    brakePedalPosition13 = 0;
  }

  //####################################################
  // Suspension front left (0 - 100%) - dummy signal generation
  suspensionsFrontLeft14 = count%100;
  
  //####################################################
  // Suspension front right (0 - 100%) - dummy signal generation
  suspensionsFrontRight15 = count%100;
  
  //####################################################
  // Suspension rear left (0 - 100%) - dummy signal generation
  suspensionsRearLeft16 = count%100;
  
  //####################################################
  // Suspension rear right (0 - 100%) - dummy signal generation
  suspensionsRearRight17 = count%100;
  
  //####################################################
  // Power - dummy signal generation
  tsPowerkW6 = (bmsVoltage4 * bmsCurrent5)/1000;
  
  //####################################################
  // Steering input (0 - 100%) - dummy signal generation
  steeringInput11 = 50*sin((count%100)/(100/(2*PI)));
 }
