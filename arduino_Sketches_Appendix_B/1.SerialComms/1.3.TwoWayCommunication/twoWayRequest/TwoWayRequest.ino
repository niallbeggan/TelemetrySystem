void setup() {
  // start serial port at 19200 bps:
  Serial.begin(19200);
}
int x=0;
void loop() {
  x=x+1;
  Serial.write("Say Hello!\n");

  while (Serial.available()>0) {
   Serial.write(Serial.read());
    delay(10);
  }

  delay(1000);
}
