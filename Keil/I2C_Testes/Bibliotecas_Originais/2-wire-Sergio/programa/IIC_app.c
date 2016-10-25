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
    P00=0;
    P1DIR|=0X01;
    P10=0X01;
}

void uart_init()
{
    CLKCTRL = 0x28;			    // ����ʱ��ԴΪ16M
    CLKLFCTRL = 0x01;

    P0DIR &= 0xF7;				// P03 (TxD)
    P0DIR |= 0x10;     			// P04 (RxD)
    P0|=0x18;

    S0CON = 0x50;
    PCON |= 0x80; 				// �����ʱ���
    WDCON |= 0x80;   			// ѡ���ڲ������ʷ�����

    S0RELL = 0xFB;
    S0RELL = 0xF3;              // ������38400

    //	ES0=1;
}

void ex_int(void)
{
    IEN0|=0X80;
    IEN0|=0X01;
    TCON|=0X01;       //�½��ش���
    INTEXP|=0x08; 	  //��P05�����ж�
    P0DIR|=0X20;	  //P05����
    P0DIR|=0x40;	  //P06����
    P05=1;
    P06=1;
}

void send(unsigned char ch)
{

    S0BUF=ch;
    while(!TI0);
    TI0=0;
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

//extern char wbuffer[];
unsigned char wbuffer[0x10]={'a','b','c','d',
'e','f','g','h',
'0','1','2','3',
'4','5','6','7',
};
void uart_service() interrupt  INTERRUPT_UART
{
    static int i=0;
    char buf;
    if(TI0)
    {TI0=0;	}
    if(RI0)
    {
        RI0=0;
        buf=S0BUF;
        wbuffer[i++]=buf;
        if(i==0x10)
        i=0;
    }

}

unsigned char keycheck(void)
{
    P1CON=0XD0;//P10
    if(!P10)
    {
        delay(10);
        if(!P10)
        { while(!P10);
            return true;  //�������¾ͷ�����
        }
    }
    return false; // ���ؼ�

}

void puts(char *str)
{
    while(*str!='\0')
    {send(*str++);
    }
}

char init_time=0;

void ex_int_service() interrupt  0
{

    static char flag=0;
    if(flag)
    flag=0;
    else
    flag=1;
    puts("...IIC test program!by syman...\n");
    LED=flag;
    init_time=1;
}
