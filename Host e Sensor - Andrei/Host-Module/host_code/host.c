#include "reg24le1.h" //definicoes basicas de pinos
#include <nRF-SPIComands.h> //Comunicacao RF
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
   P1DIR = 0x00;   // Tudo output
   P2DIR = 0xFF;
   P0CON = 0x00;  	// All general I/O
   P1CON = 0x00;  	// All general I/O
   P2CON = 0x00;  	// All general I/O
}

void setup(){
  iniciarIO();
  rf_init(ADDR_HOST,ADDR_HOST,10,RF_DATA_RATE_2Mbps,RF_TX_POWER_0dBm);
  hal_uart_init(UART_BAUD_115K2);
}

void main(){
  setup();
  while(1){
    if(hal_uart_chars_available()){
      if(hal_uart_getchar() == UART_START){ //first byte should be start
        switch (hal_uart_getchar()) { //the actual command
          case CMD_START:
          //Return Ok command and activate led
          //Reset FIFO and wait 5ms
          //Start Timer Aquisition
          break;
          case CMD_STOP:
          //Stop Timer
          //Return ok and reset led
          break;
          case CMD_CONNECTION:
          //scan rf web for available sensors
          //return list of sensors numbers
          break;
          case CMD_CALIBRATE:
          //send confimation command
          //run callibration routine
          //send flag that all is done
          break;
          default:
          //i don't know what to do here
          break;
        } /*END SWITCH*/
      } /*END IF START COMMAND*/
    } /*END IF UART AVAILABLE*/
    if(newPayload){
      send_packet_to_computer(rx_buf[0], rx_buf, payloadWidth-1);
      sta = 0;
      newPayload = 0;
    }
  } /*END INFINITE LOOP*/
} /*END MAIN FUNCTION*/
