#define INTERRUPT_RFIRQ	9

#define TX_ADR_WIDTH    5   								// 5 bytes TX(RX) address width
#define TX_PLOAD_WIDTH  30  								// 30 bytes TX payload
#define NRO_SENSOR			10									// Number of sensor in the WBAN

uint8_t const ADDR_HOST[TX_ADR_WIDTH] = {0xc6,0xc2,0xc2,0xc2,0xc2}; // Define a host adr
uint8_t const ADDR_SENSOR[NRO_SENSOR] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x10}; // Define the sensors address

uint8_t data rx_buf[TX_PLOAD_WIDTH];
uint8_t data tx_buf[TX_PLOAD_WIDTH];

uint8_t bdata sta;
sbit	RX_DR	= sta^6;
sbit	TX_DS	= sta^5;
sbit	MAX_RT = sta^4;

bit newPayload = 0;    											// Flag to show new Payload from host
uint8_t payloadWidth = 0;

/**************************************************/
uint8_t SPI_RW(uint8_t value)
{
  SPIRDAT = value;			 			 							//spidat

  while(!(SPIRSTAT & 0x02));  							// wait for byte transfer finished

  return SPIRDAT;             							// return SPI read value
}
/**************************************************/
uint8_t SPI_RW_Reg(uint8_t reg, uint8_t value)
{
	uint8_t status;

  	RFCSN = 0;                   						// CSN low, init SPI transaction£
  	status = SPI_RW(reg);      							// select register
  	SPI_RW(value);             							// ..and write value to it..
  	RFCSN = 1;                   						// CSN high again  £¨rfcon^1

  	return(status);            							// return nRF24L01 status byte
}
/**************************************************/
uint8_t SPI_Read(uint8_t reg)
{
	uint8_t reg_val;

  	RFCSN = 0;                											// CSN low, initialize SPI communication...
  	SPI_RW(reg);            												// Select register to read from..
  	reg_val = SPI_RW(0);    												// ..then read registervalue
  	RFCSN = 1;                											// CSN high, terminate SPI communication RF

  	return(reg_val);        												// return register value
}
/**************************************************/
uint8_t SPI_Read_Buf(uint8_t reg, uint8_t *pBuf, uint8_t bytes)
{
	uint8_t status,byte_ctr;

  	RFCSN = 0;                    								// Set CSN low, init SPI tranaction
  	status = SPI_RW(reg);       									// Select register to write to and read status byte

  	for(byte_ctr=0;byte_ctr<bytes;byte_ctr++)
    	pBuf[byte_ctr] = SPI_RW(0);    							// Perform SPI_RW to read byte from nRF24L01

  	RFCSN = 1;                         					  // Set CSN high again

  	return(status);                    				  	// return nRF24L01 status byte
}
/**************************************************/
uint8_t SPI_Write_Buf(uint8_t reg, uint8_t *pBuf, uint8_t bytes)
{
	uint8_t status,byte_ctr;

  	RFCSN = 0;                   									// Set CSN low, init SPI tranaction
  	status = SPI_RW(reg);    											// Select register to write to and read status byte
  	for(byte_ctr=0; byte_ctr<bytes; byte_ctr++) 	// then write all byte in buffer(*pBuf)
    	SPI_RW(*pBuf++);
  	RFCSN = 1;                 										// Set CSN high again
  	return(status);          											// return nRF24L01 status byte
}

/**************************************************/
void RX_Mode(void)
{
	sta = 0;
	newPayload = 0;	
	RFCE=0;																					//RFCON ^ 0: RF controle transceptor 1: Ativar
  SPI_RW_Reg(WRITE_REG + CONFIG, 0x0f);   				// Set PWR_UP bit, enable CRC(2 bytes) & Prim:RX. RX_DR enabled..
  RFCE = 1; 																			// Set CE pin high to enable RX device
}
/**************************************************/
void TX_Mode_NOACK(short int payloadLength) 
{
	RFCE=0;							   														//RFCON ^ 0: RF controle transceptor 1: Ativar
  SPI_RW_Reg(WRITE_REG + CONFIG, 0x0E);     				//???? Set PWR_UP bit, enable CRC(2 bytes) & Prim:TX. MAX_RT & TX_DS enabled..
	SPI_Write_Buf(WRITE_REG + RX_ADDR_P0, ADDR_HOST, TX_ADR_WIDTH);
  SPI_Write_Buf(WR_TX_PLOAD, tx_buf, payloadLength); // Writes data to TX payload
																										 //Endereço da porta P2, matrizes tx_buf (), o comprimento da matriz é enviada)
	RFCE=1;		       																	 //RFCON ^ 0: RF controle transceptor 1: Desativar
}

/**************************************************/
void RF_IRQ(void) interrupt INTERRUPT_RFIRQ
{
	sta=SPI_Read(STATUS);																// read register STATUS's value
	if(RX_DR)																						// if receive data ready (RX_DR) interrupt
	{
		SPI_Read_Buf(RD_RX_PLOAD,rx_buf,TX_PLOAD_WIDTH);	// read receive payload from RX_FIFO buffer
		SPI_RW_Reg(FLUSH_RX,0);
		newPayload = 1;
		payloadWidth = SPI_Read(R_RX_PLD_WIDTH);  				// Return the number of bytes on receved payload
		if(payloadWidth > 32) 
		{
			payloadWidth = 0;
			SPI_RW_Reg(FLUSH_RX,0);
			newPayload = 0;
		}
	}
	if(MAX_RT)
		SPI_RW_Reg(FLUSH_TX,0);
	if(TX_DS)
		SPI_RW_Reg(FLUSH_TX,0);
	SPI_RW_Reg(WRITE_REG+STATUS,0x70);								// clear RX_DR or TX_DS or MAX_RT interrupt flag
}

/**************************************************/
void rf_init(void)
{
		
	RFCE = 0;                                   // Radio chip enable low
	RFCKEN = 1;                                 // Radio clk enable
  SPI_Write_Buf(WRITE_REG + TX_ADDR, ADDR_HOST, TX_ADR_WIDTH);
	SPI_Write_Buf(WRITE_REG + RX_ADDR_P0, ADDR_HOST, TX_ADR_WIDTH);
	SPI_RW_Reg(WRITE_REG + EN_AA, 0x00);      	// Disable Auto.Ack:Pipe0
  SPI_RW_Reg(WRITE_REG + EN_RXADDR, 0x01);  	// Enable Pipe0 (only pipe0)
  SPI_RW_Reg(WRITE_REG + SETUP_RETR, 0x00); 	// Time to automatic retransmition selected: 250us, retransmition disabled
  SPI_RW_Reg(WRITE_REG + RF_CH, 40);        	// Select RF channel 40
  SPI_RW_Reg(WRITE_REG + RF_SETUP, 0x07);   	// TX_PWR:0dBm, Datarate:1Mbps, LNA:HCURR
	SPI_RW_Reg(WRITE_REG + DYNPD, 0x01); 				// Ativa Payload dinâmico em data pipe 0
	SPI_RW_Reg(WRITE_REG + FEATURE, 0x07);			// Ativa Payload dinâmico, com ACK e comando W_TX_PAY
}

