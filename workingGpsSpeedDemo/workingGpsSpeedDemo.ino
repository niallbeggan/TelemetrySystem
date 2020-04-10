#include <AltSoftSerial.h>
#include <NMEAGPS.h>
#include <GPSport.h>
#include <Streamers.h>



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

void setup() {
  altser.begin(9600);
  Serial.begin(57600);
}


void loop()
{
  
  while (gps.available(gpsPort)) {
    currentFix = gps.read();
    if(currentFix.valid.speed) {
      carSpeed3 = currentFix.speed_kph();
    }
    noOfSatellites = gps.sat_count;
  }


   if (Serial.available()>0) {
   replyIf1 = Serial.read();
 }
 
 if(replyIf1 == "49") { // When laptop sends "1"  //ASCII value for 1.
  
  sendSerialAsTwoBytes(empty);
  sendSerialAsTwoBytes(empty);

  
  sendSerialAsTwoBytes(carSpeed3);

  
  sendSerialAsTwoBytes(empty);
  sendSerialAsTwoBytes(empty);
  sendSerialAsTwoBytes(empty);
  sendSerialAsTwoBytes(empty);
  sendSerialAsTwoBytes(empty);
  sendSerialAsTwoBytes(empty);
  sendSerialAsTwoBytes(empty);
  sendSerialAsTwoBytes(empty);
  sendSerialAsTwoBytes(empty);
  sendSerialAsTwoBytes(empty);
  sendSerialAsTwoBytes(empty);
  sendSerialAsTwoBytes(empty);
  sendSerialAsTwoBytes(empty);
  sendSerialAsTwoBytes(empty);
  
  sendSerialAsTwoBytes(noOfSatellites);
  sendTimestampOverSerialAs4Bytes();

  replyIf1 = "0"; // dont reply again until next request
 }
}

void sendSerialAsTwoBytes(float value) {
  int decimalValue = round(value * 10); // Send with one decimal place. Currently only sending whole integers anyway.
  byte small = (byte) (decimalValue & 0xFF);
  byte big = (byte) ((decimalValue >> 8) & 0xFF);
  Serial.write(big);
  Serial.write(small);
}

void sendTimestampOverSerialAs4Bytes() {
  UTCtimeStamp = UTCtimeStamp + 1; // Seconds accuracy for now
  
  byte b[4];
  for (int i=0; i<4; i++) {
    b[i]=((UTCtimeStamp>>(i*8)) & 0xff); //extract the right-most byte of the shifted variable
    Serial.write(b[i]);
  }
}
