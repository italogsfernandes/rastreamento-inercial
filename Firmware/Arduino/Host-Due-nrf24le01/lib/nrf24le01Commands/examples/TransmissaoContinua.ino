#include <nrf24le01Module.h>

nrf24le01 tx_nrf(2,3,4);

void setup(){
  Serial.begin(9600);
  tx_nrf.rf_init(tx_nrf.ADDR_HOST,tx_nrf.ADDR_HOST,10,RF_DATA_RATE_2Mbps,RF_TX_POWER_0dBm);
  Serial.print("Transmitter with polling iniciado...\n");
}

void loop() {
  tx_buf[0] = 0x42;
  tx_nrf.TX_Mode_NOACK(1);
  if(newPayload){
    Serial.print(rx_buf[0],HEX);
    if(rx_buf[0] == 0x00){
      Serial.print(" - turn on signal received by sensor.\n");
    } else {
      Serial.print(" - Nao reconhecido.\n");
    }
    sta = 0;
    newPayload = 0;
  }
  delay(1000);

  tx_buf[0] = 0x53;
  tx_nrf.TX_Mode_NOACK(1);
  if(newPayload){
    Serial.print(rx_buf[0],HEX);
    if(rx_buf[0] == 0x01){
      Serial.print(" - turn off signal received by sensor.\n");
    } else {
      Serial.print(" - Nao reconhecido.\n");
    }
    sta = 0;
    newPayload = 0;
  }
  delay(1000);
}
