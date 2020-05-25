// This program is part of a propagation delay test between a computer and receiving arduino, through the two radio modules.
// To run the test the telemetry application needs to be run in QtCreator
// Uncomment all the lines in the serialportthread.cpp file with the comment "// UNCOMMENT FOR DELAY TEST"
// The telemtetry application starts a timer when it sends a message and stops it when it gets the reply
// It then prints the elapsed time to the console.
// Written by Niall Beggan 27/4/2020
// Last updated : 2/5/2020 - Added more comments

void setup() {
  Serial.begin(57600); // Begin radio transceiver serial communication
}

int receiveTime = 0;
int replyTime = 0;
int empty = 0;

void loop() {
  if (Serial.available()>0) {
    Serial.read();
    byte empty = 0;
    for(int i = 0; i < 144; i++) {
        Serial.write(empty); // These are just send so the application will accept the packet 
    }
  }
}
