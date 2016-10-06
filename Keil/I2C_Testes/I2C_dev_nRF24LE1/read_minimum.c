#include "reg24le1.h"
#include "IIC_app.h"
#include "intrins.h"

//XXX: To implement
void setup_i2c_mpu(void);
void requisitarAccelMPU6050(void);
void enviar_pacote_inercial(void);

void main()
{
    //lógica:
    //iniciar i2c
    //configurar sensores
    //iniciar rf
    //Ao se precionar o botão
    //  realizar leitura
    //  despachar leitura
	IIC_init();
}
//XXX: impementar
/**************************************************/
/************MPU**********************************/
void setup_i2c_mpu(void){
    //iniciar i2c
    //Set the register Power Management to start
	writebyte(0x6B, 0x00);
}

void requisitarAccelMPU6050(void){
    //Ler 6 bytes a partir de 0x3B
    //Sendo esses:
    // [ACCEL_XOUT_H] [ACCEL_XOUT_L]
    // [ACCEL_YOUT_H] [ACCEL_YOUT_L]
    // [ACCEL_ZOUT_H] [ACCEL_ZOUT_L]
	multyread(readings,int len)
    i2c_mpu_readBytes(MPU_endereco,0x3B, 6,readings);
}

void enviar_pacote_inercial(void){
    LED2 = 1;
    tx_buf[0] = MY_SUB_ADDR;
    for(i=1;i<7;i++){
        tx_buf[i] = readings[i-1];
    }
    //enviando e retornando ao padrao:
    TX_Mode_NOACK(7);
    RX_Mode();
	LED2 = 0;
}
