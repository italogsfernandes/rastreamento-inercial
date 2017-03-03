import serial
from time import sleep

class nrf:
	CMD_OK = 0x00 #Ack - Uart Command
    CMD_ERROR = 0x01 #Error flag - Uart Command
    CMD_START = 0x02 #Start Measuring - Uart Command
    CMD_STOP = 0x03 #Stop Measuring - Uart Command
    CMD_CONNECTION = 0x04 #Teste Connection - Uart Command
    CMD_CALIBRATE = 0x05 #Calibrate Sensors Command
    CMD_DISCONNECT = 0x06 #Some sensor has gone disconected
    CMD_READ = 0x07 #Request a packet of readings
    CMD_SET_PACKET_TYPE = 0x08
    CMD_GET_ACTIVE_SENSORS = 0x09 #Retorna a variavel do host active sensors
    CMD_TEST_RF_CONNECTION = 0x0A
    CMD_LIGHT_UP_LED = 0x0B
    CMD_TURN_OFF_LED =  0x0C
	porta = serial.Serial()
	def __init__(self,porta,rate):
		self.porta = serial.Serial(porta,rate)
	
	def readall(self):
		return self.porta.read(self.porta.inWaiting())

	def sendcmd(self,c):
		self.porta.write([0x53,c])
		sleep(0.1)
		return self.readall()
