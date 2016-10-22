/***********************************************/
/* I2C Master example code
/* Send 100 indentical bytes
/* Read back 100 bytes
/* Verify if they are increased by 1
/************************************************/

#include "nrf24le1.h"
#include <hal_w2_isr.h>
#include "hal_delay.h"

uint8_t flag =0;

void delay_timer0(void);

void main()
{
	uint8_t i;
	xdata uint8_t data_string[100];
	//Indicate startup process by flashing the LED1 

//*************************** Init GPIO Pins	
	P0DIR = 0x00;
	P1DIR = 0xFF;
	P2DIR = 0xFF;
	P3DIR = 0xFF;
//*************************** Init I2C
  hal_w2_configure_master(HAL_W2_400KHZ);
 	data_string[0]=0;
	EA= 1;
	i=0;
	
	while(1)	
	{
	
		//store data_string[0] for verification.
		flag=data_string[0];
		//Sending 100 indentical bytes to slave 
		//P0=0xFF;
		hal_w2_write(0x07, data_string, 100);
		//Reading 100 byte from slave (after increased value by 1 on the slave)
		//P0=0xAA;
        hal_w2_read(0x07,data_string,100);
		
		P0=data_string[0]; //Output the byte (optional)
	
		//Verification
		flag++;
		for (i=0;i<99; i++) 
		if (data_string[i]!=flag)//not matched
		{
			P0=0xAA;while(1); 
		}
	}
}



void I2C_IRQ (void) interrupt INTERRUPT_SERIAL
{

	I2C_IRQ_handler();
}