#ifndef DMP_H
/* Se a biblioteca mpu.h não for definida, defina-a.
Verificar é preciso para que não haja varias chamadas da
mesma biblioteca. */
#define	DMP_H

#include <hal_w2_isr.h>
#include "stdint.h"
#include "stdbool.h"
#include "mpu6050_reg.h"
#include "stdlib.h"//malloc e free
#include <string.h> //memcmp 
#define	MPU_endereco MPU6050_DEFAULT_ADDRESS

void mpu_initialize(void);
bool mpu_testConnection(void);
void getMotion6_packet(uint8_t *packet6);
void setXAccelOffset(int16_t offset); void setYAccelOffset(int16_t offset); void setZAccelOffset(int16_t offset);
void setXGyroOffset(int16_t offset); void setYGyroOffset(int16_t offset); void setZGyroOffset(int16_t offset);

void setMemoryBank(uint8_t bank, bool prefetchEnabled, bool userBank);
uint16_t getFIFOCount();
uint8_t getIntStatus();
void setDMPEnabled(bool enabled);
void resetFIFO();
uint16_t dmpGetFIFOPacketSize();
void getFIFOBytes(uint8_t *data_ptr, uint8_t data_len);

uint8_t xdata buffer[14]; //usado em testConnection e getIntStatus
uint8_t xdata *dmpPacketBuffer;
uint16_t xdata dmpPacketSize;
uint8_t xdata malloc_memory_pool[500];

void mpu_8051_malloc_setup(){
    init_mempool (&malloc_memory_pool, sizeof(malloc_memory_pool));
}

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

int16_t getXAccelOffset() {
    i2c_mpu_readBytes(MPU_endereco, MPU6050_RA_XA_OFFS_H, 2, buffer);
    return (((int16_t)buffer[0]) << 8) | buffer[1];
}
int16_t getYAccelOffset() {
    i2c_mpu_readBytes(MPU_endereco, MPU6050_RA_YA_OFFS_H, 2, buffer);
    return (((int16_t)buffer[0]) << 8) | buffer[1];
}
int16_t getZAccelOffset() {
    i2c_mpu_readBytes(MPU_endereco, MPU6050_RA_ZA_OFFS_H, 2, buffer);
    return (((int16_t)buffer[0]) << 8) | buffer[1];
}
int16_t getXGyroOffset() {
    i2c_mpu_readBytes(MPU_endereco, MPU6050_RA_XG_OFFS_USRH, 2, buffer);
    return (((int16_t)buffer[0]) << 8) | buffer[1];
}
int16_t getYGyroOffset() {
    i2c_mpu_readBytes(MPU_endereco, MPU6050_RA_YG_OFFS_USRH, 2, buffer);
    return (((int16_t)buffer[0]) << 8) | buffer[1];
}
int16_t getZGyroOffset() {
    i2c_mpu_readBytes(MPU_endereco, MPU6050_RA_ZG_OFFS_USRH, 2, buffer);
    return (((int16_t)buffer[0]) << 8) | buffer[1];
}

void setMemoryBank(uint8_t bank, bool prefetchEnabled, bool userBank) {
    bank &= 0x1F;
    if (userBank) bank |= 0x20;
    if (prefetchEnabled) bank |= 0x40;
    i2c_mpu_writeByte(MPU_endereco, MPU6050_RA_BANK_SEL, bank);
}

uint16_t getFIFOCount() {
    i2c_mpu_readBytes(MPU_endereco, MPU6050_RA_FIFO_COUNTH, 2, buffer);
    return (((uint16_t)buffer[0]) << 8) | buffer[1];
}

uint8_t getIntStatus() {
    i2c_mpu_readByte(MPU_endereco, MPU6050_RA_INT_STATUS, buffer);
    return buffer[0];
}

void setDMPEnabled(bool enabled) {
    i2c_mpu_writeBit(MPU_endereco, MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_DMP_EN_BIT, enabled);
}

void resetFIFO() {
    i2c_mpu_writeBit(MPU_endereco, MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_FIFO_RESET_BIT, true);
}

uint16_t dmpGetFIFOPacketSize() {
    return dmpPacketSize;
}

void getFIFOBytes(uint8_t *data_ptr, uint8_t data_len) {
    if(data_len > 0){
        i2c_mpu_readBytes(MPU_endereco, MPU6050_RA_FIFO_R_W, data_len, data_ptr);
    } else {
    	*data_ptr = 0;
    }
}


#endif
