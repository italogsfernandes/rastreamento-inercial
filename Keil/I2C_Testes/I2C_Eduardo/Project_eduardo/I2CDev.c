#include <stdint.h>
#include <stdbool.h>
#include "I2CDev.h"

/// SLAVE GENERAL FUNCTIONS
//NOTE: isso sera usada?
// se sim nao entendi oq ta acontecendo, segundo datasheet:
/*
1: Respond to the general call address (0x00), as
well as the address defined in WIRE2ADR.
0: Respond only to the address defined in
WIRE2ADR
*/
void generalAddr(bool genResp)
{
  if(genResp)
    W2CON0 |= 0x80;   // Set broadcast enable
  else
    W2CON0 &= 0x7F;
}

//BUG: nessa aqui eu bugei
// comparar com /Keil/I2C_dev_nRF24LE1/I2C_dev
void setClock(bool aux)
{
  if(aux)
    W2CON0 |= 0x40;   // Set clockStop bit
  else
    W2CON0 &= 0xBF;
}

//BUG: precisa dar clear msm?
//0: No interrupt caused by stop condition.
//Cleared when reading W2CON1.
// NOTE: stopI2C nao faz o mesmo?
void stopCond(bool aux)
{
  if(aux)
    W2CON0 &= 0xDF;   // Clear xStop bit
  else
    W2CON0 |= 0x20;
}

//NOTE: Função restrita do slave, não necessaria
void addrMatch(bool aux)
{
  if(aux)
    W2CON0 &= 0xEF;   // Clear xStart bit
  else
    W2CON0 |= 0x10;
}

//BUG: por que & 0x7F?
void setSlaveAddr(uint8_t slaveAddr)
{
  W2SADR = (slaveAddr & 0x7F);  // Set 7 bit adress of the slave
}


/// MASTER GENERAL FUNCTIONS
//NOTE: cuidao ao definir a maskIrq on
// nem sempre isso sera assim então dessa forma vc limita a biblioteca
// mas ficou boa essa
void startI2C(void)
{
  W2CON0 |= 0x10;   // Set xStart bit
  W2CON1 |= 0x20
}

//NOTE: envia a stopcondition
void stopI2C(void)
{
  W2CON0 |= 0x20;   // Set xStop bit
}

/// GENERAL FUNCTIONS
//BUG: nessa aqui eu bugei
// comparar com /Keil/I2C_dev_nRF24LE1/I2C_dev
void operationMode(bool aux)
{
  if(aux)
    W2CON0 |= 0x02;   // Set masterSelect bit
  else
    W2CON0 &= 0xFD;
}

//NOTE: concordo
//  en -> true ativando
//  en -> false desativado
void enableI2C (bool en)
{
  if(en)
    W2CON0 |= 0x01;   // Set wire2Enable bit
  else
    W2CON0 &= 0xFE;
}

//NOTE: de acordo com essa
// isr -> true interrupts enabled without maskIrq
// isr -> true interrupts disabled by the maskIrq
void enableISR(bool isr)
{
  if(isr)
    W2CON1 &= 0xDF;    // Clear maskIrq bit
  else
    W2CON1 |= 0x20;
}

//NOTE: aproveitei essa
uint8_t getStatus(void)
{
  return W2CON1;
}

//BUG: ué????
void writeByte(uint8_t addr, uint8_t dados, uint8_t dir)
{
  W2DAT = ((addr << 1) | dir);
}

//NOTE: resumida em define
uint8_t readByte(void)
{
  return W2DAT;
}
