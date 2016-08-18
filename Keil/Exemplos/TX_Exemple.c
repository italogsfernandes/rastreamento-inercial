/**
* @brief This example sends packets continuously. The contents of P0 are
* sent in the first payload byte (byte 0).
*
* The example shows the minimum required setup for transmitting packets to a
* primary receiver (PRX) device.
*
* The following default radio parameters are being used:
* - RF channel 2
* - 2 Mbps data rate
* - TX address 0xE7E7E7E7E7
* - 1 byte CRC
*
* The project @ref esb_prx_example can be used as a counterpart for receiving the data.
*
*/
// Referências:
// http://techshlok.com/blog/wireless-communication-using-nrf24le1/

#include "nrf24le1.h" //I/O para NRF24LE1
#include "hal_clk.h" // Contem funções de clock

#include &lt;stdint.h&gt; //TODO: undestand: standard integers library
#include "hal_nrf.h" //Comunicação wireless

#include &lt;stdbool.h&gt; //TODO: undestand: standard boolean library

//variaveis globais
static bool volatile radio_busy;

void main(void){
  uint8_t payload[3]; //TODO: undestand: payload to be transmitted

  #ifdef MCU_NRF24LE1
  while(hal_clk_get_16m_source() != HAL_CLK_XOSC16M){
    // Aguardando o oscilador. 16 MHz
  }
  #endif

  #ifdef MCU_NRF24LU1P
  //Ativando SPI do radio
  //TODO: Pq a diferença com o RX?
  RFCTL = 0x10U;
  #endif

  // Ativa o clock do radio
  //TODO: Por que essa diferença com o RX?
  RFCKEN = 1U;
  // Ativa a interrupção do RF
  RF = 1U;
  // Ativa a interrupção global
  EA = 1U;

  // Ligando a energia do radio
  hal_nrf_set_power_mode(HAL_NRF_PWR_UP);

  //Loop infinito
  for(;;){
    //TODO: alterar e entender
    // Put P0 contents in payload[0]
    payload[0] = ~P0; // escreve o complemento de Port0

    //TODO: esta função escreve?
    // Escreve a payload no radio TX FIFO
    hal_nrf_write_tx_payload(payload,3U);

    // Altera o sinal do radio CE para iniciar a transmissão
    CE_PULSE();

    radio_busy = true;
    //Aguarda o radio operar
    while (radio_busy){
    }
  }
}

//Interrpção do radio
NRF_ISR(){
  uint8_t irq_flags;
  // Le e limpa as flags IRQ do radio
  irq_flags = hal_nrf_get_clear_irq_flags();

  switch (irq_flags) {
    // transmissão com sucesso
    case (1 &lt;&lt; (uint8_t)HAL_NRF_TX_DS):
    radio_busy = false;
    // Os dados foram enviados
    break;

    // Transmissão falhou (maximum re-transmits)
    case (1 &lt;&lt; (uint8_t)HAL_NRF_MAX_RT):
    // When a MAX_RT interrupt occurs the TX payload will not be removed from the TX FIFO.
    // If the packet is to be discarded this must be done manually by flushing the TX FIFO.
    // Alternatively, CE_PULSE() can be called re-starting transmission of the payload.
    // (Will only be possible after the radio irq flags are cleared)
    hal_nrf_flush_tx();
    radio_busy = false;
    break;

    default:
    break;
  }
}
