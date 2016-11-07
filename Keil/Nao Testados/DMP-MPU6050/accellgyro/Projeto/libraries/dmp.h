#ifndef DMP_H
/* Se a biblioteca mpu.h não for definida, defina-a.
Verificar é preciso para que não haja varias chamadas da
mesma biblioteca. */
#define DMP_H

#ifndef MPU_endereco
#define MPU_endereco 0x68
#endif

uint8_t buffer[14]; //usado em testConnection e getIntStatus

void mpu_initialize(void);
bool mpu_testConnection(void);
void getMotion6_packet(uint8_t *packet6);

//Set offsets
void setXAccelOffset(int16_t offset);
void setYAccelOffset(int16_t offset);
void setZAccelOffset(int16_t offset);
void setXGyroOffset(int16_t offset);
void setYGyroOffset(int16_t offset);
void setZGyroOffset(int16_t offset);
//Get offsets
int16_t getXAccelOffset(void);
int16_t getYAccelOffset(void);
int16_t getZAccelOffset(void);
int16_t getXGyroOffset(void);
int16_t getYGyroOffset(void);
int16_t getZGyroOffset(void);

#endif
