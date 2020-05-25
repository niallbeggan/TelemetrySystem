// This program tests propagation delay between computer and receiving arduino, 
// through the two radio trasnceiver modules. It connects a hardware interrupt on
// the receiving arduino to the tx pin of the USB - Serial converter connecting
// the computer and the radio transmiiter, and measures the time between transmission
// and receipt. Both must have common ground to work, and the interrupt pin must have
// a pull down resistor. To run this test open a serial port terminl (I used termite)
// and send any data through the radio. You should receive a reply with the delay.

// Results: 31mS +- 1mS. Rising or Falling edge interrupt made no difference.

// Written by Niall Beggan 27/4/2020
// Last updated: 2/5/2020 - Added comments

void setup() {
  Serial.begin(57600); // Begin radio transceiver serial communication
  attachInterrupt(digitalPinToInterrupt(2),interruptFunction,FALLING); // PPS hardware interrupt setup. This can only be on pins 2 & 3 on nano
}

unsigned long timeFirstBitWasReceived = 0;
unsigned long timeFirstBitWasSent = 0;
int measureFirstInterruptOnly = 0;

void loop() {
    if(Serial.available()>0) {
      timeFirstBitWasReceived = millis();
      while(Serial.available()) {
        Serial.read();
        delay(10);
      }
      
      Serial.print("First bit was sent at (mS): ");
      Serial.println(timeFirstBitWasSent);
      Serial.print("First bit was received at (mS): ");
      Serial.println(timeFirstBitWasReceived);
      Serial.print("Time taken to send (mS): ");
      Serial.println(timeFirstBitWasReceived - timeFirstBitWasSent);
      measureFirstInterruptOnly = 0;
    }
}

void interruptFunction() {
  if(measureFirstInterruptOnly == 0) {
    timeFirstBitWasSent = millis();  // Update current start of second time for the current second
    measureFirstInterruptOnly = 1;
  } 
}
