void setup() {
  // start serial port at 19200 bps:
  Serial.begin(19200);
}
int x=0;
void loop() {
  x=x+1;
  //Serial.write("Hello");
  Serial.print(x,DEC);
  Serial.write("\n");
  delay(300);
}
