
void setup() {
  // put your setup code here, to run once:
  Serial.begin(38400);
  Serial1.begin(38400);
}

void loop() {
  while(Serial1.available()){
    Serial.write(Serial1.read());
  }
}
