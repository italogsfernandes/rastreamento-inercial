#include <nrf24le01.h>

nrf24le01 tx_nrf(4);

void setup(){
  tx_nrf.rf_init();
  tx_nrf.RX_Mode();
}

void loop() {
  tx_nrf.TX_Mode_NOACK(1);
  delay(100);
}
