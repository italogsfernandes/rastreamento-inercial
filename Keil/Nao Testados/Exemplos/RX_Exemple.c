/*
* @brief This example monitors for data and writes the first byte (byte 0) of the
* received payloads to P0.
*
* The example shows the minimum required setup for receiving packets from a
* primary transmitter (PTX) device.
*
* The following default radio parameters are being used:
* - RF channel 2
* - 2 Mbps data rate
* - RX address 0xE7E7E7E7E7 (pipe 0) and 0xC2C2C2C2C2 (pipe 1)
* - 1 byte CRC
*
* The project @ref esb_ptx_example can be used as a counterpart for transmitting the data.
*
*/
// Referências:
// http://techshlok.com/blog/wireless-communication-using-nrf24le1/

#include "nrf24le1.h" //I/O para NRF24LE1
#include "hal_clk.h" // Contem funções de clock

#include &lt;stdint.h&gt; //TODO: undestand: standard integers library
#include "hal_nrf.h" //Comunicação wireless

// Global variables
uint8_t payload[3]; // payload que será recebida


// main function
void main()
{

  #ifdef MCU_NRF24LE1
  while(hal_clk_get_16m_source() != HAL_CLK_XOSC16M){
    // Aguardando o oscilador. 16 MHz
  }
  #endif

  #ifdef MCU_NRF24LU1P
  //Ativando SPI do radio
  //TODO: Pq a diferenaça com o TX?
  RFCTL = 0x10;
  #endif

  // Set P0 as output
  P0DIR = 0;

  // Ativa o clock do radio
  //TODO: pq essa diferença com o TX?
  RFCKEN = 1;

  // Ativa a interrupção do RF
  RF = 1;
  // Ativa a interrupção global
  EA = 1;

  // Configura o radio como um receptor primario (PTX)
  //TODO: pq o transmissor não foi setado com: HAL_NRF_PTX
  hal_nrf_set_operation_mode(HAL_NRF_PRX);

  // Configura o comprimento da payload como 3 bytes
  hal_nrf_set_rx_payload_width((int)HAL_NRF_PIPE0, 3);

  // Ligando a energia do radio
  hal_nrf_set_power_mode(HAL_NRF_PWR_UP);

  // Ativando o Receptor
  CE_HIGH();

  // Loop infinito
  for(;;){}
}

// Interrupção do Radio
NRF_ISR()
{
  uint8_t irq_flags;

  // Le e limpa as flags IRQ do radio
  irq_flags = hal_nrf_get_clear_irq_flags();

  // Se recebeu dados
  if((irq_flags &amp; (1&lt;&lt;(uint8_t)HAL_NRF_RX_DR)) &gt; 0)
  {
    // Le os dados
    while(!hal_nrf_rx_fifo_empty())
    {
      hal_nrf_read_rx_payload(payload);
    }

    // Escreve a payload[0] recebida na port0
    P0 = payload[0];
  }
}
