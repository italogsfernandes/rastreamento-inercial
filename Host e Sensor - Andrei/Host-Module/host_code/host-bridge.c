#include "Nordic\reg24le1.h" //definicoes basicas de pinos
#include "nRF-SPIComands.h" //Comunicacao RF
#include "hal_uart.h" //Comunicacao Serial Uart
#include "hal_delay.h"
#include "pacotes_inerciais.h"

//UART Packet: Start Signal - Command
#define UART_START_SIGNAL  0x53

//////////////////////
//Functions in Host //
//////////////////////

/**
 * Trata os comandos recebidos por UART,
 * redirecionando-os como pacotes RF formados por:
 * [Start] [Length] [Data] [..] [checksum]
 * Estar atento ao limite máximo da payload.
 */
void uart_communication_handler();

/**
 * Trata os comandos recebidos por RF,
 * redirecionando-os para a interface serial.
 */
void rf_communication_handler();

///////////////////
//Implementation //
///////////////////

/**
* Seta os pinos do nrf como saidas e entradas de acordo com as funcoes desejadas
*/
void iniciarIO(void){
  P0DIR = 0x00;   // Tudo output
  P1DIR = 0x00;   // Tudo output
  P0CON = 0x00; P1CON = 0x00; //Reseting PxCON registers
  P0DIR &= ~(1<<3);//P03 = Uart tx = output
  P0DIR |= 1<<4;//P04 = UART Rx = input
}

void setup(){
  iniciarIO();
  hal_uart_init(UART_BAUD_9K6);
  rf_init(ADDR_HOST,ADDR_HOST,10,RF_DATA_RATE_2Mbps,RF_TX_POWER_0dBm);
  P06 = 1;delay_ms(500);	P06 = 0;delay_ms(500);
  P06 = 1;delay_ms(500);	P06 = 0;delay_ms(500);
}

void main(){
  setup();
  while(1){ //Loop

    /////////////////////
    //Comunicao Serial //
    /////////////////////
    if(hal_uart_chars_available()){
      uart_communication_handler();
    } /*END IF UART AVAILABLE*/

    ///////////////////
    //Comunicacao RF //
    ///////////////////
    if(newPayload){
      rf_communication_handler();
    }
  } /*END INFINITE LOOP*/
} /*END MAIN FUNCTION*/

/////////////////////
//FUNCIONS in Host //
/////////////////////

//START - LEN - DATA - END
//TODO: figure out how to proced with an error?
void uart_communication_handler(){
  uint8_t packet_len = 0;
  uint8_t i = 0;
  uint8_t checksum = 0;
  if(hal_uart_getchar() == UART_START_SIGNAL){ //first byte should be start
    packet_len = hal_uart_getchar();
    checksum = 0;
    for (i = 0; i < packet_len; i++) {
      rx_buf[i] = hal_uart_getchar();
      checksum += rx_buf[i];
    }
    checksum = (~checksum) + 1;
    if(hal_uart_getchar() == checksum){
      switch (rx_buf[3]) {
        case CMD_LIGHT_UP_LED:
        P06 = 1;
        break;
        case CMD_TURN_OFF_LED:
        P06 = 0;
        break;
      }
      TX_Mode_NOACK(packet_len);
      RX_Mode();
    }
  }
}

void rf_communication_handler(){
  uint8_t i = 0;
  uint8_t checksum = 0;
  hal_uart_putchar(UART_START_SIGNAL);//Start
  hal_uart_putchar(payloadWidth);//Len
  for (i = 0; i < payloadWidth; i++) {
    hal_uart_putchar(rx_buf[i]);//Data
    checksum += rx_buf[i];
  }
  checksum = (~checksum) + 1;
  hal_uart_putchar(checksum);//CC
}
