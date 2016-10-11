#include<Wire.h>

char to_send = 5;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(38400);
  Wire.begin(0x07 =);
  Wire1.begin();
  Wire.onReceive(receivingEvent);
  Wire.onRequest(requestingEvent);
  Serial.println("Ligado..");
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial.available()) {
    Wire1.beginTransmission(0x07);
    Wire1.write(Serial.read());
    Wire1.endTransmission();
  }
  delay(100); //delay de segurança
}

void receivingEvent(int howmany) {
  Serial.print("\n***RECEIVED***\n");
  while (Wire.available()) {
    Serial.println(Wire.read());
  }
}
void requestingEvent() {
  Wire.write(to_send);
}

