#include "Timer.h"


#define Aquire_Freq 100
#define LED_STATUS 5
#define UART_BAUDRATE 9600

#define rf_RX 10 //where the TX cable must be connected TX -> RX
#define rf_TX 11 //where the RX cable must be connected RX -> TX


//Subenderecos dos sensores possiveis de existir na rede
uint8_t body_sensors[16] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
//Cada bit representa se um sensor esta ativo na rede ou não
uint16_t active_sensors = 0x00;//0000 0000 0000 0000

SoftwareSerial rfSerial(rf_RX, rf_TX); // RX, TX

Timer aquire_timer;

void setup() {
  pinMode(LED_STATUS, OUTPUT);
  Serial.begin(UART_BAUDRATE); //Communication with pc
  rfSerial.begin(UART_BAUDRATE); //Communication with rf
  Serial.println("Arduino Iniciado...");
  //Piscadelas:
  digitalWrite(LED_STATUS, HIGH); delay(250);
  digitalWrite(LED_STATUS, LOW); delay(250);
  digitalWrite(LED_STATUS, HIGH); delay(250);
  digitalWrite(LED_STATUS, LOW); delay(250);
  //aquire_timer.every(1000/Aquire_Freq, takeReading);
}

void loop() {
  if(Serial.available()){
    uart_communication_handler();
  }
  if(rfSerial.available()){
    rf_communication_handler();
  }
  aquire_timer.update();
}

void takeReading(){
  send_rf_command(CMD_READ,BROADCAST_ADDR);
  timer_elapsed = 0;
  P06 = !P06;
}

void uart_communication_handler(){
  if(Serial.read() == UART_START_SIGNAL){ //first byte should be start
    switch (Serial.read()) { //the actual command
      case CMD_START:
      Serial.write(CMD_OK);
      //TODO:
      //send_cmd_to_all_addrs(CMD_START);//Reset FIFO inside sensors
      delay(10); //Wait for at least 5 miliseconds
      //start_T0();//Start Timer Aquisition
      digitalWrite(LED_STATUS,HIGH);
      break;
      case CMD_STOP:
      //TODO:
      //stop_T0();//Stop Timer
      //send_cmd_to_all_addrs(CMD_STOP);//Send Stop to sensors
      Serial.write(CMD_OK);//Return ok
      digitalWrite(LED_STATUS,LOW);
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
}

void wait_rf_response(uint8_t sensor){
  while (1) {
    if(newPayload){ //Se algum sensor respondeu,(nao especifica qual mas algum)
      if(rx_buf[0] == sensor){
        rf_communication_handler();
        sta = 0;
        newPayload = 0;
        break;
      } else { //se outro sensor responder ignorar
        sta = 0;
        newPayload = 0;
      }
    }
  }
}

void send_cmd_to_all_addrs(uint8_t cmd2send){
  uint8_t i;
  for (i = 0; i < 16; i++) { //para cada sensor possivel
    send_rf_command(cmd2send,body_sensors[i]);
  }
}

void send_cmd_to_all_addrs_with_arg(uint8_t cmd2send,uint8_t agr2send){
  uint8_t i;
  for (i = 0; i < 16; i++) { //para cada sensor possivel
    send_rf_command_with_arg(cmd2send,agr2send,body_sensors[i]);
  }
}

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
