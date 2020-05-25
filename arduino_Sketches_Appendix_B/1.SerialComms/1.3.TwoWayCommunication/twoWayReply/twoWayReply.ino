void setup() {
  // start serial port at 9600 bps:
  Serial.begin(19200);
}

void loop() {
  if(Serial.available()>0) {
    while (Serial.available()>0) {
       Serial.read();
        delay(10);
    }
    Serial.write("Replying: Hi\n");
  }
}
