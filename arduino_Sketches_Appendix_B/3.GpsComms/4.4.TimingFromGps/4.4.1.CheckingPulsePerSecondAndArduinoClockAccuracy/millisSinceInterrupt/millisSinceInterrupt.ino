// This is a program that checks a pin that reads a re occuring pulse, 
// and outputs the milliseconds since the last pulse occured.
// This was written as a test program to check the accuracy 
// that could be achieved using an arduino to measure time between
// the Uranus 622f GPS module's PPS (pulse per second) output (PIN 6, P1PS).
// Result: +- 1 mS accuracy was acheieved.

// Written by Niall Beggan 23/4/2020 

// A demonstration of this being used for accurate timing can be found in "GpsDemoWithTimestamp.ino"

// Set up serial comms and hardware interrupt
void setup() {
  attachInterrupt(digitalPinToInterrupt(2),interruptFunction,RISING);
  Serial.begin(57600);
}

// Declare variables
unsigned long millisSincePreviousPulse = 0;
unsigned long millisAtPreviousPulse = 0;
unsigned long millisAtCurrentPulse = 0;
int notFirstPulse = 0;
bool stopafter1 = false;

// Main loop
void loop() {
 if((digitalRead(2) == HIGH) && (stopafter1 == false) && (notFirstPulse > 2)) {
  delay(100);
  Serial.print("Millis since previous pulse: "); // Should be close to 1000
  Serial.print(millisSincePreviousPulse);
  Serial.print("\n");

//  Serial.print("Millis at current pulse: ");
//  Serial.print(millisAtCurrentPulse);
//  Serial.print("\n\n");
//  Serial.print("Millis at previous pulse: ");
//  Serial.print(millisAtPreviousPulse);
//  Serial.print("\n");
  stopafter1 = true;
 }

 if(digitalRead(2) == LOW) {
  stopafter1 = false;
 }
 
}

// This interrupt function gets the milliseconds at the pulse and uses 
// it to work out the milliseconds since the previous pulse
void interruptFunction() {
  millisAtPreviousPulse = millisAtCurrentPulse; // Previous pulses current time becomes the new previous time.
  if(notFirstPulse > 0) {
    millisSincePreviousPulse = millis() - millisAtPreviousPulse; // Calculate gap time
  }
  if(notFirstPulse < 3)
    notFirstPulse = notFirstPulse + 1;
  millisAtCurrentPulse = millis();  // Update current time for this pulse
}
