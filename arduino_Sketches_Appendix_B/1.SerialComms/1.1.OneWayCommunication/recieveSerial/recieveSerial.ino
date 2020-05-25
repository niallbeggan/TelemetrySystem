void setup() {
  // start serial port at 9600 bps:
  Serial.begin(19200);
}

void loop() {
  if (Serial.available()>0) {
      Serial.write("SOMETHING: ");
      Serial.write(Serial.read());
      Serial.write("\n");
  }
}
