// Referências:
// http://techshlok.com/blog/wireless-communication-using-nrf24le1/
/* Enviando dados
*   Parametros:
*       - RF channel:  RF_CH = 2 (default)
*       - Data rate: RF_DR_HIGH = 01 (dafault) 2 Mps
*       - RX_ADDR_P0 = 0xE7E7E7E7E7 (default), 0xC2C2C2C2C2 (pipe 1) (default)
*       - EN_RXADDR = Default, pipe 0 e pipe 1
*       - CRC: 1 byte, EN_CRC = 1, CRCO = 0, (defaults)
*       - Power: 0 dBm (default)
*       - PRIM_RX = 1 (PRX)
*
* - Adicionar bibliotecas necessárias
* - Inicializar payload e demais variaveis
* - FIXME:  while(hal_clk_get_16m_source() != HAL_CLK_XOSC16M)?
* - Ativar SPI do radio (FIXME: COMO? Oq é RFCTL?)
* - Ativar o clock do Radio.
* - Ativar interrupções - FIXME: COMO FUNCIONA?
* - PRIM_RX = 1 (PRX) -  hal_nrf_set_operation_mode(HAL_NRF_PRX);
* - Configurar o tamanho da RX-Payload igual ao da TX. hal_nrf_set_rx_payload_width((int)HAL_NRF_PIPE0, 3);
* - PWR_UP = 1 - Power up -  Utilizar: hal_nrf_set_power_mode(HAL_NRF_PWR_UP);
* - CE_HIGH() - Ativar CE para estar recebendo dados (ver figura 6)
* - Verificar interrupção de RX_DR - FIXME: COMO FUNCIONA?
*/
//FIXME: Biblioteca "hal_nrf.h"
