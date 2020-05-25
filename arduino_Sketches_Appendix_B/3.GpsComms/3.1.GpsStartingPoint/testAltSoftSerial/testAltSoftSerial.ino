// This program relays any information received from the GPS module Uranus "662-f"
// to the serial port to be viewed on a computer.
// Written by Niall Beggan
// Last updated: 1/5/2020 - comprehensive commenting

// Include the AltSoftSerial library
#include <AltSoftSerial.h>

// Declare an AltSoftSerial object
AltSoftSerial altser; // Needs pins 8 and 9 on a nano. 

// Set up the serial communication
void setup() {
  altser.begin(9600);
  Serial.begin(9600);
}

// Forward any data received to the computer serial port
void loop() {
  if (altser.available()) {
    Serial.write(altser.read());
  }
}
