void setup() {
  // start serial port at 57600 bps:
  Serial.begin(57600);
}

String replyIf1 = "0";
long UTCtimeStamp = 0;

//####################################################
// Initialise all signals.
//####################################################

bool eStop1 = 1;  
float bmsTemp2 = 0;
float carSpeed3 = 0;
float bmsVoltage4 = 0;
float bmsCurrent5 = 0;
float powerkW6 = 0;
float leftMotorVoltage7 = 0; // Not represented in Application yet 
float rightMotorVoltage8 = 0; // Not represented in Application yet 
float leftMotorCurrent9 = 0; // Not represented in Application yet 
float rightMotorCurrent10 = 0; // Not represented in Application yet 
float steeringInput11 = 0; // Not represented in Application yet 
float acceleratorPedalPosition12 = 0;

// Further signals not yet on Raimonds CANBus ID List
float brakePedalPosition13 = 0;
float suspensionsFrontLeft14 = 0;
float suspensionsFrontRight15 = 0;
float suspensionsRearLeft16 = 0;
float suspensionsRearRight17 = 0;

int noOfSatellites = 0;
//####################################################

int count = 0; // just for dummySignals

void loop() {
 if (Serial.available()>0) {
   replyIf1 = Serial.read();
 }
 
 if(replyIf1 == "49") { // When laptop sends "1"  //ASCII value for 1.
  
  generateDummyValues(); // Makes dummy signal values

  // Scale signals up to two bytes and send
  sendSerialAsTwoBytes(1);
  sendSerialAsTwoBytes(bmsTemp2); // Takes a float up to 0 - 6500. Sends it to 1 decimal place.
  sendSerialAsTwoBytes(carSpeed3);
  sendSerialAsTwoBytes(bmsVoltage4);
  sendSerialAsTwoBytes(bmsCurrent5);
  sendSerialAsTwoBytes(powerkW6); // Power
  
  sendSerialAsTwoBytes(leftMotorVoltage7);
  sendSerialAsTwoBytes(rightMotorVoltage8);
  sendSerialAsTwoBytes(leftMotorCurrent9);
  sendSerialAsTwoBytes(rightMotorCurrent10);

  sendSerialAsTwoBytes(steeringInput11);
  sendSerialAsTwoBytes(acceleratorPedalPosition12);

  // Further signals not yet on Raimonds CANBus ID List
  sendSerialAsTwoBytes(brakePedalPosition13);
  sendSerialAsTwoBytes(suspensionsFrontLeft14);
  sendSerialAsTwoBytes(suspensionsFrontRight15);
  sendSerialAsTwoBytes(suspensionsRearLeft16);
  sendSerialAsTwoBytes(suspensionsRearRight17);
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

void generateDummyValues() { // Generates dummy data for example & sys testing
  if(count == 1200) { // 1200 samples, about 10 a second, therefore roughly 2 minute loop
    count = 0;
  }
  count++;
  if(count > 30) {
    eStop1 = 1;
  }
  //
  bmsTemp2 = round( 30*sqrt(sqrt(sqrt(count))) + (bmsCurrent5/60) - 10);
  // Speed dummy signal
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
  //
  bmsVoltage4 = (76 - sqrt(count/2)) - (bmsCurrent5/60);
  //
  bmsCurrent5 = carSpeed3*3;
  //
  leftMotorVoltage7 = bmsVoltage4 - 3;
  //
  rightMotorVoltage8 = bmsVoltage4 - 3;
  //
  //leftMotorCurrent9 = round(bmsCurrent5/2);
  leftMotorCurrent9 = 50*sin((count%100)/(100/(2*PI)));
  //
  rightMotorCurrent10 = 0;
  // further signals
  acceleratorPedalPosition12 = carSpeed3;
  //
  if(acceleratorPedalPosition12 == 0) {
    brakePedalPosition13 = 100;
  }
  else {
    brakePedalPosition13 = 0;
  }
  //
  suspensionsFrontLeft14 = count%100;
  //
  suspensionsFrontRight15 = count%100;
  //
  suspensionsRearLeft16 = count%100;
  //
  suspensionsRearRight17 = count%100;
  //
  powerkW6 = (bmsVoltage4 * bmsCurrent5)/1000;
  //
  steeringInput11 = 50*sin((count%100)/(100/(2*PI)));
  //
  if(count < 30)
    noOfSatellites = 0;
  if(count > 30)
    noOfSatellites = 4;
  if(count > 60)
    noOfSatellites = 5;
  if(count > 90)
    noOfSatellites = 6;
 }
