// Written by Niall Beggan to test the SlashDevin/NeoGps library, around mid March 2020
// Last Updated: 1/5/2020 - Added comprehensive commenting

#include <AltSoftSerial.h>
#include <NMEAGPS.h>
#include <GPSport.h>
#include <Streamers.h>


// Set up a new serial object
AltSoftSerial altser; // Use altsoftserial instead of softserial.

// Set up gps object
NMEAGPS gps;
gps_fix currentFix;

int noOfSatellites = 0;

// Begin serial communication
void setup() {
  altser.begin(9600);
  Serial.begin(57600);
}


void loop()
{
  while (gps.available(gpsPort)) { // Check when a NMEA message has been received
    currentFix = gps.read(); // Parse message

    noOfSatellites = gps.sat_count; // Get the number of satellites
    Serial.print("Number of satellites located: "); // Print number of satellites
    Serial.print(noOfSatellites);
    Serial.print("\n");
    
    if(currentFix.valid.speed) { // Check if a speed has been received
      Serial.print("Speed is: ");
      Serial.print(currentFix.speed_kph()); // Print speed in kph
      Serial.print("\n");
    }
    
    if(currentFix.valid.time) { // Check if time has been received
      Serial.print("Time is: "); // Print time
      Serial.print(currentFix.dateTime.hours);
      Serial.print(":");
      Serial.print(currentFix.dateTime.minutes);
      Serial.print(":");
      Serial.print(currentFix.dateTime.seconds);
      Serial.print("\n");
    }
  }
}
