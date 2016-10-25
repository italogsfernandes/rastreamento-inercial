int sensor = 2;
int16_t Xac = 14, Yac=15, Zac=31222;
char leitura;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(38400);
  Serial1.begin(38400);
  mostrarleituras();
}

void loop() {
  if (Serial1.available()) {
    leitura = Serial1.read();
    if (leitura == 0x53) {
      //Serial.print("Start Byte Received\n");
      while(Serial1.available()<1){delay(1);}
      if (Serial1.read() == 7) {
        //Serial.print("7 bytes de leitura reconhecidos\n");
        while(Serial1.available()<7){delay(1);}
        sensor = Serial1.read();
        Xac = (Serial1.read() << 8) | Serial1.read();
        Yac = (Serial1.read() << 8) | Serial1.read();
        Zac = (Serial1.read() << 8) | Serial1.read();
        while(Serial1.available()<1){delay(1);}
        if (Serial1.read() == 0x04) {
          mostrarleituras();
        }
      }
    } else{
      Serial.print(leitura);
    }
  }
}

void mostrarleituras(){
  Serial.print(sensor);Serial.print(":\t");
  Serial.print((float)Xac/16382); Serial.print("\t");
  Serial.print((float)Yac/16382); Serial.print("\t");
  Serial.print((float)Zac/16382); Serial.print("\n");  
}

