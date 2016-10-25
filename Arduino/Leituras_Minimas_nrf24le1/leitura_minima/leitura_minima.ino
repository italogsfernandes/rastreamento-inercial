int16_t Xac, Yac, Zac;
char leitura;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(38400);
  Serial1.begin(38400);
}

void loop() {
  //NOTE: Corrigindo o bug de acordo com as palavras do sergio. testar.
  
  if (Serial1.available()>0) {
    leitura = Serial1.read();
    if (leitura == 0x01 || leitura == 0x02) {
      while(Serial1.available()<7);
      Xac = Serial1.read() << 8 | Serial1.read();
      Yac = Serial1.read() << 8 | Serial1.read();
      Zac = Serial1.read() << 8 | Serial1.read();
      if(Serial1.read() == '\n'){ //se começou com 0x01 ou 0x02 e terminou com \n então o pacote é valido
        Serial.print(int(leitura)); Serial.print(":\t");
        Serial.print(Xac); Serial.print("\t");
        Serial.print(Yac); Serial.print("\t");
        Serial.print(Zac); Serial.print("\t\n");
      }
      
    } else {
      Serial.print("\n*****A leitura nao eh um pacote inercial****\n");
      while(leitura != '\n'){
        Serial.print(leitura = Serial1.read());
      }
    }
  }
}
