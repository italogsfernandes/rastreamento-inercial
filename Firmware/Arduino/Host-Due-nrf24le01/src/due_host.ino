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
#include "DueTimer.h"

//Subenderecos usados no sistema
#define HOST_SUB_ADDR 0xFF //Sub addr do host
//UART Packet: Start Signal - Command
#define UART_START_SIGNAL  0x53
/////////////
//Comandos //
/////////////
#define CMD_OK  (uint8_t) 0x00 //Ack - Uart Command
#define CMD_ERROR (uint8_t) 0x01 //Error flag - Uart Command
#define CMD_START (uint8_t) 0x02 //Start Measuring - Uart Command
#define CMD_STOP  (uint8_t) 0x03 //Stop Measuring - Uart Command
#define CMD_CONNECTION  (uint8_t) 0x04 //Teste Connection - Uart Command
#define CMD_CALIBRATE (uint8_t) 0x05 //Calibrate Sensors Command
#define CMD_DISCONNECT (uint8_t) 0x06 //Some sensor has gone disconected
#define CMD_READ (uint8_t) 0x07 //Request a packet of readings
#define CMD_SET_PACKET_TYPE (uint8_t) 0x08
#define CMD_GET_ACTIVE_SENSORS (uint8_t) 0x09 //Retorna a variavel do host active sensors
#define CMD_SET_ACTIVE_SENSORS (uint8_t) 0x0A //Altera a variavel do host active sensors
#define CMD_TEST_RF_CONNECTION (uint8_t) 0x0B
#define CMD_LIGHT_UP_LED (uint8_t) 0x0C
#define CMD_TURN_OFF_LED (uint8_t) 0x0D

////////////////////////
//Pacotes de leituras //
////////////////////////
#define PACKET_TYPE_ACEL      0x01
#define PACKET_TYPE_GIRO      0x02
#define PACKET_TYPE_MAG       0x03
#define PACKET_TYPE_M6        0x04
#define PACKET_TYPE_M9        0x05
#define PACKET_TYPE_QUAT      0x06
#define PACKET_TYPE_FIFO_NO_MAG       0x07
#define PACKET_TYPE_FIFO_ALL_READINGS     0x08

/////////////
// Defines //
/////////////

#define POLLING_TIMEOUT 25000 //25 milliseconds of maximum wait time. tempo medio: 385
#define UART_TIMEOUT 2 //2 milliseconds of maximum wait time por byte.
#define Aquire_Freq 100
#define LED_STATUS 10
#define UART_BAUDRATE 115200

//Subenderecos dos sensores possiveis de existir na rede
uint8_t body_sensors[16] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
//Cada bit representa se um sensor esta ativo na rede ou não
uint16_t active_sensors = 0x00;//0000 0000 0000 0000
//Polling and other timeouts variables
unsigned long timeout_init_time, timeout_actual_time;

//Variables for storing raw data from accelerometers gyroscope and magnetometer
int16_t ax, ay, az; //Accel
int16_t gx, gy, gz; //Gyro
int16_t mx, my, mz; //Mag
//Calibration
uint8_t calibIt = 0; //Counter for the number of iterations in calibration
uint8_t calibCounter = 0; //Counter for the number of samples read during calibration
uint8_t calibStep = 1; //Determines which step of the calibration should be performed
uint8_t accelTol = 0; //Error tolerance for accelerometer readings
uint8_t gyroTol = 0; //Error tolerance for gyroscope readings
uint8_t timeTol = 10; //Maximum duration of the calibration process (seconds)
int16_t accelBuffer[3] = {0,0,0}; //Buffer to store the mean values from accel
int16_t gyroBuffer[3] = {0,0,0};  //Buffer to store the mean values from gyro


nrf24le01Module host_nrf(2,3,4);

void setup(){
  pinMode(LED_STATUS, OUTPUT);
  Serial.begin(UART_BAUDRATE); //Communication with Sofware
  host_nrf.rf_init(host_nrf.ADDR_HOST,host_nrf.ADDR_HOST,10,RF_DATA_RATE_2Mbps,RF_TX_POWER_0dBm); //RF Communication
  Serial.print("Arduino HOST Iniciado...\n");
  //Piscadas
  digitalWrite(LED_STATUS, HIGH); delay(500);  digitalWrite(LED_STATUS, LOW); delay(500);
  digitalWrite(LED_STATUS, HIGH); delay(500);  digitalWrite(LED_STATUS, LOW); delay(500);
}


void loop() {
  if(Serial.available()>=2){
    uart_communication_handler();
  }
}

void takeReading(){
  send_cmd_to_active_sensors(CMD_READ);
}

////////////////
//Serial UART //
////////////////

void wait_Serial_bytes(uint8_t how_many){
  timeout_init_time = millis() + UART_TIMEOUT*how_many;
  while(Serial.available()<how_many && millis() < timeout_init_time);
}

void uart_communication_handler(){
  if(Serial.read() == UART_START_SIGNAL){ //first byte should be start
    switch (Serial.read()) { //the actual command
      case CMD_START:
      Serial.write(CMD_OK);
      send_cmd_to_all_addrs(CMD_START);//Reset FIFO inside sensors
      delay(10); //Wait for at least 5 miliseconds
      Timer3.attachInterrupt(takeReading).start(1000000/Aquire_Freq);
      digitalWrite(LED_STATUS,HIGH);
      break;
      case CMD_STOP:
      Timer3.stop();
      send_cmd_to_all_addrs(CMD_STOP);//Send Stop to sensors
      Serial.write(CMD_OK);//Return ok
      digitalWrite(LED_STATUS,LOW);
      break;
      case CMD_CONNECTION:
      test_connection();
      break;
      case CMD_CALIBRATE:
      wait_Serial_bytes(1);
      digitalWrite(LED_STATUS,HIGH);
      calibrate(Serial.read());
      //send_cmd_to_active_sensors(CMD_CALIBRATE);
      break;
      case CMD_SET_PACKET_TYPE:
      wait_Serial_bytes(1);
      send_cmd_to_active_sensors_with_arg(CMD_SET_PACKET_TYPE,Serial.read());
      break;
      case CMD_GET_ACTIVE_SENSORS:
      Serial.write(active_sensors);
      break;
      case CMD_SET_ACTIVE_SENSORS:
      wait_Serial_bytes(1);
      active_sensors = Serial.read();
      Serial.write(CMD_OK);
      break;
      case CMD_TEST_RF_CONNECTION:
      wait_Serial_bytes(1);
      send_rf_command(CMD_TEST_RF_CONNECTION,Serial.read()); //Requisita conexao
      wait_rf_response();
      break;
      case CMD_LIGHT_UP_LED:
      wait_Serial_bytes(1);
      send_rf_command(CMD_LIGHT_UP_LED,Serial.read()); //Requisita conexao
      wait_rf_response();
      digitalWrite(LED_STATUS, HIGH);
      break;
      case CMD_TURN_OFF_LED:
      wait_Serial_bytes(1);
      send_rf_command(CMD_TURN_OFF_LED,Serial.read()); //Requisita conexao
      wait_rf_response();
      digitalWrite(LED_STATUS, LOW);
      break;
      case CMD_READ:
      send_cmd_to_active_sensors(CMD_READ);
      break;
    } /*END SWITCH*/
  } /*END IF START COMMAND*/
}


////////////////
//Calibration //
////////////////


void calibrate(uint8_t sensor){
    //Configures the accel and gyro offsets to zero
    send_offset_values(sensor,0,0,0,0,0,0);
    //Configurando sensor para ler accel e gyro
    send_rf_command_with_arg(CMD_SET_PACKET_TYPE,PACKET_TYPE_M6, sensor);
    //Variables that needs to be cleared for calibration
    calibIt = 0;
    calibCounter = 0;
    calibStep = 1;
    //Small delay
    delay(50);
    //Triggers the calibration method
    //Timer3.attachInterrupt(timerCalibration).start(sampPeriod);
    digitalWrite(LED_STATUS,HIGH);
}


//////////////////////
//RF High Functions //
//////////////////////

void test_connection(){
    uint8_t i;
    for (i = 0; i < 16; i++) { //para cada sensor possivel
        Serial.print("Connecting to sensor ");
        Serial.print(body_sensors[i],HEX);
        Serial.print("... ");
        send_rf_command(CMD_CONNECTION,body_sensors[i]); //Requisita conexao
        wait_rf_response();
    }
    Serial.print("Sensores conectados: ");
    Serial.print(active_sensors,BIN);
    Serial.print(".\n");
}

void wait_rf_response(){
  timeout_init_time = micros() + POLLING_TIMEOUT;
  while(1){
    timeout_actual_time = micros();
    if(timeout_actual_time>timeout_init_time){
      Serial.print("POLLING_TIMEOUT reached!\n");
      break;
    }
    if(!digitalRead(host_nrf.RFIRQ)){
      host_nrf.RF_IRQ();
      if(host_nrf.newPayload){
         Serial.print("Sensor response with: ");
         Serial.print(POLLING_TIMEOUT - timeout_init_time + timeout_actual_time);
         Serial.print(" microseconds.\n");
        rf_communication_handler();
      }
      break;
    }
  }
}

void rf_communication_handler(){
  //Redirects the packet
  Serial.write(UART_START_SIGNAL);//Assembling packet and sending
  Serial.write(host_nrf.payloadWidth);
  Serial.write(host_nrf.rx_buf,host_nrf.payloadWidth);
  switch (host_nrf.rx_buf[1]) {
    case CMD_CONNECTION://sensor alive and responding
    active_sensors |= 1<<host_nrf.rx_buf[0];
    break;
    case CMD_DISCONNECT: //Some sensor has some problem
    active_sensors &= (~(1<<host_nrf.rx_buf[0]));
    break;
  }
  host_nrf.sta = 0;
  host_nrf.newPayload = 0;
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
        Serial.print("Sending cmd to sensor ");
        Serial.print(body_sensors[i],HEX);
        Serial.print("... ");
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

/////////////////////
//RF Low Functions //
/////////////////////

void send_rf_data(uint8_t *data2send, uint8_t data_len){
  uint8_t i;
  for(i = 0; i < data_len; i++){
    host_nrf.tx_buf[i] = data2send[i];
  }
  host_nrf.TX_Mode_NOACK(data_len);
}

void send_rf_command(uint8_t cmd2send, uint8_t sensor_id){
  host_nrf.tx_buf[0] = sensor_id;
  host_nrf.tx_buf[1] = cmd2send;
  host_nrf.TX_Mode_NOACK(2);
}

void send_rf_command_with_arg(uint8_t cmd2send,uint8_t arg2send, uint8_t sensor_id){
  host_nrf.tx_buf[0] = sensor_id;
  host_nrf.tx_buf[1] = cmd2send;
  host_nrf.tx_buf[2] = arg2send;
  host_nrf.TX_Mode_NOACK(2);
}

void send_offset_values(uint8_t sensor_id,int16_t X_ac,int16_t Y_ac,int16_t Z_ac,int16_t X_gy,int16_t Y_gy,int16_t Z_gy){
    host_nrf.tx_buf[0] = sensor_id;
    host_nrf.tx_buf[1] = CMD_CALIBRATE;
    host_nrf.tx_buf[2] = (uint8_t) (X_ac >> 8);
    host_nrf.tx_buf[3] = (uint8_t) X_ac;
    host_nrf.tx_buf[4] = (uint8_t) (Y_ac >> 8);
    host_nrf.tx_buf[5] = (uint8_t) Y_ac;
    host_nrf.tx_buf[6] = (uint8_t) (Z_ac >> 8);
    host_nrf.tx_buf[7] = (uint8_t) Z_ac;
    host_nrf.tx_buf[8] = (uint8_t) (X_gy >> 8);
    host_nrf.tx_buf[9] = (uint8_t) X_gy;
    host_nrf.tx_buf[10] = (uint8_t) (Y_gy >> 8);
    host_nrf.tx_buf[11] = (uint8_t) Y_gy;
    host_nrf.tx_buf[12] = (uint8_t) (Z_gy >> 8);
    host_nrf.tx_buf[13] = (uint8_t) Z_gy;
    host_nrf.TX_Mode_NOACK(14);
}
