#include "nrf24le1.h"
#include "stdint.h"
#include "reg24le1.h" //Definiï¿½ï¿½es de muitos endereï¿½os de registradores.
#include "stdbool.h" //Booleanos
#include "API.h"
#include <pacotes_inerciais.h>
#include "hal_w2_isr.h"
#include "hal_delay.h"
#include <simple_timer.h>
#include <nRF-SPIComands.h>
#include <dmp.h>

//Size of DMP packages
#define PSDMP 42
#define PS 20

//Subenderecos usados no sistema
#define HOST_SUB_ADDR 0xFF //Sub addr do host
#define MY_SUB_ADDR 0x00 //Id do sensor

#define CMD_OK  0x00 //Ack - Uart Command
#define CMD_ERROR 0x01 //Error flag - Uart Command
#define CMD_START 0x02 //Start Measuring - Uart Command
#define CMD_STOP  0x03 //Stop Measuring - Uart Command
#define CMD_CONNECTION  0x04 //Teste Connection - Uart Command
#define CMD_CALIBRATE 0x05 //Calibrate Sensors Command
#define CMD_DISCONNECT 0x06 //Some sensor has gone disconected
#define CMD_GET_SENSOR_FIFO 0x07
#define CMD_SET_PACKET_TYPE 0x08

#define  STATUS_LED  P03

#define EN_DMP_READY_FLAG sensor_status &= 0x08
#define DIS_DMP_READY_FLAG sensor_status |= ~0x08
#define EN_MPU_CALIBRATED_FLAG sensor_status &= 0x04
#define DIS_MPU_CALIBRATED_FLAG sensor_status |= ~0x04
#define EN_MPU_CONNECTED_FLAG sensor_status &= 0x02
#define DIS_MPU_CONNECTED_FLAG sensor_status |= ~0x02
#define EN_SENSOR_ON_FLAG sensor_status &= 0x01
#define DIS_SENSOR_ON_FLAG sensor_status |= ~0x01
uint8_t sensor_status = 0x01; // [dmp_ready][mpu_calibrated][mpu_connected][On]
uint8_t packet_type = PACKET_TYPE_QUAT; //Tipo de pacote que o sensor obtera

//TODO: Document
void iniciarIO(void){
  P0DIR = 0xF7;   // 1111 0111 - 1/0 = In/Out - Output: P0.3
  P1DIR = 0xFF;   // Tudo input
  P2DIR = 0xFF;
  P0CON = 0x00;  	// All general I/O
  //NOTE: PQ p1com?
  P1CON |= 0x53;  	// All general I/O
  P2CON = 0x00;  	// All general I/O
}

//TODO:Document
void initial_setup_dmp(){
  mpu_8051_malloc_setup(); //Malloc pool for mpu library

  if(mpu_testConnection()){
    EN_MPU_CONNECTED_FLAG;
    mpu_initialize(); //Initializes the IMU
    uint8_t ret =  dmpInitialize();  //Initializes the DMP
    delay(50);

    if(ret == 0)
    {
      EN_DMP_READY_FLAG;
      setDMPEnabled(true);
      setXAccelOffset(871);
      setYAccelOffset(1527);
      setZAccelOffset(1988);
      setXGyroOffset(36);
      setYGyroOffset(-37);
      setZGyroOffset(-1);

    }
    else
    {
      senf_rf_error_flag();
    }
  }
}

//TODO: Document
void setup() {
  iniciarIO();
  setup_T0_freq(Aquire_Freq,1);//Time em 100.00250006250157Hz
  rf_init(ADDR_HOST,ADDR_HOST,10,RF_DATA_RATE_2Mbps,RF_TX_POWER_0dBm);
  hal_w2_configure_master(HAL_W2_100KHZ); //I2C
  initial_setup_dmp();//MPU_6050 and DPM
  luzes_iniciais();
}

void main(void) {
  setup();
  while(1){ //Loop
    ////////////////////
    //Comunicacao RF //
    ////////////////////
    if(newPayload){
      //verifica se o sinal eh direficionado para mim
      if(rx_buf[0] == MY_SUB_ADDR){
        switch(rx_buf[1]){
          case CMD_GET_SENSOR_FIFO:
          DataAcq();
          break;
          case CMD_START:
          resetFIFO();//Reset the sensor fifo
          delay_ms(5);//wait reseting fifo
          break;
          case CMD_STOP:
          // '-'
          break;
          case CMD_CALIBRATE:
          start_T0();//Timer calibration
          break;
          case CMD_SET_PACKET_TYPE:
          packet_type = rx_buf[2]; //Seta o tipo de pacote
          default:
          //i don't know what to do here
          break;
        }/*END SWITCH*/
      }/*END IF MY SUB ADDR*/
      sta = 0;
      newPayload = 0;
    } /* END Comunicacao RF */

    //////////////////////////
    //TIMER for Calibration //
    //////////////////////////
    if(timer_elapsed){
      calibrationRoutine();
      timer_elapsed = 0;
    }
  }/*END LOOP*/
}/*END MAIN*/

/*
v -> x = (packet[28] << 8) | packet[29];
v -> y = (packet[32] << 8) | packet[33];
v -> z = (packet[36] << 8) | packet[37];


uint8_t MPU6050::dmpGetGyro(int16_t *data, const uint8_t* packet) {
    // TODO: accommodate different arrangements of sent data (ONLY default supported now)
    if (packet == 0) packet = dmpPacketBuffer;
    data[0] = (packet[16] << 8) | packet[17];
    data[1] = (packet[20] << 8) | packet[21];
    data[2] = (packet[24] << 8) | packet[25];
    return 0;
}*/
/* ================================================================================================ *
 | Default MotionApps v2.0 42-byte FIFO packet structure:                                           |
 |                                                                                                  |
 | [QUAT W][      ][QUAT X][      ][QUAT Y][      ][QUAT Z][      ][GYRO X][      ][GYRO Y][      ] |
 |   0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  |
 |                                                                                                  |
 | [GYRO Z][      ][ACC X ][      ][ACC Y ][      ][ACC Z ][      ][      ]                         |
 |  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38  39  40  41                          |
 * ================================================================================================ */
#define MOTIONAPPS_FIFO_I_QUAT_WH 0 //Index of it in FIFO from DMP
#define MOTIONAPPS_FIFO_I_QUAT_WL 1 //Index of it in FIFO from DMP
#define MOTIONAPPS_FIFO_I_QUAT_XH 4 //Index of it in FIFO from DMP
#define MOTIONAPPS_FIFO_I_QUAT_XL 5 //Index of it in FIFO from DMP
#define MOTIONAPPS_FIFO_I_QUAT_YH 8 //Index of it in FIFO from DMP
#define MOTIONAPPS_FIFO_I_QUAT_YL 9 //Index of it in FIFO from DMP
#define MOTIONAPPS_FIFO_I_QUAT_ZH 12 //Index of it in FIFO from DMP
#define MOTIONAPPS_FIFO_I_QUAT_ZL 13 //Index of it in FIFO from DMP
#define MOTIONAPPS_FIFO_I_ACCEL_XH  28 //Index of it in FIFO from DMP
#define MOTIONAPPS_FIFO_I_ACCEL_XL  29 //Index of it in FIFO from DMP
#define MOTIONAPPS_FIFO_I_ACCEL_YH  32 //Index of it in FIFO from DMP
#define MOTIONAPPS_FIFO_I_ACCEL_YL  33 //Index of it in FIFO from DMP
#define MOTIONAPPS_FIFO_I_ACCEL_ZH  36 //Index of it in FIFO from DMP
#define MOTIONAPPS_FIFO_I_ACCEL_ZL  37 //Index of it in FIFO from DMP
#define MOTIONAPPS_FIFO_I_GYRO_XH 16 //Index of it in FIFO from DMP
#define MOTIONAPPS_FIFO_I_GYRO_XL 17 //Index of it in FIFO from DMP
#define MOTIONAPPS_FIFO_I_GYRO_YH 20 //Index of it in FIFO from DMP
#define MOTIONAPPS_FIFO_I_GYRO_YL 21 //Index of it in FIFO from DMP
#define MOTIONAPPS_FIFO_I_GYRO_ZH 24 //Index of it in FIFO from DMP
#define MOTIONAPPS_FIFO_I_GYRO_ZL 25 //Index of it in FIFO from DMP

void DataAcq(){
  uint8_t i = 0;
  numbPackets = getFIFOCount()/PSDMP;//floor
  for (size_t i = 0; i < numbPackets; i++) {
    getFIFOBytes(fifoBuffer, PSDMP);  //read a packet from FIFO

    tx_buf[0] = MY_SUB_ADDR;
    tx_buf[1] = packet_type;
    switch (packet_type) {
      case PACKET_TYPE_ACEL://Acelerometer [X][Y][Z]
      tx_buf[2] = fifoBuffer[MOTIONAPPS_FIFO_I_ACCEL_XH];
      tx_buf[3] = fifoBuffer[MOTIONAPPS_FIFO_I_ACCEL_XL];//X_AC
      tx_buf[4] = fifoBuffer[MOTIONAPPS_FIFO_I_ACCEL_YH];
      tx_buf[5] = fifoBuffer[MOTIONAPPS_FIFO_I_ACCEL_YL];//Y_AC
      tx_buf[6] = fifoBuffer[MOTIONAPPS_FIFO_I_ACCEL_ZH];
      tx_buf[7] = fifoBuffer[MOTIONAPPS_FIFO_I_ACCEL_ZL];//Z_AC
      break;
      case PACKET_TYPE_GIRO://Gyroscope [X][Y][Z]
      tx_buf[2] = fifoBuffer[MOTIONAPPS_FIFO_I_GYRO_XH];
      tx_buf[3] = fifoBuffer[MOTIONAPPS_FIFO_I_GYRO_XL];//X_GY
      tx_buf[4] = fifoBuffer[MOTIONAPPS_FIFO_I_GYRO_YH];
      tx_buf[5] = fifoBuffer[MOTIONAPPS_FIFO_I_GYRO_YL];//Y_GY
      tx_buf[6] = fifoBuffer[MOTIONAPPS_FIFO_I_GYRO_ZH];
      tx_buf[7] = fifoBuffer[MOTIONAPPS_FIFO_I_GYRO_ZL];//Z_GY
      break;
      case PACKET_TYPE_MAG://Magnetometer [X][Y][Z]
      //TODO: MAG
      tx_buf[2] = fifoBuffer[];tx_buf[3] = fifoBuffer[];//X
      tx_buf[4] = fifoBuffer[];tx_buf[5] = fifoBuffer[];//Y
      tx_buf[6] = fifoBuffer[];tx_buf[7] = fifoBuffer[];//Z
      break;
      case PACKET_TYPE_M6://Motion6 [Acel][Gyro]
      tx_buf[2] = fifoBuffer[MOTIONAPPS_FIFO_I_ACCEL_XH];
      tx_buf[3] = fifoBuffer[MOTIONAPPS_FIFO_I_ACCEL_XL];//X_AC
      tx_buf[4] = fifoBuffer[MOTIONAPPS_FIFO_I_ACCEL_YH];
      tx_buf[5] = fifoBuffer[MOTIONAPPS_FIFO_I_ACCEL_YL];//Y_AC
      tx_buf[6] = fifoBuffer[MOTIONAPPS_FIFO_I_ACCEL_ZH];
      tx_buf[7] = fifoBuffer[MOTIONAPPS_FIFO_I_ACCEL_ZL];//Z_AC
      tx_buf[8] = fifoBuffer[MOTIONAPPS_FIFO_I_GYRO_XH];
      tx_buf[9] = fifoBuffer[MOTIONAPPS_FIFO_I_GYRO_XL];//X_GY
      tx_buf[10] = fifoBuffer[MOTIONAPPS_FIFO_I_GYRO_YH];
      tx_buf[11] = fifoBuffer[MOTIONAPPS_FIFO_I_GYRO_YL];//Y_GY
      tx_buf[12] = fifoBuffer[MOTIONAPPS_FIFO_I_GYRO_ZH];
      tx_buf[13] = fifoBuffer[MOTIONAPPS_FIFO_I_GYRO_ZL];//Z_GY
      break;
      case PACKET_TYPE_M9://Motion9 [Acel][Gyro][Mag]
      tx_buf[2] = fifoBuffer[MOTIONAPPS_FIFO_I_ACCEL_XH];
      tx_buf[3] = fifoBuffer[MOTIONAPPS_FIFO_I_ACCEL_XL];//X_AC
      tx_buf[4] = fifoBuffer[MOTIONAPPS_FIFO_I_ACCEL_YH];
      tx_buf[5] = fifoBuffer[MOTIONAPPS_FIFO_I_ACCEL_YL];//Y_AC
      tx_buf[6] = fifoBuffer[MOTIONAPPS_FIFO_I_ACCEL_ZH];
      tx_buf[7] = fifoBuffer[MOTIONAPPS_FIFO_I_ACCEL_ZL];//Z_AC
      tx_buf[8] = fifoBuffer[MOTIONAPPS_FIFO_I_GYRO_XH];
      tx_buf[9] = fifoBuffer[MOTIONAPPS_FIFO_I_GYRO_XL];//X_GY
      tx_buf[10] = fifoBuffer[MOTIONAPPS_FIFO_I_GYRO_YH];
      tx_buf[11] = fifoBuffer[MOTIONAPPS_FIFO_I_GYRO_YL];//Y_GY
      tx_buf[12] = fifoBuffer[MOTIONAPPS_FIFO_I_GYRO_ZH];
      tx_buf[13] = fifoBuffer[MOTIONAPPS_FIFO_I_GYRO_ZL];//Z_GY
      //TODO: MAG
      tx_buf[14] = fifoBuffer[];tx_buf[15] = fifoBuffer[];//X_Mag
      tx_buf[16] = fifoBuffer[];tx_buf[17] = fifoBuffer[];//Y_Mag
      tx_buf[18] = fifoBuffer[];tx_buf[19] = fifoBuffer[];//Z_Mag
      break;
      case PACKET_TYPE_QUAT://Quaternion [W][X][Y][Z]
      tx_buf[2] = fifoBuffer[MOTIONAPPS_FIFO_I_QUAT_WH];
      tx_buf[3] = fifoBuffer[MOTIONAPPS_FIFO_I_QUAT_WL];//W_quat
      tx_buf[4] = fifoBuffer[MOTIONAPPS_FIFO_I_QUAT_XH];
      tx_buf[5] = fifoBuffer[MOTIONAPPS_FIFO_I_QUAT_XL];//X_quat
      tx_buf[6] = fifoBuffer[MOTIONAPPS_FIFO_I_QUAT_YH];
      tx_buf[7] = fifoBuffer[MOTIONAPPS_FIFO_I_QUAT_YL];//Y_quat
      tx_buf[8] = fifoBuffer[MOTIONAPPS_FIFO_I_QUAT_ZH];
      tx_buf[9] = fifoBuffer[MOTIONAPPS_FIFO_I_QUAT_ZL];//Z_quat
      break;
      case PACKET_FIFO_M6://FIFO_Motion6 [Quat][Motion6]
      tx_buf[2] = fifoBuffer[MOTIONAPPS_FIFO_I_QUAT_WH];
      tx_buf[3] = fifoBuffer[MOTIONAPPS_FIFO_I_QUAT_WL];//W_quat
      tx_buf[4] = fifoBuffer[MOTIONAPPS_FIFO_I_QUAT_XH];
      tx_buf[5] = fifoBuffer[MOTIONAPPS_FIFO_I_QUAT_XL];//X_quat
      tx_buf[6] = fifoBuffer[MOTIONAPPS_FIFO_I_QUAT_YH];
      tx_buf[7] = fifoBuffer[MOTIONAPPS_FIFO_I_QUAT_YL];//Y_quat
      tx_buf[8] = fifoBuffer[MOTIONAPPS_FIFO_I_QUAT_ZH];
      tx_buf[9] = fifoBuffer[MOTIONAPPS_FIFO_I_QUAT_ZL];//Z_quat
      tx_buf[10] = fifoBuffer[MOTIONAPPS_FIFO_I_ACCEL_XH];
      tx_buf[11] = fifoBuffer[MOTIONAPPS_FIFO_I_ACCEL_XL];//X_AC
      tx_buf[12] = fifoBuffer[MOTIONAPPS_FIFO_I_ACCEL_YH];
      tx_buf[13] = fifoBuffer[MOTIONAPPS_FIFO_I_ACCEL_YL];//Y_AC
      tx_buf[14] = fifoBuffer[MOTIONAPPS_FIFO_I_ACCEL_ZH];
      tx_buf[15] = fifoBuffer[MOTIONAPPS_FIFO_I_ACCEL_ZL];//Z_AC
      tx_buf[16] = fifoBuffer[MOTIONAPPS_FIFO_I_GYRO_XH];
      tx_buf[17] = fifoBuffer[MOTIONAPPS_FIFO_I_GYRO_XL];//X_GY
      tx_buf[18] = fifoBuffer[MOTIONAPPS_FIFO_I_GYRO_YH];
      tx_buf[19] = fifoBuffer[MOTIONAPPS_FIFO_I_GYRO_YL];//Y_GY
      tx_buf[20] = fifoBuffer[MOTIONAPPS_FIFO_I_GYRO_ZH];
      tx_buf[21] = fifoBuffer[MOTIONAPPS_FIFO_I_GYRO_ZL];//Z_GY
      break;
      case PACKET_FIFO_M9://FIFO_Motion9 [Quat][Motion9]
      tx_buf[2] = fifoBuffer[MOTIONAPPS_FIFO_I_QUAT_WH];
      tx_buf[3] = fifoBuffer[MOTIONAPPS_FIFO_I_QUAT_WL];//W_quat
      tx_buf[4] = fifoBuffer[MOTIONAPPS_FIFO_I_QUAT_XH];
      tx_buf[5] = fifoBuffer[MOTIONAPPS_FIFO_I_QUAT_XL];//X_quat
      tx_buf[6] = fifoBuffer[MOTIONAPPS_FIFO_I_QUAT_YH];
      tx_buf[7] = fifoBuffer[MOTIONAPPS_FIFO_I_QUAT_YL];//Y_quat
      tx_buf[8] = fifoBuffer[MOTIONAPPS_FIFO_I_QUAT_ZH];
      tx_buf[9] = fifoBuffer[MOTIONAPPS_FIFO_I_QUAT_ZL];//Z_quat
      tx_buf[10] = fifoBuffer[MOTIONAPPS_FIFO_I_ACCEL_XH];
      tx_buf[11] = fifoBuffer[MOTIONAPPS_FIFO_I_ACCEL_XL];//X_AC
      tx_buf[12] = fifoBuffer[MOTIONAPPS_FIFO_I_ACCEL_YH];
      tx_buf[13] = fifoBuffer[MOTIONAPPS_FIFO_I_ACCEL_YL];//Y_AC
      tx_buf[14] = fifoBuffer[MOTIONAPPS_FIFO_I_ACCEL_ZH];
      tx_buf[15] = fifoBuffer[MOTIONAPPS_FIFO_I_ACCEL_ZL];//Z_AC
      tx_buf[16] = fifoBuffer[MOTIONAPPS_FIFO_I_GYRO_XH];
      tx_buf[17] = fifoBuffer[MOTIONAPPS_FIFO_I_GYRO_XL];//X_GY
      tx_buf[18] = fifoBuffer[MOTIONAPPS_FIFO_I_GYRO_YH];
      tx_buf[19] = fifoBuffer[MOTIONAPPS_FIFO_I_GYRO_YL];//Y_GY
      tx_buf[20] = fifoBuffer[MOTIONAPPS_FIFO_I_GYRO_ZH];
      tx_buf[21] = fifoBuffer[MOTIONAPPS_FIFO_I_GYRO_ZL];//Z_GY
      //TODO: MAG
      tx_buf[22] = fifoBuffer[];tx_buf[23] = fifoBuffer[];//X_Mag
      tx_buf[24] = fifoBuffer[];tx_buf[25] = fifoBuffer[];//Y_Mag
      tx_buf[26] = fifoBuffer[];tx_buf[27] = fifoBuffer[];//Z_Mag
      break;
      default:
      //ãanh,sorry?
      break;
    }/*END of Switch packet type*/
  }/*END for every packet*/
}/*End of DataAcq*/

//interrupção o I2C
void I2C_IRQ (void) interrupt INTERRUPT_SERIAL {
  I2C_IRQ_handler();
}
