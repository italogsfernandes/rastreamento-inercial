/* ------------------------------------------------------------------------------
  FEDERAL UNIVERSITY OF UBERLANDIA
  Faculty of Electrical Engineering
  Biomedical Engineering Lab
  Uberlândia, Brazil
  ------------------------------------------------------------------------------
  Author: Italo G S Fernandes;
  contact: italogsfernandes@gmail.com;
  URLs: www.biolab.eletrica.ufu.br
        https://github.com/BIOLAB-UFU-BRAZIL
        https://github.com/italogfernandes
  ------------------------------------------------------------------------------
*/
#include <nrf24le01Module.h>
#include <DueTimer.h>
/////////////
// Defines //
/////////////
#define freq_teste 5
#define LED_STATUS 13
#define UART_BAUDRATE 115200
#define CMD_READ 0x07
#define CMD_LIGHT_UP_LED 0x0C
#define CMD_TURN_OFF_LED 0x0D

//RF PACKET Index

#define RF_PF_NOMAG_I_ACCEL_X_H 1
#define RF_PF_NOMAG_I_ACCEL_X_L 2
#define RF_PF_NOMAG_I_ACCEL_Y_H 3
#define RF_PF_NOMAG_I_ACCEL_Y_L 4
#define RF_PF_NOMAG_I_ACCEL_Z_H 5
#define RF_PF_NOMAG_I_ACCEL_Z_L 6

#define RF_PF_NOMAG_I_GYRO_X_H  7
#define RF_PF_NOMAG_I_GYRO_X_L  8
#define RF_PF_NOMAG_I_GYRO_Y_H  9
#define RF_PF_NOMAG_I_GYRO_Y_L  10
#define RF_PF_NOMAG_I_GYRO_Z_H  11
#define RF_PF_NOMAG_I_GYRO_Z_L  12

#define RF_PF_NOMAG_I_QUAT_W_H  13
#define RF_PF_NOMAG_I_QUAT_W_L  14
#define RF_PF_NOMAG_I_QUAT_X_H  15
#define RF_PF_NOMAG_I_QUAT_X_L  16
#define RF_PF_NOMAG_I_QUAT_Y_H  17
#define RF_PF_NOMAG_I_QUAT_Y_L  18
#define RF_PF_NOMAG_I_QUAT_Z_H  19
#define RF_PF_NOMAG_I_QUAT_Z_L  20

unsigned long POLLING_TIMEOUT = 1000000 / freq_teste; // 10 milliseconds of maximum wait time. tempo medio: 385
const double sampPeriod = (1.0 / freq_teste) * 1000000;

//Polling and other timeouts variables
unsigned long timeout_init_time, timeout_actual_time;
//Timer and other timeouts variables
unsigned long timer_init_time, timer_actual_time;

String serialOp;

float q[4]; // [w, x, y, z] quaternion container
float a[3]; // [x, y, z] acceleration container
float g[3]; // [x, y, z] gyroscope container

bool timer_active = false;

nrf24le01Module host_nrf(2, 3, 4);//(uint8_t _RFIRQ_pin, uint8_t _RFCE_pin, uint8_t _RFCSN_pin)

void timerDataAcq();

void setup() {
  pinMode(LED_STATUS, OUTPUT);
  Serial.begin(UART_BAUDRATE); //Communication with Sofware
  host_nrf.rf_init(host_nrf.ADDR_HOST, host_nrf.ADDR_HOST, 10, RF_DATA_RATE_2Mbps, RF_TX_POWER_0dBm); //RF Communication

  Serial.print("Arduino HOST Iniciado...\n");
  Serial.println("Envie um comando: CMDSTART, CMDSTOP, CMDREAD, CMDLED, CMDTESTE...");
  //Piscadas
  digitalWrite(LED_STATUS, HIGH); delay(500);  digitalWrite(LED_STATUS, LOW); delay(500);
  digitalWrite(LED_STATUS, HIGH); delay(500);  digitalWrite(LED_STATUS, LOW); delay(500);

  //Timer3.attachInterrupt(takeReading).start(sampPeriod);
}


void loop() {

  if (Serial.available() > 0) {
    serialOp = Serial.readString();
    if (serialOp == "CMDSTART") {
      digitalWrite(LED_STATUS, HIGH);
      //Timer3.attachInterrupt(takeReading).start(sampPeriod);
      timer_active = true;
    }
    else if (serialOp == "CMDSTOP")
    {
      digitalWrite(LED_STATUS, LOW);
      //Timer3.stop();
      timer_active = false;
    }
    else if (serialOp == "CMDREAD")
    {
      takeReading();
    }
    else if (serialOp == "CMDLED")
    {
      send_rf_command_to(CMD_LIGHT_UP_LED, 1); delay(1000);
      send_rf_command_to(CMD_TURN_OFF_LED, 1); delay(1000);
    }
    else if (serialOp == "CMDTESTE")
    {
      send_rf_command_to(0xAA, 1);
    }
  }

  if(timer_active){
    takeReading();
  }
}

void takeReading() {
  send_rf_command_to(CMD_READ, 1);
}

//////////////////////
//RF High Functions //
//////////////////////

void wait_rf_response() {
  timeout_init_time = micros() + POLLING_TIMEOUT;
  while (1) {
    timeout_actual_time = micros();
    if (timeout_actual_time > timeout_init_time) {
      Serial.println("-");
      break;
    }
    if (!digitalRead(host_nrf.RFIRQ)) {
      host_nrf.RF_IRQ();
      if (host_nrf.newPayload) {
        Serial.print("Sensor response with: ");
        Serial.print(POLLING_TIMEOUT - timeout_init_time + timeout_actual_time);
        Serial.print(" microseconds.\n");
        rf_communication_handler();
      }
      break;
    }
  }
}

void rf_communication_handler() {
  //Redirects the packet
  //Packet: [id] [ax] [] [ay] [] [az] [] [gx] [] [gy] [] [gz] [] [qw] [] [qx] [] [qy] [] [qz] []
  //Aceleracao
  a[0] = (float) ((host_nrf.tx_buf[RF_PF_NOMAG_I_ACCEL_X_H] << 8) | host_nrf.tx_buf[RF_PF_NOMAG_I_ACCEL_X_L]) / 8192.0f;
  a[1] = (float) ((host_nrf.tx_buf[RF_PF_NOMAG_I_ACCEL_Y_H] << 8) | host_nrf.tx_buf[RF_PF_NOMAG_I_ACCEL_Y_H]) / 8192.0f;
  a[2] = (float) ((host_nrf.tx_buf[RF_PF_NOMAG_I_ACCEL_Z_H] << 8) | host_nrf.tx_buf[RF_PF_NOMAG_I_ACCEL_Z_H]) / 8192.0f;

  //Giroscopio
  g[0] = (float) ((host_nrf.tx_buf[RF_PF_NOMAG_I_GYRO_X_H] << 8) | host_nrf.tx_buf[RF_PF_NOMAG_I_GYRO_X_L]) / 131.0f;
  g[1] = (float) ((host_nrf.tx_buf[RF_PF_NOMAG_I_GYRO_Y_H] << 8) | host_nrf.tx_buf[RF_PF_NOMAG_I_GYRO_Y_L]) / 131.0f;
  g[2] = (float) ((host_nrf.tx_buf[RF_PF_NOMAG_I_GYRO_Z_H] << 8) | host_nrf.tx_buf[RF_PF_NOMAG_I_GYRO_Z_L]) / 131.0f;

  //Quaternion
  q[0] = (float) ((host_nrf.tx_buf[RF_PF_NOMAG_I_QUAT_W_H] << 8) | host_nrf.tx_buf[RF_PF_NOMAG_I_QUAT_W_L]) / 16384.0f;
  q[1] = (float) ((host_nrf.tx_buf[RF_PF_NOMAG_I_QUAT_X_H] << 8) | host_nrf.tx_buf[RF_PF_NOMAG_I_QUAT_X_L]) / 16384.0f;
  q[2] = (float) ((host_nrf.tx_buf[RF_PF_NOMAG_I_QUAT_Y_H] << 8) | host_nrf.tx_buf[RF_PF_NOMAG_I_QUAT_Y_L]) / 16384.0f;
  q[3] = (float) ((host_nrf.tx_buf[RF_PF_NOMAG_I_QUAT_Z_H] << 8) | host_nrf.tx_buf[RF_PF_NOMAG_I_QUAT_Z_L]) / 16384.0f;

  //Quaternions
  Serial.print(q[0]);
  Serial.print("\t");
  Serial.print(q[1]);
  Serial.print("\t");
  Serial.print(q[2]);
  Serial.print("\t");
  Serial.print(q[3]);
  Serial.print("\t-\t");
  //accel in G
  Serial.print(a[0]);
  Serial.print("\t");
  Serial.print(a[1]);
  Serial.print("\t");
  Serial.print(a[2]);
  Serial.print("\t-\t");
  //g[1]ro in degrees/s
  Serial.print(g[0]);
  Serial.print("\t");
  Serial.print(g[1]);
  Serial.print("\t");
  Serial.print(g[2]);
  Serial.print("\t\n");

  host_nrf.sta = 0;
  host_nrf.newPayload = 0;
}

/////////////////////
//RF Low Functions //
/////////////////////

void send_rf_data(uint8_t *data2send, uint8_t data_len) {
  uint8_t i;
  for (i = 0; i < data_len; i++) {
    host_nrf.tx_buf[i] = data2send[i];
  }
  host_nrf.TX_Mode_NOACK(data_len);
}

void send_rf_command_to(uint8_t cmd2send, uint8_t sensor_id) {
  host_nrf.tx_buf[0] = sensor_id;
  host_nrf.tx_buf[1] = cmd2send;
  host_nrf.TX_Mode_NOACK(2);
  wait_rf_response();
}
