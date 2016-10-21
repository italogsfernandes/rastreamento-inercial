#include <stdint.h>
#include <stdbool.h>
#include "I2CDev.h"

/// SLAVE GENERAL FUNCTIONS
void generalAddr(bool genResp)
{
  if(genResp)
    W2CON0 |= 0x80;   // Set broadcast enable
  else
    W2CON0 &= 0x7F;
}

void setClock(bool aux)
{
  if(aux)
    W2CON0 |= 0x40;   // Set clockStop bit
  else
    W2CON0 &= 0xBF;
}

void stopCond(bool aux)
{
  if(aux)
    W2CON0 &= 0xDF;   // Clear xStop bit
  else
    W2CON0 |= 0x20;
}

void addrMatch(bool aux)
{
  if(aux)
    W2CON0 &= 0xEF;   // Clear xStart bit
  else
    W2CON0 |= 0x10;
}

void setSlaveAddr(uint8_t slaveAddr)
{
  W2SADR = (slaveAddr & 0x7F);  // Set 7 bit adress of the slave
}


/// MASTER GENERAL FUNCTIONS
void startI2C(void)
{
  W2CON0 |= 0x10;   // Set xStart bit
  W2CON1 |= 0x20
}

void stopI2C(void)
{
  W2CON0 |= 0x20;   // Set xStop bit
}

/// GENERAL FUNCTIONS
void operationMode(bool aux)
{
  if(aux)
    W2CON0 |= 0x02;   // Set masterSelect bit
  else
    W2CON0 &= 0xFD;
}

void enableI2C (bool en)
{
  if(en)
    W2CON0 |= 0x01;   // Set wire2Enable bit
  else
    W2CON0 &= 0xFE;
}

void enableISR(bool isr)
{
  if(isr)
    W2CON1 &= 0xDF;    // Clear maskIrq bit
  else
    W2CON1 |= 0x20;
}

uint8_t getStatus(void)
{
  retunr W2CON1;
}

void writeByte(uint8_t addr, uint8_t dados, uint8_t dir)
{
  W2DAT = ((addr << 1) | dir);
}

uint8_t readByte(void)
{
  return W2DAT;
}
