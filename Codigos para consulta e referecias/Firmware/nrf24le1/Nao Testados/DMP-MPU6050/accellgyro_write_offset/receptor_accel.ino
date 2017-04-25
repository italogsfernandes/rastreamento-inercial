#define UART_START_FLAG	0x53
#define UART_END_FLAG 0x04
#define UART_PACKET_LENGHT 13

int sensor = 2;
int16_t Xac = 14, Yac=15, Zac=31222;
int16_t Xgy = 14, Ygy=15, Zgy=31222;
char leitura_1;
char leitura_2;
void setup() {
    // put your setup code here, to run once:
    Serial.begin(38400);
    Serial1.begin(38400);
    mostrarleituras();
}

void loop() {
    if (Serial1.available()) {
        while(Serial1.available()<2){delay(1);} //aguarda 2 bytes
        leitura_1 = Serial1.read();
        leitura_2 = Serial1.read();
        if (leitura_1 == UART_START_FLAG && leitura_2 == UART_PACKET_LENGHT) {
            while(Serial1.available()<UART_PACKET_LENGHT){delay(1);} //aguarda o pacote estar completo
            //recebendo dados:
            sensor = Serial1.read();
            Xac = (Serial1.read() << 8) | Serial1.read();
            Yac = (Serial1.read() << 8) | Serial1.read();
            Zac = (Serial1.read() << 8) | Serial1.read();
            Xgy = (Serial1.read() << 8) | Serial1.read();
            Ygy = (Serial1.read() << 8) | Serial1.read();
            Zgy = (Serial1.read() << 8) | Serial1.read();

            while(Serial1.available()<1){delay(1);}//aguardando end_signal
            if (Serial1.read() == UART_END_FLAG) { //se eng signal recebido entao mostra leituras
                mostrarleituras();
            }
        } else {
            Serial.print(leitura_1);
            Serial.print(leitura_2);
        }
    }
}

void mostrarleituras(){
    Serial.print(sensor);Serial.print(":\t");
    Serial.print((float)Xac/16384); Serial.print("\t");
    Serial.print((float)Yac/16384); Serial.print("\t");
    Serial.print((float)Zac/16384); Serial.print("\t");
    Serial.print((float)Xgy/262); Serial.print("\t");
    Serial.print((float)Ygy/262); Serial.print("\t");
    Serial.print((float)Zgy/262); Serial.print("\n");
}

