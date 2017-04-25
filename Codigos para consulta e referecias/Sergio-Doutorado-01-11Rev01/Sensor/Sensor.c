#include <intrins.h>
#include <reg24le1.h>
#include <stdint.h>
#include <stdbool.h>

#define	PIN32
#ifdef 	PIN32
sbit S2 = P1^4;                               	// 1/0=no/press
sbit S1 = P0^2;                               	// 1/0=no/press

sbit LED_VD = P0^6;                             // 1/0=light/dark
sbit LED_VM = P0^3;                             // 1/0=light/dark
#endif

#include "API.h"
#include "app.h"
#include "nRF-SPIComands.h"

#define INTERRUPT_UART0 4   								// UART0, Receive & Transmitt interrupt
#define INTERRUPT_TMR0	1

#define BROADCAST				0										// Endereço 0 indica transmissão BROADCAST
#define TAM_FIFO 				120	  							// Tamanho da FIFO 120 bytes
#define MYADDR					0x09									// Endereço deste sensor. Deve seguir o vetor ADDR_SENSOR
#define N_SAMPLE_PYL	  13									// Number of samples in 16 bits on RF Payload
#define N_BYTES_CFG      6     
#define WAIT_SENSOR      2          				// Maximum wait time for the answer from sensor in ms
#define MAX_PER					 8									// Número máximo de sensores que este sensor pode rotear

/*Macros removed with the use of the corresponding bit for example: determining variable A minimum bits, you can, if (A & BIT0)*/

#define BIT0  0x01
#define BIT1  0x02
#define BIT2  0x04
#define BIT3  0x08
#define BIT4  0x10
#define BIT5  0x20
#define BIT6  0x40
#define BIT7  0x80



uint8_t pdata FIFO[TAM_FIFO];							// Define FIFO software
uint8_t pdata index_in = 0;
uint8_t pdata index_out = 0;
uint8_t pdata nDataFIFO = 0;
bit FIFOempty = 1, FIFOfull = 0;

uint8_t pdata pldInf = 0;									// Information about the payload in the next transmition

uint8_t pdata nMaxPer;						    		// Número atual de sensores que este sensor pode rotear
uint8_t pdata endPld;											// Endereço inicial para verificação de roteamento. 
																								// O valor inicial deve ser 0xFF para ficar fora da faixa
																								// de endereços da WBAN
uint8_t pdata nDataPld; 									// Quantidade de dados no payload

/**************************************************/
// Variáveis do TMR0
uint8_t pdata NBT0H  =   0xE5;			// Este tempo
uint8_t pdata NBT0L  =   0xF6;			// equivale a
uint8_t pdata NOVT0  =   0x00;			// Freq. de Amostragem
uint8_t pdata NPRT0H =   0x00;			// de 200Hz
uint8_t pdata NPRT0L =   0x00;			//
uint8_t pdata AQ_TIMEH = 0x00;			// AQ_TIME -> Aquisition Time	
uint8_t pdata AQ_TIMEL = 0x05;			// AQ_TIME = AQ_TIMEH * 256 + AQ_TIMEL
uint8_t pdata count;	
uint16_t  pdata NBT0;

/**************************************************/
// Variáveis de controle roteamento

uint8_t TMR_ROTL  = 0xE8;		// Este tempo equivale a um
uint8_t TMR_ROTH  = 0x03;		// tempo de roteamento de 1 seg.
//uint16_t  TMR_ROT;					// Tempo em ms que o sensor roteará
																	// um sensor específico
uint8_t TMR_ROT = 0xC8;			// Nro de bases de tempo de T0 no qual 
																	// o sensor roteará um sensor específico
uint8_t tmrAsRot  = 0xC8;   // Contador de bases de tempo de T0
																	// 0xC8 equivale a 200 bases de tempo de T0
bit flagRot = 0;
uint8_t pdata vetEndR[MAX_PER];
/**************************************************/
void start_T0(void)
{
	TMOD=0x31;						// Select Timer 1 --> STOPPED, Timer 0 --> TIMER/16 bits
	TH0= 0xFF;						// Estou do Timer 0 após um pulso.
	TL0= 0xFE;
	ET0=1;								// Active interrupt on Timer 0
	EA=1;									// Active all interrupts
	TR0=1;								// Timer 0 --> RUN
}
/**************************************************/
void stop_T0(void)
{
	TMOD=0x31;						// Select Timer 1 --> STOPPED, Timer 0 --> TIMER/16 bits
	ET0=0;								// Disable interrupt on Timer 0
	TR0=0;								// Timer 0 --> STOPPED
	EA=1;									// Active all interrupts
}

/**************************************************/
//** AD feature configuration function *** /
//* Function Name: adc_config (); no parameters passed, used to initialize the AD conversion * /
void adcInit(void)
{
	ADCCON2  = 0x00; // set to single-step conversion and energy, speed 2ksps
	ADCCON3 |= 0xE0; // precision 12bit, the data right justified
	ADCDATH &= 0xF0; // conversion result register is cleared
	ADCDATL &= 0x00;
	P0DIR   |= 0x07; // Set the input channel converted to 0,1,2
	P0      &= 0xF8; // initialize port is low
}

/**************************************************/
uint8_t popFIFO()
{
	uint8_t	 aux;
	 	
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
void pushFIFO(uint8_t dado)
{
	if(nDataFIFO == TAM_FIFO)
	{
		FIFOfull = 1;
		return;
	}
	else
		FIFOfull = 0;
		//popFIFO();
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
void delay(uint16_t x)
{
    uint16_t i,j;
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
void readADCtoFIFO (uint8_t num)
{
		//uint16_t res = 0;
		//static uint8_t num = 0;
		ADCCON1 = BIT7 + (num << 2) + BIT0; // set the conversion of the channel, set the reference VDD, and start
		while (!(ADCCON1 & BIT6)); // wait for start
		while ((ADCCON1 & BIT6)); // wait for the completion of the conversion
		pushFIFO(ADCDATH & 0x0F);
		pushFIFO(ADCDATL);
}


/***************************************************/
void removeEnd(uint8_t pos)
{
	char lastPos;
	
	lastPos = nMaxPer--;	// lastPos aponta para a última posição ocupada no vetor vetEndR
	while(pos < lastPos)
	{
		vetEndR[pos] = vetEndR[pos + 1];
		pos++;
	}
}

/**************************************************/
void TMR0_IRQ(void) interrupt INTERRUPT_TMR0
{
//	if(tmrAsRot==0)
//	{
//		//removeEnd(0);
//		tmrAsRot = TMR_ROT;
//	}
//	else
//		tmrAsRot--;
	
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
uint8_t checkRot(uint8_t pos, uint8_t EndS)
{
	do
	{
		if(vetEndR[pos] == EndS)
		{
			pldInf = 0x20;
			return(pos);
		}
		else
			pos++;
	}while(pos < nMaxPer);
	pldInf = 0x00;
	return(pos);
}
	

/***************************************************/
void txData(uint8_t addrSen)
{
		uint8_t BioSampleH,BioSampleL, i, T_SAMPLE;
	
	tx_buf[0] = addrSen; 													// Address of this sensor
	tx_buf[1] = ((FIFOfull*4+pldInf)<<4) + (nDataFIFO>>3);     			// pldInf(4bits)+FIFO(4bits)	
	T_SAMPLE = 2*N_SAMPLE_PYL;
	for(i=0;i<T_SAMPLE;i=i+2)
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
	nDataPld = i;
	tx_buf[2] = nDataPld;													// Number of samples data bytes					
	nDataPld = nDataPld + 3;
	//tx_buf[1] = pldInf + (nDataFIFO>>3);     			// pldInf(4bits)+FIFO(4bits)					
	TX_Mode_NOACK(nDataPld);
}



/***************************************************/
void saveEndSen()
{
	uint8_t pos;
	
	newPayload = 0;
	if(nMaxPer < MAX_PER)
	{
		if(endPld == rx_buf[0])
		{
			pos = checkRot(0,endPld);
			if((pldInf & 0x20) == 0x00)    					// Endereço ainda não consta em vetEndR
			{
				vetEndR[nMaxPer] = endPld;
				nMaxPer++;
			}
		}
		else
			endPld = rx_buf[0];
	}
}

/***************************************************/
//          INIT SENSOR
void initSensor()
{
	count = NOVT0;
	tmrAsRot = TMR_ROT;
	
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
	stop_T0();
	nMaxPer = 0;
	endPld=0xFF;
	tmrAsRot = TMR_ROT;
	newPayload = 0;
	LED_VD = 1;
	LED_VM = 1;
	delay_time(100000);
	LED_VM = 0;	
	LED_VD = 0;
}

/***************************************************/
//          - - >    M A I N    < - - 
/**************************************************/
void main(void)
{
	uint8_t j=0, pos;
	
	initSensor();
	rfInit();                        
	adcInit(); 								  // LE1 AD conversion initialization 
	EA = 1;  										// Enable global IRQ
	RF = 1; 										// Radio IRQ enable																		
	
	RX_Mode();										// Enable receive values

	while(1)
	{
//		while(!S2)
//		{
//			newPayload = 0;
//		}
		if(newPayload)							// finish received
		{
			//sta = 0;
			LED_VM=1;  //!LED_VM;
			if(rx_buf[0] == BROADCAST)
			{
				/**************************************/
				// 0x02 -> Command START ACQUISITION
				/**************************************/
				if(rx_buf[1] == 0x02)  
				{
					index_in  = 0;
					index_out = 0;
					nDataFIFO = 0;
					FIFOempty = 1;
					start_T0();
					LED_VD = 1;
					LED_VM = 0;
					newPayload = 0;
				}
				/**************************************/
				// 0x03 -> Command CANCEL ACQUISITION
				/**************************************/
				if(rx_buf[1] == 0x03)  
				{
					stop_T0();
//					LED_VD = 1;
//					LED_VM = 1;
//					delay(10000);
//					LED_VD = 0;
//					LED_VM = 0;
					newPayload = 0;
				}					
				/**************************************/
				// 0xAA -> Command will return the number of data on FIFO
				/**************************************/
//				if(rx_buf[1] == 0xAA)  
//				{
//					pushFIFO(j);
//					tx_buf[0] = MYADDR; 													// Address of this sensor
//					tx_buf[1] = nDataFIFO;
//					tx_buf[2] = j;
//					TX_Mode_NOACK(3);
//					RX_Mode();
//					j++;
//				}		
				/**************************************/
				// 0x08 -> Command RESET SENSOR
				/**************************************/				
				if(rx_buf[1] == 0x08)  
				{
					initSensor();
					rfInit();                        
					adcInit(); 								  // LE1 AD conversion initialization 
					LED_VM = 1;
					delay_time(5000);
					LED_VM = 0;	
					delay_time(5000);					
					LED_VM = 1;
					delay_time(5000);
					LED_VM = 0;						
					EA = 1;  											// Enable global IRQ
					RF = 1; 											// Radio IRQ enable																		
					RX_Mode();										// Enable receive values
				}		
			}
			else
			{
				if(rx_buf[0] == MYADDR)
				{				
					/**************************************/
					// 0x01 -> Command CONFIG SENSOR
					/**************************************/
					if(rx_buf[1] == 0x01)  
					{
						NBT0H     = rx_buf[2];
						NBT0L     = rx_buf[3];
						NOVT0     = rx_buf[4];	
						NPRT0H    = rx_buf[5];
						NPRT0L    = rx_buf[6];
						AQ_TIMEL  = rx_buf[7];
						AQ_TIMEH  = rx_buf[8];
						TMR_ROTH  = rx_buf[9];
						TMR_ROTL  = rx_buf[10];
						NBT0      = (256 * NBT0H) + NBT0L;				
						TMR_ROT   = (256 * TMR_ROTH) + TMR_ROTL;
						tmrAsRot  = TMR_ROT;
						tx_buf[0] = MYADDR; 									// Address of this sensor
						tx_buf[1] = 0x20;										  // pldInf(4bits)+FIFO(4bits)
						tx_buf[2] = 0x01;										  // 0x01 -> Data received: OK
						TX_Mode_NOACK(3);
						LED_VD = 1;
						delay_time(30000);					
						LED_VD = 0;
						LED_VM = 0;
					}
					/**************************************/
					// 0x05 -> Command READ SENSOR from FIFO
					/**************************************/
					if(rx_buf[1] == 0x05)
					{
						pldInf = 0x00;
						txData(MYADDR);
						//LED_VM = 0;
					}
									
					/**************************************/
					// 0x15 -> Command RE-READ SENSOR
					// Retransmit last payload
					/**************************************/
					if(rx_buf[1] == 0x15)  
					{
						TX_Mode_NOACK(nDataPld);
					}	
	
					/**************************************/
					// 0x25 -> Command READ SENSOR with Router Cancel
					/**************************************/
					if(rx_buf[1] == 0x25)
					{
						flagRot = 0;
						pos = checkRot(0,rx_buf[2]);
						removeEnd(pos);
						pldInf = 0x00;
						txData(MYADDR);
						LED_VM = 0;
					}
				
					/**************************************/
					// 0x35 -> Command READ SENSOR from FIFO
					/**************************************/
					if(rx_buf[1] == 0x35)
					{
						pldInf = 0x10;
						txData(MYADDR);
						LED_VM = 0;
					}
					/**************************************/
					// 					    readDataQry
					// 0x06 -> Command READ SENSOR with Query
					// 0x26 -> Command READ SENSOR with Query and Cancel
					/**************************************/
					if((rx_buf[1] == 0x06)||(rx_buf[1] == 0x26))
					{
						if(rx_buf[1] == 0x26)
						{
							flagRot = 0;
							pos = checkRot(0,rx_buf[3]);
							removeEnd(pos);
						}
						pos = checkRot(0, rx_buf[2]);
						txData(MYADDR);
					}

					/**************************************/
					// 0x07 -> Command READ SENSOR by routing
					/**************************************/
					if(rx_buf[1] == 0x07)
					{
						tx_buf[0] = rx_buf[2];
						tx_buf[1]= 0x35;
						TX_Mode_NOACK(2);
						RX_Mode();
						delay(WAIT_SENSOR);
						if(newPayload)
						{
							pldInf = rx_buf[1] & 0xF0;
							if((pldInf & 0x10) == 0x10)
							{
								for(j = 0;j < payloadWidth; j++)
									tx_buf[j] = rx_buf[j];
								tx_buf[1] = rx_buf[1] & 0x0F;						// Zera a variável pldInf
								TX_Mode_NOACK(payloadWidth);
							}
						}
					}
				}
				
				//  T E S T E
//				else
//					saveEndSen();
			}
			/**************************************/
		}
	}
}

