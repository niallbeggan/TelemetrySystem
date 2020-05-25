// This program receives CAN messages with timestamp and forwards them to a serial port
// to be viewed on a computer. It is the code that would be on the telemetry CAN node
// if the GPS was not used. It sends a null value for the noOfSatellites field. This
// was written as an intermediary program to check everything worked before integrating
// the GPS module

// Written by Niall Beggan 30/4/2020
// Last updated: 8/5/2020 - added CAN Fail counter


//####################################################
// Include neccessary libraries

#include <SPI.h>          //SPI is used to talk to the CAN Controller
#include <mcp_can.h>


//####################################################
// Declare variables

// CAN Bus variables
MCP_CAN CAN(10);          //set SPI Chip Select to pin 10
unsigned char len = 8;
unsigned char buf[8];
unsigned int canID;

// Radio variables
String replyIf1 = "0";

// Signal variables, with timestamp
byte eStop1[8] = {0, 0, 0, 0, 0, 0, 0, 0}; // Default signal = 0, timestamp = 0. (If CAN has failed)
byte bmsTemp2[8] = {0, 0, 0, 0, 0, 0, 0, 0}; // If radio fails, default singal is = -1, timestamp = -1
byte carSpeed3[8] = {0, 0, 0, 0, 0, 0, 0, 0};
byte bmsVoltage4[8] = {0, 0, 0, 0, 0, 0, 0, 0};
byte bmsCurrent5[8] = {0, 0, 0, 0, 0, 0, 0, 0};
byte tsPowerkW6[8] = {0, 0, 0, 0, 0, 0, 0, 0};
byte leftMotorVoltage7[8] = {0, 0, 0, 0, 0, 0, 0, 0};
byte rightMotorVoltage8[8] = {0, 0, 0, 0, 0, 0, 0, 0};
byte leftMotorCurrent9[8] = {0, 0, 0, 0, 0, 0, 0, 0};
byte rightMotorCurrent10[8] = {0, 0, 0, 0, 0, 0, 0, 0};
byte steeringInput11[8] = {0, 0, 0, 0, 0, 0, 0, 0};
byte acceleratorPedalPosition12[8] = {0, 0, 0, 0, 0, 0, 0, 0};

// Further signals not yet on Raimonds CANBus ID List
byte brakePedalPosition13[8] = {0, 0, 0, 0, 0, 0, 0, 0};
byte suspensionsFrontLeft14[8] = {0, 0, 0, 0, 0, 0, 0, 0};
byte suspensionsFrontRight15[8] = {0, 0, 0, 0, 0, 0, 0, 0};
byte suspensionsRearLeft16[8] = {0, 0, 0, 0, 0, 0, 0, 0};
byte suspensionsRearRight17[8] = {0, 0, 0, 0, 0, 0, 0, 0};

// Retransmission Counters
const int maxRetransmissionCount = 5;

int eStop1ResendCounter = 0; // If CAN has failed the resend counter will never be reset to zero, after a resend count of 5, zero is sent
int bmsTemp2ResendCounter = 0;
int carSpeed3ResendCounter = 0;
int bmsVoltage4ResendCounter = 0;
int bmsCurrent5ResendCounter = 0;
int tsPowerkW6ResendCounter = 0;
int leftMotorVoltage7ResendCounter = 0;
int rightMotorVoltage8ResendCounter = 0;
int leftMotorCurrent9ResendCounter = 0;
int rightMotorCurrent10ResendCounter = 0;
int steeringInput11ResendCounter = 0;
int acceleratorPedalPosition12ResendCounter = 0;
int brakePedalPosition13ResendCounter = 0;
int suspensionsFrontLeft14ResendCounter = 0;
int suspensionsFrontRight15ResendCounter = 0;
int suspensionsRearLeft16ResendCounter = 0;
int suspensionsRearRight17ResendCounter = 0;

//####################################################
// Set up CAN SPI communication

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
  Serial.begin(57600); //  Begin radio comms
}


//####################################################
// Main loop

void loop() {
  assignUpdatedCANValues();
    
  if (Serial.available()>0) {
    replyIf1 = Serial.read();
  }
  if(replyIf1 == "49") { // When laptop sends "1"  //ASCII value for 1.
    sendRadioMessage();
    resetSignalIfNoCanUpdate();
    replyIf1 = "0"; // Reset
  }
}


//####################################################
// Updates the CAN data fields. They get sent whether the data is new or not

void assignUpdatedCANValues() {
  if(CAN_MSGAVAIL == CAN.checkReceive()) {    //check if data is coming
    CAN.readMsgBuf(&len, buf);    // Read data,  len: data length, buf: data buffer
    canID = CAN.getCanId(); // Get the ID of the incoming message 
        
    // Check which Can ID was received and assign the data to the correct variable
    
    // EStop CAN ID
    if(canID == 0x1) {
      memcpy(eStop1, buf, sizeof(buf[0])*len);
      eStop1ResendCounter = 0;
    }
    
    // Temp Can ID
    if(canID == 0x2) {
      memcpy(bmsTemp2, buf, sizeof(buf[0])*len);
      bmsTemp2ResendCounter = 0;
    }
    
    // Car speed Can ID
    if(canID == 0x3) {
      memcpy(carSpeed3, buf, sizeof(buf[0])*len);
      carSpeed3ResendCounter = 0;
    }
    
    // BMS Voltage CAN ID
    if(canID == 0x4) {
      memcpy(bmsVoltage4, buf, sizeof(buf[0])*len);
      bmsVoltage4ResendCounter = 0;
    }
    
    // BMS Current CAN ID
    if(canID == 0x5) {
      memcpy(bmsCurrent5, buf, sizeof(buf[0])*len);
      bmsCurrent5ResendCounter = 0;
    }
    
    // Traction system power (in kW!)
    if(canID == 0x6) {
      memcpy(tsPowerkW6, buf, sizeof(buf[0])*len);
      tsPowerkW6ResendCounter = 0;
    }
    
    // Left motor voltage CAN ID
    if(canID == 0x7) {
      memcpy(leftMotorVoltage7, buf, sizeof(buf[0])*len);
      leftMotorVoltage7ResendCounter = 0;
    }
    
    // Right motor voltage CAN ID
    if(canID == 0x8) {
      memcpy(rightMotorVoltage8, buf, sizeof(buf[0])*len);
      rightMotorVoltage8ResendCounter = 0;
    }
    
    // Left motor current CAN ID
    if(canID == 9) {
      memcpy(leftMotorCurrent9, buf, sizeof(buf[0])*len);
      leftMotorCurrent9ResendCounter = 0;
    }
    
    // Right motor current CAN ID
    if(canID == 10) {
      memcpy(rightMotorCurrent10, buf, sizeof(buf[0])*len);
      rightMotorCurrent10ResendCounter = 0;
    }
    
    // Steering input (%) CAN ID
    if(canID == 11) {
      memcpy(steeringInput11, buf, sizeof(buf[0])*len);
      steeringInput11ResendCounter = 0;
    }
    
    // Accelerator pedal position CAN ID (% pressed)
    if(canID == 12) {
      memcpy(acceleratorPedalPosition12, buf, sizeof(buf[0])*len);
      acceleratorPedalPosition12ResendCounter = 0;
    }

    // Brake pedal position CAN ID (%) pressed
    if(canID == 13) {
      memcpy(brakePedalPosition13, buf, sizeof(buf[0])*len);
      brakePedalPosition13ResendCounter = 0;
    }

    // Left front suspension CAN ID (% compressed)
    if(canID == 14) {
      memcpy(suspensionsFrontLeft14, buf, sizeof(buf[0])*len);
      suspensionsFrontLeft14ResendCounter = 0;
    }

    // Right front suspension CAN ID (% compressed)
    if(canID == 15) {
      memcpy(suspensionsFrontRight15, buf, sizeof(buf[0])*len);
      suspensionsFrontRight15ResendCounter = 0;
    }

    // Rear left suspension CAN ID (% compressed)
    if(canID == 16) {
      memcpy(suspensionsRearLeft16, buf, sizeof(buf[0])*len);
      suspensionsRearLeft16ResendCounter = 0;
    }

    // Rear right suspension CAN ID (% compressed)
    if(canID == 17) {
      memcpy(suspensionsRearRight17, buf, sizeof(buf[0])*len);
      suspensionsRearRight17ResendCounter = 0;
    }
  }
}


//####################################################
// Send radio message function

void sendRadioMessage() {
  serialWriteByteArray(eStop1);
  serialWriteByteArray(bmsTemp2);
  serialWriteByteArray(carSpeed3);
  serialWriteByteArray(bmsVoltage4);
  serialWriteByteArray(bmsCurrent5);
  serialWriteByteArray(tsPowerkW6);
  serialWriteByteArray(leftMotorVoltage7);
  serialWriteByteArray(rightMotorVoltage8);
  serialWriteByteArray(leftMotorCurrent9);
  serialWriteByteArray(rightMotorCurrent10);
  serialWriteByteArray(steeringInput11);
  serialWriteByteArray(acceleratorPedalPosition12);
  serialWriteByteArray(brakePedalPosition13);
  serialWriteByteArray(suspensionsFrontLeft14);
  serialWriteByteArray(suspensionsFrontRight15);
  serialWriteByteArray(suspensionsRearLeft16);
  serialWriteByteArray(suspensionsRearRight17);

  serialWriteByteArray(eStop1); // Satellite replacement value

  incrementResendCounters();
}


//####################################################
// Increment fail counters
void incrementResendCounters() {
   eStop1ResendCounter += 1;
   bmsTemp2ResendCounter += 1;
   carSpeed3ResendCounter += 1;
   bmsVoltage4ResendCounter += 1;
   bmsCurrent5ResendCounter += 1;
   tsPowerkW6ResendCounter += 1;
   leftMotorVoltage7ResendCounter += 1;
   rightMotorVoltage8ResendCounter += 1;
   leftMotorCurrent9ResendCounter += 1;
   rightMotorCurrent10ResendCounter += 1;
   steeringInput11ResendCounter += 1;
   acceleratorPedalPosition12ResendCounter += 1;
   brakePedalPosition13ResendCounter += 1;
   suspensionsFrontLeft14ResendCounter += 1;
   suspensionsFrontRight15ResendCounter += 1;
   suspensionsRearLeft16ResendCounter += 1;
   suspensionsRearRight17ResendCounter += 1;
}

//####################################################
// Reset signal to zero if CAN node has not sent an update within the last maxRetransmissionCount number of radio transmission
void resetSignalIfNoCanUpdate() {
  if(eStop1ResendCounter > maxRetransmissionCount) {
    eStop1[0] = 0;
    eStop1[1] = 0;
    eStop1[2] = 0;
    eStop1[3] = 0;
    eStop1[4] = 0; // Set timestamp to 0 , a CAN error status
    eStop1[5] = 0;
    eStop1[6] = 0;
    eStop1[7] = 0;
  }
  if(bmsTemp2ResendCounter > maxRetransmissionCount) {
    bmsTemp2[0] = 0;
    bmsTemp2[1] = 0;
    bmsTemp2[2] = 0;
    bmsTemp2[3] = 0;
    bmsTemp2[4] = 0; // Set timestamp to 0 , a CAN error status
    bmsTemp2[5] = 0;
    bmsTemp2[6] = 0;
    bmsTemp2[7] = 0;
    
  }
  if(carSpeed3ResendCounter > maxRetransmissionCount) {
    carSpeed3[0] = 0;
    carSpeed3[1] = 0;
    carSpeed3[2] = 0;
    carSpeed3[3] = 0;
    carSpeed3[4] = 0; // Set timestamp to 0 , a CAN error status
    carSpeed3[5] = 0;
    carSpeed3[6] = 0;
    carSpeed3[7] = 0;
  }
  if(bmsVoltage4ResendCounter > maxRetransmissionCount) {
    bmsVoltage4[0] = 0;
    bmsVoltage4[1] = 0;
    bmsVoltage4[2] = 0;
    bmsVoltage4[3] = 0;
    bmsVoltage4[4] = 0; // Set timestamp to 0 , a CAN error status
    bmsVoltage4[5] = 0;
    bmsVoltage4[6] = 0;
    bmsVoltage4[7] = 0;
  }
  if(bmsCurrent5ResendCounter > maxRetransmissionCount) {
    bmsCurrent5[0] = 0;
    bmsCurrent5[1] = 0;
    bmsCurrent5[2] = 0;
    bmsCurrent5[3] = 0;
    bmsCurrent5[4] = 0; // Set timestamp to 0 , a CAN error status
    bmsCurrent5[5] = 0;
    bmsCurrent5[6] = 0;
    bmsCurrent5[7] = 0;
  }
  if(tsPowerkW6ResendCounter > maxRetransmissionCount) {
    tsPowerkW6[0] = 0;
    tsPowerkW6[1] = 0;
    tsPowerkW6[2] = 0;
    tsPowerkW6[3] = 0;
    tsPowerkW6[4] = 0; // Set timestamp to 0 , a CAN error status
    tsPowerkW6[5] = 0;
    tsPowerkW6[6] = 0;
    tsPowerkW6[7] = 0;
  }
  if(leftMotorVoltage7ResendCounter > maxRetransmissionCount) {
    leftMotorVoltage7[0] = 0;
    leftMotorVoltage7[1] = 0;
    leftMotorVoltage7[2] = 0;
    leftMotorVoltage7[3] = 0;
    leftMotorVoltage7[4] = 0; // Set timestamp to 0 , a CAN error status
    leftMotorVoltage7[5] = 0;
    leftMotorVoltage7[6] = 0;
    leftMotorVoltage7[7] = 0;
  }
  if(rightMotorVoltage8ResendCounter > maxRetransmissionCount) {
    rightMotorVoltage8[0] = 0;
    rightMotorVoltage8[1] = 0;
    rightMotorVoltage8[2] = 0;
    rightMotorVoltage8[3] = 0;
    rightMotorVoltage8[4] = 0; // Set timestamp to 0 , a CAN error status
    rightMotorVoltage8[5] = 0;
    rightMotorVoltage8[6] = 0;
    rightMotorVoltage8[7] = 0;
  }
  if(leftMotorCurrent9ResendCounter > maxRetransmissionCount) {
    leftMotorCurrent9[0] = 0;
    leftMotorCurrent9[1] = 0;
    leftMotorCurrent9[2] = 0;
    leftMotorCurrent9[3] = 0;
    leftMotorCurrent9[4] = 0; // Set timestamp to 0 , a CAN error status
    leftMotorCurrent9[5] = 0;
    leftMotorCurrent9[6] = 0;
    leftMotorCurrent9[7] = 0;
  }
  if(rightMotorCurrent10ResendCounter > maxRetransmissionCount) {
    rightMotorCurrent10[0] = 0;
    rightMotorCurrent10[0] = 0;
    rightMotorCurrent10[2] = 0;
    rightMotorCurrent10[3] = 0;
    rightMotorCurrent10[4] = 0; // Set timestamp to 0 , a CAN error status
    rightMotorCurrent10[5] = 0;
    rightMotorCurrent10[6] = 0;
    rightMotorCurrent10[7] = 0;
  }
  if(steeringInput11ResendCounter > maxRetransmissionCount) {
    steeringInput11[0] = 0;
    steeringInput11[1] = 0;
    steeringInput11[2] = 0;
    steeringInput11[3] = 0;
    steeringInput11[4] = 0; // Set timestamp to 0 , a CAN error status
    steeringInput11[5] = 0;
    steeringInput11[6] = 0;
    steeringInput11[7] = 0;
  }
  if(acceleratorPedalPosition12ResendCounter > maxRetransmissionCount) {
    acceleratorPedalPosition12[0] = 0;
    acceleratorPedalPosition12[1] = 0;
    acceleratorPedalPosition12[2] = 0;
    acceleratorPedalPosition12[3] = 0;
    acceleratorPedalPosition12[4] = 0; // Set timestamp to 0 , a CAN error status
    acceleratorPedalPosition12[5] = 0;
    acceleratorPedalPosition12[6] = 0;
    acceleratorPedalPosition12[7] = 0;
  }
  if(brakePedalPosition13ResendCounter > maxRetransmissionCount) {
    brakePedalPosition13[0] = 0;
    brakePedalPosition13[1] = 0;
    brakePedalPosition13[2] = 0;
    brakePedalPosition13[3] = 0;
    brakePedalPosition13[4] = 0; // Set timestamp to 0 , a CAN error status
    brakePedalPosition13[5] = 0;
    brakePedalPosition13[6] = 0;
    brakePedalPosition13[7] = 0;
  }
  if(suspensionsFrontLeft14ResendCounter > maxRetransmissionCount) {
    suspensionsFrontLeft14[0] = 0;
    suspensionsFrontLeft14[1] = 0;
    suspensionsFrontLeft14[2] = 0;
    suspensionsFrontLeft14[3] = 0;
    suspensionsFrontLeft14[4] = 0; // Set timestamp to 0 , a CAN error status
    suspensionsFrontLeft14[5] = 0;
    suspensionsFrontLeft14[6] = 0;
    suspensionsFrontLeft14[7] = 0;
  }
  if(suspensionsFrontRight15ResendCounter > maxRetransmissionCount) {
    suspensionsFrontRight15[0] = 0;
    suspensionsFrontRight15[1] = 0;
    suspensionsFrontRight15[2] = 0;
    suspensionsFrontRight15[3] = 0;
    suspensionsFrontRight15[4] = 0; // Set timestamp to 0 , a CAN error status
    suspensionsFrontRight15[5] = 0;
    suspensionsFrontRight15[6] = 0;
    suspensionsFrontRight15[7] = 0;
  }
  if(suspensionsRearLeft16ResendCounter > maxRetransmissionCount) {
    suspensionsRearLeft16[0] = 0;
    suspensionsRearLeft16[1] = 0;
    suspensionsRearLeft16[2] = 0;
    suspensionsRearLeft16[3] = 0;
    suspensionsRearLeft16[4] = 0; // Set timestamp to 0 , a CAN error status
    suspensionsRearLeft16[5] = 0;
    suspensionsRearLeft16[6] = 0;
    suspensionsRearLeft16[7] = 0;
  }
  if(suspensionsRearRight17ResendCounter > maxRetransmissionCount) {
    suspensionsRearRight17[0] = 0;
    suspensionsRearRight17[1] = 0;
    suspensionsRearRight17[2] = 0;
    suspensionsRearRight17[3] = 0;
    suspensionsRearRight17[4] = 0; // Set timestamp to 0 , a CAN error status
    suspensionsRearRight17[5] = 0;
    suspensionsRearRight17[6] = 0;
    suspensionsRearRight17[7] = 0;
  }
}


//####################################################
// Sends 8 byte array to the serial port
void serialWriteByteArray(byte Array[8]) {
    for (int i=0; i<8; i++) {
    Serial.write(Array[i]);
  }
}
