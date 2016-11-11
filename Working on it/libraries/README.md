# Bibliotecas

## hal_w2_isr.h
* Descrição: Realiza comunicação I2C com os sensores:
* Funções:
- [x] @mentions, #refs, [links](), **formatting**, and <del>tags</del> supported
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
	- [x] <del>set*AccelOffset</del>
	- [x] <del>set*GyroOffset</del>
	- [ ] getXAccelOffset - Depende de readWord
	- [ ] getYAccelOffset - Depende de readWord
	- [ ] getZAccelOffset - Depende de readWord
	- [ ] getXGyroOffset - Depende de readWord
	- [ ] getYGyroOffset - Depende de readWord
	- [ ] getZGyroOffset - Depende de readWord
	* DMP
	- [ ] dmpInitialize - Depende de muita coisa (olhar abaixo)
	- [ ] setDMPEnabled - Depende de **writeBit**
	- [ ] dmpGetFIFOPacketSize - Depende de **so de boa vontade**
	- [ ] dmpGetQuaternion - Depende de getFIFO
		* dmpInitialize depende de:
		- [ ] setMemoryBank - Depende de **writeByte**
		- [ ] get*GyroOffsetTC - Depende de **readBits**
		- [ ] pgm_read_byte - XXX: ?
		- [ ] setDMPEnabled - Depende de **writeBit**
		- [ ] writeProgMemoryBlock - Depende de writeMemoryBlock
		- [ ] writeProgDMPConfigurationSet - Depende de writeDMPConfigurationSet
		- [ ] getFIFOCount - Depende de **readByte**
		- [ ] resetFIFO - Depende de **writeBit**
		- [ ] getFIFOBytes - Depende de **readBytes**
		- [ ] readMemoryBlock - Depende das mesmas coisas de writeMemoryBlock
		- [ ] getIntStatus - Depende de **readByte**
		- [ ] writeDMPConfigurationSet
			* Depende de:
			- [ ] writeMemoryBlock
			- [ ] setMemoryBank - Depende de **writeByte**
			- [ ] pgm_read_byte - XXX: ?
			- [ ] memcmp - XXX: ?
			- [ ] malloc e free - [http://www.keil.com/support/man/docs/c51/c51_malloc.htm]()
		- [ ] writeMemoryBlock
			* Depende de:
			- [ ] setMemoryBank - Depende de **writeByte**
			- [ ] pgm_read_byte - XXX: ?
			- [ ] memcmp - XXX: ? 
			- [ ] malloc e free - [http://www.keil.com/support/man/docs/c51/c51_malloc.htm]()