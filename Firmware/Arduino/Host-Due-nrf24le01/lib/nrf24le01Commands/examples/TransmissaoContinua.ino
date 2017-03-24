#include <nrf24le01Commands.h>

nrf24le01 tx_nrf(2,3,4);

void setup(){
  Serial.begin(9600);
  tx_nrf.rf_init(tx_nrf.ADDR_HOST,tx_nrf.ADDR_HOST,10,RF_DATA_RATE_2Mbps,RF_TX_POWER_0dBm);
}

void loop() {
  tx_buf[0] = 0x42;
  tx_nrf.TX_Mode_NOACK(1);
  delay(1000);
}
