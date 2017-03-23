#include "nrf24le01.h"

nrf24le01::nrf24le01(uint8_t parametro){
  Serial.print("oie");
}

///////////
//PUBLIC //
///////////

/**************************************************/
void nrf24le01::rf_init(void){
  // Radio + SPI setup
  pinMode(RFIRQ, INPUT);  // Define RFIRQ as input to receive IRQ from nRF24L01+
  pinMode(RFCE, OUTPUT);  // Define RFCE as output to control nRF24L1+ Chip Enable
  pinMode(RFCSN, OUTPUT); // Define RFCSN as output to control nRF24L1+ SPI
  SPI.begin();            // start the SPI library:
  newPayload = 0;
  sta = 0;
  TX_OK = 0;
  RX_OK = 0;
  digitalWrite(RFCSN,1);                        // Set CSN low, init SPI tranaction
  digitalWrite(RFCE,0);                         // Radio chip enable low
  SPI_Write_Buf(W_REGISTER + TX_ADDR, ADDR_HOST, TX_ADR_WIDTH);
  SPI_Write_Buf(W_REGISTER + RX_ADDR_P0, ADDR_HOST, TX_ADR_WIDTH);
  SPI_RW_Reg(W_REGISTER + EN_AA, 0x00);        // Disable Auto.Ack:Pipe0
  SPI_RW_Reg(W_REGISTER + EN_RXADDR, 0x01);    // Enable Pipe0 (only pipe0)
  SPI_RW_Reg(W_REGISTER + AW, 0x03);           // 5 bytes de endereço
  SPI_RW_Reg(W_REGISTER + SETUP_RETR, 0x00);   // Tempo de retransmissão automática de 250us, retransmissão desabilitada
  SPI_RW_Reg(W_REGISTER + RF_CH, 90);          // Select RF channel 90. Fo = 2,490 GHz
  SPI_RW_Reg(W_REGISTER + RF_SETUP, 0x07);     // TX_PWR:0dBm, Datarate:1Mbps, LNA:HCURR
  SPI_RW_Reg(W_REGISTER + DYNPD, 0x01);        // Ativa Payload dinâmico em data pipe 0
  SPI_RW_Reg(W_REGISTER + FEATURE, 0x07);      // Ativa Payload dinâmico, com ACK e comando W_TX_PAY
  SPI_RW_Reg(FLUSH_TX,0);
  SPI_RW_Reg(FLUSH_RX,0);
  SPI_RW_Reg(W_REGISTER+NRF_STATUS,0x70);
}
/**************************************************/
void nrf24le01::RX_Mode(void){
  newPayload = 0;
  sta = 0;
  RX_OK = 0;
  digitalWrite(RFCE,0);                         // Radio chip enable low -> Standby-1
  SPI_RW_Reg(W_REGISTER + CONFIG, 0x1F);        // Set PWR_UP bit, enable CRC(2 bytes) & Prim:RX. RX_DR enabled..
  digitalWrite(RFCE,1);                         // Set CE pin high to enable RX Mode
}

/**************************************************/
void nrf24le01::TX_Mode_NOACK(uint8_t payloadLength){
  digitalWrite(RFCE,0);                                            // Radio chip enable low -> Standby-1
  SPI_RW_Reg(W_REGISTER + CONFIG, 0x1E);                           // Set PWR_UP bit, enable CRC(2 bytes) & Prim:TX. RX_DR enabled.
  SPI_Write_Buf(W_TX_PAYLOAD_NOACK, tx_buf, payloadLength);        // Writes data to TX payload
                                                                   // Endereço da porta P2, matrizes tx_buf (), o comprimento da matriz é enviada)
  TX_OK = 0;
  digitalWrite(RFCE,1);                                            // Set CE pin high to enable TX Mode
  delayMicroseconds(12);
  digitalWrite(RFCE,0);                                            // Radio chip enable low -> Standby-1
  tempo = micros() + 1000;
  do
  {
    tempoAtual = micros();
    if (!digitalRead(RFIRQ))
      RF_IRQ();
  }while (!((TX_OK)|(tempoAtual>tempo)));
  if((tempoAtual>tempo))
    Serial.println("Time Out!");Serial.print('\n');Serial.print('\0');
}


////////////
//Private //
////////////

/***************************************************/
uint8_t nrf24le01::SPI_RW(uint8_t value){
  uint8_t SPIData;

  SPIData = SPI.transfer(value);

  return SPIData;                   // return SPI read value
}
/**************************************************/
uint8_t nrf24le01::SPI_RW_Reg(uint8_t reg, uint8_t value){
  uint8_t status;

    digitalWrite(RFCSN,0);                      // CSN low, initiate SPI transaction£
    status = SPI_RW(reg);           // select register
    SPI_RW(value);                  // ..and write value to it..
    digitalWrite(RFCSN,1);                      // CSN high again  £¨rfcon^1

    return(status);                 // return nRF24L01 status byte
}
/**************************************************/
uint8_t nrf24le01::SPI_Read_Status(void){
  uint8_t reg_val;

    digitalWrite(RFCSN,0);                          // CSN low, initialize SPI communication...
    reg_val = SPI_RW(NOP);                            // ..then read register value
    digitalWrite(RFCSN,1);                          // CSN high, terminate SPI communication RF
    return(reg_val);                                // return register value
}
/**************************************************/
uint8_t nrf24le01::SPI_Read(uint8_t reg){
  uint8_t reg_val;

    digitalWrite(RFCSN,0);                          // CSN low, initialize SPI communication...
    SPI_RW(reg);                                    // Select register to read from..
    reg_val = SPI_RW(0);                            // ..then read register value
    digitalWrite(RFCSN,1);                          // CSN high, terminate SPI communication RF
    return(reg_val);                                // return register value
}
/**************************************************/
uint8_t nrf24le01::SPI_Read_Buf(uint8_t reg, uint8_t *pBuf, uint8_t bytes){
  uint8_t status,byte_ctr;

    digitalWrite(RFCSN,0);                                    // Set CSN low, init SPI tranaction
    status = SPI_RW(reg);                         // Select register to write to and read status byte

    for(byte_ctr=0;byte_ctr<bytes;byte_ctr++)
      pBuf[byte_ctr] = SPI_RW(0);                 // Perform SPI_RW to read byte from nRF24L01

    digitalWrite(RFCSN,1);                                    // Set CSN high again

    return(status);                               // return nRF24L01 status byte
}
/**************************************************/
uint8_t nrf24le01::SPI_Write_Buf(uint8_t reg, uint8_t *pBuf, uint8_t bytes){
  uint8_t status,byte_ctr;

    digitalWrite(RFCSN,0);                        // Set CSN low, init SPI tranaction
    status = SPI_RW(reg);                         // Select register to write to and read status byte
    for(byte_ctr=0; byte_ctr<bytes; byte_ctr++)   // then write all byte in buffer(*pBuf)
       SPI_RW(*pBuf++);

    digitalWrite(RFCSN,1);                        // Set CSN high again
    return(status);                               // return nRF24L01 status byte
}


//////////////
//Interrupt //
//////////////

//TODO: this gonna work?
/**************************************************/
void nrf24le01::RF_IRQ(void)
{
  sta=SPI_Read(NRF_STATUS);
  if(bitRead(sta,RX_DR))                                  // if receive data ready (RX_DR) interrupt
  {
    RX_OK = 1;
    newPayload = 1;
    SPI_Read_Buf(R_RX_PAYLOAD,rx_buf,PAYLOAD_WIDTH);     // read receive payload from RX_FIFO buffer
    payloadWidth = SPI_Read(R_RX_PLD_WIDTH);              // Retorna o número de bytes no payload recebido
    if(payloadWidth > 32)
    {
      payloadWidth = 0;
      newPayload = 0;
    }
    SPI_RW_Reg(FLUSH_RX,0);
  }
  if(bitRead(sta,TX_DS))
  {
    TX_OK = 1;
    SPI_RW_Reg(FLUSH_TX,0);
  }
  SPI_RW_Reg(W_REGISTER+NRF_STATUS,0x70);
}
