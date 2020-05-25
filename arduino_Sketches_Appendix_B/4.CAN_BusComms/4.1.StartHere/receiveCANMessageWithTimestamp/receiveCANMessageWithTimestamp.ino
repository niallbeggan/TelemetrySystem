// This program receives CAN messages and prints them to a serial port
// to be viewed on a computer
// Written by Niall Beggan 30/4/2020
// Last updated: 7/5/2020 - changed all values to big endian

// The format of the messages is: 8 bytes
// byte 0 - 1 - Signal - LSB first
// byte 2 - 5 - Timestamp Seconds - LSB first
// byte 6 - 7 - Timestamp milliSeconds - LSB first


//####################################################
// Include libraries

#include <SPI.h>          //SPI is used to talk to the CAN Controller
#include <mcp_can.h>


//####################################################
// Declare variables

MCP_CAN CAN(10);          //set SPI Chip Select to pin 10
unsigned char len = 0;
unsigned char buf[8];
unsigned int canID;


//####################################################
// Main loop

void setup() {
  Serial.begin(57600);  
  // Tries to initialize, if failed --> it will loop here for ever
START_INIT:
  if (CAN_OK == CAN.begin(CAN_100KBPS)) { // Setting CAN baud rate to 500Kbps
    Serial.println("beginning CAN comms");
    // NOP
  }
  else {
    delay(100);
    goto START_INIT;
  }
}

// Constantly check for messages, looping and printing them to the serial port
void loop() {
  if (CAN_MSGAVAIL == CAN.checkReceive()) {   //check if data is coming
    Serial.println("\n\nMessage received");
    CAN.readMsgBuf(&len, buf);    // Read data,  len: data length, buf: data buffer
    canID = CAN.getCanId(); // Get the ID of the incoming message

    short signalValue = 0;
    int UTC_millis = 0;
    unsigned long UTC_time_seconds = 0;

    // Convert signal from two bytes to int
    signalValue = buf[1];
    signalValue = signalValue << 8;
    signalValue = signalValue + buf[0];
    signalValue = signalValue / 10;

    //
    int x = 0;
    for (int z = 2; z < 6; z = z + 1) {
      unsigned long tmp = buf[z];
      UTC_time_seconds += (tmp) << (x * 8);
      x = x + 1;
    }

    UTC_millis = buf[7];
    UTC_millis = UTC_millis << 8; // Shift left 8
    UTC_millis = UTC_millis + buf[6];

    // Print data
    Serial.print("Signal Value :");
    Serial.print(signalValue);
    Serial.print(" UTC_time :");
    Serial.print(UTC_time_seconds);
    Serial.print(".");
    Serial.print(UTC_millis);
    Serial.print("\n");

    // Reset variables
    canID = 0;
    signalValue = 0;
    UTC_time_seconds = 0;
  }
}
