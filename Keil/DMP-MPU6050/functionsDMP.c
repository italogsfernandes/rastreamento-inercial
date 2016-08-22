

//Biblioteca para I/O no nRF24Le1
#include "nrf24le1.h"

//Bibliotecas do exemplo de I2C
#include <hal_w2_isr.h> //Biblioteca para o I2C. TODO: Selecionar entre essa e a outra disponivel (hal_w2)
#include "hal_delay.h"

#define MPU_endereco 0x68
//************************TODO****************//
// -------------------Algo semelhante ao writeBit e writeBits
//Feito isso o negocio fica quase 100% pronto
//Ver na biblioteca original para entender como funcionam essas bibliotecas
//-------------------readBits
//-------------------writeBit
//-------------------writeBits
//-------------------delay
//-------------------readByte
//-------------------readBytes
//********************************************///
/*
    NOTE: Description of how the code works (when it isn't self evident).
    XXX: Warning about possible pitfalls, can be used as NOTE:XXX:.
    HACK: Not very well written or malformed code to circumvent a problem/bug. Should be used as HACK:FIXME:.
    FIXME: This works, sort of, but it could be done better. (usually code written in a hurry that needs rewriting).
    BUG: There is a problem here.
    TODO: No problem, but addtional code needs to be written, usually when you are skipping something.
*/
//************FUNÇÕES Utilizadas no arduino *************//
//  - mpu.initialize(); -NOTE: Feito(Verificar)
//  - mpu.testConnection(); -NOTE: Feito(Verificar)
//  - mpu.dmpInitialize(); -FIXME: Pedir ajudar
//  - mpu.set*Offset(int); - A fazer
//  - mpu.setDMPEnabled(bool); -NOTE: Feito(Verificar)
//  - mpu.getIntStatus(); -NOTE: Feito(Verificar)
//  - mpu.dmpGetFIFOPacketSize(); - FIXME: Ué?
//  - mpu.getFIFOCount(); - NOTE: Feito(Verificar)
//  - mpu.resetFIFO(); - NOTE: Feito(Verificar)
//  - mpu.getFIFOBytes(fifoBuffer, packetSize); - NOTE: Feito(Verificar), FIXME: Implementar variaveis globais
//  - mpu.dmpGetQuaternion(&q, fifoBuffer); - NOTE: Feito(Verificar)
//  - mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz); - NOTE: Doing
//**********************************************************//
/*
TODO: Informal tasks/features that are pending completion.
FIXME (XXX) Areas of problematic or ugly code needing refactoring or cleanup.
BUG: Reported defects tracked in bug database.
IDEA: Possible RFE(Requests For Enhancement: Roadmap items not yet implemented.) candidates, but less formal than RFE.
HACK: Temporary code to force inflexible functionality, or simply a test change, or workaround a known problem.
NOTE: Sections where a code reviewer found something that needs discussion or further investigation.
REVIEW: File-level indicator that review was conducted.
*/

//TODO: Melhorar implementação e uso de variaveis globais
uint8_t buffer[14]; //usado em testConnection e getIntStatus
uint16_t dmpPacketSize; //usado em dmpGetPacketSize e dmpInitialize

void initialize(){
  //setClockSource(MPU6050_CLOCK_PLL_XGYRO);
  /*setClockSource(0x01);
   *                   devAddr, regAddr, bitStart, length, data);
   * I2Cdev::writeBits(devAddr,   0x6B,         2,      3, 0x01);
   * I2Cdev::writeBits(devAddr, MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_CLKSEL_BIT, MPU6050_PWR1_CLKSEL_LENGTH, source);
   */

  // Resumindo: Implementar writeBits abaixo:
  writeBits(MPU_endereco, 0x6B, 2, 3, 0x01); //setClockSource


  /************************************/
  /* setFullScaleGyroRange(MPU6050_GYRO_FS_250);
   * setFullScaleAccelRange(MPU6050_ACCEL_FS_2);
   * Vou levar em conta que por padrão
   * as escalas estão definidas em +-250graus/s e +-2g
   * Se precisar dps me preocupo com isso?
  */
  /************************************/
  /* setSleepEnabled(false)
   * I2Cdev::writeBit(devAddr, 0x6B, 6, false);
   * I2Cdev::writeBit(devAddr, MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_SLEEP_BIT, enabled);
   */

  // Resumindo: Implementar writeBit abaixo:
  writeBit(MPU_endereco, 0x6B, 6, false); //setSleepEnabled(false);
}

//TODO: Verificar se vou precisar de biblioteca pra este tipo?
bool testConnection(){
    //return getDeviceID() == 0x34;
    // getDeviceID:
    /*
        I2Cdev::readBits(devAddr, MPU6050_RA_WHO_AM_I, MPU6050_WHO_AM_I_BIT, MPU6050_WHO_AM_I_LENGTH, buffer);
        I2Cdev::readBits(devAddr,  0x75, 6, 6, buffer);
                         devAddr, regAddr, bit start,length,  *data, timeout
        return buffer[0];
    */
    //TODO: otimizar para nao ser necessario o uso do buffer
    readBits(MPU_endereco, 0x75, 6, 6, buffer);
    return buffer[0] == 0x34;
}

void dmpInitialize(){
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
    delay(30);
  */
  //TODO: Parei por aqui ontem
  writeBit(MPU_endereco, 0x6B, 7, true);
  delay(30); //TODO: implementar função de delay

  // disable sleep mode
  writeBit(MPU_endereco, 0x6B, 6, false); //setSleepEnabled(false);

  // get MPU hardware revision
}

//setOffsets

void setDMPEnabled(bool enabled){
    //I2Cdev::writeBit(devAddr, MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_DMP_EN_BIT, enabled);
    //I2Cdev::writeBit(devAddr, 0x6A, 7, true);
    //                 devAddr, regAddr,Bit, value
    writeBit(devAddr, 0x6A, 7, enabled);
}

uint8_t getIntStatus(){
    //I2Cdev::readByte(devAddr, MPU6050_RA_INT_STATUS, buffer);
    //I2Cdev::readByte(devAddr, 0x3A, buffer);
    //                 devAddr, regAddr, data timeout
    readByte(MPU_endereco, 0x3A, buffer)
    return buffer[0];
}

uint16_t dmpGetFIFOPacketSize() {
    return dmpPacketSize;
}

uint16_t getFIFOCount() {
    // I2Cdev::readBytes(devAddr, MPU6050_RA_FIFO_COUNTH, 2, buffer);
    // I2Cdev::readBytes(devAddr, 0x72, 2, buffer);
    //                   devAddr, regAddr, length, *data timeout
    // return (((uint16_t)buffer[0]) << 8) | buffer[1];
    readBytes(devAddr, 0x72, 2, buffer);
    return (((uint16_t)buffer[0]) << 8) | buffer[1];
}

void resetFIFO() {
    //I2Cdev::writeBit(devAddr, MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_FIFO_RESET_BIT, true);
    //I2Cdev::writeBit(devAddr, 0x6A, 2, true);
    //                  devAddr, regAddr, bit, value
    writeBit(devAddr, 0x6A, 2, true);
}

void getFIFOBytes(uint8_t *data, uint8_t length) {
    if(length > 0){
        readBytes(devAddr, 0x74, length, data);
    } else {
    	*data = 0;
    }
}

uint8_t dmpGetQuaternion(int16_t *data, const uint8_t* packet) {
    if (packet == 0) packet = dmpPacketBuffer;
    data[0] = ((packet[0] << 8) | packet[1]);
    data[1] = ((packet[4] << 8) | packet[5]);
    data[2] = ((packet[8] << 8) | packet[9]);
    data[3] = ((packet[12] << 8) | packet[13]);
    return 0;
}

uint8_t dmpGetQuaternion(Quaternion *q, const uint8_t* packet) {
    int16_t qI[4];
    uint8_t status = dmpGetQuaternion(qI, packet);
    if (status == 0) {
        q -> w = (float)qI[0] / 16384.0f;
        q -> x = (float)qI[1] / 16384.0f;
        q -> y = (float)qI[2] / 16384.0f;
        q -> z = (float)qI[3] / 16384.0f;
        return 0;
    }
    return status; // int16 return value, indicates error if this line is reached
}
