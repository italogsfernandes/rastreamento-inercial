/* ------------------------------------------------------------------------------
 * FEDERAL UNIVERSITY OF UBERLANDIA
 * Faculty of Electrical Engineering
 * Biomedical Engineering Lab
 * Uberlândia, Brazil
 * ------------------------------------------------------------------------------
 * Author: Italo G S Fernandes; Andrei Nakagawa, MSc
 * contact: italogsfernandes@gmail.com; nakagawa.andrei@gmail.com
 * URLs: www.biolab.eletrica.ufu.br
 *       https://github.com/BIOLAB-UFU-BRAZIL
 *       https://github.com/italogfernandes
 * ------------------------------------------------------------------------------
 * Description:
 * ------------------------------------------------------------------------------
 * Acknowledgements
 *  - We would like to thank the open-source community that provided many of the
 *  source codes necessary for creating this firmware.
 *  - Jeff Rowberg as the main developer of the I2Cdevlib
 *  - Luis Ródenas: Our calibration routine is totally based on his code
 * ------------------------------------------------------------------------------
 */
 #include <nrf24le01Module.h>

 nrf24le01Module tx_nrf(2,3,4);

 void setup(){
   Serial.begin(9600);
   tx_nrf.rf_init(tx_nrf.ADDR_HOST,tx_nrf.ADDR_HOST,10,RF_DATA_RATE_2Mbps,RF_TX_POWER_0dBm);
   Serial.print("Transmitter with polling iniciado...\n");
 }

 void loop() {
   tx_nrf.tx_buf[0] = 0x42;
   tx_nrf.TX_Mode_NOACK_Polling(1);
   if(tx_nrf.newPayload){
     Serial.print(tx_nrf.rx_buf[0],HEX);
     if(tx_nrf.rx_buf[0] == 0x00){
       Serial.print(" - turn on signal received by sensor.\n");
     } else {
       Serial.print(" - Nao reconhecido.\n");
     }
     tx_nrf.sta = 0;
     tx_nrf.newPayload = 0;
   }
   delay(1000);

   tx_nrf.tx_buf[0] = 0x53;
   tx_nrf.TX_Mode_NOACK_Polling(1);
   if(tx_nrf.newPayload){
     Serial.print(tx_nrf.rx_buf[0],HEX);
     if(tx_nrf.rx_buf[0] == 0x01){
       Serial.print(" - turn off signal received by sensor.\n");
     } else {
       Serial.print(" - Nao reconhecido.\n");
     }
     tx_nrf.sta = 0;
     tx_nrf.newPayload = 0;
   }
   delay(1000);
 }

/*
#include "DueTimer.h"
#include "inertial_packets.h"
#include "nrf24le01Commands.h"

#define Aquire_Freq 100
#define MAX_WAIT_TIME 10 // milliseconds
#define LED_STATUS 5
#define UART_BAUDRATE 115200

//Subenderecos dos sensores possiveis de existir na rede
uint8_t body_sensors[16] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
   0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
//Cada bit representa se um sensor esta ativo na rede ou não
uint16_t active_sensors = 0x00;//0000 0000 0000 0000

//Dados recebidos via RF:

unsigned long timeout_init_time;

void setup() {
  pinMode(LED_STATUS, OUTPUT);
  //Communication with pc
  Serial.begin(UART_BAUDRATE);
  //Communication with rf
  tx_nrf.rf_init(tx_nrf.ADDR_HOST,tx_nrf.ADDR_HOST,10,RF_DATA_RATE_2Mbps,RF_TX_POWER_0dBm);
  Serial.println("Arduino HOST Iniciado...");
  //Piscadelas:
  digitalWrite(LED_STATUS, HIGH); delay(500);
  digitalWrite(LED_STATUS, LOW); delay(500);
  digitalWrite(LED_STATUS, HIGH); delay(500);
  digitalWrite(LED_STATUS, LOW); delay(500);
}

void loop() {
  if(Serial.available()>=2){
    uart_communication_handler();
  }
  if(newPayload){
    rf_communication_handler();
  }
}

void takeReading(){
  send_cmd_to_active_sensors(CMD_READ);
}

void uart_communication_handler(){
  if(Serial.read() == UART_START_SIGNAL){ //first byte should be start
    switch (Serial.read()) { //the actual command
      case CMD_START:
      Serial.write(CMD_OK);
      send_cmd_to_all_addrs(CMD_START);//Reset FIFO inside sensors
      delay(10); //Wait for at least 5 miliseconds
      timer_id = aquire_timer.every(1000/Aquire_Freq, takeReading);//Start Timer Aquisition
      digitalWrite(LED_STATUS,HIGH);
      break;
      case CMD_STOP:
      aquire_timer.stop(timer_id);//Stop Timer
      send_cmd_to_all_addrs(CMD_STOP);//Send Stop to sensors
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
      while(Serial.available());//NOTE: infinit loop ATTENTION
      send_cmd_to_active_sensors_with_arg(CMD_SET_PACKET_TYPE,Serial.read());
      break;
      case CMD_GET_ACTIVE_SENSORS:
      Serial.write(active_sensors);
      break;
      case CMD_SET_ACTIVE_SENSORS:
      while(Serial.available());//NOTE: infinit loop ATTENTION
      active_sensors = Serial.read();
      Serial.write(CMD_OK);
      break;
      case CMD_TEST_RF_CONNECTION:
      send_cmd_to_all_addrs(CMD_TEST_RF_CONNECTION);
      break;
      case CMD_LIGHT_UP_LED:
      send_cmd_to_all_addrs(CMD_LIGHT_UP_LED);
      digitalWrite(LED_STATUS, HIGH);
      break;
      case CMD_TURN_OFF_LED:
      send_cmd_to_all_addrs(CMD_TURN_OFF_LED);
      digitalWrite(LED_STATUS, LOW);
      break;
      case CMD_READ:
      send_cmd_to_active_sensors(CMD_READ);
      break;
    } /*END SWITCH*//*
  } /*END IF START COMMAND*//*
}

uint8_t rf_communication_handler(){
  //Redirects the packet
  Serial.write(UART_START_SIGNAL);//Assembling packet and sending
  Serial.write(payloadWidth);
  Serial.write(rx_buf,payloadWidth);
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

//NOTE: DONE! Next job: implement as a library

void wait_Serial_bytes(uint8_t how_many){
  timeout_init_time = millis();
  while(Serial.available()<how_many && millis() - timeout_init_time < MAX_WAIT_TIME*how_many);
}

void wait_rf_response(){
  timeout_init_time = millis();
  while (1) {
    if(newPayload){ //Se chegou algo no host.
        rf_communication_handler();
        break;
    }
    if(millis() - timeout_init_time > MAX_WAIT_TIME){
      break;
    }
  }
}

void send_rf_data(uint8_t *data2send, uint8_t data_len){
  for(i = 0; i < data_len; i++){
    tx_buf[i] = data2send[i];
  }
  TX_Mode_NOACK(data_len);
  RX_Mode();
}

void send_rf_command(uint8_t cmd2send, uint8_t sensor_id){
  tx_buf[0] = sensor_id;
  tx_buf[1] = cmd2send;
  TX_Mode_NOACK(2);
  RX_Mode();
}

void send_rf_command_with_arg(uint8_t cmd2send,uint8_t arg2send, uint8_t sensor_id){
  tx_buf[0] = sensor_id;
  tx_buf[1] = cmd2send;
  tx_buf[2] = arg2send;
  TX_Mode_NOACK(2);
  RX_Mode();
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
      wait_rf_response();
    }
  }
}

void send_cmd_to_active_sensors_with_arg(uint8_t cmd2send,uint8_t agr2send){
  uint8_t i;
  for (i = 0; i < 16; i++) { //para cada sensor possivel
    if(active_sensors & (1<<i)){//se esta ativo
      send_rf_command_with_arg(cmd2send,agr2send,body_sensors[i]);
      wait_rf_response();
    }
  }
}
*/
