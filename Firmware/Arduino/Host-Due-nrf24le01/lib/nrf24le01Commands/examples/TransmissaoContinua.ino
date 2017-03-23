#include <nrf24le01.h>

nrf24le01 tx_nrf(4);

//Enderecos:
//Definido como endere�o da pipe 0
uint8_t code ADDR_HOST[TX_ADR_WIDTH] = {0xE7,0xE7,0xE7,0xE7,0xE7}; // Define a host adr

void setup(){
  tx_nrf.rf_init();
  tx_nrf.RX_Mode();
}

void loop() {
  tx_nrf.TX_Mode_NOACK(1);
  delay(100);
}
