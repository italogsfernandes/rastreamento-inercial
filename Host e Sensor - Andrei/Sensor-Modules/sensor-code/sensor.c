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

#define CMD_OK  0x00 //Ack - Uart Command
#define CMD_ERROR 0x01 //Error flag - Uart Command
#define CMD_START 0x02 //Start Measuring - Uart Command
#define CMD_STOP  0x03 //Stop Measuring - Uart Command
#define CMD_CONNECTION  0x04 //Teste Connection - Uart Command
#define CMD_CALIBRATE 0x05 //Calibrate Sensors Command
#define CMD_DISCONNECT 0x06 //Some sensor has gone disconected
#define CMD_GET_SENSOR_FIFO 0x07

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

//TODO: thing again
void ler_dmp(){
  //send_packet_to_host(UART_PACKET_TYPE_STRING,"AGUARDA_INT",11);delay_ms(10);
  //send_packet_to_host(UART_PACKET_TYPE_HEX,(uint8_t *) &mpuInterrupt,1);delay_ms(10);
  //send_packet_to_host(UART_PACKET_TYPE_UINT16,(uint8_t *)&fifoCount,2);delay_ms(10);
  while (!mpuInterrupt || fifoCount < packetSize){
    fifoCount = getFIFOCount();
  }
  //send_packet_to_host(UART_PACKET_TYPE_STRING,"INT_RECEIVED",12);delay_ms(10);
  // reset interrupt flag and get INT_STATUS byte
  mpuInterrupt = false;
  mpuIntStatus = getIntStatus();

  // get current FIFO count
  fifoCount = getFIFOCount();
  //send_packet_to_host(UART_PACKET_TYPE_STRING,"FIFO_COUNT",10);delay_ms(10);
  //send_packet_to_host(UART_PACKET_TYPE_UINT16,(uint8_t *)&fifoCount,2);delay_ms(10);
  //send_packet_to_host(UART_PACKET_TYPE_STRING,"INT_STATUS",10);delay_ms(10);
  //send_packet_to_host(UART_PACKET_TYPE_HEX,&mpuIntStatus,1);delay_ms(10);
  // check for overflow (this should never happen unless our code is too inefficient)
  if ((mpuIntStatus & 0x10) || fifoCount == 1024) {

    // reset so we can continue cleanly
    resetFIFO();
    send_packet_to_host(UART_PACKET_TYPE_STRING,"FIFO overflow!",14);//delay_ms(10);
    // otherwise, check for DMP data ready interrupt (this should happen frequently)
  } else if (mpuIntStatus & 0x02) {
    //send_packet_to_host(UART_PACKET_TYPE_STRING,"OK_READING",10);delay_ms(10);
    // wait for correct available data length, should be a VERY short wait
    while (fifoCount < packetSize) fifoCount = getFIFOCount();

    // read a packet from FIFO
    getFIFOBytes(fifoBuffer, packetSize);

    //send_packet_to_host(UART_PACKET_TYPE_HEX,fifoBuffer,14);

    dmpGetPacket16bits(packet_16bits,fifoBuffer);
    getMotion6_packet(packet_motion6);
    packet_16bits[8] = packet_motion6[6];packet_16bits[9] = packet_motion6[7];
    packet_16bits[10] = packet_motion6[8];packet_16bits[11] = packet_motion6[9];
    packet_16bits[12] = packet_motion6[10];packet_16bits[13] = packet_motion6[11];

    packet_16bits[14] = packet_motion6[0];packet_16bits[15] = packet_motion6[1];
    packet_16bits[16] = packet_motion6[2];packet_16bits[17] = packet_motion6[3];
    packet_16bits[18] = packet_motion6[4];packet_16bits[19] = packet_motion6[5];
    send_packet_to_host(UART_PACKET_TYPE_FIFO_NO_MAG,packet_16bits,20);


    // track FIFO count here in case there is > 1 packet available
    // (this lets us immediately read more without waiting for an interrupt)
    fifoCount -= packetSize;
  }
}

void main(void) {
  setup();
  while(1){

    if(newPayload){
      //verifica se o sinal eh direficionado para mim
      if(rx_buf[0] == MY_SUB_ADDR){
        switch(rx_buf[1]){
          case Sinal_request_data:
          send_packet_to_host(UART_PACKET_TYPE_STRING,"On",2);delay_ms(10);
          LED = 1;
          start_T0();
          break;
          case Sinal_LEDS:
          stop_T0();
          LED = 0;
          send_packet_to_host(UART_PACKET_TYPE_STRING,"Off",3);delay_ms(10);
          break;
        }
      }
      sta = 0;
      newPayload = 0;
    }
    //timer tick
    if(timer_flag <= 0){
      ler_dmp();
      timer_flag = 1;
    }
  }
}

//interrupção o I2C
void I2C_IRQ (void) interrupt INTERRUPT_SERIAL{
  I2C_IRQ_handler();
}
