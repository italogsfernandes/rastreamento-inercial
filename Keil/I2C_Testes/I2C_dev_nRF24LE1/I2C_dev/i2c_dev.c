#include "reg24le1.h"
#include "IIC_app.h"
#include "stdbool.h"
#include "intrins.h"

#define xStop W2CON0|=0x20
#define xStart  W2CON0|= 0x10
#define FREQ_STANDART_MODE W2CON0&=0xF3; W2CON0|=0x04
#define FREQ_FAST_MODE  W2CON0&=0xF3; W2CON0|=0x08
#define masterSelect W2CON0|=0x02
#define slaveSelect W2CON0 &=0x02
#define wire2Enable W2CON0|=0x01
#define wire2Disable W2CON0&=0xFE
#define maskIrq W2CON1|= 0x20
#define ACK (W2CON1&0X02)
#define dataReady (W2CON1&0X01)

void i2c_setup(void)
{
    /*
    W2CON0 |= 0x01; // Set "wire2Enable" bit
    W2CON1 = ~(BIT_5);    // Clear "maskIrq" bit




    */

    IEN0|=0X80; //Enable interrupts //1000 0000
    IEN0|=0X01; //XXX: 1: Enable Interrupt From Pin (IFP) interrupt.
    TCON|=0X01; //XXX: fazendo algo com o timer
    INTEXP|=0x08; //XXX: 1: Enable GP INT0 (from pin) 0 to IFP
    //XXX: PQ não:
    //INTEXP |= 0x04;  // Enable 2 wire interrupts
    //IEN1 |= 0x04; // Enable 2-wire complete interrupt
    //W2CON1 = 0x00;

    //BUG:NOTE:OQ é SPIF?
    //SPIF = 0;

    P0DIR|=0X20; //P05 as output
    P0DIR|=0x40; //P06 as output
    P05=1; //W2SDA
    // P04 = W2SCL
    P06=1; //XXX: WHY P06?????????
}
void i2c_begin(void)
{
    FREQSEL(I2C_FASTMODE);
    MODE(MASTER);
    //XXX:BUG: PQ TA DESATIVANDO AS INTERRUPCOES?
    W2CON1|=0x20;   //Disable all interrupts, why?
    W2SADR=0x00;
    EN2WIRE();  //ativa o i2c
}

void i2c_write(uint8_t devAddr, uint8_t data_to_write){

}

void i2c_read(uint8_t devAddr, uint8_t *data_ptr){


}
