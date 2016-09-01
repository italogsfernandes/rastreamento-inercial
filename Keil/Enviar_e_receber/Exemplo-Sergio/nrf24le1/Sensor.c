#include <intrins.h>
#include <reg24le1.h>
#include <stdint.h>
#include <stdbool.h>
#include "API.h"
#include "app.h"
#include "nRF-SPIComands.h"

#define INTERRUPT_UART0 4   								// UART0, Receive & Transmitt interrupt
#define INTERRUPT_TMR0	1

#define BROADCAST				0										// Endere�o 0 indica transmiss�o BROADCAST
#define TAM_FIFO 				120	  							// Tamanho da FIFO 100 bytes
#define MYADDR					0x09								// Endere�o deste sensor. Deve seguir o vetor ADDR_SENSOR
#define N_SAMPLE_PYL	  13									// Number of samples in 16 bits on RF Payload
#define N_BYTES_CFG      6

/*Macros removed with the use of the corresponding bit for example: determining variable A minimum bits, you can, if (A & BIT0)*/

#define BIT0  0x01
#define BIT1  0x02
#define BIT2  0x04
#define BIT3  0x08
#define BIT4  0x10
#define BIT5  0x20
#define BIT6  0x40
#define BIT7  0x80

#define	PIN32
#ifdef 	PIN32
sbit S2 = P1^4;                               	// 1/0=no/press
sbit S1 = P0^2;                               	// 1/0=no/press

sbit LED_VD = P0^6;                             // 1/0=light/dark
sbit LED_VM = P0^3;                             // 1/0=light/dark
#endif

unsigned char pdata FIFO[TAM_FIFO];									// Define FIFO software
unsigned char index_in = 0;
unsigned char index_out = 0;
unsigned char nDataFIFO = 0;
bit FIFOempty = 1;

unsigned char infPld = 0;												// Information about the payload in the next transmition

/**************************************************/
// Vari�veis do TMR0
unsigned char NBT0H  = 0xE5;			// Este tempo
unsigned char NBT0L  = 0xF6;			// equivale a
unsigned char NOVT0  = 0x00;			// Freq. de Amostragem
unsigned char NPRT0H = 0x00;			// de 200Hz
unsigned char NPRT0L = 0x00;			//
unsigned char count;

/**************************************************/
void start_T0(void)
{
	TMOD=0x31;						// Select Timer 1 --> STOPPED, Timer 0 --> TIMER/16 bits
	TH0= NBT0H;
	TL0= NBT0L;
	ET0=1;								// Active interrupt on Timer 0
	EA=1;									// Active all interrupts
	TR0=1;								// Timer 0 --> RUN
}
/**************************************************/
void stop_T0(void)
{
	TMOD=0x31;						// Select Timer 1 --> STOPPED, Timer 0 --> TIMER/16 bits
	TH0= NBT0H;
	TL0= NBT0L;
	ET0=0;								// Active interrupt on Timer 0
	EA=1;									// Active all interrupts
	TR0=0;								// Timer 0 --> RUN
}

/**************************************************/
//** AD feature configuration function *** /
//* Function Name: adc_config (); no parameters passed, used to initialize the AD conversion * /
void adc_init(void)
{
	ADCCON2  = 0x00; // set to single-step conversion and energy, speed 2ksps
	ADCCON3 |= 0xE0; // precision 12bit, the data right justified
	ADCDATH &= 0xF0; // conversion result register is cleared
	ADCDATL &= 0x00;
	P0DIR   |= 0x07; // Set the input channel converted to 0,1,2
	P0      &= 0xF8; // initialize port is low
}

/**************************************************/
unsigned char popFIFO()
{
	unsigned char	 aux;

	if(nDataFIFO > 0)
	{
		//LED_VM=!LED_VM;							//only one bit
		aux = FIFO[index_out];
		index_out++;
		if(index_out == TAM_FIFO)
			index_out = 0;
		nDataFIFO--;
		return aux;
	}
	else
	{
		FIFOempty = 1;
		return 0;
	}
}
/**************************************************/
void pushFIFO(unsigned char dado)
{
	if(nDataFIFO == TAM_FIFO)
		popFIFO();
	FIFO[index_in] = dado;
	index_in++;
	if(nDataFIFO<TAM_FIFO)
		nDataFIFO++;
	if(index_in == TAM_FIFO)
		index_in = 0;
	FIFOempty = 0;
//	LED_VD=!LED_VD;							//only one bit
}

/**************************************************/
void delay_time(unsigned long int atr)
{
	atr = 2 * atr;
	while(atr!=0)
	{
    _nop_();
		atr--;
	}
}
/**************************************************/
void delay(unsigned int x)
{
    unsigned int i,j;
    i=0;
    for(i=0;i<x;i++)
    {
       j=508;
       while(j--);
    }
}
/**************************************************/
//* Read AD conversion result performance function, pip_num save the channel number, returns AD conversion result * /
//* Defined static variables through loop reads 0.1.2 three channels * /
void readADCtoFIFO (unsigned char num)
{
		//unsigned int res = 0;
		//static unsigned char num = 0;
		ADCCON1 = BIT7 + (num << 2) + BIT0; // set the conversion of the channel, set the reference VDD, and start
		while (!(ADCCON1 & BIT6)); // wait for start
		while ((ADCCON1 & BIT6)); // wait for the completion of the conversion
		pushFIFO(ADCDATH & 0x0F);
		pushFIFO(ADCDATL);
}

/**************************************************/
void TMR0_IRQ(void) interrupt INTERRUPT_TMR0
{
	if(!NOVT0)
	{
		//LED_VD=1;
		TH0= NBT0H;
		TL0= NBT0L;
		readADCtoFIFO(0); 					// read the AD conversion result of the selected channel
		LED_VD=!LED_VD;							//only one bit
	}
	else													// Neste caso TH0=TL0=0
	{
		if(count==0)
		{
	//		LED_VD=1;
			TH0= NPRT0H;
			TL0= NPRT0L;
			count=NOVT0;
		//	LED_VD=!LED_VD;						//only one bit
			readADCtoFIFO(0); 				// read the AD conversion result of the selected channel
			LED_VD=!LED_VD;
		}
		else
		{
			count--;
		}
	}
}

/***************************************************/
//          - - >    M A I N    < - -
/**************************************************/
void main(void)
{
	unsigned char BioSampleH,BioSampleL, i, NDataPyl, j=0;;
	count = NOVT0;

	// Set up GPIO
	P0DIR = 0xB7;                 // Output: P0.3 e P0.6
	P1DIR = 0xFF;                 // Output: P0.0 - P0.2, Input: P0.3 - P0.5	 0xFF
	P2DIR = 0xFF;
	P0CON = 0x00;                 // All general I/O
	P1CON = 0x00;                 // All general I/O
	P2CON = 0x00;                 // All general I/O

	index_in  = 0;
	index_out = 0;
	nDataFIFO = 0;
	FIFOempty = 1;

	LED_VD = 1;
	LED_VM = 1;
	delay_time(100000);
	LED_VM = 0;
	LED_VD = 0;

	rf_init();
	adc_init(); 								  // LE1 AD conversion initialization
	EA = 1;  											// Enable global IRQ
	RF = 1; 											// Radio IRQ enable

	RX_Mode();										// Enable receive values

	while(1)
	{
		if(newPayload)							// finish received
		{
			//sta = 0;

			if(rx_buf[0] == BROADCAST)
			{
				/**************************************/
				// 0x02 -> Command START ACQUISITION
				/**************************************/
				if(rx_buf[1] == 0x02)
				{
					start_T0();
					newPayload = 0;
				}
				/**************************************/
				// 0x05 -> Command CANCEL ACQUISITION
				/**************************************/
				if(rx_buf[1] == 0x05)
				{
					stop_T0();
					LED_VD = 0;
					LED_VM = 0;
					newPayload = 0;
				}
				/**************************************/
				// 0x08 -> Command UNDEFINED
				/**************************************/
				if(rx_buf[1] == 0x08)
				{
					pushFIFO(j);
					tx_buf[0] = MYADDR; 													// Address of this sensor
					tx_buf[1] = nDataFIFO;
					tx_buf[2] = j;
					TX_Mode_NOACK(3);
					while (!(TX_DS|MAX_RT)); 								// Check if the package was sent and try again if not
					RX_Mode();
					j++;
				}
				/**************************************/
				// 0x09 -> Command RESET SENSOR
				/**************************************/
				if(rx_buf[1] == 0x09)
				{
					index_in  = 0;
					index_out = 0;
					nDataFIFO = 0;
					FIFOempty = 1;
					stop_T0();
					newPayload = 0;
					LED_VD = 1;
					LED_VM = 1;
					delay_time(100000);
					LED_VD = 0;
					LED_VM = 0;
				}
			}
			if(rx_buf[0] == MYADDR)
			{
				/**************************************/
				// 0x03 -> Command READ SENSOR from FIFO
				/**************************************/
				if(rx_buf[1] == 0x03)
				{
					tx_buf[0] = MYADDR; 													// Address of this sensor
					for(i=0;i<2*N_SAMPLE_PYL;i=i+2)
					{
						EA = 0;  																		// Disable global IRQ
						BioSampleH = popFIFO();
						BioSampleL = popFIFO();
						EA = 1;  																		// Enable global IRQ
						if(FIFOempty)									  						// If FIFO empty, discard BioSample
							break;
						else
						{
							tx_buf[i+3] = BioSampleH;   							// 8 bits MSB data AD sample
							tx_buf[i+4] = BioSampleL;   							// 8 bits LSB data AD sample
						}
					}
					NDataPyl = i;
					tx_buf[2] = NDataPyl;													// Number of samples data bytes
					NDataPyl = NDataPyl + 3;
					tx_buf[1] = (infPld<<4) + (nDataFIFO>>3);     // INF(4bits)+FIFO(4bits)
					TX_Mode_NOACK(NDataPyl);
					while (!(TX_DS|MAX_RT)); 											// Check if the package was sent and try again if not
					RX_Mode();
				}
				/**************************************/
				// 0x13 -> Command RE-READ SENSOR
				// Retransmit last payload
				/**************************************/
				if(rx_buf[1] == 0x13)
				{
					TX_Mode_NOACK(NDataPyl);
					while (!(TX_DS|MAX_RT)); 								// Check if the package was sent and try again if not
					RX_Mode();
				}
				/**************************************/
				// 0x06 -> Command CONFIG SENSOR
				/**************************************/
				if(rx_buf[1] == 0x06)
				{
					NBT0H  = rx_buf[2];
					NBT0L  = rx_buf[3];
					NOVT0  = rx_buf[4];
					NPRT0H = rx_buf[5];
					NPRT0L = rx_buf[6];

					tx_buf[0] = MYADDR; 									// Address of this sensor
					tx_buf[1] = 0x06;										// INF(4bits)+FIFO(4bits)
					tx_buf[2] = 0x01;										// 0x01 -> Data received: OK
					TX_Mode_NOACK(3);
					while (!(TX_DS|MAX_RT)); 								// Check if the package was sent and try again if not
					RX_Mode();
					LED_VD = 1;
					delay_time(30000);
					LED_VD = 0;
				}
			}
		}
	}
}
