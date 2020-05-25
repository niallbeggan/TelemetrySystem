// This program receives CAN messages and forwards them to a serial port/radio transceiver 
// to be viewed on a computer, upon receiving a request. It also communicates with the GPS
// module to get accurate time, and speed values and sends these also

// Written by Niall Beggan 1/5/2020
// Last updated: 8/5/2020 - added CAN fail counter


//####################################################
// Include libraries

#include <SPI.h>  // SPI is used to talk to the CAN Controller
#include <mcp_can.h>  // CAN library
#include <AltSoftSerial.h>  // GPS Libraries
#include <NMEAGPS.h>  // GPS Libraries
#include <GPSport.h>  // GPS Libraries
#include <Streamers.h>  // GPS Libraries


//####################################################
// Declare variables

// GPS VARIABLES
AltSoftSerial altser; // Set up a new serial object for GPS module

// Set up gps object
NMEAGPS gps;
gps_fix currentFix;

// GPS Time variables
long NMEA_timeSeconds = 0; // Time of day in seconds max 86400
long PPS_timeSeconds = 0; // Time of day in seconds max 86400
long millisAtStartOfSecond = 0; // This is the start of second point of reference for milli-second time value calculations. It comes from the GPS P1PS/1PPS/PPS pin.
bool timeIsCorrect = 0;

// GPS Speed variables
// Speed timestamp variables for this demo
long speedTimestampMillis = 0; // Time between seconds, in milli seconds
long speedTimestampSeconds = 0; // Time of day in seconds

float carSpeedSatellite = 0; // From satellite
int noOfSatellites = 0; // How many satellites the GPS can see. Needs at least 4. Preferably > 5.


// CAN Variables
MCP_CAN CAN(10);          // Set SPI Chip Select to pin 10
unsigned char len = 8; // This works when len is zero?
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

// Further signals not yet on Raimonds CAN bus ID List

byte brakePedalPosition13[8] = {0, 0, 0, 0, 0, 0, 0, 0};
byte suspensionsFrontLeft14[8] = {0, 0, 0, 0, 0, 0, 0, 0};
byte suspensionsFrontRight15[8] = {0, 0, 0, 0, 0, 0, 0, 0};
byte suspensionsRearLeft16[8] = {0, 0, 0, 0, 0, 0, 0, 0};
byte suspensionsRearRight17[8] = {0, 0, 0, 0, 0, 0, 0, 0};

byte satellites[8] = {0, 0, 0, 0, 0, 0, 0, 0};

// Retransmission Counters
int maxRetransmissionCount = 5;

int CANResendCounter = 0; // If CAN has failed the resend counter will never be reset to zero, after a resend count of 5, an error code is sent

//####################################################
// Start all comms

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

  Serial.begin(57600); //  Begin radio communication

  altser.begin(9600); // Begin GPS serial communication
  attachInterrupt(digitalPinToInterrupt(2),interruptFunction,RISING); // GPS - PPS hardware interrupt setup. This can only be on pins 2 & 3 on nano
}


//####################################################
// Main loop

void loop() {

  // GPS Speed Section - This will happen once per seconds. GPS updates at 1hz
  if (gps.available(gpsPort)) {
    currentFix = gps.read();
    setNMEA_timeSeconds(); // Checks if seconds was correctly updated by the pulse
    setGPS_speed(); // Sets speed AND assigns the timestamp to it
    setNumberOfSatellites(); // Like above
  }
  
  assignUpdatedCANValues(); // This will happen many times per second. 18 CAN messages are receeived approx every 0.2 seconds
    
  if (Serial.available()>0) { // This will happen every time a request is received. For now every 0.2 seconds
    replyIf1 = Serial.read();
  }
  if(replyIf1 == "49") { // When laptop sends "1"  // ASCII value for 1. // This will happen every time a request is received. For now every 0.2 seconds
    sendRadioMessage();
    resetSignalIfNoCanUpdate();
    replyIf1 = "0"; // Reset
  }
}


//####################################################
// Updates with new data from CAN bus

void assignUpdatedCANValues() {
if(CAN_MSGAVAIL == CAN.checkReceive()) {  // Check if data is coming
    CAN.readMsgBuf(&len, buf);    // Read data,  len: data length, buf: data buffer
    canID = CAN.getCanId(); // Get the ID of the incoming message 
    CANResendCounter = 0;
        
    // Check which Can ID was received and assign the data to the correct variable
    
    // EStop CAN ID
    if(canID == 0x1) {
      memcpy(eStop1, buf, sizeof(buf[0])*len);
    }
    
    // Temp Can ID
    if(canID == 0x2) {
      memcpy(bmsTemp2, buf, sizeof(buf[0])*len);
    }
    
    // Car speed Can ID   // I AM USING GPS SPEED FOR NOW
//    if(canID == 0x3) {
//      memcpy(carSpeed3, buf, sizeof(buf[0])*len);
//    }
    
    // BMS Voltage CAN ID
    if(canID == 0x4) {
      memcpy(bmsVoltage4, buf, sizeof(buf[0])*len);
    }
    
    // BMS Current CAN ID
    if(canID == 0x5) {
      memcpy(bmsCurrent5, buf, sizeof(buf[0])*len);
    }
    
    // Traction system power (in kW!)
    if(canID == 0x6) {
      memcpy(tsPowerkW6, buf, sizeof(buf[0])*len);
    }
    
    // Left motor voltage CAN ID
    if(canID == 0x7) {
      memcpy(leftMotorVoltage7, buf, sizeof(buf[0])*len);
    }
    
    // Right motor voltage CAN ID
    if(canID == 0x8) {
      memcpy(rightMotorVoltage8, buf, sizeof(buf[0])*len);
    }
    
    // Left motor current CAN ID
    if(canID == 9) {
      memcpy(leftMotorCurrent9, buf, sizeof(buf[0])*len);
    }
    
    // Right motor current CAN ID
    if(canID == 10) {
      memcpy(rightMotorCurrent10, buf, sizeof(buf[0])*len);
    }
    
    // Steering input (%) CAN ID
    if(canID == 11) {
      memcpy(steeringInput11, buf, sizeof(buf[0])*len);
    }
    
    // Accelerator pedal position CAN ID (% pressed)
    if(canID == 12) {
      memcpy(acceleratorPedalPosition12, buf, sizeof(buf[0])*len);
    }

    // Brake pedal position CAN ID (%) pressed
    if(canID == 13) {
      memcpy(brakePedalPosition13, buf, sizeof(buf[0])*len);
    }

    // Left front suspension CAN ID (% compressed)
    if(canID == 14) {
      memcpy(suspensionsFrontLeft14, buf, sizeof(buf[0])*len);
    }

    // Right front suspension CAN ID (% compressed)
    if(canID == 15) {
      memcpy(suspensionsFrontRight15, buf, sizeof(buf[0])*len);
    }

    // Rear left suspension CAN ID (% compressed)
    if(canID == 16) {
      memcpy(suspensionsRearLeft16, buf, sizeof(buf[0])*len);
    }

    // Rear right suspension CAN ID (% compressed)
    if(canID == 17) {
      memcpy(suspensionsRearRight17, buf, sizeof(buf[0])*len);
    }
    else {
      // NOP yet
    }
  }
}


//####################################################
// Sends all data to the radio transceiver

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

  serialWriteByteArray(satellites); // Satellite

  incrementResendCounter();
}


//####################################################
// Writes 8 byte array to serial port

void serialWriteByteArray(byte Array[8]) {
    for (int i=0; i<8; i++) {
    Serial.write(Array[i]);
  }
}


//####################################################
// Gets the current time between seconds

int getCurrentMillis() {
  float currentMillis = 111; // Error, function wont return accurate millis until GPS has switched to 1 pulse per second mode, aligned at the start of each second.
  if(timeIsCorrect == true) { // This is true once a NMEA gps message has been received from the GPS & satellite count is > 3
    currentMillis = millis() - millisAtStartOfSecond; // Gives u how many milli seconds into the current second the time is.
    if((currentMillis > 999) && (currentMillis < 1003)) { // Sometimes 1000 or 1001 can occur
      currentMillis = 999;
    }
  }
  return currentMillis;
}


//####################################################
// Sets the gps speed varible with new speed data and timestamp

void setGPS_speed() {
  if(currentFix.valid.speed) {
    carSpeedSatellite = currentFix.speed_kph();
    setSpeedTimestamp();
  }
  
  // Split signal into two bytes and add to carSpeed3
  short oneDecimalPlaceValue = round(carSpeedSatellite * 10); // Send with one decimal place. Currently only sending whole integers anyway.
  byte small = (byte) (oneDecimalPlaceValue & 0xFF);
  byte big = (byte) ((oneDecimalPlaceValue >> 8) & 0xFF);
  carSpeed3[0] = big;
  carSpeed3[1] = small;

  // Split timestamp into 4 bytes and add it to the message
  for (int i = 0; i < 4; i++) {
    carSpeed3[i+2] = ((speedTimestampSeconds >> (i*8)) & 0xFF); // Extract the right-most byte of the shifted variable
  }
  carSpeed3[6] = 0;
  carSpeed3[7] = 0;
}


//####################################################
// Reads the NMEA GPS message and double checks the time is right

void setNMEA_timeSeconds() {
  if(currentFix.valid.time) {
    double hrs = 0;
    int mins = 0;
    int secs = 0;
    hrs = currentFix.dateTime.hours;
    mins = currentFix.dateTime.minutes; // Cant use minutes for function and variable name
    secs = currentFix.dateTime.seconds;
    NMEA_timeSeconds = secs + (mins*60) + (hrs*60*60);
    if(PPS_timeSeconds != NMEA_timeSeconds) {
      PPS_timeSeconds = NMEA_timeSeconds; // PPS_timeStampSeconds should already be updated by PPS,but wont be correct on start up, etc.
      timeIsCorrect = false; // Time is not correct. An error has occured. By the time a string has been received the pulse should already have incremented the time.
    }
    else if((PPS_timeSeconds == NMEA_timeSeconds) && (noOfSatellites > 3)) { // Time is now synced correctly
      timeIsCorrect = true;
    }
  }
}


//####################################################
// Sets the speed timestamp

void setSpeedTimestamp() {
    speedTimestampSeconds = PPS_timeSeconds; // Dont use millis for reasons explained in 3.GpsComms
}


//####################################################
// Sets the number of satellites variable with new data

void setNumberOfSatellites() {
  noOfSatellites = gps.sat_count;

    // Split signal into two bytes and add to carSpeed3
  short oneDecimalPlaceValue = round(noOfSatellites * 10); // Send with one decimal place. Currently only sending whole integers anyway.
  byte small = (byte) (oneDecimalPlaceValue & 0xFF);
  byte big = (byte) ((oneDecimalPlaceValue >> 8) & 0xFF);
  satellites[0] = big;
  satellites[1] = small;

  // Split timestamp into 4 bytes and add it to the message
  for (int i = 0; i < 4; i++) {
    satellites[i+2] = ((speedTimestampSeconds >> (i*8)) & 0xFF); // Extract the right-most byte of the shifted variable // GPS speed & satellite timestamp are the same
  }
  satellites[6] = 0;
  satellites[7] = 0;
}


//####################################################
// Increment fail counters
void incrementResendCounter() {
   CANResendCounter += 1;
}

//####################################################
// Reset signal to zero if CAN node has not sent an update within the last maxRetransmissionCount number of radio transmission
void resetSignalIfNoCanUpdate() {
  if(CANResendCounter > maxRetransmissionCount) {
    eStop1[0] = 0;
    eStop1[1] = 0;
    eStop1[2] = 0;
    eStop1[3] = 0;
    eStop1[4] = 0; // Set timestamp to 0 , a CAN error status
    eStop1[5] = 0;
    eStop1[6] = 0;
    eStop1[7] = 0;

    bmsTemp2[0] = 0;
    bmsTemp2[1] = 0;
    bmsTemp2[2] = 0;
    bmsTemp2[3] = 0;
    bmsTemp2[4] = 0; // Set timestamp to 0 , a CAN error status
    bmsTemp2[5] = 0;
    bmsTemp2[6] = 0;
    bmsTemp2[7] = 0;

//    carSpeed3[0] = 0;
//    carSpeed3[1] = 0;
//    carSpeed3[2] = 0;
//    carSpeed3[3] = 0;
//    carSpeed3[4] = 0; // Set timestamp to 0 , a CAN error status
//    carSpeed3[5] = 0;
//    carSpeed3[6] = 0; // Using GPS speed
//    carSpeed3[7] = 0;

    bmsVoltage4[0] = 0;
    bmsVoltage4[1] = 0;
    bmsVoltage4[2] = 0;
    bmsVoltage4[3] = 0;
    bmsVoltage4[4] = 0; // Set timestamp to 0 , a CAN error status
    bmsVoltage4[5] = 0;
    bmsVoltage4[6] = 0;
    bmsVoltage4[7] = 0;
    
    bmsCurrent5[0] = 0;
    bmsCurrent5[1] = 0;
    bmsCurrent5[2] = 0;
    bmsCurrent5[3] = 0;
    bmsCurrent5[4] = 0; // Set timestamp to 0 , a CAN error status
    bmsCurrent5[5] = 0;
    bmsCurrent5[6] = 0;
    bmsCurrent5[7] = 0;

    tsPowerkW6[0] = 0;
    tsPowerkW6[1] = 0;
    tsPowerkW6[2] = 0;
    tsPowerkW6[3] = 0;
    tsPowerkW6[4] = 0; // Set timestamp to 0 , a CAN error status
    tsPowerkW6[5] = 0;
    tsPowerkW6[6] = 0;
    tsPowerkW6[7] = 0;

    leftMotorVoltage7[0] = 0;
    leftMotorVoltage7[1] = 0;
    leftMotorVoltage7[2] = 0;
    leftMotorVoltage7[3] = 0;
    leftMotorVoltage7[4] = 0; // Set timestamp to 0 , a CAN error status
    leftMotorVoltage7[5] = 0;
    leftMotorVoltage7[6] = 0;
    leftMotorVoltage7[7] = 0;

    rightMotorVoltage8[0] = 0;
    rightMotorVoltage8[1] = 0;
    rightMotorVoltage8[2] = 0;
    rightMotorVoltage8[3] = 0;
    rightMotorVoltage8[4] = 0; // Set timestamp to 0 , a CAN error status
    rightMotorVoltage8[5] = 0;
    rightMotorVoltage8[6] = 0;
    rightMotorVoltage8[7] = 0;

    leftMotorCurrent9[0] = 0;
    leftMotorCurrent9[1] = 0;
    leftMotorCurrent9[2] = 0;
    leftMotorCurrent9[3] = 0;
    leftMotorCurrent9[4] = 0; // Set timestamp to 0 , a CAN error status
    leftMotorCurrent9[5] = 0;
    leftMotorCurrent9[6] = 0;
    leftMotorCurrent9[7] = 0;

    rightMotorCurrent10[0] = 0;
    rightMotorCurrent10[1] = 0;
    rightMotorCurrent10[2] = 0;
    rightMotorCurrent10[3] = 0;
    rightMotorCurrent10[4] = 0; // Set timestamp to 0 , a CAN error status
    rightMotorCurrent10[5] = 0;
    rightMotorCurrent10[6] = 0;
    rightMotorCurrent10[7] = 0;

    steeringInput11[0] = 0;
    steeringInput11[1] = 0;
    steeringInput11[2] = 0;
    steeringInput11[3] = 0;
    steeringInput11[4] = 0; // Set timestamp to 0 , a CAN error status
    steeringInput11[5] = 0;
    steeringInput11[6] = 0;
    steeringInput11[7] = 0;

    acceleratorPedalPosition12[0] = 0;
    acceleratorPedalPosition12[1] = 0;
    acceleratorPedalPosition12[2] = 0;
    acceleratorPedalPosition12[3] = 0;
    acceleratorPedalPosition12[4] = 0; // Set timestamp to 0 , a CAN error status
    acceleratorPedalPosition12[5] = 0;
    acceleratorPedalPosition12[6] = 0;
    acceleratorPedalPosition12[7] = 0;

    brakePedalPosition13[0] = 0;
    brakePedalPosition13[1] = 0;
    brakePedalPosition13[2] = 0;
    brakePedalPosition13[3] = 0;
    brakePedalPosition13[4] = 0; // Set timestamp to 0 , a CAN error status
    brakePedalPosition13[5] = 0;
    brakePedalPosition13[6] = 0;
    brakePedalPosition13[7] = 0;

    suspensionsFrontLeft14[0] = 0;
    suspensionsFrontLeft14[1] = 0;
    suspensionsFrontLeft14[2] = 0;
    suspensionsFrontLeft14[3] = 0;
    suspensionsFrontLeft14[4] = 0; // Set timestamp to 0 , a CAN error status
    suspensionsFrontLeft14[5] = 0;
    suspensionsFrontLeft14[6] = 0;
    suspensionsFrontLeft14[7] = 0;

    suspensionsFrontRight15[0] = 0;
    suspensionsFrontRight15[1] = 0;
    suspensionsFrontRight15[2] = 0;
    suspensionsFrontRight15[3] = 0;
    suspensionsFrontRight15[4] = 0; // Set timestamp to 0 , a CAN error status
    suspensionsFrontRight15[5] = 0;
    suspensionsFrontRight15[6] = 0;
    suspensionsFrontRight15[7] = 0;
    
    suspensionsRearLeft16[0] = 0;
    suspensionsRearLeft16[1] = 0;
    suspensionsRearLeft16[2] = 0;
    suspensionsRearLeft16[3] = 0;
    suspensionsRearLeft16[4] = 0; // Set timestamp to 0 , a CAN error status
    suspensionsRearLeft16[5] = 0;
    suspensionsRearLeft16[6] = 0;
    suspensionsRearLeft16[7] = 0;

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
// Interrupt function sets new time in seconds of the day

void interruptFunction() {
  millisAtStartOfSecond = millis();  // Update current start of second time for the current second
  PPS_timeSeconds = PPS_timeSeconds + 1; // Increment second count.
}
