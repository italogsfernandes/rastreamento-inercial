#include "reg24le1.h"
#include "stdbool.h"

#ifndef IIC_H__
#define IIC_H__

/* Aterações:
*   - LED removido
*   -
*/

#define MASTER 0X02
#define SLAVE  0x00

#define READY (W2CON1&0X01)	  //if ready ==1,not ready==0
#define ACK   (W2CON1&0X02 )  //1;no ack and 0 ack
#define EN2WIRE()  W2CON0|=0x01;//enable 2 wire
#define DISABLE2WIRE() 	W2CON0&=0xFE;
#define STOP()    W2CON0|=0x20;
#define START()   W2CON0|=0x10;
#define FREQSEL(x)  W2CON0|=(x<<2);
#define MODE(x)	   W2CON0&=(0xff-0x02);W2CON0|=x;  //master or slave

//some functions
void IIC_init(void);
void Io_config(void);
void ex_int(void);
//NOTE: Adicionadas por italo fernandes:
bool i2c_mpu_writeByte(uint8_t devAddr, uint8_t regAddr, uint8_t data_to_write);
bool i2c_mpu_writeBytes(uint8_t devAddr, uint8_t regAddr, uint8_t data_len, uint8_t *data_ptr);
bool i2c_mpu_readByte(uint8_t devAddr, uint8_t regAddr, uint8_t *data_ptr);
bool i2c_mpu_readBytes(uint8_t devAddr, uint8_t regAddr, uint8_t data_len, uint8_t *data_ptr);

#endif
