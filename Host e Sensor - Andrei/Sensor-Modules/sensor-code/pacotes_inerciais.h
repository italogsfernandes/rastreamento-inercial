#ifndef PACOTES_INERCIAIS_H
/* Se a biblioteca mpu.h não for definida, defina-a.
Verificar é preciso para que não haja varias chamadas da
mesma biblioteca. */
#define	PACOTES_INERCIAIS_H

#include <nRF-SPIComands.h>

#define UART_START_SIGNAL  0x53
#define UART_END_SIGNAL  0x04

#define PACKET_TYPE_ACEL      0x01
#define PACKET_TYPE_GIRO      0x02
#define PACKET_TYPE_MAG       0x03
#define PACKET_TYPE_M6        0x04
#define PACKET_TYPE_M9        0x05
#define PACKET_TYPE_QUAT      0x06
#define PACKET_TYPE_FIFO_NO_MAG       0x07
#define PACKET_TYPE_FIFO_ALL_READINGS     0x08

#define PACKET_TYPE_STRING    0x09
#define PACKET_TYPE_HEX       0x0A
#define PACKET_TYPE_BIN       0x0B
#define PACKET_TYPE_UINT16    0x0C
#define PACKET_TYPE_INT16     0x0D
#define PACKET_TYPE_FLOAT16		0x0E
#define PACKET_TYPE_UINT8		0x0F
#define PACKET_TYPE_INT8			0x10



#endif
