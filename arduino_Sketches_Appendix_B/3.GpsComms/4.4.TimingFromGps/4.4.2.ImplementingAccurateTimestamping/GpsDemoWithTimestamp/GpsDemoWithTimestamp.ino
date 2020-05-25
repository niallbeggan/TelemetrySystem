// This program reads Gps speed from the Uranus 622f GPS module and forwards 
// it to the telemetry application with a precise timestamp. The timestamp is not a 
// true reflection of when the speed was measured (on purpose). The speed comes from 
// the NMEA messages at the start of every second. However it takes time for the 
// GPS module to read, parse, and write the NMEA message. This program gives the 
// time when the NMEA message is received. This is normally around 430mS or 700mS 
// after the 1 second pulse. The reason this is done is to show how the time can be 
// found accurately (to 2mS) at any point between the 1 second pulses from the Gps. 
// This functionality can be extended in future to allow accurate timestamping from 
// all CAN nodes by syncing up time on the nodes.

// Written by: Niall Beggan
// Last Updated: 7/5/2020 - changed all values to big endian

// Include libraries

#include <AltSoftSerial.h>
#include <NMEAGPS.h>
#include <GPSport.h>
#include <Streamers.h>

// Initialise variables
String replyIf1 = "0";

// GPS Time variables
long NMEA_timeSeconds = 0; // Time of day in seconds max 86400
long PPS_timeSeconds = 0; // Time of day in seconds max 86400
long millisAtStartOfSecond = 0; // This is the start of second point of reference for milli-second time value calculations. It comes from the GPS P1PS/1PPS/PPS pin.
bool timeIsCorrect = 0;

// Speed timestamp variables for this demo
long speedTimestampMillis = 0; // Time between seconds, in milli seconds
long speedTimestampSeconds = 0; // Time of day in seconds

// Signal values
float empty = 0;
float carSpeed3 = 0;
int noOfSatellites = 0; // How many satellites the GPS can see. Needs at least 4. Preferably > 5.

// Set up a new serial object
AltSoftSerial altser;

// Set up gps object
NMEAGPS gps;
gps_fix currentFix;

// Set up serial comms and hardware interrupt
void setup() {
  altser.begin(9600); // Begin GPS serial communication
  Serial.begin(57600); // Begin radio transceiver serial communication
  attachInterrupt(digitalPinToInterrupt(2),interruptFunction,RISING); // PPS hardware interrupt setup. This can only be on pins 2 & 3 on nano
}

// Get speed and time variables from GPS module and adds them to a radio packet
// and sends the packet to the telemetry application when receive a request.
void loop() {
  if (gps.available(gpsPort)) {
    currentFix = gps.read();
    setNMEA_timeSeconds();
    setGPS_speed();
    setNumberOfSatellites();
  }

  if (Serial.available()>0) {
    replyIf1 = Serial.read();
  }
 
  if(replyIf1 == "49") { // When laptop sends "1"  //ASCII value for 1.
  
    byte empty = 0; // Empty byte
    for(int i = 0; i < 16; i++) { // Fill packet with zeros so application accepts it
      Serial.write(empty);
    }
    
    sendSerialAsTwoBytes(carSpeed3); // Send speed with timestamp
    
    for(int i = 0; i < 112; i++) { // Fill packet with zeros so application accepts it
      Serial.write(empty);
    }
    
    sendSerialAsTwoBytes(noOfSatellites); // Send satellite count with timestamp
    
    replyIf1 = "0"; // dont reply again until next request
  }
}

void sendSerialAsTwoBytes(float value) {
  int decimalValue = round(value * 10); // Send with one decimal place.
  byte small = (byte) (decimalValue & 0xFF);
  byte big = (byte) ((decimalValue >> 8) & 0xFF);
  Serial.write(small);
  Serial.write(big);

  sendTimestampOverSerialAs6Bytes(speedTimestampSeconds, speedTimestampMillis); // Send timestamp
}

// Splits seconds into bytes and sends it to radio.
void sendTimestampOverSerialAs6Bytes(long S, long mS) {  
  byte b[4];
  for (int i=0; i<4; i++) {
    b[i]=((S>>(i*8)) & 0xff); // Extract the right-most byte of the shifted variable
    Serial.write(b[i]);
  }
  sendTimestampMillisOverSerialAs2Bytes(mS); // timestamp_millis (recorded as soon as speed measure is received, sent as two bytes
}

// Writes milliseconds of the timestamp to the radio
void sendTimestampMillisOverSerialAs2Bytes(long UTC_millis) {
  int decimalValue =   UTC_millis;// UTC_millis
  byte small = (byte) (decimalValue & 0xFF);
  byte big = (byte) ((decimalValue >> 8) & 0xFF);
  Serial.write(small);
  Serial.write(big);
}

// Gets the milliseconds since the last second
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

// Assigns the gps speed to the carSpeed3 variable and records the current time as the timestamp.
void setGPS_speed() {
  if(currentFix.valid.speed) {
    carSpeed3 = currentFix.speed_kph();
    setSpeedTimestamp();
  }
}

void setNMEA_timeSeconds() { // NMEA seconds are received in the gps message. 
  // If the pulse occured and incremented the seconds variable, 
  // the NMEA_timeSeconds should match the PPS_timeSeconds. On startup, 
  // NMEA_timeSeconds will initially set the value of pps seconds
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

// Example of how to use getCurrentMillis() to get an accurate timestamp;
void setSpeedTimestamp() {
    speedTimestampMillis = getCurrentMillis(); // Record millis for timestamp
    speedTimestampSeconds = PPS_timeSeconds;
}

// Sets the noOfSatellites varible.
void setNumberOfSatellites() {
  noOfSatellites = gps.sat_count;
}

// Interrupt function to increment the seconds count
void interruptFunction() {
  millisAtStartOfSecond = millis();  // Update current start of second time for the current second
  PPS_timeSeconds = PPS_timeSeconds + 1; // Increment second count.
}
