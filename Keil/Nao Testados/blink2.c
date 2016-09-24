#include "reg24le1.h" // I/O header file for NRF24LE1
#include "hal_delay.h" // header file containing delay functions

#define	PIN32
#ifdef 	PIN32
sbit LED1 = P0^3; // 1/0=light/dark
sbit LED2 = P0^6; // 1/0=light/dark
#endif

void main() {
    //Registar name P0DIR - ADDR = 0x93
    /*The PxDIR registers determine the direction
    * of the pins and the PxCON registers contain
    * the functional options for input and output
    * pin operation
    */
    //Output: dir = 0, Input: dir = 1.
  P0DIR = 0xB7; // Set P03 e P06 of PORT0 as output

  while(1) // infinite loop
  {
      //Pin0 de Port0 value
      //Endereço: 0x80
    LED1 = 1; // make Pin0 of Port0 low
    LED2 = 0
    delay_ms(1000); // delay of 1 second
    LED1 = 0; // make Pin0 of Port0 High
    LED2 = 1;
    delay_ms(1000); // delay of 1 second
  }
}
