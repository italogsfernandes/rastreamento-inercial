
#include "reg24le1.h" 

#ifndef APP_H__
#define APP_H__

//sbit LED2=P0^6;
//sbit LED1=P0^3;                               

#define true 1
#define false 0

extern void uart_init(void);
extern void send(unsigned char tmp);
extern unsigned char getch(void);
extern void puts(unsigned char *s);
extern void io_config(void);
extern void delayx(int x);

#endif
