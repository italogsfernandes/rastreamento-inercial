#include "reg24le1.h"
#include "stdbool.h"
#include <stdint.h>


#ifndef I2C_DEV_
#define I2C_DEV_

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

#define I2C_FASTMODE 0x02
#define I2C_STANDARTMODE 0x01

#endif

///XXX:BUG:TODO:NOTE:XXX: WHAT IS THIS THING????????
#define HAL_W2_WAIT_FOR_INTERRUPT {while(!SPIF); SPIF = 0; }
