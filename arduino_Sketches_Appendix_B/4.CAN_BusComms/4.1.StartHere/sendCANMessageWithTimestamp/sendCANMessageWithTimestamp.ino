// This program sends a value & a timestamp over a CAN bus.
// It was written to check if the MCP2515 modules were setup
// correctly and work as expected.
// Written by Niall Beggan
// Last updated: 7/5/2020 - changed all values to big endian

// The format of the messages is: 8 bytes
// byte 0 - 1 - Signal - LSB first
// byte 2 - 5 - Timestamp Seconds - LSB first
// byte 6 - 7 - Timestamp milliSeconds - LSB first


//####################################################
// Include libraries

#include <SPI.h>          // SPI library is used to talk to the CAN controller
#include <mcp_can.h>      // CAN bus shield library


//####################################################
// Set up CAN, SPI & Serial communication

MCP_CAN CAN(10);          // Set SPI chip select to pin 10

void setup() {
  Serial.begin(57600);
  //Tries to initialize, if failed --> it will loop here for ever
  START_INIT:

  //Setting CAN baud rate to 100kbps
  if (CAN_OK == CAN.begin(CAN_100KBPS))
  {
    Serial.println("CAN Bus Shield Init Ok!");
  }
  else {
    delay(1000);
    goto START_INIT;
  }
}


//####################################################
// Declare variables

long UTCtimeStamp = 50000;
int UTC_millis = 999;
float signalValue = 77.7;


//####################################################
// Main loop

void loop() {
  // To send a value call the function e.g. send value of 50 with CAN_ID of 3
  Serial.println("Sending message");
  sendCAN_Message_with_timestamp(signalValue, 3, UTCtimeStamp, UTC_millis);
  delay(1000);
}


//####################################################
// Function to send a value and timestamp as 8 bytes.

void sendCAN_Message_with_timestamp(float signalValue, int CAN_ID, long seconds, int milliseconds) { // This function takes signed 2 byte value (-3276.8 up to 3276.7), adds a dummy timestamp and sends it onto the bus!

  // Initialise 8 byte message array
  unsigned char message[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  
  // Split signal into two bytes and add to message
  short oneDecimalPlaceValue = round(signalValue * 10); // Send with one decimal place. Currently only sending whole integers anyway.
  byte small = (byte) (oneDecimalPlaceValue & 0xFF);
  byte big = (byte) ((oneDecimalPlaceValue >> 8) & 0xFF);
  message[0] = small;
  message[1] = big;

  // Split timestamp seconds into 4 bytes and add it to the message
  for (int i = 0; i < 4; i++) {
    message[i+2] = ((seconds >> (i*8)) & 0xFF); // Extract the right-most byte of the shifted variable
  }

  // Split the timestamp milliseconds into two bytes and add it to the message
  oneDecimalPlaceValue = milliseconds;
  small = (byte) (oneDecimalPlaceValue & 0xFF);
  big = (byte) ((oneDecimalPlaceValue >> 8) & 0xFF);
  message[6] = small;
  message[7] = big;

  // Send message!
  CAN.sendMsgBuf(CAN_ID, 0, 8, message);
  delay(2); // Delay between sends because something is really slow.. ? If you dont, messages get dropped
}
