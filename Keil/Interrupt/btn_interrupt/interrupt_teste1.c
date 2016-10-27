/*
INT0 is P0.5 and for INT1 is P0.6.
First write 1 in bit7 of IEN0 to enable global interrupts and 1 in bit0.
Write 1 in bit3 of INTEXP to select INT0.
We have to write ISR for INT0.
*/



#include"reg24le1.h"         // I/O header file for NRF24LE1
#include &lt;stdint.h&gt;     // header file containing standard I/O functions
#include"hal_delay.h"      // header file containing delay functions
#include"isrdef24le1.h"    //header file containing Interrupt Service Routine definition for NRF24LE1
void main() // main code
{
    P0DIR = 0xf0;           // make upper 4 bits of Port0 as input
    P1DIR = 0;                  // set Port1 as output
    P1 = 0x00;                 // make all pins of Port1 low
    IEN0 = 0x81;              // enable interrupt from pin
    INTEXP = 0x08;       // enable INT0
    while(1);                     // infinite loop, wait for interrupt

}
EXT_INT0_ISR()  // Interrupt Service Routine
{
    P1 = 0xff;                                  // make all pins of Port1 high
    delay_ms(1000);                   // delay of 1 second
    P1 = 0x00;                            // make all pins of Port1 Low
    delay_ms(1000);                 // delay of 1 second
}
