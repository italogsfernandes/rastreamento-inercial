//Subenderecos usados no sistema
#define MY_SUB_ADDR 0x01
/*
Módulos GY521 - Offsets aproximados
Id)	Xac		Yac		Zac		Xgy		Ygy		Zgy
1)	-499	2739	1242	16		109		33
2)	-2479	-1117	1459	42		21		60
3)	1252	-205	1890	-339	-46		18
4)	-365	529		1699	139		50		36
*/

#include "nrf24le1.h"
#include "reg24le1.h" //Definiï¿½ï¿½es de muitos endereï¿½os de registradores.
#include "nRF-SPIComands.h" //Comunicacao RF
#include "hal_w2_isr.h" //Comunicacao I2C
#include "hal_delay.h" //delay
#include "pacotes_inerciais.h" //pacotes para enviar
#include "dmp.h" //configuracao e uso da dmp da mpu6050

#define  STATUS_LED  P03
#define PSDMP 42

uint8_t xdata packet_type = PACKET_TYPE_QUAT; //Tipo de pacote que o sensor obtera
uint8_t xdata fifoBuffer[64] = {0};
uint8_t xdata numbPackets;

////////////////////////
//Functions in Sensor //
////////////////////////

/**
* Inicia a DMP, deve ser chamada durante a configuracao
*/
void initial_setup_dmp() large;

/**
* Realiza uma leitura da fifo da dmp e envia para o host
*/
void DataAcq() large;

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
		//Pisca o led 2 vezes indicando que iniciou
		STATUS_LED = 1; delay_ms(250); STATUS_LED = 0; delay_ms(250); //1
    rf_init(ADDR_HOST,ADDR_HOST,10,RF_DATA_RATE_2Mbps,RF_TX_POWER_0dBm);
		
		STATUS_LED = 1; delay_ms(250); STATUS_LED = 0; delay_ms(250);//2
    hal_w2_configure_master(HAL_W2_400KHZ); //I2C
			
		STATUS_LED = 1; delay_ms(250); STATUS_LED = 0; delay_ms(250);//3
    initial_setup_dmp();//MPU_6050 and DPM
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
                    DataAcq();
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
										send_inertial_packet_by_rf(PACKET_TYPE_QUAT,fifoBuffer,MY_SUB_ADDR);
										break;
										default:
                    //Inverte o led indicando que recebeu um comando desconhecido ou nao implementado
                    STATUS_LED = 1; delay_ms(500); STATUS_LED = 0; delay_ms(500);
                    //STATUS_LED = !STATUS_LED;
                    break;
                }/*END SWITCH*/
            }/*END IF MY SUB ADDR*/
        } /* END Comunicacao RF */
    }/*END LOOP*/
}/*END MAIN*/

///////////////////////
//FUNCIONS in Sensor //
///////////////////////

void initial_setup_dmp() large {
    uint8_t ret;
		
		STATUS_LED = 1; delay_ms(250); STATUS_LED = 0; delay_ms(250);//4
		mpu_8051_malloc_setup(); //Malloc pool for mpu library

		STATUS_LED = 1; delay_ms(250); STATUS_LED = 0; delay_ms(250);//5
		ret = (uint8_t) mpu_testConnection();

		STATUS_LED = 1; delay_ms(250); STATUS_LED = 0; delay_ms(250);//6
    if(ret){
			STATUS_LED = 1; delay_ms(250); STATUS_LED = 0; delay_ms(250);//7
      mpu_initialize(); //Initializes the IMU
			
			STATUS_LED = 1; delay_ms(250); STATUS_LED = 0; delay_ms(250);//8
      ret =  dmpInitialize();  //Initializes the DMP
			
			STATUS_LED = 1; delay_ms(250); STATUS_LED = 0; delay_ms(250);//9
      delay_ms(50);
			
			STATUS_LED = 1; delay_ms(250); STATUS_LED = 0; delay_ms(250);//10
      if(ret == 0)
      {
				STATUS_LED = 1; delay_ms(250); STATUS_LED = 0; delay_ms(250);//11
        setDMPEnabled(true);
				
				STATUS_LED = 1; delay_ms(250); STATUS_LED = 0; delay_ms(250);//12
			/*Calibrated at 03 Mai 2017*/
      setXAccelOffset(-520);
      setYAccelOffset(632);
      setZAccelOffset(914);
      setXGyroOffset(22);
      setYGyroOffset(-8);
      setZGyroOffset(26);
				STATUS_LED = 1; delay_ms(250); STATUS_LED = 0; delay_ms(250);//13
       }
    }
}



void DataAcq() large {
    uint8_t i = 0;
    numbPackets = getFIFOCount() / PSDMP;//floor
    for (i = 0; i < numbPackets; i++) {
        getFIFOBytes(fifoBuffer, PSDMP);  //read a packet from FIFO
    }/*END for every packet*/
		
		tx_buf[0] = MY_SUB_ADDR;
		tx_buf[1] = fifoBuffer[0];
    tx_buf[2] = fifoBuffer[1];//W_quat
    tx_buf[3] = fifoBuffer[4];
    tx_buf[4] = fifoBuffer[5];//X_quat
    tx_buf[5] = fifoBuffer[8];
    tx_buf[6] = fifoBuffer[9];//Y_quat
    tx_buf[7] = fifoBuffer[12];
    tx_buf[8] = fifoBuffer[13];//Z_quat
    TX_Mode_NOACK(9);
		RX_Mode();
}/*End of DataAcq*/

//interrupção do I2C - NOT USED
void I2C_IRQ (void) interrupt INTERRUPT_SERIAL {
    I2C_IRQ_handler();
}
