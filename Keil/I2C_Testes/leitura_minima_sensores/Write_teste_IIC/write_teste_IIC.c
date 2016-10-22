#include "reg24le1.h" //Definições de muitos endereços de registradores.
#include "stdint.h" //inteiros uint8_t, int8_t, uint16_t....
#include "stdbool.h" //Bolleanos

#include "intrins.h"
 /*************General********************/
void delay_ms(unsigned int x);
void luzes_iniciais(void);
//Pushbuttons
sbit S1  = P0^2;    // 1/0=no/press
sbit S2  = P1^4;    // 1/0=no/press
//LEDS
sbit LED1 = P0^3; // 1/0=light/dark
sbit LED2 = P0^6; // 1/0=light/dark


/*********LIRBARY***********/
#define startaddr 0x00
#define endaddr   0x10 
#define slaveaddr 0x07    //define a slave device's address 

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


void IIC_init(void)
{
    EN2WIRE();
	//FREQSEL(0x01);
    W2CON0 &= 0xF3;	  // Frequência
	W2CON0 |= 0x04;
	W2CON0 |= 0x02;      // Master select
    W2CON1 = 0x00;  //maskirq
    W2SADR = 0x00;
}
void ex_int(void)
{
    //IEN0|=0X80;
    //IEN0|=0X01;
    //TCON|=0X01;       //?½??????
	////INTEXP|=0x04; 	  //??P05???????
	//IEN1 |= 0x04;
}

void writebyte(unsigned int addr,unsigned char dat)
{
    START();
    W2DAT=((addr)<<1)+0;//write
	//while(ACK);
    W2DAT=dat;
	//while(ACK);
    STOP();
}
/*************END-LIBRARY***************/

void setup(void){
    // Set up GPIO
    P0DIR = 0x87;   // 1011 0111 - 1/0 = In/Out - Output: P0.3 e P0.6
    P1DIR = 0xFF;   // Tudo input
    P2DIR = 0xFF;
    P0CON = 0x00;  	// All general I/O
    P1CON = 0x00;  	// All general I/O
    P2CON = 0x00;  	// All general I/O
	P04 = 1;
	P05 = 1;

	IEN0|=0X80;
    IEN0|=0X01;
    TCON|=0X01;       //?½??????
	INTEXP|=0x04; 	  //??P05???????
	IEN1 |= 0x04;

	//ex_int();
	//IIC_init();
    luzes_iniciais();
}
void main(void){
  setup();
  while(1){
	if(!S1){
		//WDCON &= 0x7f; //ï¿½Ø±Õ´ï¿½ï¿½ï¿½
		IIC_init();//initial iic
		delay_ms(100);
		writebyte(0x07,0x98);
		LED1 = !LED1;
		delay_ms(100); //evita ruidos
		while(!S1); //espera soltar o botao
		delay_ms(100);
	}
    if(!S2){
		LED2 = !LED2;
		LED1 = !LED1;
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
/****************END-General*************************/
