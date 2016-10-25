/* Explicação:
  - Configura I2C e RF
  - Caso btn1 seja apertado:
          escreve um valor qlqr(0x102) no barramento i2c
  - Caso btn2 seja apertado:
          le um valor e manda para o radio
  - No arduino:
        - redireciona i2c received para serial0
        - redireciona serial0 para i2c sending
*/
#include "reg24le1.h" //registers address
#include "stdint.h" //inteiros uint8_t, int8_t, uint16_t....
#include "stdbool.h" //Booleanos
#include "intrins.h"
#include "API.h"

/*************************LIBRARY********************/
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

void I2C_init(void);
uint8_t wire_read(uint8_t devAddr);
bool wire_write(uint8_t devAddr, uint8_t value_to_write);
#define arduino_endereco 0x68
/***********************END-LIBRARY******************/

/*************General********************/
void delay_ms(unsigned int x);
void luzes_iniciais(void);
//Pushbuttons
sbit S1  = P0^2;    // 1/0=no/press
sbit S2  = P1^4;    // 1/0=no/press
//LEDS
sbit LED1 = P0^3; // 1/0=light/dark
sbit LED2 = P0^6; // 1/0=light/dark

void setup(void){
    // Set up GPIO
    P0DIR = 0xB7;   // 1011 0111 - 1/0 = In/Out - Output: P0.3 e P0.6
    P1DIR = 0xFF;   // Tudo input
    P2DIR = 0xFF;
    P0CON = 0x00;  	// All general I/O
    P1CON = 0x00;  	// All general I/O
    P2CON = 0x00;  	// All general I/O

    //I2C
    I2C_init();
    //Finishing
    EA = 1;
    luzes_iniciais();
}

void main(void){
  setup();
  while(1){
	if(!S1){
		wire_write(arduino_endereco,0x102);
		delay_ms(100); //evita ruidos
		while(!S1); //espera soltar o botao
		delay_ms(100);
	}
    if(!S2){
		if(wire_read(arduino_endereco)==0x05){
		LED2 = !LED2;
		} else {
		LED1 = !LED1;
		}
		delay_ms(100); //evita ruidos
		while(!S2); //espera soltar o botao
		delay_ms(100);
    }
  }
}
/****************General*************************/
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
/***************WIRE LIBRARY*********************/
void I2C_init(void){
    //original
    FREQSEL(0x02); //W2CON0|=(0x01<<2) fast mode: 400KHz
    MODE(MASTER); //W2CON0&=(0xfd);W2CON0|=0x02;  //master or slave
    //NOTE: ja tentei trocar isso pro 0xDF e colocar um  &
    W2CON1|=0x20; //disable all interrupts
    W2SADR=0x00; //endere�o = 0x00
    EN2WIRE();  //W2CON0|=0x01;//enable 2 wire
}
uint8_t wire_read(uint8_t devAddr){
    uint8_t data_read;
    START();
    W2DAT=((devAddr)<<1)+1;//read from slave
    while(ACK);
    while(!READY){
      LED1 = 1;
    }
    LED1 = 0;
    data_read=W2DAT;
    STOP();
    return data_read;
}
bool wire_write(uint8_t devAddr,uint8_t value_to_write){
    bool ack_received;
	uint8_t w2_status = 0;
	uint8_t data_ready = 0;

    START(); //W2CON0|=0x10;
    W2DAT=((devAddr)<<1)+0;//write
	//wait data ready

    while (!data_ready){
		w2_status = W2CON1;
		data_ready = (w2_status & 0x01);
	}
    return w2_status;

    while(ACK);
	if(!ACK){ //IF ACK
        W2DAT=value_to_write;
    }
    ack_received = !ACK;
    STOP();	//W2CON0|=0x20;
    return ack_received;
}
/***************END - WIRE LIBRARY*********************/
