int16_t Xac, Yac, Zac;
int8_t Sensor_desejado;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(38400);
  Serial1.begin(38400);
}

void loop() {
  //BUG: Melhorar metodo de saber ql sensor foi,
  //      do jeito que ta vai dar certo não.
  //      Muita margem pra erro
  if (Serial1.available()) {
    if ( (Sensor_desejado = Serial1.read()) == 0x02) {
      Xac = Serial1.read() << 8 | Serial1.read();
      Yac = Serial1.read() << 8 | Serial1.read();
      Zac = Serial1.read() << 8 | Serial1.read();
      Serial.print(Sensor_desejado); Serial.print("\t");
      Serial.print(Xac); Serial.print("\t");
      Serial.print(Yac); Serial.print("\t");
      Serial.print(Zac); Serial.print("\t\n");
    } else {
      Serial.write(Sensor_desejado);
    }
  }
}
