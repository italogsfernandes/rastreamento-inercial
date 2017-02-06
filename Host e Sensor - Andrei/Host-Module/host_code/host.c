#include "reg24le1.h" //definicoes basicas de pinos
#include "nRF-SPIComands.h" //Comunicacao RF
#include "hal_uart.h" //Comunicacao Serial Uart

//Subenderecos usados na rede
#define HOST_SUB_ADDR 0x00
#define SENSOR_1_SUB_ADDR 0x01
#define SENSOR_2_SUB_ADDR 0x02
#define SENSOR_3_SUB_ADDR 0x03
#define SENSOR_4_SUB_ADDR 0x04


/*
UART Packet: Start Signal - Command
*/
#define UART_START 0x00

#define CMD_OK  0x00 //Ack - Uart Command
#define CMD_ERROR 0x01 //Error flag - Uart Command
#define CMD_START 0x02 //Start Measuring - Uart Command
#define CMD_STOP  0x03 //Stop Measuring - Uart Command
#define CMD_CONNECTION  0x04 //Teste Connection - Uart Command
#define CMD_CALIBRATE 0x05 //Calibrate Sensors Command



//TODO: verify
void iniciarIO(void){
   P0DIR = 0xB7;   // 1011 0111 - 1/0 = In/Out - Output: P0.3 e P0.6 - Input: P0.4 e outros
   P1DIR = 0xFF;   // Tudo input
   P2DIR = 0xFF;
   P0CON = 0x00;  	// All general I/O
   P1CON = 0x00;  	// All general I/O
   P2CON = 0x00;  	// All general I/O
}

void setup(){
  iniciarIO();
  hal_uart_init(UART_BAUD_115K2);
}
uint8_t received_command;
void main(){
  setup();
  while(1){
    if(hal_uart_chars_available()){
      if(hal_uart_getchar() == UART_START){ //first byte should be start
        received_command = hal_uart_getchar();
        switch (received_command) {
          case CMD_START:
          //do something
          break;
          case CMD_STOP:
          //do something
          break;
          case CMD_CONNECTION:
          //do something
          break;
          case CMD_CALIBRATE:
          //do something else, break
          break;
          default:
          //do something
          break;
        }
      }
    }
  }
}
