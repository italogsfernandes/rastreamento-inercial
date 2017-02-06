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
uint8_t xdata * xdata dmpPacketBuffer;
uint16_t xdata dmpPacketSize;
uint8_t xdata malloc_memory_pool[500];

void mpu_8051_malloc_setup() large {
    init_mempool (&malloc_memory_pool, sizeof(malloc_memory_pool));
}
void setClockSource(uint8_t source) large {
    i2c_mpu_writeBits(MPU_endereco, MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_CLKSEL_BIT, MPU6050_PWR1_CLKSEL_LENGTH, source);
}
void setFullScaleGyroRange(uint8_t range) large {
    i2c_mpu_writeBits(MPU_endereco, MPU6050_RA_GYRO_CONFIG, MPU6050_GCONFIG_FS_SEL_BIT, MPU6050_GCONFIG_FS_SEL_LENGTH, range);
}
void setFullScaleAccelRange(uint8_t range) large {
    i2c_mpu_writeBits(MPU_endereco, MPU6050_RA_ACCEL_CONFIG, MPU6050_ACONFIG_AFS_SEL_BIT, MPU6050_ACONFIG_AFS_SEL_LENGTH, range);
}
void setSleepEnabled(bool enabled) large {
    i2c_mpu_writeBit(MPU_endereco, MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_SLEEP_BIT, enabled);
}
void mpu_initialize() large {
  setClockSource(MPU6050_CLOCK_PLL_XGYRO);
  setFullScaleGyroRange(MPU6050_GYRO_FS_250);
  setFullScaleAccelRange(MPU6050_ACCEL_FS_2);
  setSleepEnabled(false);
}
bool mpu_testConnection() large {
    i2c_mpu_readBits(MPU_endereco,MPU6050_RA_WHO_AM_I, MPU6050_WHO_AM_I_BIT, MPU6050_WHO_AM_I_LENGTH, buffer);
    return buffer[0] == 0x34;
}
void getMotion6_packet(uint8_t xdata * packet6) large {
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
void getMotion6_variables(int16_t *ax,int16_t *ay,int16_t *az,int16_t *gx, int16_t *gy, int16_t *gz) large {
    i2c_mpu_readBytes(MPU_endereco, 0x3B, 14, buffer);
    *ax = buffer[0] << 8 | buffer[1];
    *ay = buffer[2] << 8 | buffer[3];
    *az = buffer[4] << 8 | buffer[5];
    *gx = buffer[8] << 8 | buffer[9];
    *gy = buffer[10] << 8 | buffer[11];
    *gz = buffer[12] << 8 | buffer[13];
}
void setXAccelOffset(int16_t offset) large {
    i2c_mpu_writeWord(MPU_endereco,  MPU6050_RA_XA_OFFS_H, offset);
}
void setYAccelOffset(int16_t offset) large {
    i2c_mpu_writeWord(MPU_endereco, MPU6050_RA_YA_OFFS_H, offset);
}
void setZAccelOffset(int16_t offset) large {
    i2c_mpu_writeWord(MPU_endereco, MPU6050_RA_ZA_OFFS_H, offset);
}
void setXGyroOffset(int16_t offset) large {
    i2c_mpu_writeWord(MPU_endereco, MPU6050_RA_XG_OFFS_USRH, offset);
}
void setYGyroOffset(int16_t offset) large {
    i2c_mpu_writeWord(MPU_endereco, MPU6050_RA_YG_OFFS_USRH, offset);
}
void setZGyroOffset(int16_t offset) large {
    i2c_mpu_writeWord(MPU_endereco, MPU6050_RA_ZG_OFFS_USRH, offset);
}

int16_t getXAccelOffset() large {
    i2c_mpu_readBytes(MPU_endereco, MPU6050_RA_XA_OFFS_H, 2, buffer);
    return (((int16_t)buffer[0]) << 8) | buffer[1];
}
int16_t getYAccelOffset() large {
    i2c_mpu_readBytes(MPU_endereco, MPU6050_RA_YA_OFFS_H, 2, buffer);
    return (((int16_t)buffer[0]) << 8) | buffer[1];
}
int16_t getZAccelOffset() large {
    i2c_mpu_readBytes(MPU_endereco, MPU6050_RA_ZA_OFFS_H, 2, buffer);
    return (((int16_t)buffer[0]) << 8) | buffer[1];
}
int16_t getXGyroOffset() large {
    i2c_mpu_readBytes(MPU_endereco, MPU6050_RA_XG_OFFS_USRH, 2, buffer);
    return (((int16_t)buffer[0]) << 8) | buffer[1];
}
int16_t getYGyroOffset() large {
    i2c_mpu_readBytes(MPU_endereco, MPU6050_RA_YG_OFFS_USRH, 2, buffer);
    return (((int16_t)buffer[0]) << 8) | buffer[1];
}
int16_t getZGyroOffset() large {
    i2c_mpu_readBytes(MPU_endereco, MPU6050_RA_ZG_OFFS_USRH, 2, buffer);
    return (((int16_t)buffer[0]) << 8) | buffer[1];
}

void setMemoryBank(uint8_t xdata bank, bool xdata prefetchEnabled, bool xdata userBank) large {
    bank &= 0x1F;
    if (userBank) bank |= 0x20;
    if (prefetchEnabled) bank |= 0x40;
    i2c_mpu_writeByte(MPU_endereco, MPU6050_RA_BANK_SEL, bank);
}

uint16_t getFIFOCount() large {
    i2c_mpu_readBytes(MPU_endereco, MPU6050_RA_FIFO_COUNTH, 2, buffer);
    return (((uint16_t)buffer[0]) << 8) | buffer[1];
}

uint8_t getIntStatus() large {
    i2c_mpu_readByte(MPU_endereco, MPU6050_RA_INT_STATUS, buffer);
    return buffer[0];
}

void setDMPEnabled(bool enabled) large {
    i2c_mpu_writeBit(MPU_endereco, MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_DMP_EN_BIT, enabled);
}

void resetFIFO() large {
    i2c_mpu_writeBit(MPU_endereco, MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_FIFO_RESET_BIT, true);
}

uint16_t dmpGetFIFOPacketSize() large {
    return dmpPacketSize;
}

void getFIFOBytes(uint8_t *data_ptr, uint8_t data_len) large {
    if(data_len > 0){
        i2c_mpu_readBytes(MPU_endereco, MPU6050_RA_FIFO_R_W, data_len, data_ptr);
    } else {
        *data_ptr = 0;
    }
}
void setMemoryStartAddress(uint8_t xdata address) large {
    i2c_mpu_writeByte(MPU_endereco, MPU6050_RA_MEM_START_ADDR, address);
}

//memory data = 8bytes
bool writeMemoryBlock(uint8_t *data_ptr, uint16_t dataSize, uint8_t bank, uint8_t address, bool verify, bool useProgMem) large {
    //BUG: pq nao da pra colocar dentro da função?
    uint8_t chunkSize_wmb;
    uint8_t * verifyBuffer_wmb;
    uint8_t * progBuffer_wmb=0;
    uint16_t i_wmb;
    uint8_t j_wmb;
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



//memory data  = 8bytes
bool writeDMPConfigurationSet(uint8_t *data_ptr, uint16_t dataSize, bool useProgMem) large {
    uint8_t * xdata progBuffer_wdcs = 0;
    uint8_t xdata success_wdcs, special_wdcs;
    uint16_t i_wdcs, j_wdcs;
    uint8_t xdata bank_wdcs, offset_wdcs, length_wdcs;
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



void readMemoryBlock(uint8_t  *data_ptr, uint16_t dataSize, uint8_t bank, uint8_t address) large {
    uint8_t xdata chunkSize_rmb;
    uint16_t i_rmb;
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


bool writeProgDMPConfigurationSet(uint8_t *data_ptr, uint16_t xdata dataSize) large {
    return writeDMPConfigurationSet(data_ptr, dataSize, true);
}
bool writeProgMemoryBlock(uint8_t *data_ptr, uint16_t dataSize, uint8_t bank, uint8_t address, bool verify) large {
    return writeMemoryBlock(data_ptr, dataSize, bank, address, verify, true);
}
void i2c_mpu_reset() large {
    i2c_mpu_writeBit(MPU_endereco, MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_DEVICE_RESET_BIT, true);
}
void setSlaveAddress(uint8_t num, uint8_t address) large {
    if (num > 3) return;
    i2c_mpu_writeByte(MPU_endereco, MPU6050_RA_I2C_SLV0_ADDR + num*3, address);
}
void setI2CMasterModeEnabled(bool enabled) large {
    i2c_mpu_writeBit(MPU_endereco, MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_I2C_MST_EN_BIT, enabled);
}
void resetI2CMaster() large {
    i2c_mpu_writeBit(MPU_endereco, MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_I2C_MST_RESET_BIT, true);
}
void setIntEnabled(uint8_t enabled) large {
    i2c_mpu_writeByte(MPU_endereco, MPU6050_RA_INT_ENABLE, enabled);
}
void setRate(uint8_t rate) large {
    i2c_mpu_writeByte(MPU_endereco, MPU6050_RA_SMPLRT_DIV, rate);
}
void setExternalFrameSync(uint8_t sync_ext) large {
    i2c_mpu_writeBits(MPU_endereco, MPU6050_RA_CONFIG, MPU6050_CFG_EXT_SYNC_SET_BIT, MPU6050_CFG_EXT_SYNC_SET_LENGTH, sync_ext);
}
void setDLPFMode(uint8_t mode) large {
    i2c_mpu_writeBits(MPU_endereco, MPU6050_RA_CONFIG, MPU6050_CFG_DLPF_CFG_BIT, MPU6050_CFG_DLPF_CFG_LENGTH, mode);
}
// DMP_CFG_1 register

uint8_t getDMPConfig1() large {
    i2c_mpu_readByte(MPU_endereco, MPU6050_RA_DMP_CFG_1, buffer);
    return buffer[0];
}
void setDMPConfig1(uint8_t config) large {
    i2c_mpu_writeByte(MPU_endereco, MPU6050_RA_DMP_CFG_1, config);
}

// DMP_CFG_2 register

uint8_t getDMPConfig2() large {
    i2c_mpu_readByte(MPU_endereco, MPU6050_RA_DMP_CFG_2, buffer);
    return buffer[0];
}
void setDMPConfig2(uint8_t config) large {
    i2c_mpu_writeByte(MPU_endereco, MPU6050_RA_DMP_CFG_2, config);
}
uint8_t getOTPBankValid() large {
    i2c_mpu_readBit(MPU_endereco, MPU6050_RA_XG_OFFS_TC, MPU6050_TC_OTP_BNK_VLD_BIT, buffer);
    return buffer[0];
}
void setOTPBankValid(bool enabled) large {
    i2c_mpu_writeBit(MPU_endereco, MPU6050_RA_XG_OFFS_TC, MPU6050_TC_OTP_BNK_VLD_BIT, enabled);
}
int8_t getXGyroOffsetTC() large {
    i2c_mpu_readBits(MPU_endereco, MPU6050_RA_XG_OFFS_TC, MPU6050_TC_OFFSET_BIT, MPU6050_TC_OFFSET_LENGTH, buffer);
    return buffer[0];
}
void setXGyroOffsetTC(int8_t offset) large {
    i2c_mpu_writeBits(MPU_endereco, MPU6050_RA_XG_OFFS_TC, MPU6050_TC_OFFSET_BIT, MPU6050_TC_OFFSET_LENGTH, offset);
}

// YG_OFFS_TC register

int8_t getYGyroOffsetTC() large {
    i2c_mpu_readBits(MPU_endereco, MPU6050_RA_YG_OFFS_TC, MPU6050_TC_OFFSET_BIT, MPU6050_TC_OFFSET_LENGTH, buffer);
    return buffer[0];
}
void setYGyroOffsetTC(int8_t offset) large {
    i2c_mpu_writeBits(MPU_endereco, MPU6050_RA_YG_OFFS_TC, MPU6050_TC_OFFSET_BIT, MPU6050_TC_OFFSET_LENGTH, offset);
}

// ZG_OFFS_TC register

int8_t getZGyroOffsetTC() large {
    i2c_mpu_readBits(MPU_endereco, MPU6050_RA_ZG_OFFS_TC, MPU6050_TC_OFFSET_BIT, MPU6050_TC_OFFSET_LENGTH, buffer);
    return buffer[0];
}
void setZGyroOffsetTC(int8_t offset) large {
    i2c_mpu_writeBits(MPU_endereco, MPU6050_RA_ZG_OFFS_TC, MPU6050_TC_OFFSET_BIT, MPU6050_TC_OFFSET_LENGTH, offset);
}

uint8_t getMotionDetectionThreshold() large {
    i2c_mpu_readByte(MPU_endereco, MPU6050_RA_MOT_THR, buffer);
    return buffer[0];
}
void setMotionDetectionThreshold(uint8_t threshold) large {
    i2c_mpu_writeByte(MPU_endereco, MPU6050_RA_MOT_THR, threshold);
}
uint8_t getZeroMotionDetectionThreshold() large {
    i2c_mpu_readByte(MPU_endereco, MPU6050_RA_ZRMOT_THR, buffer);
    return buffer[0];
}

void setZeroMotionDetectionThreshold(uint8_t threshold) large {
    i2c_mpu_writeByte(MPU_endereco, MPU6050_RA_ZRMOT_THR, threshold);
}
uint8_t getMotionDetectionDuration() large {
    i2c_mpu_readByte(MPU_endereco, MPU6050_RA_MOT_DUR, buffer);
    return buffer[0];
}

void setMotionDetectionDuration(uint8_t duration) large {
    i2c_mpu_writeByte(MPU_endereco, MPU6050_RA_MOT_DUR, duration);
}

uint8_t getZeroMotionDetectionDuration() large {
    i2c_mpu_readByte(MPU_endereco, MPU6050_RA_ZRMOT_DUR, buffer);
    return buffer[0];
}

void setZeroMotionDetectionDuration(uint8_t duration) large {
    i2c_mpu_writeByte(MPU_endereco, MPU6050_RA_ZRMOT_DUR, duration);
}
bool getFIFOEnabled() large {
    i2c_mpu_readBit(MPU_endereco, MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_FIFO_EN_BIT, buffer);
    return buffer[0];
}

void setFIFOEnabled(bool enabled) large {
    i2c_mpu_writeBit(MPU_endereco, MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_FIFO_EN_BIT, enabled);
}
void resetDMP() large {
    i2c_mpu_writeBit(MPU_endereco, MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_DMP_RESET_BIT, true);
}


uint8_t dmpInitialize() large {
    uint8_t xgOffsetTC,ygOffsetTC,zgOffsetTC;
		uint8_t dmpUpdate[16], j;
		uint16_t pos = 0;
		uint16_t fifoCount;
		uint8_t fifoBuffer[128];

		uint8_t b;
		uint8_t data_to_write;
		uint8_t mask;
		// reset device
    //DEBUG_PRINTLN(F("\n\nResetting MPU6050..."));
		send_packet_to_host(UART_PACKET_TYPE_STRING,"Resetting MPU6050",17);delay_ms(10);
    i2c_mpu_reset();
    delay_ms(30); // wait after reset

    // enable sleep mode and wake cycle
    /*Serial.println(F("Enabling sleep mode..."));
    setSleepEnabled(true);
    Serial.println(F("Enabling wake cycle..."));
    setWakeCycleEnabled(true);*/

    // disable sleep mode
    //DEBUG_PRINTLN(F("Disabling sleep mode..."));
		send_packet_to_host(UART_PACKET_TYPE_STRING,"Disabling sleep mode",20);delay_ms(10);
    setSleepEnabled(false);
    // get MPU hardware revision
    //DEBUG_PRINTLN(F("Selecting user bank 16..."));
		send_packet_to_host(UART_PACKET_TYPE_STRING,"Selecting user bank 16",22);delay_ms(10);
    setMemoryBank(0x10, true, true);
    //DEBUG_PRINTLN(F("Selecting memory byte 6..."));
		send_packet_to_host(UART_PACKET_TYPE_STRING,"Selecting memory byte 6",23);delay_ms(10);
    setMemoryStartAddress(0x06);
    //DEBUG_PRINTLN(F("Checking hardware revision..."));
    //DEBUG_PRINT(F("Revision @ user[16][6] = "));
    //DEBUG_PRINTLNF(readMemoryByte(), HEX);
    //DEBUG_PRINTLN(F("Resetting memory bank selection to 0..."));
		send_packet_to_host(UART_PACKET_TYPE_STRING,"Resetting memory bank",21);delay_ms(10);
    setMemoryBank(0, false, false);

    // check OTP bank valid
    //DEBUG_PRINTLN(F("Reading OTP bank valid flag..."));
    //DEBUG_PRINT(F("OTP bank is "));
   // DEBUG_PRINTLN(getOTPBankValid() ? F("valid!") : F("invalid!"));

    // get X/Y/Z gyro offsets
    //DEBUG_PRINTLN(F("Reading gyro offset TC values..."));
		send_packet_to_host(UART_PACKET_TYPE_STRING,"Reading gyro offset TC",22);delay_ms(10);
		xgOffsetTC = getXGyroOffsetTC();
		ygOffsetTC = getYGyroOffsetTC();
		zgOffsetTC = getZGyroOffsetTC();
    //DEBUG_PRINT(F("X gyro offset = "));
    //DEBUG_PRINTLN(xgOffsetTC);
    //DEBUG_PRINT(F("Y gyro offset = "));
    //DEBUG_PRINTLN(ygOffsetTC);
    //DEBUG_PRINT(F("Z gyro offset = "));
    //DEBUG_PRINTLN(zgOffsetTC);

    // setup weird slave stuff (?)
    //DEBUG_PRINTLN(F("Setting slave 0 address to 0x7F..."));
		send_packet_to_host(UART_PACKET_TYPE_STRING,"Setting slave 0",15);delay_ms(10);
    setSlaveAddress(0, 0x7F);
    //DEBUG_PRINTLN(F("Disabling I2C Master mode..."));
		send_packet_to_host(UART_PACKET_TYPE_STRING,"Disabling I2C Master",20);delay_ms(10);
    setI2CMasterModeEnabled(false);
    //DEBUG_PRINTLN(F("Setting slave 0 address to 0x68 (self)..."));
    setSlaveAddress(0, 0x68);
    ///DEBUG_PRINTLN(F("Resetting I2C Master control..."));
    resetI2CMaster();
    delay_ms(20);

    // load DMP code into memory banks
    //DEBUG_PRINT(F("Writing DMP code to MPU memory banks ("));
    //DEBUG_PRINT(MPU6050_DMP_CODE_SIZE);
    //DEBUG_PRINTLN(F(" bytes)"));
		//bool writeProgMemoryBlock(const uint8_t *data, uint16_t dataSize, uint8_t bank=0, uint8_t address=0, bool verify=true);
		send_packet_to_host(UART_PACKET_TYPE_STRING,"Writing DMP code",16);delay_ms(10);
		if (writeProgMemoryBlock(dmpMemory, MPU6050_DMP_CODE_SIZE,0,0,true)) {
        //DEBUG_PRINTLN(F("Success! DMP code written and verified."));
				send_packet_to_host(UART_PACKET_TYPE_STRING,"Success! DMP code",17);delay_ms(10);
        // write DMP configuration
        //DEBUG_PRINT(F("Writing DMP configuration to MPU memory banks ("));
        //DEBUG_PRINT(MPU6050_DMP_CONFIG_SIZE);
        //DEBUG_PRINTLN(F(" bytes in config def)"));
				send_packet_to_host(UART_PACKET_TYPE_STRING,"Writing DMP code2",17);delay_ms(10);
        if (writeProgDMPConfigurationSet(dmpConfig, MPU6050_DMP_CONFIG_SIZE)) {
            //DEBUG_PRINTLN(F("Success! DMP configuration written and verified."));
						send_packet_to_host(UART_PACKET_TYPE_STRING,"Success! DMP code2",18);delay_ms(10);
            //DEBUG_PRINTLN(F("Setting clock source to Z Gyro..."));
						send_packet_to_host(UART_PACKET_TYPE_STRING,"Setting clock source to Z",25);delay_ms(10);
            setClockSource(MPU6050_CLOCK_PLL_ZGYRO);

            //DEBUG_PRINTLN(F("Setting DMP and FIFO_OFLOW interrupts enabled..."));
						send_packet_to_host(UART_PACKET_TYPE_STRING,"DMP and FIFO_OFLOW int",22);delay_ms(10);
            setIntEnabled(0x12);

            //DEBUG_PRINTLN(F("Setting sample rate to 200Hz..."));
						send_packet_to_host(UART_PACKET_TYPE_STRING,"rate to 200Hz",13);delay_ms(10);
            setRate(4); // 1khz / (1 + 4) = 200 Hz

            //DEBUG_PRINTLN(F("Setting external frame sync to TEMP_OUT_L[0]..."));
						send_packet_to_host(UART_PACKET_TYPE_STRING,"external frame sync",19);delay_ms(10);
						//BUG: software travando aqui
						//setExternalFrameSync(MPU6050_EXT_SYNC_TEMP_OUT_L);
						//i2c_mpu_writeBits(MPU_endereco, 0x1A, 5, 3, 0x1);

						data_to_write = 0x1;
						send_packet_to_host(UART_PACKET_TYPE_HEX,&data_to_write,1);delay_ms(10);
						send_packet_to_host(UART_PACKET_TYPE_HEX,&data_to_write,1);delay_ms(10);
						if(i2c_mpu_readByte(MPU_endereco, 0x1A, &b)!= 0){
							send_packet_to_host(UART_PACKET_TYPE_STRING,"read success!",13);delay_ms(10);
						} else {
							send_packet_to_host(UART_PACKET_TYPE_STRING,"read not success!",17);delay_ms(10);
						}

						send_packet_to_host(UART_PACKET_TYPE_HEX,&b,1);delay_ms(10);
						mask = ((1 << 5) - 1) << (5 - 3 + 1);
						data_to_write <<= (5 - 3 + 1); // shift data into correct position
						data_to_write &= mask; // zero all non-important bits in data
						b &= ~(mask); // zero all important bits in existing byte
						b |= data_to_write; // combine data with existing byte
						send_packet_to_host(UART_PACKET_TYPE_HEX,&b,1);delay_ms(10);

						if(i2c_mpu_writeByte(MPU_endereco, 0x1A, b)!= 0){
							send_packet_to_host(UART_PACKET_TYPE_STRING,"write success!",14);delay_ms(10);
						} else {
							send_packet_to_host(UART_PACKET_TYPE_STRING,"write not success!",18);delay_ms(10);
						}



						send_packet_to_host(UART_PACKET_TYPE_STRING,"external frame ok",17);delay_ms(10);
            //DEBUG_PRINTLN(F("Setting DLPF bandwidth to 42Hz..."));
						send_packet_to_host(UART_PACKET_TYPE_STRING,"DLPF bandwidth to 42Hz",22);delay_ms(10);
            setDLPFMode(MPU6050_DLPF_BW_42);

            //DEBUG_PRINTLN(F("Setting gyro sensitivity to +/- 2000 deg/sec..."));
						send_packet_to_host(UART_PACKET_TYPE_STRING,"gyro sen..ity to 2000",21);delay_ms(10);
            setFullScaleGyroRange(MPU6050_GYRO_FS_2000);

            //DEBUG_PRINTLN(F("Setting DMP configuration bytes (function unknown)..."));
						send_packet_to_host(UART_PACKET_TYPE_STRING,"function unknown",16);delay_ms(10);
            setDMPConfig1(0x03);
            setDMPConfig2(0x00);

            //DEBUG_PRINTLN(F("Clearing OTP Bank flag..."));
						send_packet_to_host(UART_PACKET_TYPE_STRING,"Clearing OTP Bank",17);delay_ms(10);
            setOTPBankValid(false);

            //DEBUG_PRINTLN(F("Setting X/Y/Z gyro offset TCs to previous values..."));
						send_packet_to_host(UART_PACKET_TYPE_STRING,"previous offsets",16);delay_ms(10);
            //setXGyroOffsetTC(xgOffsetTC);
						//i2c_mpu_writeBits(MPU_endereco, MPU6050_RA_XG_OFFS_TC, MPU6050_TC_OFFSET_BIT, MPU6050_TC_OFFSET_LENGTH, xgOffsetTC);

						send_packet_to_host(UART_PACKET_TYPE_HEX,&xgOffsetTC,1);delay_ms(10);
						if(i2c_mpu_readByte(MPU_endereco, 0x00, &b)!= 0){
							send_packet_to_host(UART_PACKET_TYPE_STRING,"read success!",13);delay_ms(10);
						} else {
							send_packet_to_host(UART_PACKET_TYPE_STRING,"read not success!",17);delay_ms(10);
						}

						send_packet_to_host(UART_PACKET_TYPE_HEX,&b,1);delay_ms(10);
						mask = ((1 << 6) - 1) << (6 - 6 + 1);
						xgOffsetTC <<= (6 - 6 + 1); // shift data into correct position
						xgOffsetTC &= mask; // zero all non-important bits in data
						b &= ~(mask); // zero all important bits in existing byte
						b |= xgOffsetTC; // combine data with existing byte
						send_packet_to_host(UART_PACKET_TYPE_HEX,&b,1);delay_ms(10);

						if(i2c_mpu_writeByte(MPU_endereco, 0x00, b)!= 0){
							send_packet_to_host(UART_PACKET_TYPE_STRING,"write success!",14);delay_ms(10);
						} else {
							send_packet_to_host(UART_PACKET_TYPE_STRING,"write not success!",18);delay_ms(10);
						}

						//setYGyroOffsetTC(ygOffsetTC);
						//i2c_mpu_writeBits(MPU_endereco, MPU6050_RA_YG_OFFS_TC, MPU6050_TC_OFFSET_BIT, MPU6050_TC_OFFSET_LENGTH, ygOffsetTC);
            send_packet_to_host(UART_PACKET_TYPE_HEX,&ygOffsetTC,1);delay_ms(10);
						if(i2c_mpu_readByte(MPU_endereco, 0x01, &b)!= 0){
							send_packet_to_host(UART_PACKET_TYPE_STRING,"read success!",13);delay_ms(10);
						} else {
							send_packet_to_host(UART_PACKET_TYPE_STRING,"read not success!",17);delay_ms(10);
						}

						send_packet_to_host(UART_PACKET_TYPE_HEX,&b,1);delay_ms(10);
						mask = ((1 << 6) - 1) << (6 - 6 + 1);
						ygOffsetTC <<= (6 - 6 + 1); // shift data into correct position
						ygOffsetTC &= mask; // zero all non-important bits in data
						b &= ~(mask); // zero all important bits in existing byte
						b |= ygOffsetTC; // combine data with existing byte
						send_packet_to_host(UART_PACKET_TYPE_HEX,&b,1);delay_ms(10);

						if(i2c_mpu_writeByte(MPU_endereco, 0x01, b)!= 0){
							send_packet_to_host(UART_PACKET_TYPE_STRING,"write success!",14);delay_ms(10);
						} else {
							send_packet_to_host(UART_PACKET_TYPE_STRING,"write not success!",18);delay_ms(10);
						}

						//setZGyroOffsetTC(zgOffsetTC);
						//i2c_mpu_writeBits(MPU_endereco, MPU6050_RA_ZG_OFFS_TC, MPU6050_TC_OFFSET_BIT, MPU6050_TC_OFFSET_LENGTH, zgOffsetTC);
						send_packet_to_host(UART_PACKET_TYPE_HEX,&zgOffsetTC,1);delay_ms(10);
						if(i2c_mpu_readByte(MPU_endereco, 0x02, &b)!= 0){
							send_packet_to_host(UART_PACKET_TYPE_STRING,"read success!",13);delay_ms(10);
						} else {
							send_packet_to_host(UART_PACKET_TYPE_STRING,"read not success!",17);delay_ms(10);
						}

						send_packet_to_host(UART_PACKET_TYPE_HEX,&b,1);delay_ms(10);
						mask = ((1 << 6) - 1) << (6 - 6 + 1);
						zgOffsetTC <<= (6 - 6 + 1); // shift data into correct position
						zgOffsetTC &= mask; // zero all non-important bits in data
						b &= ~(mask); // zero all important bits in existing byte
						b |= ygOffsetTC; // combine data with existing byte
						send_packet_to_host(UART_PACKET_TYPE_HEX,&b,1);delay_ms(10);

						if(i2c_mpu_writeByte(MPU_endereco, 0x02, b)!= 0){
							send_packet_to_host(UART_PACKET_TYPE_STRING,"write success!",14);delay_ms(10);
						} else {
							send_packet_to_host(UART_PACKET_TYPE_STRING,"write not success!",18);delay_ms(10);
						}


            //DEBUG_PRINTLN(F("Setting X/Y/Z gyro user offsets to zero..."));
            //setXGyroOffset(0);
            //setYGyroOffset(0);
            //setZGyroOffset(0);

            //DEBUG_PRINTLN(F("Writing final memory update 1/7 (function unknown)..."));
            send_packet_to_host(UART_PACKET_TYPE_STRING,"Writing 1/7",11);delay_ms(10);
            for (j = 0; j < 4 || j < dmpUpdate[2] + 3; j++, pos++) dmpUpdate[j] = pgm_read_byte(&dmpUpdates[pos]);

					//bool writeProgMemoryBlock(const uint8_t *data, uint16_t dataSize, uint8_t bank=0, uint8_t address=0, bool verify=true);
						writeMemoryBlock(dmpUpdate + 3, dmpUpdate[2], dmpUpdate[0], dmpUpdate[1],0,true);

            //DEBUG_PRINTLN(F("Writing final memory update 2/7 (function unknown)..."));
						send_packet_to_host(UART_PACKET_TYPE_STRING,"Writing 2/7",11);delay_ms(10);
            for (j = 0; j < 4 || j < dmpUpdate[2] + 3; j++, pos++) dmpUpdate[j] = pgm_read_byte(&dmpUpdates[pos]);
						//bool writeProgMemoryBlock(const uint8_t *data, uint16_t dataSize, uint8_t bank=0, uint8_t address=0, bool verify=true);
						writeMemoryBlock(dmpUpdate + 3, dmpUpdate[2], dmpUpdate[0], dmpUpdate[1],0,true);

            //DEBUG_PRINTLN(F("Resetting FIFO..."));
            resetFIFO();

            //DEBUG_PRINTLN(F("Reading FIFO count..."));
						fifoCount = getFIFOCount();


            //DEBUG_PRINT(F("Current FIFO count="));
            //DEBUG_PRINTLN(fifoCount);
            getFIFOBytes(fifoBuffer, fifoCount);

            //DEBUG_PRINTLN(F("Setting motion detection threshold to 2..."));
            setMotionDetectionThreshold(2);

            //DEBUG_PRINTLN(F("Setting zero-motion detection threshold to 156..."));
            setZeroMotionDetectionThreshold(156);

            //DEBUG_PRINTLN(F("Setting motion detection duration to 80..."));
            setMotionDetectionDuration(80);

            //DEBUG_PRINTLN(F("Setting zero-motion detection duration to 0..."));
            setZeroMotionDetectionDuration(0);

            //DEBUG_PRINTLN(F("Resetting FIFO..."));
            resetFIFO();

            //DEBUG_PRINTLN(F("Enabling FIFO..."));
            setFIFOEnabled(true);

            //DEBUG_PRINTLN(F("Enabling DMP..."));
            setDMPEnabled(true);

            //DEBUG_PRINTLN(F("Resetting DMP..."));
            resetDMP();

            //DEBUG_PRINTLN(F("Writing final memory update 3/7 (function unknown)..."));
						send_packet_to_host(UART_PACKET_TYPE_STRING,"Writing 3/7",11);delay_ms(10);
            for (j = 0; j < 4 || j < dmpUpdate[2] + 3; j++, pos++) dmpUpdate[j] = pgm_read_byte(&dmpUpdates[pos]);
            writeMemoryBlock(dmpUpdate + 3, dmpUpdate[2], dmpUpdate[0], dmpUpdate[1],0,true);

            //DEBUG_PRINTLN(F("Writing final memory update 4/7 (function unknown)..."));
						send_packet_to_host(UART_PACKET_TYPE_STRING,"Writing 4/7",11);delay_ms(10);
            for (j = 0; j < 4 || j < dmpUpdate[2] + 3; j++, pos++) dmpUpdate[j] = pgm_read_byte(&dmpUpdates[pos]);
            writeMemoryBlock(dmpUpdate + 3, dmpUpdate[2], dmpUpdate[0], dmpUpdate[1],0,true);

            //DEBUG_PRINTLN(F("Writing final memory update 5/7 (function unknown)..."));
						send_packet_to_host(UART_PACKET_TYPE_STRING,"Writing 5/7",11);delay_ms(10);
            for (j = 0; j < 4 || j < dmpUpdate[2] + 3; j++, pos++) dmpUpdate[j] = pgm_read_byte(&dmpUpdates[pos]);
            writeMemoryBlock(dmpUpdate + 3, dmpUpdate[2], dmpUpdate[0], dmpUpdate[1],0,true);

            //DEBUG_PRINTLN(F("Waiting for FIFO count > 2..."));
						send_packet_to_host(UART_PACKET_TYPE_STRING,"Waiting for FIFO count > 2",26);delay_ms(10);
            send_packet_to_host(UART_PACKET_TYPE_STRING,"FIFO count =",12);delay_ms(10);
						send_packet_to_host(UART_PACKET_TYPE_UINT16,(uint8_t *) &fifoCount,2);delay_ms(1);
						while ((fifoCount) < 3){
							fifoCount = getFIFOCount();
							send_packet_to_host(UART_PACKET_TYPE_UINT16,(uint8_t *) &fifoCount,2);delay_ms(1);
						}

            //DEBUG_PRINT(F("Current FIFO count="));
            //DEBUG_PRINTLN(fifoCount);
            //DEBUG_PRINTLN(F("Reading FIFO data..."));
            getFIFOBytes(fifoBuffer, fifoCount);

            //DEBUG_PRINTLN(F("Reading interrupt status..."));

            //DEBUG_PRINT(F("Current interrupt status="));
            //DEBUG_PRINTLNF(getIntStatus(), HEX);

            //DEBUG_PRINTLN(F("Reading final memory update 6/7 (function unknown)..."));
						send_packet_to_host(UART_PACKET_TYPE_STRING,"Writing 6/7",11);delay_ms(10);
            for (j = 0; j < 4 || j < dmpUpdate[2] + 3; j++, pos++) dmpUpdate[j] = pgm_read_byte(&dmpUpdates[pos]);
            readMemoryBlock(dmpUpdate + 3, dmpUpdate[2], dmpUpdate[0], dmpUpdate[1]);

            //DEBUG_PRINTLN(F("Waiting for FIFO count > 2..."));
						send_packet_to_host(UART_PACKET_TYPE_STRING,"Waiting for FIFO count > 2",26);delay_ms(10);
						while ((fifoCount) < 3){
							fifoCount = getFIFOCount();
						}

            //DEBUG_PRINT(F("Current FIFO count="));
            //DEBUG_PRINTLN(fifoCount);

            //DEBUG_PRINTLN(F("Reading FIFO data..."));
            getFIFOBytes(fifoBuffer, fifoCount);

            //DEBUG_PRINTLN(F("Reading interrupt status..."));

            //DEBUG_PRINT(F("Current interrupt status="));
            //DEBUG_PRINTLNF(getIntStatus(), HEX);

            //DEBUG_PRINTLN(F("Writing final memory update 7/7 (function unknown)..."));
						send_packet_to_host(UART_PACKET_TYPE_STRING,"Writing 7/7",11);delay_ms(10);
            for (j = 0; j < 4 || j < dmpUpdate[2] + 3; j++, pos++) dmpUpdate[j] = pgm_read_byte(&dmpUpdates[pos]);
            writeMemoryBlock(dmpUpdate + 3, dmpUpdate[2], dmpUpdate[0], dmpUpdate[1],0,true);

            //DEBUG_PRINTLN(F("DMP is good to go! Finally."));

            //DEBUG_PRINTLN(F("Disabling DMP (you turn it on later)..."));
            setDMPEnabled(false);

            //DEBUG_PRINTLN(F("Setting up internal 42-byte (default) DMP packet buffer..."));
            dmpPacketSize = 42;
            /*if ((dmpPacketBuffer = (uint8_t *)malloc(42)) == 0) {
                return 3; // TODO: proper error code for no memory
            }*/

            //DEBUG_PRINTLN(F("Resetting FIFO and clearing INT status one last time..."));
            resetFIFO();
            getIntStatus();
        } else {
            //DEBUG_PRINTLN(F("ERROR! DMP configuration verification failed."));
						send_packet_to_host(UART_PACKET_TYPE_STRING,"Success! DMP not2",17);delay_ms(10);
            return 2; // configuration block loading failed
        }
    } else {
        //DEBUG_PRINTLN(F("ERROR! DMP code verification failed."));
				send_packet_to_host(UART_PACKET_TYPE_STRING,"Success! DMP not1",17);delay_ms(10);
        return 1; // main binary block loading failed
    }
		send_packet_to_host(UART_PACKET_TYPE_STRING,"Success! DMP init",17);delay_ms(10);
    return 0; // success
}
uint8_t dmpGetQuaternion_int16(int16_t *data_ptr, const uint8_t* packet) large {
    // TODO: accommodate different arrangements of sent data (ONLY default supported now)
    if (packet == 0) packet = dmpPacketBuffer;
    data_ptr[0] = ((packet[0] << 8) | packet[1]);
    data_ptr[1] = ((packet[4] << 8) | packet[5]);
    data_ptr[2] = ((packet[8] << 8) | packet[9]);
    data_ptr[3] = ((packet[12] << 8) | packet[13]);
    return 0;
}
uint8_t dmpGetPacket16bits(uint8_t *data_ptr, uint8_t *packet) large {
    // TODO: accommodate different arrangements of sent data (ONLY default supported now)
    //if (packet == 0) packet = dmpPacketBuffer;
		data_ptr[0] = packet[0]; data_ptr[1] = packet[1];
		data_ptr[2] = packet[4];data_ptr[3] = packet[5];
		data_ptr[4] = packet[8];data_ptr[5] = packet[9];
		data_ptr[6] = packet[12];data_ptr[7] = packet[13];

		data_ptr[8] = packet[16];data_ptr[9] = packet[17];
		data_ptr[10] = packet[20];data_ptr[11] = packet[21];
		data_ptr[12] = packet[24];data_ptr[13] = packet[25];

		data_ptr[14] = packet[28];data_ptr[15] = packet[29];
		data_ptr[16] = packet[32];data_ptr[17] = packet[33];
		data_ptr[18] = packet[36];data_ptr[19] = packet[37];
    return 0;
}
#endif
