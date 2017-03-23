#ifndef nrf24le01_commands_h
#define nrf24le01_commands_h

//TODO: descobrir sobre library: #include <nRF24L01.h>
//TODO: And change the name of this library

#include <Arduino.h>
#include <SPI.h>

// Definições da rotina de interrupção
#define RX_DR               6
#define TX_DS               5
#define MAX_RT              4

#define PAYLOAD_WIDTH      32   // 30 bytes on TX payload
#define TX_ADR_WIDTH        5   // 5 bytes TX(RX) address width

class nrf24le01{
public:
  nrf24le01(uint8_t RFIRQ_pin, uint8_t RFCE_pin, uint8_t RFCSN_pin);
  uint8_t ADDR_HOST[TX_ADR_WIDTH] = {0xc6,0xc2,0xc2,0xc2,0xc2};   // Define a static host adr
  uint8_t rx_buf[PAYLOAD_WIDTH];    // Define lenght of rx_buf and tx_buf
  uint8_t tx_buf[PAYLOAD_WIDTH];
  uint8_t payloadWidth = 0;
  bool newPayload = 0;    // Flag to indicate that there's a new payload sensor
  void rf_init(void);
  void RX_Mode(void);
  void TX_Mode_NOACK(uint8_t payloadLength);

private:
  uint8_t sta;
  uint8_t TX_OK = 0;
  uint8_t RX_OK = 0;
  uint8_t RFIRQ;  //Pino do arduino conectado a IRQ
  uint8_t RFCE;  //Pino do arduino conectado a CE
  uint8_t RFCSN;  //Pino do arduino conectado a CSN
  uint8_t SPI_RW(uint8_t value);
  uint8_t SPI_RW_Reg(uint8_t reg, uint8_t value);
  uint8_t SPI_Read_Status(void);
  uint8_t SPI_Read(uint8_t reg);
  uint8_t SPI_Read_Buf(uint8_t reg, uint8_t *pBuf, uint8_t bytes);
  uint8_t SPI_Write_Buf(uint8_t reg, uint8_t *pBuf, uint8_t bytes);
  void RF_IRQ(void);
};



#endif
