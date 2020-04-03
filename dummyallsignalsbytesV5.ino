void setup() {
  // start serial port at 57600 bps:
  Serial.begin(57600);
}

String replyIf1 = "0";

//####################################################
// Initialise all signals.
//####################################################

bool eStop1 = 1;  
float bmsTemp2 = 20;
float carSpeed3 = 0;
float bmsVoltage4 = 75.6;
float bmsCurrent5 = 0;
float motorVoltage6 = 0;
float motorCurrent7 = 0;
// Further signals
float acceleratorPedalPosition9 = 0;
float brakePedalPosition10 = 0;
float suspensionsFrontLeft11 = 0;
float suspensionsFrontRight12 = 0;
float suspensionsRearLeft13 = 0;
float suspensionsRearRight14 = 0;

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
  sendSerialAsTwoBytes(motorVoltage6);
  sendSerialAsTwoBytes(motorCurrent7);

  sendSerialAsTwoBytes(100); // Power
  sendSerialAsTwoBytes(acceleratorPedalPosition9);
  sendSerialAsTwoBytes(brakePedalPosition10);
  sendSerialAsTwoBytes(suspensionsFrontLeft11);
  sendSerialAsTwoBytes(suspensionsFrontRight12);
  sendSerialAsTwoBytes(suspensionsRearLeft13);
  sendSerialAsTwoBytes(suspensionsRearRight14);

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

void generateDummyValues() { // Generates dummy data
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
  bmsVoltage4 = round( (76 - sqrt(count/2)) - (bmsCurrent5/60) );
  //
  bmsCurrent5 = carSpeed3*3;
  //
  motorVoltage6 = bmsVoltage4 - 3;
  //
  motorCurrent7 = round(bmsCurrent5/2);
  //further signals
  acceleratorPedalPosition9 = carSpeed3;
  //
  if(acceleratorPedalPosition9 == 0) {
    brakePedalPosition10 = 100;
  }
  else {
    brakePedalPosition10 = 0;
  }
  //
  suspensionsFrontLeft11 = count%100;
  //
  suspensionsFrontRight12 = count%100;
  //
  suspensionsRearLeft13 = count%100;
  //
  suspensionsRearRight14 = count%100;
 }
