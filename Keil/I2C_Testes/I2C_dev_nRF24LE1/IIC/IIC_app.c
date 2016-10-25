#include "reg24le1.h"
#include "IIC_app.h"
#include "stdbool.h"
#include "intrins.h"


void IIC_init(void)
{
    FREQSEL(2);
    MODE(MASTER);
    W2CON1|=0x20;     //�������е��ж�
    W2SADR=0x00;
    EN2WIRE();        //ʹ��2-wire
}
//TODO: Testar sem, acho que eh desnecessaria
void ex_int(void)
{
    IEN0|=0X80;
    IEN0|=0X01;
    TCON|=0X01;       //�½��ش���
    INTEXP|=0x08; 	  //��P05�����ж�
    P0DIR|=0X20;//0010 0000	  //P05����
    P0DIR|=0x40;//0100 0000	  //P06����
    P05=1;
    P06=1;
}
//TODO: testar sem acho q é desnecessaria
void Io_config(void)
{
    //LED p00
    P0DIR&=0XFE;      //LED ����
    P00=0;//XXX: why?
    P1DIR|=0X01;
    P10=0X01;
}

/*********************MINHAS MODIFICAÇOES***********************/

bool i2c_mpu_writeByte(uint8_t devAddr, uint8_t regAddr, uint8_t data){
    return i2c_mpu_writeBytes(devAddr, regAddr, 1, &data);
}

bool i2c_mpu_writeBytes(uint8_t devAddr, uint8_t regAddr, uint8_t data_len, uint8_t *data_ptr) {
    bool ack_received;
    START();
    W2DAT=((devAddr+0xa0)<<1)+0;//write
    if(!ACK){ //IF ACK
        W2DAT=regAddr;
    }
    //BUG: antes isso era um if? ue?
    while(!ACK && data_len-- > 0) {
        W2DAT=*data_ptr++ ;
        numlimit++;
        if(numlimit==16){
            return false;
        }
    }
    ack_received = !ACK;
    STOP();
    return ack_received;
}

bool i2c_mpu_readByte(uint8_t devAddr, uint8_t regAddr, uint8_t *data_ptr) {
    return i2c_mpu_readBytes(devAddr, regAddr, 1, data_ptr);
}


bool i2c_mpu_readBytes(uint8_t devAddr, uint8_t regAddr, uint8_t data_len, uint8_t *data_ptr) {
    bool ack_received;
    START();
    W2DAT=((devAddr+0xa0)<<1)+0;//write from slave
    while(ACK);
    W2DAT=regAddr;
    while(ACK);

    START();
    W2DAT=((devAddr+0xa0)<<1)+1;//read from slave
    while(ACK);
    while(data_len-- && !ACK)
    {
        while(!READY);
        *data_ptr++=W2DAT;
    }
    ack_received = !ACK;
    STOP();
    return ack_received;
}
