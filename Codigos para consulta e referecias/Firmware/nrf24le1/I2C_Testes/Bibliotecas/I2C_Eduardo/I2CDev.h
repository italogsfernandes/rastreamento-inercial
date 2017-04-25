#ifndef I2CDev_H_
#define I2CDev_H_

#include "reg24le1.h"
#include <stdint.h>
#include <stdbool.h>

//NOTE: definições nao usadas na library
#define I2C_IDLE    0x00      // Define the clock frequency
#define I2C_100K    0x01      // Define the clock frequency
#define I2C_400K    0x02      // Define the clock frequency
#define STOP_COND   0x08      // Bit to interrupt caused by stop condition
#define ADDR_MATCH  0x04      // Bit to interrupt caused by adress match
#define DATA_READY  0X01      // Bit to interrupt caused by byte transmitted/receveid
#define ACK  (W2CON1 & 0X02)  // 1 = NACK; 0 = ACK

/// SLAVE GENERAL FUNCTIONS
void generalAddr(bool genResp);
/*
 * Function to set the slave to respond to a general call adress
 */

void setClock(bool aux);
/*
 * Function to set the clock frequency
 */

void stopCond(bool aux);
/*
 * Function enable interrupt when stop condition is detected
 */

void addrMatch(bool aux);
/*
 * Function enable interupt on adress match
 */

void setSlaveAddr(uint8_t slaveAddr);
/*
 * Function to set slave adress (7 bit)
 */

/// MASTER GENERAL FUNCTINS
void startI2C(void);
/*
 * Function to start transmitting.
 */

void stopI2C(void);
/*
 * Function to stop transmitting.
 */

/// GENERAL FUNCTIONS

void operationMode(bool aux);
/*
 * Function to set the operation mode of the I2C.
 * select between master and slave
 */

void enableI2C(bool en);
/*
 * Function to enable the I2C.
 */

void enableISR(bool isr);
/*
 * Function to enable all interrupts.
 */

uint8_t getStatus(void);
/*
 * Function that returns the status of the 2-wire.
 * @remark Bit 4:0 are cleared when read.
 */

void writeByte(uint8_t addr, uint8_t dados, uint8_t dir);
/*
 * Function to write to the I2C data register.
 */

uint8_t readByte(void);
/*
 * Function to read from the I2C data register.
 */

#endif
