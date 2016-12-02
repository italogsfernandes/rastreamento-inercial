#ifndef UART_BASICS_H
/* Se a biblioteca mpu.h não for definida, defina-a.
Verificar é preciso para que não haja varias chamadas da
mesma biblioteca. */
#define	UART_BASICS_H

/**************************************************/
void uart_init(void){
    ES0 = 0;                      // Disable UART0 interrupt while initializing(1:??????????? INE0^4)
    REN0 = 1;                     // Enable receiver(1:??????????? S0CON^4)
    SM0 = 0;                      // Mode 1..  ??8???g? SM0 SM1??01??
    SM1 = 1;                      // ..8 bit variable baud rate
    PCON |= 0x80;                 // SMOD = 1(????0?????????)
    ADCON |= 0x80;                // Select internal baud rate generator
                                  // (ADCON??????0??????????J?????'???????????????????? )
    S0RELL = 0xf3;                // baudrate 38400
    S0RELH = 0x03;
    TI0 = 0;                      // S0CON^1:?????????????????????????
    S0BUF=0x00;                   //????0????????J???
}
/**************************************************/
void uart_putchar(uint8_t x){
    while (!TI0);
    TI0=0;
    S0BUF=x;
}
/*****************************/
// Repeated putchar to print a string
void putstring(char *s){
    while(*s != 0)
        uart_putchar(*s++);
}


#endif
