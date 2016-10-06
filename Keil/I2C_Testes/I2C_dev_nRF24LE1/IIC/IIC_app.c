#include "reg24le1.h"
#include "IIC_app.h"
#include "intrins.h"

void delay(unsigned int dx)
{
    unsigned int di;
    for(;dx>0;dx--)
    for(di=120;di>0;di--)
    {
        ;
    }
}

void IIC_init()
{
    FREQSEL(2);
    MODE(MASTER);
    W2CON1|=0x20;     //�������е��ж�
    W2SADR=0x00;
    EN2WIRE();        //ʹ��2-wire
}

void Io_config()
{
    //LED p00
    P0DIR&=0XFE;      //LED ����
    P00=0;//XXX: why?
    P1DIR|=0X01;
    P10=0X01;
}

unsigned char readbyte(unsigned int addr)
{
    unsigned char byte;
    START();
    W2DAT=((slaveaddr+0xa0)<<1)+0;//write from slave
    while(ACK);
    W2DAT=addr;
    while(ACK);
    START();
    W2DAT=((slaveaddr+0xa0)<<1)+1;//read from slave
    while(ACK);
    while(!READY);
    byte=W2DAT;
    STOP();
    return byte;

}
//ѡ��д
void writebyte(unsigned int addr,unsigned char dat)
{
    unsigned char byte=dat;
    START();
    W2DAT=((slaveaddr+0xa0)<<1)+0;//write
    if(!ACK) //IF ACK
    W2DAT=addr;
    if(!ACK)
    W2DAT=byte;
    STOP();
}

//������
//XXX: cade o addr do register?
void multyread(char *buffer,int len)
{
    char *cbuffer=buffer;
    W2DAT=((slaveaddr+0xa0)<<1)+1;//read from slave
    if(!ACK) //IF ACK
    {
        while(len--)
        {
            while(!READY);
            *cbuffer++=W2DAT;
        }

        STOP();
    }

}

//ҳд
void multwrite(char *buffer,int addr)
{
    char * cbuffer=buffer;
    char numlimit=0;
    START();
    W2DAT=((slaveaddr+0xa0)<<1)+0;//write from slave
    if(!ACK) //IF ACK
    W2DAT=addr;
    if(!ACK)
    {
        W2DAT=*cbuffer++ ;
        numlimit++;
        if(numlimit==16)
        return;
    }

}
/*********************MINHAS MODIFICAÇOES***********************/

void i2c_mpu_writeByte(uint8_t devAddr, uint8_t regAddr, uint8_t data){
    return i2c_mpu_writeBytes(devAddr, regAddr, 1, &data);
}

void i2c_mpu_writeBytes(uint8_t devAddr, uint8_t regAddr, uint8_t data_len, uint8_t *data_ptr) {
    START();
    W2DAT=((devAddr+0xa0)<<1)+0;//write
    if(!ACK){ //IF ACK
        W2DAT=regAddr;
    }
    //antes isso era um if? ue?
    //XXX: trocar por while
    if(!ACK) {
        W2DAT=*data_ptr++ ;
        numlimit++;
        if(numlimit==16)
        return false;
    }

    if(!ACK)
    W2DAT=byte;
    STOP();
}

void i2c_mpu_readByte(uint8_t devAddr, uint8_t regAddr, uint8_t *data_ptr) {
    i2c_mpu_readBytes(devAddr, regAddr, 1, data_ptr);
}


void i2c_mpu_readBytes(uint8_t devAddr, uint8_t regAddr, uint8_t data_len, uint8_t *data_ptr) {
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
    STOP();
}
