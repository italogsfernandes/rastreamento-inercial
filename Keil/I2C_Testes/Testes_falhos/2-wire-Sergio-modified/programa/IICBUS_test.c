#include "reg24le1.h"
#include "IIC_app.h"
#include "intrins.h"

//  Pushbuttons
sbit S2  = P1^4;    // 1/0 = no/press


void italo_delay_ms(unsigned int x)
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

void main()
{
    unsigned int i;
    unsigned char flag=0;
    Io_config();
    //uart_init();
    ex_int();
    //puts("--just a iic test by syman--2010,9,10\n");
    WDCON &= 0x7f;
    IIC_init();//initial iic
    while(1)
    {        if(!S2){
            writebyte(0x07,0xA2);
			italo_delay_ms(100);
			while(!S2); //espera soltar o botao
  			italo_delay_ms(100);
		}

    }
