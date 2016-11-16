#ifndef DMP_H
/* Se a biblioteca mpu.h não for definida, defina-a.
Verificar é preciso para que não haja varias chamadas da
mesma biblioteca. */
#define DMP_H

#include <hal_w2_isr.h>
#include "stdint.h"
#include "stdbool.h"
#include "mpu6050_reg.h"
#include "stdlib.h"//malloc e free
#include <string.h> //memcmp 
#define MPU_endereco MPU6050_DEFAULT_ADDRESS

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

void setMemoryBank(uint8_t xdata bank, bool xdata prefetchEnabled, bool xdata userBank) {
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
void setMemoryStartAddress(uint8_t address) {
    i2c_mpu_writeByte(MPU_endereco, MPU6050_RA_MEM_START_ADDR, address);
}

//BUG: pq nao da pra colocar dentro da função?
uint8_t xdata chunkSize_wmb;
uint8_t xdata *verifyBuffer_wmb;
uint8_t xdata *progBuffer_wmb=0;
uint16_t xdata i_wmb;
uint8_t xdata j_wmb;
bool writeMemoryBlock(uint8_t xdata *data_ptr, uint16_t xdata dataSize, uint8_t xdata bank, uint8_t xdata address, bool xdata verify, bool xdata useProgMem) {
    setMemoryBank(bank,false,false);
    setMemoryStartAddress(address);
    if (verify) verifyBuffer_wmb = (uint8_t *)malloc(MPU6050_DMP_MEMORY_CHUNK_SIZE);
    if (useProgMem) progBuffer_wmb = (uint8_t *)malloc(MPU6050_DMP_MEMORY_CHUNK_SIZE);
    for (i_wmb = 0; i_wmb < dataSize;) {
        // determine correct chunk size according to bank position and data size
        chunkSize_wmb = MPU6050_DMP_MEMORY_CHUNK_SIZE;

        // make sure we don't go past the data size
        if (i_wmb + chunkSize_wmb > dataSize) chunkSize_wmb = dataSize - i_wmb;

        // make sure this chunk doesn't go past the bank boundary (256 bytes)
        if (chunkSize_wmb > 256 - address) chunkSize_wmb = 256 - address;
        
        if (useProgMem) {
            // write the chunk of data as specified
            for (j_wmb = 0; j_wmb < chunkSize_wmb; j_wmb++) progBuffer_wmb[j_wmb] = pgm_read_byte(data_ptr + i_wmb + j_wmb);
        } else {
            // write the chunk of data as specified
            progBuffer_wmb = (uint8_t *)data_ptr + i_wmb;
        }

        i2c_mpu_writeBytes(MPU_endereco, MPU6050_RA_MEM_R_W, chunkSize_wmb, progBuffer_wmb);

        // verify data if needed
        if (verify && verifyBuffer_wmb) {
            setMemoryBank(bank,false,false);
            setMemoryStartAddress(address);
            i2c_mpu_readBytes(MPU_endereco, MPU6050_RA_MEM_R_W, chunkSize_wmb, verifyBuffer_wmb);
            if (memcmp(progBuffer_wmb, verifyBuffer_wmb, chunkSize_wmb) != 0) {
                free(verifyBuffer_wmb);
                if (useProgMem) free(progBuffer_wmb);
                return false; // uh oh.
            }
        }

        // increase byte index by [chunkSize_wmb]
        i_wmb += chunkSize_wmb;

        // uint8_t automatically wraps to 0 at 256
        address += chunkSize_wmb;

        // if we aren't done, update bank (if necessary) and address
        if (i_wmb < dataSize) {
            if (address == 0) bank++;
            setMemoryBank(bank,false,false);//BUG:XXX:TODO: estou usando false e false
            setMemoryStartAddress(address);
        }
    }
    if (verify) free(verifyBuffer_wmb);
    if (useProgMem) free(progBuffer_wmb);
    return true;
}

uint8_t xdata *progBuffer_wdcs = 0;
uint8_t xdata success_wdcs, special_wdcs;
uint16_t xdata i_wdcs, j_wdcs;
uint8_t xdata bank_wdcs, offset_wdcs, length_wdcs;
bool writeDMPConfigurationSet(uint8_t xdata *data_ptr, uint16_t xdata dataSize, bool xdata useProgMem) {
    if (useProgMem) {
        progBuffer_wdcs = (uint8_t *)malloc(8); // assume 8-byte blocks, realloc later if necessary
    }

    // config set data is a long string of blocks with the following structure:
    // [bank_wdcs] [offset_wdcs] [length_wdcs] [byte[0], byte[1], ..., byte[length_wdcs]]
    
    for (i_wdcs = 0; i_wdcs < dataSize;) {
        if (useProgMem) {
            bank_wdcs = pgm_read_byte(data_ptr + i_wdcs++);
            offset_wdcs = pgm_read_byte(data_ptr + i_wdcs++);
            length_wdcs = pgm_read_byte(data_ptr + i_wdcs++);
        } else {
            bank_wdcs = data_ptr[i_wdcs++];
            offset_wdcs = data_ptr[i_wdcs++];
            length_wdcs = data_ptr[i_wdcs++];
        }

        // write data or perform special action
        if (length_wdcs > 0) {
            // regular block of data to write
            if (useProgMem) {
                if (sizeof(progBuffer_wdcs) < length_wdcs) progBuffer_wdcs = (uint8_t *)realloc(progBuffer_wdcs, length_wdcs);
                for (j_wdcs = 0; j_wdcs < length_wdcs; j_wdcs++) progBuffer_wdcs[j_wdcs] = pgm_read_byte(data_ptr + i_wdcs + j_wdcs);
            } else {
                progBuffer_wdcs = (uint8_t *)data_ptr + i_wdcs;
            }
            success_wdcs = writeMemoryBlock(progBuffer_wdcs, length_wdcs, bank_wdcs, offset_wdcs, true,false);
            i_wdcs += length_wdcs;
        } else {
            // special instruction
            // NOTE: this kind of behavior (what and when to do certain things)
            // is totally undocumented. This code is in here based on observed
            // behavior only, and exactly why (or even whether) it has to be here
            // is anybody's guess for now.
            if (useProgMem) {
                special_wdcs = pgm_read_byte(data_ptr + i_wdcs++);
            } else {
                special_wdcs = data_ptr[i_wdcs++];
            }
            /*Serial.print("Special command code ");
            Serial.print(special, HEX);
            Serial.println(" found...");*/
            if (special_wdcs == 0x01) {
                // enable DMP-related interrupts
                
                //setIntZeroMotionEnabled(true);
                //setIntFIFOBufferOverflowEnabled(true);
                //setIntDMPEnabled(true);
                i2c_mpu_writeByte(MPU_endereco, MPU6050_RA_INT_ENABLE, 0x32);  // single operation

                success_wdcs = true;
            } else {
                // unknown special command
                success_wdcs = false;
            }
        }
        
        if (!success_wdcs) {
            if (useProgMem) free(progBuffer_wdcs);
            return false; // uh oh
        }
    }
    if (useProgMem) free(progBuffer_wdcs);
    return true;
}

uint8_t xdata chunkSize_rmb;
uint16_t xdata i_rmb;
void readMemoryBlock(uint8_t xdata *data_ptr, uint16_t xdata dataSize, uint8_t xdata bank, uint8_t xdata address) {
    setMemoryBank(bank,false,false);
    setMemoryStartAddress(address);
        for (i_rmb=0; i_rmb < dataSize;) {
        // determine correct chunk size according to bank position and data_ptr size
        chunkSize_rmb = MPU6050_DMP_MEMORY_CHUNK_SIZE;

        // make sure we don't go past the data_ptr size
        if (i_rmb + chunkSize_rmb > dataSize) chunkSize_rmb = dataSize - i_rmb;

        // make sure this chunk doesn't go past the bank boundary (256 bytes)
        if (chunkSize_rmb > 256 - address) chunkSize_rmb = 256 - address;

        // read the chunk of data_ptr as specified
        i2c_mpu_readBytes(MPU_endereco, MPU6050_RA_MEM_R_W, chunkSize_rmb, data_ptr + i_rmb);
        
        // increase byte index by [chunkSize_rmb]
        i_rmb += chunkSize_rmb;

        // uint8_t automatically wraps to 0 at 256
        address += chunkSize_rmb;

        // if we aren't done, update bank (if necessary) and address
        if (i_rmb < dataSize) {
            if (address == 0) bank++;
            setMemoryBank(bank,false,false);
            setMemoryStartAddress(address);
        }
    }
}
bool writeProgDMPConfigurationSet(uint8_t xdata *data_ptr, uint16_t xdata dataSize) {
    return writeDMPConfigurationSet(data_ptr, dataSize, true);
}

#endif
