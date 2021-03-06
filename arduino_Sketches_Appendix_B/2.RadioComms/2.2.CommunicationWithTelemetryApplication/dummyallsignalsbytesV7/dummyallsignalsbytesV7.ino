// This Arduino program sends dummy values to the serial port when it receieves the 
// ASCII charachter for 1 i.e. a byte with the value 49.
// Written by Niall Beggan
// Last Updated: 7/5/2020 - changed all values to big endian


//####################################################
// Set up Serial communication

void setup() {
  // Start serial communication at 57600 bps:
  Serial.begin(57600);
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

// Not on CAN bus && no plans to be
int noOfSatellites = 0;

// This stores any incoming byte from the serial port.
String replyIf1 = "0";

// These variables are just for the generateDummyValues function
int count = 0;
long UTCtimeStamp = 70000;
int UTC_millis = 0;


//####################################################
// Main loop

void loop() {
 if (Serial.available()>0) {
   replyIf1 = Serial.read();
 }
 
 if(replyIf1 == "49") { // When laptop sends "1"  //ASCII value for 1.
  
  generateDummyValues(); // Makes dummy signal values

  // Scale signals up to two bytes and send
  sendSerialAsTwoBytes(1);
  sendSerialAsTwoBytes(bmsTemp2); // Takes a float within the range -3200.0 up to 3200.0. Sends it with 1 decimal place.
  sendSerialAsTwoBytes(carSpeed3);
  sendSerialAsTwoBytes(bmsVoltage4);
  sendSerialAsTwoBytes(bmsCurrent5);
  sendSerialAsTwoBytes(tsPowerkW6); // Power
  
  sendSerialAsTwoBytes(leftMotorVoltage7);
  sendSerialAsTwoBytes(rightMotorVoltage8);
  sendSerialAsTwoBytes(leftMotorCurrent9);
  sendSerialAsTwoBytes(rightMotorCurrent10);

  sendSerialAsTwoBytes(steeringInput11);
  sendSerialAsTwoBytes(acceleratorPedalPosition12);

  // NOT ON CAN BUS ID LIST YET
  sendSerialAsTwoBytes(brakePedalPosition13);
  sendSerialAsTwoBytes(suspensionsFrontLeft14);
  sendSerialAsTwoBytes(suspensionsFrontRight15);
  sendSerialAsTwoBytes(suspensionsRearLeft16);
  sendSerialAsTwoBytes(suspensionsRearRight17);
  
  // Not on CAN bus && no plans to be
  sendSerialAsTwoBytes(noOfSatellites);

  replyIf1 = "0"; // Dont reply again until next request
 }
}


//####################################################
// Serial write function that includes timestamp.

void sendSerialAsTwoBytes(float value) {
  int decimalValue = round(value * 10); // Send with one decimal place. Currently only sending whole integers anyway.
  byte small = (byte) (decimalValue & 0xFF);
  byte big = (byte) ((decimalValue >> 8) & 0xFF);
  Serial.write(small);
  Serial.write(big);

  sendTimestampOverSerialAs4Bytes(); // seconds
  sendTimestampMillisOverSerialAs2Bytes(); // miilliseconds
}


//####################################################
// Serial write timestamp seconds function

void sendTimestampOverSerialAs4Bytes() {  
  byte b[4];
  for (int i=0; i<4; i++) {
    b[i]=((UTCtimeStamp>>(i*8)) & 0xff); //extract the right-most byte of the shifted variable
    Serial.write(b[i]);
  }
}


//####################################################
// Serial write timestamp milliseconds function

void sendTimestampMillisOverSerialAs2Bytes() {
  int decimalValue =   UTC_millis; // UTC_millis
  byte small = (byte) (decimalValue & 0xFF);
  byte big = (byte) ((decimalValue >> 8) & 0xFF);
  Serial.write(small);
  Serial.write(big);
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
    UTCtimeStamp += 1;
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

  //####################################################
  // Number of satellites
  if(count < 30)
    noOfSatellites = 0;
  if(count > 30)
    noOfSatellites = 4;
  if(count > 60)
    noOfSatellites = 5;
  if(count > 90)
    noOfSatellites = 6;
 }
