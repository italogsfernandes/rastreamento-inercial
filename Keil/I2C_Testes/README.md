# Como configurar e usar o I2C
...Segundo nrf24LE1 Product Specification.

## General

* Set '1' to wire2Enable, before doing anything.
* Sempre ler W2DAT.
* Updates maskIrq before anythig.
* Start and Stop conditions can be set immediately after the last TX data write.
## Configurar



## Enviar dados

* **Start Condition**: Determined by W2CON0[4].
* **Slave Address**: Write the slave address in W2DAT and a '0' at the end to indicates the direction.
W2DAT = `[address][0]`. In code: `W2DAT = (slaveaddress<<1)+0x00`
* **ACK**: The ACK is stored in W2CON1[1], it means that the Slave is prepared to receive data.
* **DATA Transfer**: The data should be tranfered to W2DAT. After every transmission the ACK should be verified.
* **STOP Condition**: Write an '1' to W2CON0[5];


## Receber dados

* **Start Condition**: Determined by W2CON0[4].
* **Slave Address**: Write the slave address in W2DAT and an '1' at the end to indicates the direction.
W2DAT = `[address][0]`. In code: `W2DAT = (slaveaddress<<1)+0x01`
* **ACK**: The ACK is stored in W2CON1[1], it means that the Slave is prepared to receive data.
* **Receiving a Byte**: The data will be received in W2DAT.And run a interruption.
* **Data Ready**: Flag after receiving a byte
* **STOP Condition**: Write a '1' to W2CON0[5];
