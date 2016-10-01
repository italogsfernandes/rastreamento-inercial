
#define MPU_endereco 0x69

int16_t Xac, Yac, Zac; //Acererometro
uint8_t readings[6];
//escrever 0x00  no register 0x6B
//Ler:


void iniciar_i2c(void)
{
    hal_w2_configure_master(HAL_W2_400KHZ);
  	EA= 1;
}

void setup_i2c_mpu(void)
{
    //iniciar i2c
    i2c_write_byte(MPU_endereco, 0x6B, 0x00)
}

void requisitarDadosMPU6050() {
    //Ler 6 bytes a partir de 0x3B
    i2c_readbytes(MPU_endereco,0x3B, 6,readings);
    //salvar em cada variavel
    Xac = readings[0] << 8 | readings[1];   //0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
    Yac = readings[2] << 8 | readings[3];   //0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
    Zac = readings[4] << 8 | readings[5];   //0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
}
