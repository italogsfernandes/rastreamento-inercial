/******************************************************/
/* I2C Slave example code
/* Wait for data, increase the first byte by 1,
/* Send back the first byte increased when requested.
/******************************************************/

#include "nrf24le1.h"
#include <hal_w2.h>
uint8_t flag =0;
uint8_t flag2 =0;
uint8_t int_count=0;
uint8_t TX =0;
uint8_t first_byte=0;
void main()
{

//	uint8_t data_string[3];
	uint8_t w2con1_value;
	uint8_t W2DAT_value;
	flag=0;
	//*************************Setup Timer0
	TMOD |= 0x01;	//16 bit timer
	ET0=1; //Enable interupt 
	EA= 1;
//*************************** Init GPIO Pins	
	P0DIR = 0x00;
	P1DIR = 0xFF;
	P2DIR = 0xFF;	
	P3DIR = 0xFF;

//*************************** Init I2C
	
	hal_w2_enable(1);  
 	hal_w2_set_clk_freq(HAL_W2_400KHZ);
 	hal_w2_set_op_mode(HAL_W2_SLAVE);
	hal_w2_set_slave_address(0x07);// Slave ADDRESS 0x07
	hal_w2_alter_clock(1);
	hal_w2_irq_adr_match_enable(1);
	hal_w2_irq_stop_cond_enable(0);
	INTEXP |= 0x04; 
	IEN1 |= 0x04;
	P0=0xAA;
	flag=0;
	while(1){P0=flag;}
	

}

void Timer0_IRQ(void) interrupt 1
{
	flag++;
	TL0=0;
	TH0=0xFF;
}

void I2C_IRQ (void) interrupt INTERRUPT_SERIAL
{
	uint8_t w2con1_value;
	uint8_t w2dat_value;
	w2con1_value = W2CON1;
 	if ((w2con1_value & 0x04) )	   //Interrupt for address matched
	{
		// Check if RX or TX
		int_count++;
		if (W2DAT&0x01) //TX
		{
			TX=1; //TX
			W2DAT=flag;
		}
		else //RX
		{
			TX=0; //RX
			first_byte=1;
		 }
	}
	else  
	{
		if ((w2con1_value & 0x01)) //  Interrupt for byte sent or received 
		{
			if (TX==1) //next byte to send
			{
				if (!(w2con1_value & 0x02))	// if not NAK
				{
						W2DAT=flag;
				}
			}
			else //byte to receive
			{
				w2dat_value=W2DAT;
				//get the value of the first byte and increase by 1 
				if (first_byte) {flag=w2dat_value+1;first_byte=0;}
			}
				
		}
	}		   
	

 }

