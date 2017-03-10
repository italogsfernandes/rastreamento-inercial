#include "Nordic\reg24le1.h" //definicoes basicas de pinos
#include "nRF-SPIComands.h" //Comunicacao RF
#include "hal_uart.h" //Comunicacao Serial Uart
#include "hal_delay.h"
#include "timer0.h"
#include "pacotes_inerciais.h"

#define MY_SUB_ADDR HOST_SUB_ADDR //Id do sensor
#define Aquire_Freq 100

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
  //void request_ack_from_sensors();

  /**
  * Envia para os sensores ativos o comando para setar o tipo de pacote
  * @param pkt_type tipo de pacote de acordo com library pacotes inerciais
  */
  //void set_packet_type(uint8_t pkt_type);

  void send_cmd_to_all_addrs(uint8_t cmd2send);
  //TODO: organizar
  void send_cmd_to_all_addrs_with_arg(uint8_t cmd2send,uint8_t agr2send);

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
    hal_uart_init(UART_BAUD_9K6);
    rf_init(ADDR_HOST,ADDR_HOST,10,RF_DATA_RATE_2Mbps,RF_TX_POWER_0dBm);
    P06 = 1;delay_ms(500);	P06 = 0;delay_ms(500);
    P06 = 1;delay_ms(500);	P06 = 0;delay_ms(500);
    hal_uart_putstring("Dispositivo Host Iniciado\n");
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

      //////////
      //TIMER //
      //////////
      if(timer_elapsed){
        send_rf_command(CMD_READ,BROADCAST_ADDR);
        timer_elapsed = 0;
        P06 = !P06;
      }
    } /*END INFINITE LOOP*/
  } /*END MAIN FUNCTION*/

  /////////////////////
  //FUNCIONS in Host //
  /////////////////////

  void rf_communication_handler(){
    //bridge, only redirect the packet to serial
    redirect_rf_pkt_to_serial(rx_buf, payloadWidth);
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

  //TODO: header da funcao
  //TODO: make the sensors responsives to all types of commands
  //TODO: remove BROADCAST_ADDR e enviar um a um
  void wait_rf_response(uint8_t sensor){
    while (1) {
      if(newPayload){ //Se algum sensor respondeu,(nao especifica qual mas algum)
        rf_communication_handler();
        break;
      }
      //TODO: melhorar o timeout
      /*
      if(timeout_cnt++ > 0xFFFF){//se timeout maior doq um numero grande ae -q
      break;
    }*/
  }
}

//TODO: organizar
void send_cmd_to_all_addrs(uint8_t cmd2send){
  uint8_t i;
  for (i = 0; i < 16; i++) { //para cada sensor possivel
    send_rf_command(cmd2send,body_sensors[i]);
  }
}
//TODO: organizar
void send_cmd_to_all_addrs_with_arg(uint8_t cmd2send,uint8_t agr2send){
  uint8_t i;
  for (i = 0; i < 16; i++) { //para cada sensor possivel
    send_rf_command_with_arg(cmd2send,agr2send,body_sensors[i]);
  }
}

//NOTE: not used yet
//NOTE: revisar polling
void send_cmd_to_active_sensors(uint8_t cmd2send){
  uint8_t i;
  for (i = 0; i < 16; i++) { //para cada sensor possivel
    if(active_sensors & (1<<i)){//se esta ativo
      send_rf_command(cmd2send,body_sensors[i]);
      wait_rf_response(body_sensors[i]);
    }
  }
}

void send_cmd_to_active_sensors_with_arg(uint8_t cmd2send,uint8_t agr2send){
  uint8_t i;
  for (i = 0; i < 16; i++) { //para cada sensor possivel
    if(active_sensors & (1<<i)){//se esta ativo
      send_rf_command_with_arg(cmd2send,agr2send,body_sensors[i]);
      wait_rf_response(body_sensors[i]);
    }
  }
}
//TODO: cabeçalho
void uart_communication_handler(){
  if(hal_uart_getchar() == UART_START_SIGNAL){ //first byte should be start
    switch (hal_uart_getchar()) { //the actual command
      case CMD_START:
      hal_uart_putchar(CMD_OK);
      send_cmd_to_active_sensors(CMD_START);//Reset FIFO inside sensors
      delay_ms(10); //Wait for at least 5 miliseconds
      start_T0();//Start Timer Aquisition
      P06 = 1;
      break;
      case CMD_STOP:
      stop_T0();//Stop Timer
      send_cmd_to_active_sensors(CMD_STOP);//Send Stop to sensors
      hal_uart_putchar(CMD_OK);//Return ok
      P06 = 0;
      break;
      case CMD_CONNECTION:
      send_cmd_to_all_addrs(CMD_CONNECTION);
      break;
      case CMD_CALIBRATE:
      send_cmd_to_active_sensors(CMD_CALIBRATE);
      break;
      case CMD_SET_PACKET_TYPE:
      send_cmd_to_active_sensors_with_arg(CMD_SET_PACKET_TYPE,hal_uart_getchar(),BROADCAST_ADDR);
      break;
      case CMD_GET_ACTIVE_SENSORS:
      hal_uart_putchar(active_sensors);
      break;
      case CMD_SET_ACTIVE_SENSORS:
      active_sensors = hal_uart_getchar();
      hal_uart_putchar(CMD_OK);
      break;
      case CMD_TEST_RF_CONNECTION:
      send_cmd_to_all_addrs(CMD_TEST_RF_CONNECTION);
      break;
      case CMD_LIGHT_UP_LED:
      send_cmd_to_all_addrs(CMD_LIGHT_UP_LED);
      P06 = 1;
      break;
      case CMD_TURN_OFF_LED:
      send_cmd_to_all_addrs(CMD_TURN_OFF_LED);
      P06 = 0;
      break;
      default:
      //Inverte o led indicando que recebeu um comando desconhecido ou nao implementado
      P06 = !P06;
      break;
    } /*END SWITCH*/
  } /*END IF START COMMAND*/
}
