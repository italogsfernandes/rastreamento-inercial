//Subenderecos usados no sistema
#define MY_SUB_ADDR 0x01
/*
Módulo GY521 - Offset aproximados
setXAccelOffset(-1692);
setYAccelOffset(-883);
setZAccelOffset(1114);
setXGyroOffset(78);
setYGyroOffset(-20);
setZGyroOffset(21);
*/

#include "nrf24le1.h"
#include "reg24le1.h" //Definiï¿½ï¿½es de muitos endereï¿½os de registradores.
#include "nRF-SPIComands.h" //Comunicacao RF
#include "hal_w2_isr.h" //Comunicacao I2C
#include "hal_delay.h" //delay
#include "pacotes_inerciais.h" //pacotes para enviar
#include "dmp.h" //configuracao e uso da dmp da mpu6050
#include "timer0.h"

#define STATUS_LED  P03
#define PSDMP 42

uint8_t xdata packet_type = PACKET_TYPE_QUAT; //Tipo de pacote que o sensor obtera
uint8_t xdata fifoBuffer[PSDMP] = {0};
uint8_t xdata numbPackets;

////////////////////////
//Functions in Sensor //
////////////////////////

/**
* Inicia a DMP, deve ser chamada durante a configuracao
*/
void initial_setup_dmp() large;

/**
* Realiza uma leitura da fifo da dmp
*/
void DataAcq() large;

/**
 *  Envia dados da fifoBuffer para o host
 */
void send_dada_read() large;

///////////////////
//Implementation //
///////////////////

/**
* Seta os pinos do nrf como saidas e entradas de acordo com as funcoes desejadas
*/
void iniciarIO(void) large {
    P0DIR = 0x00;   // Tudo output
    P1DIR = 0x00;   // Tudo output
    P0CON = 0x00; P1CON = 0x00; //Reseting PxCON registers

    P0DIR &= ~(1<<4);//P04 = w2scl = output
    P0DIR &= ~(1<<3);//P03 = Status led = output
    //NOTE: PQ p1com?
    P1CON |= 0x53; // All general I/O 0101 0011
}

void blink_status_led() large {
  STATUS_LED = 1; delay_ms(250); STATUS_LED = 0; delay_ms(250);
}

void setup() large {
    iniciarIO();
		//Pisca o led 2 vezes indicando que iniciou
		blink_status_led(); //1
    rf_init(ADDR_HOST,ADDR_HOST,10,RF_DATA_RATE_2Mbps,RF_TX_POWER_0dBm);

		blink_status_led();//2
    hal_w2_configure_master(HAL_W2_400KHZ); //I2C

		blink_status_led();//3
    initial_setup_dmp();//MPU_6050 and DPM

    setup_T0_ticks(6666,1);
    start_T0();
}

void main(void) {
    setup();
    while(1){ //Loop
        ////////////////////
        //Comunicacao RF //
        ////////////////////
        if(newPayload){
            sta = 0;
            newPayload = 0;
            //verifica se o sinal eh direficionado para mim
            if(rx_buf[0] == MY_SUB_ADDR){
                switch(rx_buf[1]){
                    case CMD_READ:
                    send_dada_read();
                    break;
                    case CMD_LIGHT_UP_LED:
                    STATUS_LED = 1;
                    send_rf_command_with_arg(CMD_LIGHT_UP_LED,CMD_OK,MY_SUB_ADDR);
                    break;
                    case CMD_TURN_OFF_LED:
                    STATUS_LED = 0;
                    send_rf_command_with_arg(CMD_TURN_OFF_LED,CMD_OK,MY_SUB_ADDR);
                    break;
										case 0xAA:
										fifoBuffer[MOTIONAPPS_FIFO_I_QUAT_WH] = 0x40;
										fifoBuffer[MOTIONAPPS_FIFO_I_QUAT_WL] = 0x00;//W_quat
										fifoBuffer[MOTIONAPPS_FIFO_I_QUAT_XH] = 0x20;
										fifoBuffer[MOTIONAPPS_FIFO_I_QUAT_XL] = 0x00;//X_quat
										fifoBuffer[MOTIONAPPS_FIFO_I_QUAT_YH] = 0x10;
										fifoBuffer[MOTIONAPPS_FIFO_I_QUAT_YL] = 0x00;//Y_quat
										fifoBuffer[MOTIONAPPS_FIFO_I_QUAT_ZH] = 0x08;
										fifoBuffer[MOTIONAPPS_FIFO_I_QUAT_ZL] = 0x00;//Z_quat
                    fifoBuffer[MOTIONAPPS_FIFO_I_ACCEL_XH] = 0x40;
                    fifoBuffer[MOTIONAPPS_FIFO_I_ACCEL_XL] = 0x00;//X_AC
                    fifoBuffer[MOTIONAPPS_FIFO_I_ACCEL_YH] = 0x20;
                    fifoBuffer[MOTIONAPPS_FIFO_I_ACCEL_YL] = 0x00;//Y_AC
                    fifoBuffer[MOTIONAPPS_FIFO_I_ACCEL_ZH] = 0x10;
                    fifoBuffer[MOTIONAPPS_FIFO_I_ACCEL_ZL] = 0x00;//Z_AC
                    fifoBuffer[MOTIONAPPS_FIFO_I_GYRO_XH] = 0x40;
                    fifoBuffer[MOTIONAPPS_FIFO_I_GYRO_XL] = 0x00;//X_GY
                    fifoBuffer[MOTIONAPPS_FIFO_I_GYRO_YH] = 0x00;
                    fifoBuffer[MOTIONAPPS_FIFO_I_GYRO_YL] = 0x20;//Y_GY
                    fifoBuffer[MOTIONAPPS_FIFO_I_GYRO_ZH] = 0x00;
                    fifoBuffer[MOTIONAPPS_FIFO_I_GYRO_ZL] = 0x10;//Z_GY
                    fifoBuffer[MOTIONAPPS_FIFO_I_QUAT_WH] = 0x00;
                    send_inertial_packet_by_rf(PACKET_TYPE_FIFO_NO_MAG,fifoBuffer,MY_SUB_ADDR);
										break;
										default:
                    //Inverte o led indicando que recebeu um comando desconhecido ou nao implementado
                    STATUS_LED = 1; delay_ms(500); STATUS_LED = 0; delay_ms(500);
                    //STATUS_LED = !STATUS_LED;
                    break;
                }/*END SWITCH*/
            }/*END IF MY SUB ADDR*/
        } /* END Comunicacao RF */

        if(timer_elapsed){
          DataAcq();
          timer_elapsed = 0;
        }
    }/*END LOOP*/
}/*END MAIN*/

///////////////////////
//FUNCIONS in Sensor //
///////////////////////

void initial_setup_dmp() large {
    uint8_t ret;

		blink_status_led();//4
		mpu_8051_malloc_setup(); //Malloc pool for mpu library

		blink_status_led();//5
		ret = (uint8_t) mpu_testConnection();

		blink_status_led();//6
    if(ret){
			blink_status_led();//7
      mpu_initialize(); //Initializes the IMU

			blink_status_led();//8
      ret =  dmpInitialize();  //Initializes the DMP

			blink_status_led();//9
      delay_ms(50);

			blink_status_led();//10
      if(ret == 0)
      {
				blink_status_led();//11
        setDMPEnabled(true);

				blink_status_led();//12
				setXAccelOffset(-1692);
				setYAccelOffset(-883);
				setZAccelOffset(1114);
				setXGyroOffset(78);
				setYGyroOffset(-20);
				setZGyroOffset(21);
				blink_status_led();//13
       }
    }
}

void send_dada_read() large {
  		tx_buf[0] = MY_SUB_ADDR;
      tx_buf[1] = fifoBuffer[MOTIONAPPS_FIFO_I_QUAT_WL];//W_quat
      tx_buf[2] = fifoBuffer[MOTIONAPPS_FIFO_I_QUAT_XH];
      tx_buf[3] = fifoBuffer[MOTIONAPPS_FIFO_I_QUAT_XL];//X_quat
      tx_buf[4] = fifoBuffer[MOTIONAPPS_FIFO_I_QUAT_YH];
      tx_buf[5] = fifoBuffer[MOTIONAPPS_FIFO_I_QUAT_YL];//Y_quat
      tx_buf[6] = fifoBuffer[MOTIONAPPS_FIFO_I_QUAT_ZH];
      tx_buf[7] = fifoBuffer[MOTIONAPPS_FIFO_I_QUAT_ZL];//Z_quat
      tx_buf[8] = fifoBuffer[MOTIONAPPS_FIFO_I_ACCEL_XH];
      tx_buf[9] = fifoBuffer[MOTIONAPPS_FIFO_I_ACCEL_XL];//X_AC
      tx_buf[10] = fifoBuffer[MOTIONAPPS_FIFO_I_ACCEL_YH];
      tx_buf[11] = fifoBuffer[MOTIONAPPS_FIFO_I_ACCEL_YL];//Y_AC
      tx_buf[12] = fifoBuffer[MOTIONAPPS_FIFO_I_ACCEL_ZH];
      tx_buf[13] = fifoBuffer[MOTIONAPPS_FIFO_I_ACCEL_ZL];//Z_AC
      tx_buf[14] = fifoBuffer[MOTIONAPPS_FIFO_I_GYRO_XH];
      tx_buf[15] = fifoBuffer[MOTIONAPPS_FIFO_I_GYRO_XL];//X_GY
      tx_buf[16] = fifoBuffer[MOTIONAPPS_FIFO_I_GYRO_YH];
      tx_buf[17] = fifoBuffer[MOTIONAPPS_FIFO_I_GYRO_YL];//Y_GY
      tx_buf[18] = fifoBuffer[MOTIONAPPS_FIFO_I_GYRO_ZH];
      tx_buf[19] = fifoBuffer[MOTIONAPPS_FIFO_I_GYRO_ZL];//Z_GY
      tx_buf[20] = fifoBuffer[MOTIONAPPS_FIFO_I_QUAT_WH];
      TX_Mode_NOACK(21);
  		RX_Mode();
}

void DataAcq() large {
    uint8_t i = 0;
    numbPackets = getFIFOCount() / PSDMP;//floor
    for (i = 0; i < numbPackets; i++) {
        getFIFOBytes(fifoBuffer, PSDMP);  //read a packet from FIFO
    }/*END for every packet*/
    STATUS_LED = !STATUS_LED;
}/*End of DataAcq*/

//interrupção do I2C - NOT USED
void I2C_IRQ (void) interrupt INTERRUPT_SERIAL {
    I2C_IRQ_handler();
}
