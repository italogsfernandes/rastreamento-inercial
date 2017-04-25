#include "reg24le1.h"
#include "IIC_app.h"
#include "intrins.h"

unsigned char rbuffer[0x10]={ '0','0','0','0',
'0','0','0','0',
'0','0','0','0',
'0','0','0','0',
};
extern char wbuffer[0x10];
extern char init_time;

void main()
{
    unsigned int i;
    unsigned char flag=0;
    Io_config();
    uart_init();
    ex_int();
    puts("--just a iic test by syman--2010,9,10\n");
    while(1)
    {
        if(LED)
        {
            WDCON &= 0x7f; //�رմ���
            if(init_time)
            {IIC_init();//initial iic
                init_time=0;}
                for(i=startaddr;i<endaddr;i++)
                {rbuffer[i]=readbyte(i);
                }
                _nop_();_nop_();_nop_();_nop_();
                _nop_();_nop_();_nop_();_nop_();
                for(i=startaddr;i<endaddr;i++)
                {writebyte(i,wbuffer[i]);
                }
            }else {
                //led Ϩ����ʱ��
                //IIC disable
                DISABLE2WIRE();
                if(init_time)
                { uart_init();
                    init_time=0;
                }
                for(i=0;i<0x10;i++)
                {
                    if((i%8==0))
                    send('\n');
                    send(rbuffer[i]);
                }
                send('\n');
                send('\n');
            }
        }
        delay(200);

    }
