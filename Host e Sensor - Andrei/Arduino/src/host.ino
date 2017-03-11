#include "Timer.h"
#include "inertial_packets.h"
#include "SoftwareSerial.h"

#define Aquire_Freq 100
#define MAX_WAIT_TIME 10 // milliseconds
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

//Dados recebidos via RF:
uint8_t rf_rx_packet_len = 0;
uint8_t rf_data[32] = {0};

unsigned long timeout_init_time;

void setup() {
  pinMode(LED_STATUS, OUTPUT);
  Serial.begin(UART_BAUDRATE); //Communication with pc
  rfSerial.begin(UART_BAUDRATE); //Communication with rf
  Serial.println("Arduino Iniciado...");
  //Piscadelas:
  digitalWrite(LED_STATUS, HIGH); delay(500);
  digitalWrite(LED_STATUS, LOW); delay(500);
  digitalWrite(LED_STATUS, HIGH); delay(500);
  digitalWrite(LED_STATUS, LOW); delay(500);
}

void loop() {
  if(Serial.available()>2){
    uart_communication_handler();
  }
  if(rfSerial.available()){
    rf_communication_handler();
  }
  aquire_timer.update();
}

void takeReading(){
  send_cmd_to_active_sensors(CMD_READ);
}

//NOTE: DONE! Next job: implement as a library

void uart_communication_handler(){
  uint8_t timer_id = 0;
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
    } /*END SWITCH*/
  } /*END IF START COMMAND*/
}

void wait_serial_bytes(uint8_t how_many){
  timeout_init_time = millis();
  while(Serial.available()<how_many && millis() - timeout_init_time < MAX_WAIT_TIME*how_many);
}

void wait_rfserial_bytes(uint8_t how_many){
  timeout_init_time = millis();
  while(rfSerial.available()<how_many && millis() - timeout_init_time < MAX_WAIT_TIME*how_many);
}

uint8_t rf_communication_handler(){
  //saves and redirects the packet
  uint8_t i = 0;
  uint8_t checksum = 0;
  if(rfSerial.read() == UART_START_SIGNAL){
    wait_rfserial_bytes(1);
    rf_rx_packet_len = rfSerial.read();
    wait_rfserial_bytes(rf_rx_packet_len);
    for(i = 0; i<rf_rx_packet_len; i++){
      rf_data[i] = rfSerial.read();
      checksum += rf_data[i];
    }
    checksum = (~checksum) + 1;
    wait_rfserial_bytes(1);
    if(rfSerial.read() == checksum){
      Serial.write(UART_START_SIGNAL);//Assembling packet and sending
      Serial.write(rf_rx_packet_len);
      Serial.write(rf_data,rf_rx_packet_len);
      Serial.write(checksum);
      switch (rf_data[1]) {
        case CMD_CONNECTION://sensor alive and responding
        active_sensors |= 1<<rf_data[0];
        break;
        case CMD_DISCONNECT: //Some sensor has some problem
        active_sensors &= (~(1<<rf_data[0]));
        break;
      }
      return 0;
    } else {
      return 1;
    }
  }
  return 2;
}

void wait_rf_response(){
  timeout_init_time = millis();
  while (1) {
    if(rfSerial.available()){ //Se chegou algo no host.
        rf_communication_handler();
        break;
    }
    if(millis() - timeout_init_time > MAX_WAIT_TIME){
      break;
    }
  }
}

void send_rf_data(uint8_t *data2send, uint8_t data_len){
  uint8_t checksum = 0;
  uint8_t i = 0;
  rfSerial.write(UART_START_SIGNAL);
  rfSerial.write(data_len);
  for(i = 0; i < data_len; i++){
    rfSerial.write(data2send[i]);
    checksum += data2send[i];
  }
  checksum = (~checksum) + 1;
  rfSerial.write(checksum);
}

void send_rf_command(uint8_t cmd2send, uint8_t sensor_id){
  uint8_t checksum = 0;
  rfSerial.write(UART_START_SIGNAL);
  rfSerial.write(2);
  rfSerial.write(sensor_id);
  rfSerial.write(cmd2send);
  checksum = sensor_id + cmd2send;
  checksum = (~checksum) + 1;
  rfSerial.write(checksum);
}

void send_rf_command_with_arg(uint8_t cmd2send,uint8_t arg2send, uint8_t sensor_id){
  uint8_t checksum = 0;
  rfSerial.write(UART_START_SIGNAL);
  rfSerial.write(2);
  rfSerial.write(sensor_id);
  rfSerial.write(cmd2send);
  rfSerial.write(arg2send);
  checksum = sensor_id + cmd2send + arg2send;
  checksum = (~checksum) + 1;
  rfSerial.write(checksum);
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

//TODO:
