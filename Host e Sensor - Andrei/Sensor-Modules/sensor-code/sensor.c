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

//Subenderecos usados no sistema
#define HOST_SUB_ADDR 0xFF //Sub addr do host
#define MY_SUB_ADDR 0x00 //Id do sensor

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

////////////////////////
//Functions in Sensor //
////////////////////////

/**
 * Inicia a DMP, deve ser chamada durante a configuracao
 */
void initial_setup_dmp();

/**
 * Realiza uma leitura da fifo da dmp e envia para o host
 */
void DataAcq();

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

  P0DIR &= ~(1<<4);//P04 = w2scl = output
  P0DIR &= ~(1<<3);//P03 = Status led = output
  //NOTE: PQ p1com?
  P1CON |= 0x53; // All general I/O 0101 0011
}

void setup() {
  iniciarIO();
  setup_T0_freq(Aquire_Freq,1);//Time em 100.00250006250157Hz
  rf_init(ADDR_HOST,ADDR_HOST,10,RF_DATA_RATE_2Mbps,RF_TX_POWER_0dBm);
  hal_w2_configure_master(HAL_W2_100KHZ); //I2C
  initial_setup_dmp();//MPU_6050 and DPM
  //Pisca o led 2 vezes indicando que iniciou
  STATUS_LED = 1; delay_ms(500); STATUS_LED = 0; delay_ms(500);
  STATUS_LED = 1; delay_ms(500); STATUS_LED = 0; delay_ms(500);
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
          case CMD_READ:
          DataAcq();
          break;
          case CMD_START:
          resetFIFO();//Reset the sensor fifo
          delay_ms(5);//wait reseting fifo
          break;
          case CMD_STOP:
          //Pica o led uma vez indicando que parou
          STATUS_LED = 1; delay_ms(500); STATUS_LED = 0; delay_ms(500);
          break;
          case CMD_CONNECTION:
          if(mpu_testConnection()){
            EN_MPU_CONNECTED_FLAG;
            send_rf_command(CMD_CONNECTION,MY_SUB_ADDR);
          } else {
            DIS_MPU_CONNECTED_FLAG;
            send_rf_command(CMD_DISCONNECT,MY_SUB_ADDR);
          }
          break;
          case CMD_CALIBRATE:
          start_T0();//Timer calibration
          break;
          case CMD_SET_PACKET_TYPE:
          packet_type = rx_buf[2]; //Seta o tipo de pacote
          default:
          //Inverte o led indicando que recebeu um comando desconhecido ou nao implementado
          STATUS_LED = !STATUS_LED;
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

///////////////////////
//FUNCIONS in Sensor //
///////////////////////

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

void DataAcq(){
  uint8_t i = 0;
  numbPackets = getFIFOCount()/PSDMP;//floor
  for (size_t i = 0; i < numbPackets; i++) {
    getFIFOBytes(fifoBuffer, PSDMP);  //read a packet from FIFO
    send_inertial_packet_by_rf(packet_type,fifoBuffer);
  }/*END for every packet*/
}/*End of DataAcq*/

//interrupção o I2C
void I2C_IRQ (void) interrupt INTERRUPT_SERIAL {
  I2C_IRQ_handler();
}
