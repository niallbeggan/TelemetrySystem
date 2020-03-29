void setup() {
  // start serial port at 9600 bps:
  Serial.begin(57600);
}

String replyIf1 = "0";

int eStop1 = 1;  //Try and define all signals so I can make a packet to send them all at once.
int bmsTemp2 = 20;
int carSpeed3 = 0;
int bmsVoltage4 = 75.6;
int bmsCurrent5 = 0;
int motorVoltage6 = 0;
int motorCurrent7 = 0;
int tsPower8 = 0;
// Further signals
int acceleratorPedalPosition9 = 0;
int brakePedalPosition10 = 0;
int suspensionsFrontLeft11 = 0;
int suspensionsFrontRight12 = 0;
int suspensionsRearLeft13 = 0;
int suspensionsRearRight14 = 0;

int count = 0; // just for functions

void loop() {
 if (Serial.available()>0) {
   replyIf1 = Serial.read();
 }
 
 if(replyIf1 == "49") { // When laptop sends "1"
  if(count == 1200) { // 1200 samples, about 10 a second, therefore roughly 2 minute loop
    count = 0;
  }
  count++;
  String message = String(eStop1) + "," 
                  + String(bmsTemp2) + ","
                  + String(carSpeed3) + ","
                  + String(bmsVoltage4) + ","
                  + String(bmsCurrent5) + ","
                  + String(motorVoltage6) + ","
                  + String(motorCurrent7) + ","
                  + String(tsPower8) + ","
                  + String(acceleratorPedalPosition9) + ","
                  + String(brakePedalPosition10) + ","
                  + String(suspensionsFrontLeft11) + ","
                  + String(suspensionsFrontRight12) + ","
                  + String(suspensionsRearLeft13) + ","
                  + String(suspensionsRearRight14) + ",\n" ;
   Serial.print(message);
   replyIf1 = "0"; // dont reply again until next request
  }
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
  //
  tsPower8 = bmsVoltage4 * bmsCurrent5;
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
