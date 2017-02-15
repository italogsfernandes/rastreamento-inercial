#include "reg24le1.h" //definicoes basicas de pinos
#include "nRF-SPIComands.h" //Comunicacao RF
#include "hal_uart.h" //Comunicacao Serial Uart
#include "timer0.h"

#define Aquire_Freq 100
//Subenderecos usados na rede
#define HOST_SUB_ADDR 0xFF

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
#define CMD_DISCONNECT 0x06 //Some sensor has gone disconected
#define CMD_GET_SENSOR_FIFO 0x07
#define CMD_SET_PACKET_TYPE 0x08
#define CMD_READ 0x09 //Request a packet of readings
#

uint8_t body_sensors[16] = {
  0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
  0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F};
uint16_t active_sensors = 0x00;//0000 0000 0000 0000

//////////////////////
//Functions in Host //
//////////////////////

/**
 * Envia um dado comando cada sensor listado como ativo.
 * Utilize request_ack_from_sensors para listar sensores.
 * @param cmd2send comando que se deseja enviar
 */
void send_cmd_to_active_sensors(uint8_t cmd2send);

/**
 * Requisita que todos os sensores possiveis retornem ack,
 * com isso é possivel listar os sensores ativos (active_sensors)
 */
void request_ack_from_sensors();

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
  setup_T0_freq(Aquire_Freq,1);//Time em 100.00250006250157Hz
  hal_uart_init(UART_BAUD_115K2);
  rf_init(ADDR_HOST,ADDR_HOST,10,RF_DATA_RATE_2Mbps,RF_TX_POWER_0dBm);
}

void main(){
  setup();
  while(1){ //Loop
    /////////////////////
    //Comunicao Serial //
    /////////////////////
    if(hal_uart_chars_available()){
      if(hal_uart_getchar() == UART_START){ //first byte should be start
        switch (hal_uart_getchar()) { //the actual command
          case CMD_START:
          hal_uart_putchar(CMD_OK);
          send_cmd_to_active_sensors(CMD_START);//Reset FIFO inside sensors
          delay_ms(5); //Wait 5 miliseconds
          start_T0();//Start Timer Aquisition
          break;
          case CMD_STOP:
          stop_T0();//Stop Timer
          send_cmd_to_active_sensors(CMD_STOP);//Send Stop to sensors
          //TODO: reaction to stop signal in sensors
          hal_uart_putchar(CMD_OK);//Return ok
          break;
          case CMD_CONNECTION:
          request_ack_from_sensors();
          break;
          case CMD_CALIBRATE:
          send_rf_command(CMD_CALIBRATE,active_sensors);
          //TODO: send flag that all is done by sensors
          break;
          default:
          //i don't know what to do here
          break;
        } /*END SWITCH*/
      } /*END IF START COMMAND*/
    } /*END IF UART AVAILABLE*/
    ///////////////////
    //Comunicacao RF //
    ///////////////////
    if(newPayload){
      //bridge, only redirect the packet to serial
      send_packet_to_computer(rx_buf[0], rx_buf, payloadWidth-1);
      switch (rx_buf[1]) {
        case CMD_CONNECTION://sensor alive and responding
          active_sensors |= 1<<rx_buf[0];
          break;
        case CMD_DISCONNECT: //Some sensor has some problem
          active_sensors &= (~(1<<rx_buf[0]));
          break;
      }
      sta = 0;
      newPayload = 0;
    }
    //////////
    //TIMER //
    //////////
    if(timer_elapsed){
      burst_read();
      timer_elapsed = 0;
    }
  } /*END INFINITE LOOP*/
} /*END MAIN FUNCTION*/


void burst_rf_read(){
  uint8_t i = 0;
  for (i = 0; i < 16; i++) { //para cada sensor possivel
    if(active_sensors & (1<<i)){//se esta ativo
      send_rf_command(CMD_READ,body_sensors[i]);//requisita o pacote de dados
    }
  }
}

void request_ack_from_sensors(){
  uint8_t i = 0;
  for (i = 0; i < 16; i++) {
    send_rf_command(CMD_CONNECTION,body_sensors[i]);
  }
}

void send_cmd_to_active_sensors(uint8_t cmd2send){
  uint8_t i;
  for (i = 0; i < 16; i++) { //para cada sensor possivel
    if(active_sensors & (1<<i)){//se esta ativo
      send_rf_command(cmd2send,body_sensors[i]);//inicia os sensores
    }
  }
}
