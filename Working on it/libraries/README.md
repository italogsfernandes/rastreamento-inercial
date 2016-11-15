# Bibliotecas

## hal_w2_isr.h
* Descrição: Realiza comunicação I2C com os sensores:
* Funções:
- [ ] com dependencia nao testada -  dependencia nao testada
- [ ] com dependencia ja testada -  **dependencia testada**
- [x] implementada
- [x] <del>testada</del>
	* Bytes
	- [x] <del>i2c_mpu_writeByte</del>
	- [x] <del>i2c_mpu_writeBytes</del>
	- [x] <del>i2c_mpu_readByte</del>
	- [x] <del>i2c_mpu_readBytes</del>
	* Bits
	- [x] <del>i2c_mpu_writeBit</del>
	- [x] <del>i2c_mpu_writeBits</del>
	- [x] <del>i2c_mpu_readBit</del>
	- [x] <del>i2c_mpu_readBits</del>
	* Words (16 bit variable)
	- [x] <del>i2c_mpu_writeWord</del>
	- [x] <del>i2c_mpu_writeWords</del>
	- [ ] i2c_mpu_readWord - Depende de readWords
	- [ ] i2c_mpu_readWords

## dmp.h
* Descrição: Realiza comunicação I2C com os sensores:
	* Inicialização:
	- [x] <del>mpu_initialize</del>
	- [x] <del>mpu_testConnection</del>
	* Getters:
	- [x] <del>getMotion6_packet</del>
	* Offsets:
	- [x] <del>set\*AccelOffset</del>
	- [x] <del>set\*GyroOffset</del>
	- [ ] get\*AccelOffset - Depende de readWord
	- [ ] get\*GyroOffset - Depende de readWord
	* DMP
	- [ ] dmpInitialize - Depende de muita coisa (olhar abaixo)
	- [x] setDMPEnabled
	- [x] dmpGetFIFOPacketSize
	- [ ] dmpGetQuaternion - Depende de getFIFO
		* dmpInitialize depende de:
		- [x] setMemoryBank
		- [x] get\*GyroOffsetTC - Depende de **readBits**
		- [x] set\*GyroOffsetTC - Depende de **writeBits**
		- [ ] pgm_read_byte - XXX: ?
		- [x] setDMPEnabled
		- [ ] writeProgMemoryBlock - Depende de writeMemoryBlock
		- [ ] writeProgDMPConfigurationSet - Depende de writeDMPConfigurationSet
		- [x] getFIFOCount
		- [x] resetFIFO
		- [x] getFIFOBytes
		- [ ] readMemoryBlock - Depende das mesmas coisas de writeMemoryBlock
		- [x] getIntStatus
		- [ ] writeDMPConfigurationSet
			* Depende de:
			- [ ] writeMemoryBlock
			- [x] setMemoryBank
			- [ ] pgm_read_byte - XXX: ?
			- [ ] memcmp - XXX: ?
			- [ ] [malloc e free](http://www.keil.com/support/man/docs/c51/c51_malloc.htm)
		- [ ] writeMemoryBlock
			* Depende de:
			- [x] setMemoryBank
			- [ ] pgm_read_byte - XXX: ?
			- [ ] memcmp - XXX: ?
			- [ ] [malloc e free](http://www.keil.com/support/man/docs/c51/c51_malloc.htm)

## Referências:
* [I2CDEV - HOME](http://www.i2cdevlib.com/)
* [I2CDEV - GITHUB - ARDUINO - MPU6050](https://github.com/jrowberg/i2cdevlib/tree/master/Arduino/MPU6050)
* [Keil - C51 - Documentação](http://www.keil.com/support/man/docs/c51/)
* [InvenSense - MPU6050 - DATASHEET](https://www.invensense.com/products/motion-tracking/6-axis/mpu-6050/)
* [Nordic Semiconductor - nRF24LE1 - DATASHEET](http://www.nordicsemi.com/eng/Products/2.4GHz-RF/nRF24LE1)
* [TeckShlok - Alguns exemplos do nRF24LE1](http://techshlok.com/blog/)
