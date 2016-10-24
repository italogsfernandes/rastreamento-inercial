#include "reg24le1.h" //registers address
#include "stdint.h" //inteiros uint8_t, int8_t, uint16_t....
#include "stdbool.h" //Booleanos
//#include "IIC_app.h" //i2c
#include "intrins.h"
#include "API.h"
#include "nRF-SPIComands.h" //radio commands

#define MASTER 0X02
#define SLAVE  0x00

#define READY (W2CON1&0X01)	  //if ready ==1,not ready==0
#define ACK   (W2CON1&0X02 )  //1;no ack and 0 ack
#define EN2WIRE()  W2CON0|=0x01;//enable 2 wire
#define DISABLE2WIRE() 	W2CON0&=0xFE;
#define STOP()    W2CON0|=0x20;
#define START()   W2CON0|=0x10;
#define FREQSEL(x)  W2CON0|=(x<<2);
#define MODE(x)	   W2CON0&=(0xff-0x02);W2CON0|=x;  //master or slave



//Subendere?os usados no sistema
#define MY_SUB_ADDR 0x01
#define OTHER_SUB_ADDR 0x02
//pacote para enviar:
// pacote = [sub_endere?o_destinatario] AXH	AXL	AYH	AYL	AZH	AZL
//Pacote para receber
// pacote = [MY_SUB_ADDR] [COMANDO]
#define Sinal_request_data 0x0A
#define Sinal_LEDS 0x0B

//Endereço I2C do sensor em 3.3V
#define MPU_endereco 0x68

//Pushbuttons
sbit S1  = P0^2;    // 1/0=no/press
sbit S2  = P1^4;    // 1/0=no/press
//LEDS
sbit LED1 = P0^3; // 1/0=light/dark
sbit LED2 = P0^6; // 1/0=light/dark

//Onde as leituras serão salvas:
uint8_t readings[6] = {0,5,0,128,0,255};
int i=0;

void delay_ms(unsigned int x);
void setup_i2c_mpu(void);
void requisitarAccelMPU6050(void);
void enviar_pacote_inercial(void);
void luzes_iniciais(void);

void I2C_init(void){

	//original
    FREQSEL(2);
    MODE(MASTER);
	//NOTE: ja tentei trocar isso pro 0xDF e colocar um  &
    W2CON1|=0x20;     //�������е��ж�
    W2SADR=0x00;
    EN2WIRE();        //ʹ��2-wire
}
bool i2c_mpu_writeBytes(uint8_t devAddr, uint8_t regAddr, uint8_t data_len, uint8_t *data_ptr) {
    bool ack_received;
	uint8_t numlimit = 0;
    START();
    W2DAT=((devAddr+0xa0)<<1)+0;//write
    if(!ACK){ //IF ACK
        W2DAT=regAddr;
    }
    //BUG: antes isso era um if? ue?
    while(!ACK && data_len-- > 0) {
        W2DAT=*data_ptr++ ;
        numlimit++;
        if(numlimit==16){
            return false;
        }
    }
    ack_received = !ACK;
    STOP();
    return ack_received;
}
bool i2c_mpu_readBytes(uint8_t devAddr, uint8_t regAddr, uint8_t data_len, uint8_t *data_ptr) {
    START();
    W2DAT=((devAddr+0xa0)<<1)+0;//write from slave
	if(!ACK){ W2DAT=regAddr;}else{return false; }
    if(!ACK){ START();}else{return false;}

    W2DAT=((devAddr+0xa0)<<1)+1;//read from slave
    //if(ACK){return false;}
	tx_buf[0] = MY_SUB_ADDR;
	i = 1;
    while(data_len-- > 0)
    {
		//BUG: XXX: O programa esta parando neste la�o
       while(!(W2CON1&0X01)){
		LED1 = 1;
		}
		LED1 = 0;
		tx_buf[i] = W2DAT;
		i++;
        //*data_ptr++=W2DAT;
    }
   	TX_Mode_NOACK(i+1);
    RX_Mode();
    STOP();
    return true;
}

uint8_t readbyte(uint8_t devAddr, uint8_t regAddr)
{
	  uint8_t data_read;
    START();
    W2DAT=((devAddr+0xa0)<<1)+0;//write from slave
    while(ACK);
    W2DAT=regAddr;
    while(ACK);
    START();
    W2DAT=((devAddr+0xa0)<<1)+1;//read from slave
    //while(ACK);
   // while(!READY){
		LED1 = 1;
	//}
	LED1 = 0;
    data_read=W2DAT;
    STOP();
    return data_read;

}

void setup(void){
	 // Set up GPIO
    P0DIR = 0xB7;   // 1011 0111 - 1/0 = In/Out - Output: P0.3 e P0.6
    P1DIR = 0xFF;   // Tudo input
    P2DIR = 0xFF;
    P0CON = 0x00;  	// All general I/O
    P1CON = 0x00;  	// All general I/O
    P2CON = 0x00;  	// All general I/O

	// Radio + SPI setup
    RFCE = 0;       // Radio chip enable low
    RFCKEN = 1;     // Radio clk enable
    RF = 1;
    rf_init();
    EA=1; //ativa as interrup��es
    RX_Mode();
	luzes_iniciais();
	//I2C_SETUP
	I2C_init();

	//Io_config(); //XXX: realmente necessaria
	//ex_int(); //XXX: realmente necessaria
	//IIC_init();//initial iic
	setup_i2c_mpu();
	//requisitarAccelMPU6050();
	EA = 1;
}
void main()
{
    //lógica:
    //iniciar i2c
    //configurar sensores
    //iniciar rf
    //Ao se precionar o botão
    //  realizar leitura
    //  despachar leitura
	setup();
	while (1) {
		if(!S1){
			enviar_pacote_inercial();
			delay_ms(100); //evita ruidos
			while(!S1); //espera soltar o botao
			delay_ms(100);
		}
		if(!S2){
			requisitarAccelMPU6050();
			delay_ms(100);
			while(!S2);//espera soltar o botao
			delay_ms(100);
		}
		if(newPayload){
			if(rx_buf[0] == MY_SUB_ADDR){
				switch (rx_buf[1]) {
					case Sinal_request_data:
						requisitarAccelMPU6050();
						enviar_pacote_inercial();
						break;
					case Sinal_LEDS:
						LED1 = !LED1;
						LED2 = !LED2;
						break;
				}
			}
			newPayload = 0;
			sta = 0;
		}
	}
}
/**************************************************/
void delay_ms(unsigned int x)
{
    unsigned int i,j;
    i=0;
    for(i=0;i<x;i++)
    {
       j=508;
           ;
       while(j--);
    }
}
/**************************************************/
void luzes_iniciais(void){
        LED1 = 1; LED2 = 0;
        delay_ms(1000);
        LED1 = 0; LED2 = 1;
        delay_ms(1000);
        LED1 = 1; LED2 = 1;
        delay_ms(1000);
        LED1 = 0; LED2 = 0;
}
/**************************************************/
/************MPU**********************************/
void setup_i2c_mpu(void){
    //iniciar i2c
    //Set the register Power Management to start
	uint8_t ligar_mpu = 0x00;
	i2c_mpu_writeBytes(MPU_endereco, 0x6B,1, &ligar_mpu);
}

void requisitarAccelMPU6050(void){
    //Ler 6 bytes a partir de 0x3B
    //Sendo esses:
    // [ACCEL_XOUT_H] [ACCEL_XOUT_L]
    // [ACCEL_YOUT_H] [ACCEL_YOUT_L]
    // [ACCEL_ZOUT_H] [ACCEL_ZOUT_L]
   	//LED2 = i2c_mpu_readBytes(MPU_endereco,0x3B, 6,readings);
	uint8_t initial_regAddr = 0x3B;

	for(i = 0;i<6; i++){
		readings[i] = readbyte(MPU_endereco, initial_regAddr+i);
	}
}

void enviar_pacote_inercial(void){
    LED2 = 1;
    tx_buf[0] = MY_SUB_ADDR;
    for(i=1;i<7;i++){
        tx_buf[i] = readings[i-1];
    }
    //enviando e retornando ao padrao:
    TX_Mode_NOACK(7);
    RX_Mode();
	LED2 = 0;
}
