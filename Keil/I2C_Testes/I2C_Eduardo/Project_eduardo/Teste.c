#include "reg24le1.h"
#include "I2CDev.h"
#include <stdint.h>

//  Pushbuttons
sbit S1  = P0^2;    // 1/0 = no/press
sbit S2  = P1^4;    // 1/0 = no/press
//  LEDS
sbit LED1 = P0^3;   // 1/0 = light/dark
sbit LED2 = P0^6;   // 1/0 = light/dark

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

void main()
{
  luzes_iniciais();

  // Init GPIO Pins
  P0DIR = 0x00;
	P1DIR = 0xFF;
	P2DIR = 0xFF;
	P3DIR = 0xFF;

  // Enable all interrupts
  EA = 1;

  // Configurações iniciais
  P05 = 1;
  P06 = 1;
//NOTE: oq ta havendo aqui?
  IEN0 |= 0X80;
  IEN0 |= 0X01;
  //NOTE: Por que nao usar a funcao de frequencia da biblioteca?
  W2CON0 |= 0x04;     // Clock frequency, STD mode
  W2CON0 &= 0xF7;
  W2SADR=0x00;

  // Set master select
  operationMode(true);

  //TODO: to ativando as interrupções mas nao tem interrupt handle?
  //ue?
  // Enable all interrupts for I2C
  enableISR(true);


  // Enable I2C
  enableI2C(true);

  //TCON |= 0X01;
  //INTEXP |= 0x04;

  while(1)
	{
	   if(!S1)
      {
  			// Initial transfer
        startI2C(); //NOTE: isso ta desativando as interrupções(ligando a mascara)
        W2DAT = ((0x68 << 1) | 0);    // 0 = WRITE
        while(ACK);
        stopI2C();
  			delay_ms(100); //evita ruidos

        startI2C();
        W2DAT = ((0x68 << 1) | 0);    // 0 = WRITE
        W2DAT = 0x00; //XXX: como a mascara ta ligada, precisaria verificar o
        //ack para nao ocorrer encavalamento de dados?
        W2DAT = 0xFF;
        stopI2C();


  			while(!S1); //espera soltar o botao
  			delay_ms(100);
		  }
		  if(!S2)
      {
  			LED1 = !LED1;
        LED2 = !LED2;
  			delay_ms(100); //evita ruidos
  			while(!S2); //espera soltar o botao
  			delay_ms(100);
		  }

	}
}
