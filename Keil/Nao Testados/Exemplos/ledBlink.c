#include "reg24le1.h" // I/O header file for NRF24LE1
#include "hal_delay.h" // header file containing delay functions

// main code
void main() {
  P0DIR = 0x00; // set PORT0 as output
  while(1) // infinite loop
  {
    P00 = 1; // make Pin0 of Port0 low
    delay_ms(1000); // delay of 1 second
    P00 = 0; // make Pin0 of Port0 High
    delay_ms(1000); // delay of 1 second
  }
}
