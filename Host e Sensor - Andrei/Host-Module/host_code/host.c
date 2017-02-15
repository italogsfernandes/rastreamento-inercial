#include "Nordic\reg24le1.h" //definicoes basicas de pinos
#include "nRF-SPIComands.h" //Comunicacao RF
#include "hal_uart.h" //Comunicacao Serial Uart
#include "timer0.h"

#define Aquire_Freq 100
//Subenderecos usados na rede
#define HOST_SUB_ADDR 0xFF

//Subenderecos dos sensores possiveis de existir na rede
uint8_t body_sensors[16] = {
  0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
  0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F};
//Cada bit representa se um sensor esta ativo na rede ou não
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
 * Envia para os sensores ativos o comando para setar o tipo de pacote
 * @param pkt_type tipo de pacote de acordo com library pacotes inerciais
 */
void set_packet_type(uint8_t pkt_type);

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
          delay_ms(10); //Wait 5 miliseconds
          start_T0();//Start Timer Aquisition
          break;
          case CMD_STOP:
          stop_T0();//Stop Timer
          send_cmd_to_active_sensors(CMD_STOP);//Send Stop to sensors
          hal_uart_putchar(CMD_OK);//Return ok
          break;
          case CMD_CONNECTION:
          request_ack_from_sensors();
          break;
          case CMD_CALIBRATE:
          send_cmd_to_active_sensors(CMD_CALIBRATE);
          //TODO: send flag that all is done by sensors
          break;
          case CMD_SET_PACKET_TYPE:
          set_packet_type(hal_uart_getchar());
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
      send_cmd_to_active_sensors(CMD_READ);
      timer_elapsed = 0;
    }
  } /*END INFINITE LOOP*/
} /*END MAIN FUNCTION*/

/////////////////////
//FUNCIONS in Host //
/////////////////////

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

void set_packet_type(uint8_t pkt_type){
  uint8_t i;
  for (i = 0; i < 16; i++) { //para cada sensor possivel
    if(active_sensors & (1<<i)){//se esta ativo
      tx_buf[0] = body_sensors[i];
      tx_buf[1] = CMD_SET_PACKET_TYPE;
      tx_buf[2] = pkt_type;
      TX_Mode_NOACK(3);
      RX_Mode();
    }
  }
}
