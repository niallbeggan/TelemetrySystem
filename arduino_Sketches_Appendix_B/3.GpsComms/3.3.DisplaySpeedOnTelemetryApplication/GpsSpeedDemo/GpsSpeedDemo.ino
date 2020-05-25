// This program reads Gps speed from the Uranus 622f GPS module and forwards 
// it to the telemetry application. If used with the radio modules, this can be used 
// to test the GPS speed readings.
// Written by: Niall Beggan
// Last Updated: 7/5/2020 - changed all values to big endian

// Include libraries

#include <AltSoftSerial.h>
#include <NMEAGPS.h>
#include <GPSport.h>
#include <Streamers.h>

// Initialise variables

String replyIf1 = "0";
long UTCtimeStamp = 0;
float empty = 0;  
float carSpeed3 = 0;
int noOfSatellites = 0;

// set up a new serial object
AltSoftSerial altser;
// set up gps object
NMEAGPS gps;
gps_fix currentFix;

// Set up serial comms
void setup() {
  altser.begin(9600);
  Serial.begin(57600);
}

// Main send loop
void loop() {
  while (gps.available(gpsPort)) { // If new Gps data is available, read it.
    currentFix = gps.read();
    if(currentFix.valid.speed) {
      carSpeed3 = currentFix.speed_kph();
    }
    noOfSatellites = gps.sat_count; // Get number of unique satellite signals being received
  }

  if (Serial.available()>0) { // Read request from Telemetry application
    replyIf1 = Serial.read();
  }
 
  if(replyIf1 == "49") { // When laptop sends "1"  //ASCII value for 1.

    byte empty = 0; // Empty byte
    for(int i = 0; i < 16; i++) { // Fill packet with zeros so application accepts it
      Serial.write(empty);
    }
    sendSerialAsTwoBytes(carSpeed3); // Send the GPS speed
    
    for(int i = 0; i < 118; i++) { // Fill packet with zeros so application accepts it
      Serial.write(empty);
    }
    sendSerialAsTwoBytes(noOfSatellites);
    for(int i = 0; i < 6; i++) { // Fill packet with zeros so application accepts it
      Serial.write(empty);
    }
    replyIf1 = "0"; // dont reply again until next request
  }
}

// Sends the signal variable in the correct format
void sendSerialAsTwoBytes(float value) {
  int decimalValue = round(value * 10); // Send with one decimal place. Currently only sending whole integers anyway.
  byte small = (byte) (decimalValue & 0xFF);
  byte big = (byte) ((decimalValue >> 8) & 0xFF);
  Serial.write(small);
  Serial.write(big);
}
