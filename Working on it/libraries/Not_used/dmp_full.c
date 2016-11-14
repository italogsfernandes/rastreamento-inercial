
//Biblioteca para I/O no nRF24Le1
#include "nrf24le1.h"

//Bibliotecas do exemplo de I2C
#include <hal_w2_isr.h>
#include "hal_delay.h"
#include "stdint.h"
#include "stdbool.h"

//************************TODO****************//
//-------------------readBits
//-------------------writeBit
//-------------------writeBits
//-------------------delay_i2c      //DONE
//-------------------readByte   //DONE
//-------------------i2c_mpu_readBytes  //DONE
//-------------------writeWord
//********************************************///

//************FUNÇÕES*************//
//  - delay_i2c(unsigned int x)
//  - mpu.initialize(); - Feito(Verificar)
//  - mpu.testConnection(); - Feito(Verificar)
//  - mpu.dmpInitialize(); -FIXME: Pedir ajudar
//  - mpu.set*Offset(int); - Feito(Verificar)
//  - mpu.get*Offset(); - Feito(Verificar)
//  - mpu.setDMPEnabled(bool); - Feito(Verificar)
//  - mpu.getIntStatus(); - Feito(Verificar)
//  - mpu.dmpGetFIFOPacketSize(); - FIXME: Ué?
//  - mpu.getFIFOCount(); - Feito(Verificar)
//  - mpu.resetFIFO(); - Feito(Verificar)
//  - mpu.getFIFOBytes(fifoBuffer, packetSize); - Feito(Verificar), FIXME: Implementar variaveis globais
//  - mpu.dmpGetQuaternion(&q, fifoBuffer); - Feito(Verificar)
//  - mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz); - Feito(Verificar)
//TODO: para facilitar a transferencia pensar sobre tranferir a FIFO
//**********************************************************//

//TODO: Melhorar implementação e uso de variaveis globais
uint8_t buffer[14]; //usado em testConnection e getIntStatus
uint16_t dmpPacketSize; //usado em dmpGetPacketSize e dmpInitialize
uint8_t *dmpPacketBuffer; //usado em dmpGetQuaternion

void delay_i2c(unsigned int x) {
    unsigned int i,j;
    i=0;
    for(i=0;i<x;i++)
    {
       j=508;
           ;
       while(j--);
    }
}

void initialize(void){
  writeBits(MPU_endereco, 0x6B, 2, 3, 0x01);//setClockSource(MPU6050_CLOCK_PLL_XGYRO);
  writeBits(MPU_endereco, 0x1B, 4, 2, 0x00);//setFullScaleGyroRange(MPU6050_GYRO_FS_250);
  writeBits(MPU_endereco, 0x1C, 4, 2, 0x00);//setFullScaleAccelRange(MPU6050_ACCEL_FS_2);
  writeBit(MPU_endereco, 0x6B, 6, false); //setSleepEnabled(false);
}

bool testConnection(void){
    readBits(MPU_endereco, 0x75, 6, 6, buffer);
    return buffer[0] == 0x34;
}

//BUG:
void dmpInitialize(void){
  /* Paços:
      - Reset device
      - Disable sleep mode
      - get MPU hardware revision
          - Selecting user bank 16
          - Selecting memory byte 6
          - Checking hardware revision
          - Reseting memory bank selection to 0
      - check OTP bank valid
      - get X/Y/Z gyro offsets
      - setup em coisas esquisitas la do slave de i2c (na biblioteca tava escrita desse jeito msm)
          - Setting slave 0 address to 0x7F
          - Disabling I2C Master mode
          - Setting slave 0 address to 0x68 (self)
          - Resetting I2C Master control
      - Load DMP code into memory banks
      - write DMP configuration
          - Setting clock source to Z Gyro
          - Setting DMP and FIFO_OFLOW interrupts enabled
          - Setting sample rate to 200HZ
          - Setting external frame sync to TEMP_OUT_L[0]
          - Setting DLPF bandwidth to 42Hz
          - Setting gyro sensitivity to +- 2000 deg/sec
          - Setting DMP configuration bytes (nem a biblioteca sabe o que isso aqui faz)
          - Clearing OTP Bank flag
          - Setting X/Y/Z gyro offset TCs to previous values (a biblioteca deu uma bugada aqui, olhar la pra rir kkkkk)
  */
  //Resetando
  /* Resetando
    reset();
    I2Cdev::writeBit(devAddr, MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_DEVICE_RESET_BIT, true);
    I2Cdev::writeBit(devAddr, 0x6B, 7, true);
                    devAddr, regAddr,Bit, value
    delay_i2c(30);
  */
  //reset
  //  I2Cdev::writeBit(devAddr, MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_DEVICE_RESET_BIT, true);
  writeBit(MPU_endereco, 0x6B, 7, true);
  delay_i2c(30);
  // disable sleep mode
  writeBit(MPU_endereco, 0x6B, 6, false); //setSleepEnabled(false);

  // get MPU hardware revision
  /*DEBUG_PRINTLN(F("Selecting user bank 16..."));*/
  setMemoryBank(0x10, true, true);
  writeByte(MPU_endereco,0x6D,bank);
  //setMemoryStartAddress(0x06);
  writeByte(MPU_endereco, 0x6E, 0x06);
  setMemoryBank(0, false, false);

  // get X/Y/Z gyro offsets
  int8_t xgOffsetTC = getXGyroOffsetTC();
  int8_t ygOffsetTC = getYGyroOffsetTC();
  int8_t zgOffsetTC = getZGyroOffsetTC();

  //BUG: nem o povo que fez a original sabia oq ta acontecendo
  //setSlaveAddress(0, 0x7F);
  writeByte(MPU_endereco, 0x25, 0x7F);
  //setI2CMasterModeEnabled(false);
  writeBit(MPU_endereco, 0x6A, 5, false);
  //setSlaveAddress(0, 0x68);
  writeByte(MPU_endereco, 0x25, 0x68);
  //resetI2CMaster()
  writeBit(MPU_endereco, 0x6A, 1, true);
  delay_i2c(20);

  // load DMP code into memory banks
  //TODO: write this functions, remenber the return condition
  if (writeProgMemoryBlock(dmpMemory, MPU6050_DMP_CODE_SIZE)) {
      //BUG: funcoes dentro do if
      if (writeProgDMPConfigurationSet(dmpConfig, MPU6050_DMP_CONFIG_SIZE)) {
          //setClockSource(MPU6050_CLOCK_PLL_ZGYRO);
          writeBits(MPU_endereco, 0x6B, 2, 3, 0x03);
          //setIntEnabled(0x12);
          writeByte(MPU_endereco, 0x38, 0x12);
          //setRate(4); // 1khz / (1 + 4) = 200 Hz
          writeByte(MPU_endereco, 0x19, 4);
          //setExternalFrameSync(MPU6050_EXT_SYNC_TEMP_OUT_L);
          writeBits(MPU_endereco, 0x1A, 5, 3, 0x1);
          //setDLPFMode(MPU6050_DLPF_BW_42);
          writeBits(MPU_endereco, 0x1A, 2, 3, 0x03);
          //setFullScaleGyroRange(MPU6050_GYRO_FS_2000);
          writeBits(MPU_endereco, 0x1B, 4, 2, 0x03);
          //setDMPConfig1(0x03);
          //setDMPConfig2(0x00);
          writeByte(MPU_endereco, 0x70, 0x03);
          writeByte(MPU_endereco, 0x71, 0x00);
          //setOTPBankValid(false);
          writeBit(MPU_endereco, 0x00, 0, false);
          //setXGyroOffsetTC(xgOffsetTC);
          //setYGyroOffsetTC(ygOffsetTC);
          //setZGyroOffsetTC(zgOffsetTC);
          writeBits(MPU_endereco, 0x00, 6, 6, xgOffsetTC);
          writeBits(MPU_endereco, 0x01, 6, 6, ygOffsetTC);
          writeBits(MPU_endereco, 0x02, 6, 6, zgOffsetTC);
          //XXX: I stopped here yesterday, ok i'm back

          //BUG: tipo de memoria a se utilizar
          uint8_t dmpUpdate[16], j;
          uint16_t pos = 0;
          //BUG: write memori block esta implementada?
          //pgm_read_byte implementada?
          //(F("Writing final memory update 1/7 (function unknown)..."))
          for (j = 0; j < 4 || j < dmpUpdate[2] + 3; j++, pos++) dmpUpdate[j] = pgm_read_byte(&dmpUpdates[pos]);
          writeMemoryBlock(dmpUpdate + 3, dmpUpdate[2], dmpUpdate[0], dmpUpdate[1]);

          //("Writing final memory update 2/7 (function unknown)..."));
          for (j = 0; j < 4 || j < dmpUpdate[2] + 3; j++, pos++) dmpUpdate[j] = pgm_read_byte(&dmpUpdates[pos]);
          writeMemoryBlock(dmpUpdate + 3, dmpUpdate[2], dmpUpdate[0], dmpUpdate[1]);

          //(F("Resetting FIFO..."));
          resetFIFO();
          //writeBit(devAddr, 0x6A, 2, true);
          //BUG: tipo de memoria
          uint16_t fifoCount = getFIFOCount();
          uint8_t fifoBuffer[128];
          getFIFOBytes(fifoBuffer, fifoCount);

          //setMotionDetectionThreshold(2);
          writeByte(MPU_endereco, 0x1F, 2);
          //setZeroMotionDetectionThreshold(156);
          writeByte(MPU_endereco, 0x21, 156);
          //setMotionDetectionDuration(80);
          writeByte(MPU_endereco, 0x20, 80);
          //setZeroMotionDetectionDuration(0);
          writeByte(MPU_endereco, 0x22, 0);

          resetFIFO();
          //writeBit(devAddr, 0x6A, 2, true);
          //setFIFOEnabled(true);
          writeBit(MPU_endereco, 0x6A, 6, true);
          setDMPEnabled(true);
          //resetDMP();
          writeBit(MPU_endereco, 0x6A, 3, true);

          //(F("Writing final memory update 3/7 (function unknown)..."));
          for (j = 0; j < 4 || j < dmpUpdate[2] + 3; j++, pos++) dmpUpdate[j] = pgm_read_byte(&dmpUpdates[pos]);
          writeMemoryBlock(dmpUpdate + 3, dmpUpdate[2], dmpUpdate[0], dmpUpdate[1]);

          //(F("Writing final memory update 4/7 (function unknown)..."));
          for (j = 0; j < 4 || j < dmpUpdate[2] + 3; j++, pos++) dmpUpdate[j] = pgm_read_byte(&dmpUpdates[pos]);
          writeMemoryBlock(dmpUpdate + 3, dmpUpdate[2], dmpUpdate[0], dmpUpdate[1]);

          //(F("Writing final memory update 5/7 (function unknown)..."));
          for (j = 0; j < 4 || j < dmpUpdate[2] + 3; j++, pos++) dmpUpdate[j] = pgm_read_byte(&dmpUpdates[pos]);
          writeMemoryBlock(dmpUpdate + 3, dmpUpdate[2], dmpUpdate[0], dmpUpdate[1]);

          //(F("Waiting for FIFO count > 2..."));
          while ((fifoCount = getFIFOCount()) < 3);

          getFIFOBytes(fifoBuffer, fifoCount);

          //(F("Reading final memory update 6/7 (function unknown)..."));
          for (j = 0; j < 4 || j < dmpUpdate[2] + 3; j++, pos++) dmpUpdate[j] = pgm_read_byte(&dmpUpdates[pos]);
          readMemoryBlock(dmpUpdate + 3, dmpUpdate[2], dmpUpdate[0], dmpUpdate[1]);

          //(F("Waiting for FIFO count > 2..."));
          while ((fifoCount = getFIFOCount()) < 3);

          getFIFOBytes(fifoBuffer, fifoCount);

          //(F("Writing final memory update 7/7 (function unknown)..."));
          for (j = 0; j < 4 || j < dmpUpdate[2] + 3; j++, pos++) dmpUpdate[j] = pgm_read_byte(&dmpUpdates[pos]);
          writeMemoryBlock(dmpUpdate + 3, dmpUpdate[2], dmpUpdate[0], dmpUpdate[1]);

          //(F("DMP is good to go! Finally."));

          //(F("Disabling DMP (you turn it on later)..."));
          setDMPEnabled(false);

          //(F("Setting up internal 42-byte (default) DMP packet buffer..."));
          dmpPacketSize = 42;
          //(F("Resetting FIFO and clearing INT status one last time..."));
          resetFIFO();
          getIntStatus();
      } else {
          //(F("ERROR! DMP configuration verification failed."));
          return 2; // configuration block loading failed
      }
  } else {
      //(F("ERROR! DMP code verification failed."));
      return 1; // main binary block loading failed
  }
  return 0; // success
}
//MOVE functions
/************/
void setMemoryBank(uint8_t bank, bool prefetchEnabled, bool userBank) {
    bank &= 0x1F;
    if (userBank) bank |= 0x20;
    if (prefetchEnabled) bank |= 0x40;
    writeByte(MPU_endereco, 0x6D, bank);
}

int8_t getXGyroOffsetTC(void) {
    readBits(MPU_endereco, 0x00, 6, 6, buffer);
    return buffer[0];
}
int8_t getYGyroOffsetTC(void) {
    readBits(MPU_endereco, 0x01, 6, 6, buffer);
    return buffer[0];
}
int8_t getZGyroOffsetTC(void) {
    readBits(MPU_endereco, 0x02, 6, 6, buffer);
    return buffer[0];
}

//BUG:
bool writeMemoryBlock(const uint8_t *data_ptr, uint16_t dataSize, uint8_t bank, uint8_t address, bool verify, bool useProgMem) {
    setMemoryBank(bank);
    //setMemoryStartAddress(address);
    writeByte(MPU_endereco, 0x6E, address);

    uint8_t chunkSize;
    uint8_t *verifyBuffer;
    uint8_t *progBuffer=0;
    uint16_t i;
    uint8_t j;

    //XXX: existe malloc no radio?
    if (verify) verifyBuffer = (uint8_t *)malloc(MPU6050_DMP_MEMORY_CHUNK_SIZE);
    if (useProgMem) progBuffer = (uint8_t *)malloc(MPU6050_DMP_MEMORY_CHUNK_SIZE);
    for (i = 0; i < dataSize;) {
        // determine correct chunk size according to bank position and data size
        chunkSize = MPU6050_DMP_MEMORY_CHUNK_SIZE;

        // make sure we don't go past the data size
        if (i + chunkSize > dataSize) chunkSize = dataSize - i;

        // make sure this chunk doesn't go past the bank boundary (256 bytes)
        if (chunkSize > 256 - address) chunkSize = 256 - address;

        if (useProgMem) {
            // write the chunk of data as specified
            for (j = 0; j < chunkSize; j++) progBuffer[j] = pgm_read_byte(data_ptr + i + j);
        } else {
            // write the chunk of data_ptr as specified
            progBuffer = (uint8_t *)data_ptr + i;
        }
        writeBytes(MPU_endereco, 0x6F, chunkSize, progBuffer);

        // verify data if needed
        if (verify && verifyBuffer) {
            setMemoryBank(bank);
            //setMemoryStartAddress(address);
            writeByte(MPU_endereco, 0x6E, address);
            i2c_mpu_readBytes(MPU_endereco, 0x6F, chunkSize, verifyBuffer);
            if (memcmp(progBuffer, verifyBuffer, chunkSize) != 0) {
                free(verifyBuffer);
                if (useProgMem) free(progBuffer);
                return false; // uh oh.
            }
        }
        // increase byte index by [chunkSize]
        i += chunkSize;
        // uint8_t automatically wraps to 0 at 256
        address += chunkSize;
        // if we aren't done, update bank (if necessary) and address
        if (i < dataSize) {
            if (address == 0) bank++;
            setMemoryBank(bank);
            //setMemoryStartAddress(address);
            writeByte(MPU_endereco, 0x6E, address);
        }
    }
    if (verify) free(verifyBuffer);
    if (useProgMem) free(progBuffer);
    return true;
}
bool writeProgMemoryBlock(const uint8_t *data_ptr, uint16_t dataSize, uint8_t bank, uint8_t address, bool verify) {
    return writeMemoryBlock(data_prt, dataSize, bank, address, verify, true);
}
bool writeDMPConfigurationSet(const uint8_t *data_ptr, uint16_t dataSize, bool useProgMem) {
    uint8_t *progBuffer = 0;
	uint8_t success, special;
    uint16_t i, j;
    if (useProgMem) {
        progBuffer = (uint8_t *)malloc(8); // assume 8-byte blocks, realloc later if necessary
    }

    // config set data is a long string of blocks with the following structure:
    // [bank] [offset] [length] [byte[0], byte[1], ..., byte[length]]
    uint8_t bank, offset, length;
    for (i = 0; i < dataSize;) {
        if (useProgMem) {
            bank = pgm_read_byte(data_ptr + i++);
            offset = pgm_read_byte(data_ptr + i++);
            length = pgm_read_byte(data_ptr + i++);
        } else {
            bank = data_ptr[i++];
            offset = data_ptr[i++];
            length = data_ptr[i++];
        }

        // write data or perform special action
        if (length > 0) {
            // regular block of data to write
            /*Serial.print("Writing config block to bank ");
            Serial.print(bank);
            Serial.print(", offset ");
            Serial.print(offset);
            Serial.print(", length=");
            Serial.println(length);*/
            if (useProgMem) {
                if (sizeof(progBuffer) < length) progBuffer = (uint8_t *)realloc(progBuffer, length);
                for (j = 0; j < length; j++) progBuffer[j] = pgm_read_byte(data_ptr + i + j);
            } else {
                progBuffer = (uint8_t *)data_ptr + i;
            }
            success = writeMemoryBlock(progBuffer, length, bank, offset, true);
            i += length;
        } else {
            // special instruction
            // NOTE: this kind of behavior (what and when to do certain things)
            // is totally undocumented. This code is in here based on observed
            // behavior only, and exactly why (or even whether) it has to be here
            // is anybody's guess for now.
            if (useProgMem) {
                special = pgm_read_byte(data_ptr + i++);
            } else {
                special = data_ptr[i++];
            }
            /*Serial.print("Special command code ");
            Serial.print(special, HEX);
            Serial.println(" found...");*/
            if (special == 0x01) {
                // enable DMP-related interrupts

                //setIntZeroMotionEnabled(true);
                //setIntFIFOBufferOverflowEnabled(true);
                //setIntDMPEnabled(true);
                writeByte(MPU_endereco, MPU6050_RA_INT_ENABLE, 0x32);  // single operation

                success = true;
            } else {
                // unknown special command
                success = false;
            }
        }

        if (!success) {
            if (useProgMem) free(progBuffer);
            return false; // uh oh
        }
    }
    if (useProgMem) free(progBuffer);
    return true;
}
bool writeProgDMPConfigurationSet(const uint8_t *data_ptr, uint16_t dataSize) {
    return writeDMPConfigurationSet(data_ptr, dataSize, true);
}


void setDMPEnabled(bool enabled){
    writeBit(MPU_endereco, 0x6A, 7, enabled);
}
uint8_t getIntStatus(void){
    i2c_mpu_readByte(MPU_endereco, 0x3A, buffer)
    return buffer[0];
}
uint16_t dmpGetFIFOPacketSize(void) {
    return dmpPacketSize;
}
uint16_t getFIFOCount(void) {
    i2c_mpu_readBytes(MPU_endereco, 0x72, 2, buffer);
    return (((uint16_t)buffer[0]) << 8) | buffer[1];
}

void resetFIFO(void) {
    writeBit(MPU_endereco, 0x6A, 2, true);
}

void getFIFOBytes(uint8_t *data_ptr, uint8_t length) {
    if(length > 0){
        i2c_mpu_readBytes(MPU_endereco, 0x74, length, data_ptr);
    } else {
    	*data_ptr = 0;
    }
}

uint8_t dmpGetQuaternion(int16_t *data_ptr, const uint8_t* packet) {
    if (packet == 0) packet = dmpPacketBuffer;
    data_ptr[0] = ((packet[0] << 8) | packet[1]);
    data_ptr[1] = ((packet[4] << 8) | packet[5]);
    data_ptr[2] = ((packet[8] << 8) | packet[9]);
    data_ptr[3] = ((packet[12] << 8) | packet[13]);
    return 0;
}

void getMotion6(int16_t* ax, int16_t* ay, int16_t* az, int16_t* gx, int16_t* gy, int16_t* gz) {
    i2c_mpu_readBytes(MPU_endereco, 0x3B, 14, buffer);
    *ax = (((int16_t)buffer[0]) << 8) | buffer[1];
    *ay = (((int16_t)buffer[2]) << 8) | buffer[3];
    *az = (((int16_t)buffer[4]) << 8) | buffer[5];
    *gx = (((int16_t)buffer[8]) << 8) | buffer[9];
    *gy = (((int16_t)buffer[10]) << 8) | buffer[11];
    *gz = (((int16_t)buffer[12]) << 8) | buffer[13];
}

void setXAccelOffset(int16_t offset) {
    writeWord(MPU_endereco,  0x06, offset);
}
void setYAccelOffset(int16_t offset) {
    writeWord(MPU_endereco, 0x08, offset);
}
void setZAccelOffset(int16_t offset) {
    writeWord(MPU_endereco, 0x0A, offset);
}
void setXGyroOffset(int16_t offset) {
    writeWord(MPU_endereco, 0x13, offset);
}
void setYGyroOffset(int16_t offset) {
    writeWord(MPU_endereco, 0x15, offset);
}
void setZGyroOffset(int16_t offset) {
    writeWord(MPU_endereco, 0x17, offset);
}
//Get
int16_t getXAccelOffset(void) {
    i2c_mpu_readBytes(MPU_endereco, 0x06, 2, buffer);
    return (((int16_t)buffer[0]) << 8) | buffer[1];
}
int16_t getYAccelOffset(void) {
    i2c_mpu_readBytes(MPU_endereco, 0x08, 2, buffer);
    return (((int16_t)buffer[0]) << 8) | buffer[1];
}
int16_t getZAccelOffset(void) {
    i2c_mpu_readBytes(MPU_endereco, 0x0A, 2, buffer);
    return (((int16_t)buffer[0]) << 8) | buffer[1];
}
int16_t getXGyroOffset(void) {
    i2c_mpu_readBytes(MPU_endereco, 0x13, 2, buffer);
    return (((int16_t)buffer[0]) << 8) | buffer[1];
}
int16_t getYGyroOffset(void) {
    i2c_mpu_readBytes(MPU_endereco, 0x15, 2, buffer);
    return (((int16_t)buffer[0]) << 8) | buffer[1];
}
int16_t getZGyroOffset(void) {
    i2c_mpu_readBytes(MPU_endereco, 0x17, 2, buffer);
    return (((int16_t)buffer[0]) << 8) | buffer[1];
}
