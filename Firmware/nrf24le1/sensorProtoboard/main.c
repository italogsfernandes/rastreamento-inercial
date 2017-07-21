//Subenderecos usados no sistema
#define MY_SUB_ADDR 0x01

#include "nrf24le1.h"
#include "reg24le1.h" //Definiï¿½ï¿½es de muitos endereï¿½os de registradores.
#include "nRF-SPIComands.h" //Comunicacao RF
#include "hal_w2_isr.h" //Comunicacao I2C
#include "hal_delay.h" //delay
#include "pacotes_inerciais.h" //pacotes para enviar
#include "dmp.h" //configuracao e uso da dmp da mpu6050
#include "timer0.h" //timer for aquisition

#define STATUS_LED  P10
#define PSDMP 42

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

void pisca_led(){
    STATUS_LED = 1; delay_ms(250); STATUS_LED = 0; delay_ms(250);
}
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
    pisca_led(); //primeira piscada
    rf_init(ADDR_HOST,ADDR_HOST,10,RF_DATA_RATE_2Mbps,RF_TX_POWER_0dBm);
    pisca_led(); //segunda piscada
    hal_w2_configure_master(HAL_W2_100KHZ);
    pisca_led(); //terceira piscada
    initial_setup_dmp();//MPU_6050 and DPM
    pisca_led(); //13
    setup_T0_ticks(6667, 1); //Timer of 200Hz for Aquisition
    pisca_led(); //14
    start_T0();
}

void main(void) {
    setup();
    while(1){ //Loop
        /////////////////////
        //Timer Aquisition //
        /////////////////////
        if(timer_elapsed){
            timer_elapsed = 0;
            numbPackets = getFIFOCount() / PSDMP;//floor
            while (numbPackets > 0) {
                getFIFOBytes(fifoBuffer, PSDMP);  //read a packet from FIFO
                numbPackets--;
            }
            STATUS_LED = !STATUS_LED;
        }
        ////////////////////
        //Comunicacao RF //
        ////////////////////
        if(newPayload){
            sta = 0;
            newPayload = 0;
            //verifica se o sinal eh direcionado para mim
            if(rx_buf[0] == MY_SUB_ADDR){
                switch(rx_buf[1]){
                    case CMD_READ:
                    tx_buf[0] = MY_SUB_ADDR;
                    tx_buf[1] = fifoBuffer[MOTIONAPPS_FIFO_I_ACCEL_XH];
                    tx_buf[2] = fifoBuffer[MOTIONAPPS_FIFO_I_ACCEL_XL];//X_AC
                    tx_buf[3] = fifoBuffer[MOTIONAPPS_FIFO_I_ACCEL_YH];
                    tx_buf[4] = fifoBuffer[MOTIONAPPS_FIFO_I_ACCEL_YL];//Y_AC
                    tx_buf[5] = fifoBuffer[MOTIONAPPS_FIFO_I_ACCEL_ZH];
                    tx_buf[6] = fifoBuffer[MOTIONAPPS_FIFO_I_ACCEL_ZL];//Z_AC
                    tx_buf[7] = fifoBuffer[MOTIONAPPS_FIFO_I_GYRO_XH];
                    tx_buf[8] = fifoBuffer[MOTIONAPPS_FIFO_I_GYRO_XL];//X_GY
                    tx_buf[9] = fifoBuffer[MOTIONAPPS_FIFO_I_GYRO_YH];
                    tx_buf[10] = fifoBuffer[MOTIONAPPS_FIFO_I_GYRO_YL];//Y_GY
                    tx_buf[11] = fifoBuffer[MOTIONAPPS_FIFO_I_GYRO_ZH];
                    tx_buf[12] = fifoBuffer[MOTIONAPPS_FIFO_I_GYRO_ZL];//Z_GY
                    tx_buf[13] = fifoBuffer[MOTIONAPPS_FIFO_I_QUAT_WH];
                    tx_buf[14] = fifoBuffer[MOTIONAPPS_FIFO_I_QUAT_WL];//W_quat
                    tx_buf[15] = fifoBuffer[MOTIONAPPS_FIFO_I_QUAT_XH];
                    tx_buf[16] = fifoBuffer[MOTIONAPPS_FIFO_I_QUAT_XL];//X_quat
                    tx_buf[17] = fifoBuffer[MOTIONAPPS_FIFO_I_QUAT_YH];
                    tx_buf[18] = fifoBuffer[MOTIONAPPS_FIFO_I_QUAT_YL];//Y_quat
                    tx_buf[19] = fifoBuffer[MOTIONAPPS_FIFO_I_QUAT_ZH];
                    tx_buf[20] = fifoBuffer[MOTIONAPPS_FIFO_I_QUAT_ZL];//Z_quat
                    TX_Mode_NOACK(21);
                    RX_Mode();
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
                    tx_buf[0] = MY_SUB_ADDR;
                    tx_buf[1] = 0x00; tx_buf[2] = 0x01; tx_buf[3] = 0x02;
                    tx_buf[4] = 0x03; tx_buf[5] = 0x04; tx_buf[6] = 0x05;
                    tx_buf[7] = 0x06; tx_buf[8] = 0x07; tx_buf[9] = 0x08;
                    tx_buf[10] = 0x09; tx_buf[11] = 0x0A; tx_buf[12] = 0x0B;
                    tx_buf[13] = 0x0C; tx_buf[14] = 0x0D; tx_buf[15] = 0x0E;
                    tx_buf[16] = 0x0F; tx_buf[17] = 0x10; tx_buf[18] = 0x11;
                    tx_buf[19] = 0x12; tx_buf[20] = 0x13;
                    TX_Mode_NOACK(21);
                    RX_Mode();
                    break;
                    default:
                    //Pisca Led caso for um comando desconhecido
                    pisca_led();
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
    mpu_8051_malloc_setup(); //Malloc pool for mpu library
    pisca_led();//4
    ret = (uint8_t) mpu_testConnection();
    pisca_led();//5
    if(ret){
        pisca_led();//6
        mpu_initialize(); //Initializes the IMU
        pisca_led();//7
        ret =  dmpInitialize();  //Initializes the DMP
        pisca_led();//8
        delay_ms(50);
        pisca_led();//9
        if(ret == 0){
            pisca_led();//10
            setDMPEnabled(true);
            pisca_led();//11
            /*Calibrated at 03 Mai 2017*/
            setXAccelOffset(-520);
            setYAccelOffset(632);
            setZAccelOffset(914);
            setXGyroOffset(22);
            setYGyroOffset(-8);
            setZGyroOffset(26);
            pisca_led();//12
        }
    }
}


//interrupção do I2C - NOT USED
void I2C_IRQ (void) interrupt INTERRUPT_SERIAL {
    I2C_IRQ_handler();
}
