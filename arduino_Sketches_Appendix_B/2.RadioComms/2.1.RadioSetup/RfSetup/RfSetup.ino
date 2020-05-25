//based on http://www.control.aau.dk/~jdn/edu/doc/arduino/sketchbook/apc220cansat/apc220cansat.ino
 
#include <SoftwareSerial.h>
 
const int pinRX = 12;
const int pinTX = 11;
const int pinSET= 13;
 
SoftwareSerial apc220(pinRX, pinTX); // Crt softserial port and bind tx/rx to appropriate PINS
 
void setupSoftAPC(void){
pinMode(pinSET, HIGH);
 
apc220.begin(9600);
}
 
void setSettings(void){
digitalWrite(pinSET, LOW); // pulling SET to low will put apc220 in config mode
delay(10); // stabilize please
apc220.println("WR 433900 4 9 6 0"); // ask for data
delay(10);
 
while (apc220.available()) {
Serial.write(apc220.read());
}
digitalWrite(pinSET, HIGH); // put apc220 back in operation
delay(200);
}
void getSettings(void) {
digitalWrite(pinSET, LOW); // pulling SET to low will put apc220 in config mode
delay(10); // stabilize please
apc220.println("RD"); // ask for data
delay(10);
 
while (apc220.available()) {
Serial.write(apc220.read());
}
digitalWrite(pinSET, HIGH); // put apc220 back in operation
delay(200);
}
 
void setup(){
Serial.begin(9600);
setupSoftAPC();
setSettings();
}

void loop(){
apc220.println("Hello World!");
delay(5000);
}
