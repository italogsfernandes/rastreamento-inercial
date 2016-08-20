

//Biblioteca para I/O no nRF24Le1
#include "nrf24le1.h"

//Bibliotecas do exemplo de I2C
#include <hal_w2_isr.h> //Biblioteca para o I2C. TODO: Selecionar entre essa e a outra disponivel (hal_w2)
#include "hal_delay.h"

#define MPU_endereco 0x68
//************************TODO****************//
// -------------------Algo semelhante ao writeBit e writeBits
//Feito isso o negocio fica quase 100% pronto readBits(MPU_endereco, 0x75, 6, 6, buffer);
//Ver na biblioteca original para entender como funcionam essas bibliotecas
//-------------------readBits
//-------------------writeBit
//-------------------writeBits
//-------------------delay
//********************************************///
//************FUNÇÕES Utilizadas no arduino *************//
//  - mpu.initialize(); -Feito(Verificar)
//  - mpu.testConnection(); -Feito(Verificar)
//  - mpu.dmpInitialize(); -Pedir ajudar
//  - mpu.set*Offset(int); -TODO
//  - mpu.setDMPEnabled(bool); - Fazendo
//  - mpu.getIntStatus(); -TODO
//  - mpu.dmpGetFIFOPacketSize(); -TODO
//  - mpu.getIntStatus() -TODO
//  - mpu.getFIFOCount(); -TODO
//  - mpu.resetFIFO(); -TODO
//  - mpu.getFIFOCount(); -TODO
//  - mpu.getFIFOBytes(fifoBuffer, packetSize); -TODO
//  - mpu.dmpGetQuaternion(&q, fifoBuffer); -TODO
//  - mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz); -TODO
//**********************************************************//


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
    uint8_t buffer[14];
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

void setDMPEnabled(bool enable){
    //I2Cdev::writeBit(devAddr, MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_DMP_EN_BIT, enabled);
    //I2Cdev::writeBit(devAddr, 0x6A, 7, true);
    //                 devAddr, regAddr,Bit, value
    writeBit(devAddr, 0x6A, 7, true);
}

//getIntStatus()

//dmpGetFIFOPacketSize()
