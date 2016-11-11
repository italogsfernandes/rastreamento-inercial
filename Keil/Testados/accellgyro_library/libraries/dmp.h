#ifndef DMP_H
/* Se a biblioteca mpu.h não for definida, defina-a.
Verificar é preciso para que não haja varias chamadas da
mesma biblioteca. */
#define	DMP_H 

#include <hal_w2_isr.h>
#include "stdint.h"
#include "stdbool.h"
#define	MPU_endereco	0x68

uint8_t xdata buffer[14]; //usado em testConnection e getIntStatus


void mpu_initialize(void){
  i2c_mpu_writeBits(MPU_endereco, 0x6B, 2, 3, 0x01);//setClockSource(MPU6050_CLOCK_PLL_XGYRO);
  i2c_mpu_writeBits(MPU_endereco, 0x1B, 4, 2, 0x00);//setFullScaleGyroRange(MPU6050_GYRO_FS_250);
  i2c_mpu_writeBits(MPU_endereco, 0x1C, 4, 2, 0x00);//setFullScaleAccelRange(MPU6050_ACCEL_FS_2);
  i2c_mpu_writeBit(MPU_endereco, 0x6B, 6, false); //setSleepEnabled(false);
}

bool mpu_testConnection(void){
    i2c_mpu_readBits(MPU_endereco, 0x75, 6, 6, buffer);
    return buffer[0] == 0x34;
}

void getMotion6_packet(uint8_t *packet6){
    i2c_mpu_readBytes(MPU_endereco, 0x3B, 14, buffer);
    packet6[0] = buffer[0]; //Xac_H
    packet6[1] = buffer[1]; //Xac_L
    packet6[2] = buffer[2]; //Yac_H
    packet6[3] = buffer[3]; //Yac_L
    packet6[4] = buffer[4]; //Zac_H
    packet6[5] = buffer[5]; //Zac_L
    packet6[6] = buffer[8]; //Xgy_H
    packet6[7] = buffer[9]; //Xgy_L
    packet6[8] = buffer[10]; //Ygy_H
    packet6[9] = buffer[11]; //Ygy_L
    packet6[10] = buffer[12]; //Zgy_H
    packet6[11] = buffer[13]; //Zgy_L
}

#endif
