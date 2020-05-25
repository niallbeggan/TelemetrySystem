void setup() {
  // start serial port at 9600 bps:
  Serial.begin(19200);
}

void loop() {
  if(Serial.available()>0) {
    Serial.write("\nSOMETHING: ");

    while (Serial.available()>0) {
        Serial.write(Serial.read());
        delay(10);
    }
  }
}
