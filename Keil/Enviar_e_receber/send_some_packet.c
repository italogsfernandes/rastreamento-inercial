// Referências:
// http://techshlok.com/blog/wireless-communication-using-nrf24le1/
/* Enviando dados
*   Parametros:
*       - RF channel:  RF_CH = 2 (default)
*       - Data rate: RF_DR_HIGH = 01 (dafault) 2 Mps
*       - TX_ADDR = 0xE7E7E7E7E7 (default)
*       - CRC: 1 byte, EN_CRC = 1, CRCO = 0, (defaults)
*       - Power: 0 dBm (default)
*       - PRIM_RX = 0 (PTX) (default)
*
*
* - Adicionar bibliotecas necessárias
* - Inicializar payload e demais variaveis
* - FIXME:  while(hal_clk_get_16m_source() != HAL_CLK_XOSC16M)?
* - Ativar SPI do radio (FIXME: COMO? Oq é RFCTL?)
* - Ativar o clock do Radio.
* - Ativar interrupções - FIXME: COMO FUNCIONA?
* - PWR_UP = 1 - Power up -  Utilizar: hal_nrf_set_power_mode(HAL_NRF_PWR_UP);
* - Colocar vetor payload na tx-fifo: hal_nrf_write_tx_payload(payload,3U); // tamanho 3U
* - CE_PULSE() - Pulsar CE para enviar a payload
* - Verificar interrupção de TX_DS e MAX_RT - FIXME: COMO FUNCIONA?
*/
//FIXME: Biblioteca "hal_nrf.h"

/**********************(purposed) Packet to Send***************************
| [QUAT W] [QUAT X] [QUAT Y] [QUAT Z] [GYRO X] [GYRO Y] [GYRO Z] |
|   0   1   2    3  4     5   6    7   8    9  10   11  12   13  |
| [MAG X ] [MAG Y ] [MAG Z ] [ACC X ] [ACC Y ] [ACC Z ] |
|  14  15  16  17   18   19   20  21  22   23   24  25  |
**************************************************************/

//Example with some numbers
uint8_t paket2send[26] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
                    21,22,23,24,25};
