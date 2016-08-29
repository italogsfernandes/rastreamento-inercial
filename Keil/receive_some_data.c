// Referências:
// http://techshlok.com/blog/wireless-communication-using-nrf24le1/
/* Enviando dados
*   Parametros:
*       - RF channel:  RF_CH = 2 (default)
*       - Data rate: RF_DR_HIGH = 01 (dafault) 2 Mps
*       - RX_ADDR_P0 = 0xE7E7E7E7E7 (default), 0xC2C2C2C2C2 (pipe 1) (default)
        - EN_RXADDR = Default, pipe 0 e pipe 1
*       - CRC: 1 byte, EN_CRC = 1, CRCO = 0, (defaults)
*       - Power: 0 dBm (default)
*
*
* - Adicionar bibliotecas necessárias
* - Inicializar payload e demais variaveis
* - Ativar SPI do radio (FIXME: COMO? Oq é RFCTL?)
* - Ativar interrupções - FIXME: COMO FUNCIONA?
* - PWR_UP = 1 - Power up -  Utilizar: hal_nrf_set_power_mode(HAL_NRF_PWR_UP);
* - Colocar vetor payload na tx-fifo: hal_nrf_write_tx_payload(payload,3U); // tamanho 3U
* - CE_PULSE() - Pulsar CE para enviar a payload
* - Verificar interrupção de TX_DS e MAX_RT - FIXME: COMO FUNCIONA?
*/
//FIXME: Biblioteca "hal_nrf.h"
