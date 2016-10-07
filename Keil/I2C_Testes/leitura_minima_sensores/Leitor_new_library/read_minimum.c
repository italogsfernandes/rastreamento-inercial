#include "reg24le1.h" //registers address
#include "stdint.h" //inteiros uint8_t, int8_t, uint16_t....
#include "stdbool.h" //Booleanos
//#include "IIC_app.h" //i2c
#include "intrins.h"
#include "API.h"
#include "nRF-SPIComands.h" //radio commands

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
	//Io_config(); //XXX: realmente necessaria
	//ex_int(); //XXX: realmente necessaria
	//IIC_init();//initial iic
	//setup_i2c_mpu();
	//requisitarAccelMPU6050();
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
			//requisitarAccelMPU6050();
			delay_ms(100);
			while(!S2);//espera soltar o botao
			delay_ms(100);
		}
		if(newPayload){
			if(rx_buf[0] == MY_SUB_ADDR){
				switch (rx_buf[1]) {
					case Sinal_request_data:
						//requisitarAccelMPU6050();
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
//void setup_i2c_mpu(void){
//    //iniciar i2c
//    //Set the register Power Management to start
//	i2c_mpu_writeByte(MPU_endereco, 0x6B, 0x00);
//}
//
//void requisitarAccelMPU6050(void){
//    //Ler 6 bytes a partir de 0x3B
//    //Sendo esses:
//    // [ACCEL_XOUT_H] [ACCEL_XOUT_L]
//    // [ACCEL_YOUT_H] [ACCEL_YOUT_L]
//    // [ACCEL_ZOUT_H] [ACCEL_ZOUT_L]
//    i2c_mpu_readBytes(MPU_endereco,0x3B, 6,readings);
//}

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
