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

/********************** Packet to Send***************************
| [QUAT W] [QUAT X] [QUAT Y] [QUAT Z] [GYRO X] [GYRO Y] [GYRO Z] |
|   0   1   2    3  4     5   6    7   8    9  10   11  12   13  |
| [MAG X ] [MAG Y ] [MAG Z ] [ACC X ] [ACC Y ] [ACC Z ] |
|  14  15  16  17   18   19   20  21  22   23   24  25  |
**************************************************************/

#include "reg24le1.h" //Definições de muitos endereços de registradores.
#include "stdint.h" //inteiros uint8_t, int8_t, uint16_t....
#include "stdbool.h" //Bolleanos
#include "API.h" //Define alguns registers e cabeçalhos de funções SPI
#include "app.h" //Some UART and io functions
#include "nRF-SPIComands.h" //rf_init, RF_IRQ, TX, RX, SPI_Write, SPI_Read ..

/*HACK: Pra que serve as bibliotecas UART.H, hal_uart.h, que estão nos arquivos
*mas não são usadas?
*/

/*NOTE: Códigos de referência:
* /rastreamento-inercial/Keil/Enviar_e_receber/Exemplo-Sergio/*
* ../Situacao_qnd_cheguei/nRF24LE1/Teste1nRF/TX/24le1TX.c
* ../Situacao_qnd_cheguei/nRF24LE1/Teste1nRF/RX/24le1RX-Rev000.c
* ../Situacao_qnd_cheguei/nRF24LE1/nRF24/Exemplos nRF24/-½-¿Á+transmission/24le1.c
* ../Situacao_qnd_cheguei/nRF24LE1/nRF24/Exemplos nRF24/-½-¿Á+Receiver/24le1.c
*/

#define	PIN32
#ifdef 	PIN32
sbit LED = P0^3; // 1/0=light/dark
#endif

//Váriaveis globais:
//Example with some numbers
uint8_t packet2send[26] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25};

void delay(unsigned int x);

/***************MAIN****************/
void main(void){
    // Set up GPIO
    P0DIR = 0xB7;   // 1011 0111 - 1/0 = In/Out - Output: P0.3 e P0.6
    P1DIR = 0xFF;   // Output: P0.0 - P0.2, Input: P0.3 - P0.5	 0xFF
    P2DIR = 0xFF;
    P0CON = 0x00;  	// All general I/O
    P1CON = 0x00;  	// All general I/O
    P2CON = 0x00;  	// All general I/O

    LED = 1;         // turn on LED
    delay(2000);
    LED = 0;         // turn off LED

    // Radio + SPI setup
    RFCE = 0;       // Radio chip enable low
    RFCKEN = 1;     // Radio clk enable
    RF = 1;

    //uart_init()
    rf_init();
    EA=1; //ativa as interrupções

    while(1){
        for(int i = 0; i<26; i++){
            tx_buf[i] = packet2send[i];
        }
        TX_Mode_NOACK();
        LED = 1;
        while(!(TX_DS|MAX_RT));
        sta = 0;
        delay(1000);
        LED = 0;
    }
}

/**************************************************/
void delay(unsigned int x)
{
    unsigned int i,j;
    i=0;
    for(i=0;i<x;i++)
    {
       j=508;
           ;
       while(j--);
    }
}
/**************************************************/
