#include "reg24le1.h" // I/O header file for NRF24LE1
#include "hal_delay.h" // header file containing delay functions


void main() {
    //Registar name P0DIR - ADDR = 0x93
    /*The PxDIR registers determine the direction
    * of the pins and the PxCON registers contain
    * the functional options for input and output
    * pin operation
    */
    //Output: dir = 0, Input: dir = 1.
  P0DIR = 0x00; // Set all PORT0 as output

  while(1) // infinite loop
  {
      //Pin0 de Port0 value
      //Endereço: 0x80
    P00 = 1; // make Pin0 of Port0 low
    delay_ms(1000); // delay of 1 second
    P00 = 0; // make Pin0 of Port0 High
    delay_ms(1000); // delay of 1 second
  }
}
