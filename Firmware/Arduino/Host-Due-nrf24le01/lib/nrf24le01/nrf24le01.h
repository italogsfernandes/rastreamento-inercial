#ifndef nrf24le01_h
#define nrf24le01_h

#include <Arduino.h>

class nrf24le01{
public:
  nrf24le01(uint8_t parametro);
  void rf_init(void);
  void RX_Mode(void);
  void TX_Mode_NOACK(uint8_t payloadLength);

private:
  uint8_t SPI_RW(uint8_t value);
  uint8_t SPI_RW_Reg(uint8_t reg, uint8_t value);
  uint8_t SPI_Read_Status(void);
  uint8_t SPI_Read(uint8_t reg);
  uint8_t SPI_Read_Buf(uint8_t reg, uint8_t *pBuf, uint8_t bytes);
  uint8_t SPI_Write_Buf(uint8_t reg, uint8_t *pBuf, uint8_t bytes);
  void RF_IRQ(void);

};



#endif
