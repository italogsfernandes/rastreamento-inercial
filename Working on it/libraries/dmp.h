#ifndef DMP_H
/* Se a biblioteca mpu.h não for definida, defina-a.
Verificar é preciso para que não haja varias chamadas da
mesma biblioteca. */
#define	DMP_H 

#include <hal_w2_isr.h>
#include "stdint.h"
#include "stdbool.h"
#include "mpu6050_reg.h"
#define	MPU_endereco MPU6050_DEFAULT_ADDRESS

uint8_t xdata buffer[14]; //usado em testConnection e getIntStatus


void mpu_initialize(void){
  i2c_mpu_writeBits(MPU_endereco, MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_CLKSEL_BIT, MPU6050_PWR1_CLKSEL_LENGTH, MPU6050_CLOCK_PLL_XGYRO);//setClockSource(MPU6050_CLOCK_PLL_XGYRO);
  i2c_mpu_writeBits(MPU_endereco, MPU6050_RA_GYRO_CONFIG, MPU6050_GCONFIG_FS_SEL_BIT, MPU6050_GCONFIG_FS_SEL_LENGTH, MPU6050_GYRO_FS_250);//setFullScaleGyroRange(MPU6050_GYRO_FS_250);
  i2c_mpu_writeBits(MPU_endereco, MPU6050_RA_ACCEL_CONFIG, MPU6050_ACONFIG_AFS_SEL_BIT, MPU6050_ACONFIG_AFS_SEL_LENGTH, MPU6050_ACCEL_FS_2);//setFullScaleAccelRange(MPU6050_ACCEL_FS_2);
  i2c_mpu_writeBit(MPU_endereco, MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_SLEEP_BIT, false); //setSleepEnabled(false);
}

bool mpu_testConnection(void){
    i2c_mpu_readBits(MPU_endereco,MPU6050_RA_WHO_AM_I, MPU6050_WHO_AM_I_BIT, MPU6050_WHO_AM_I_LENGTH, buffer);
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

void setXAccelOffset(int16_t offset) {
    i2c_mpu_writeWord(MPU_endereco,  MPU6050_RA_XA_OFFS_H, offset);
}
void setYAccelOffset(int16_t offset) {
    i2c_mpu_writeWord(MPU_endereco, MPU6050_RA_YA_OFFS_H, offset);
}
void setZAccelOffset(int16_t offset) {
    i2c_mpu_writeWord(MPU_endereco, MPU6050_RA_ZA_OFFS_H, offset);
}
void setXGyroOffset(int16_t offset) {
    i2c_mpu_writeWord(MPU_endereco, MPU6050_RA_XG_OFFS_USRH, offset);
}
void setYGyroOffset(int16_t offset) {
    i2c_mpu_writeWord(MPU_endereco, MPU6050_RA_YG_OFFS_USRH, offset);
}
void setZGyroOffset(int16_t offset) {
    i2c_mpu_writeWord(MPU_endereco, MPU6050_RA_ZG_OFFS_USRH, offset);
}

#endif
