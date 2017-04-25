C51 COMPILER V9.00   IIC_APP                                                               11/13/2010 14:52:47 PAGE 1   


C51 COMPILER V9.00, COMPILATION OF MODULE IIC_APP
OBJECT MODULE PLACED IN IIC_app.OBJ
COMPILER INVOKED BY: D:\Program Files\KEIL C  V4\C51\BIN\C51.EXE IIC_app.c LARGE BROWSE DEBUG OBJECTEXTEND

line level    source

*** WARNING C500 IN LINE 1 OF IIC_APP.C: LICENSE ERROR (R208: RENEW LICENSE ID CODE (LIC))

   1          #include "reg24le1.h"
   2          #include "IIC_app.h"
   3          #include "intrins.h"
   4          
   5          void delay(unsigned int dx)
   6          {
   7   1      unsigned int di;
   8   1        for(;dx>0;dx--)
   9   1          for(di=120;di>0;di--)
  10   1                  {
  11   2                      ;
  12   2                      }
  13   1       }
  14                                          
  15          void IIC_init()
  16          {
  17   1      FREQSEL(2);
  18   1      MODE(MASTER);
  19   1      W2CON1|=0x20;     //屏蔽所有的中断
  20   1      W2SADR=0x00;
  21   1      EN2WIRE();        //使能2-wire
  22   1      }
  23          
  24          void Io_config()
  25          {
  26   1      //LED p00
  27   1      P0DIR&=0XFE;      //LED 输出
  28   1      P00=0;
  29   1      P1DIR|=0X01;
  30   1      P10=0X01;
  31   1      }
  32          
  33          void uart_init()
  34          {
  35   1          CLKCTRL = 0x28;                         // 设置时钟源为16M  
  36   1              CLKLFCTRL = 0x01; 
  37   1      
  38   1              P0DIR &= 0xF7;                          // P03 (TxD) 
  39   1              P0DIR |= 0x10;                          // P04 (RxD)  
  40   1              P0|=0x18;        
  41   1                      
  42   1              S0CON = 0x50;  
  43   1              PCON |= 0x80;                           // 波特率倍增
  44   1              WDCON |= 0x80;                          // 选择内部波特率发生器
  45   1              
  4