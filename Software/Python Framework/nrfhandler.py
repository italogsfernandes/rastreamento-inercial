# -*- coding: utf-8 -*-
#------------------------------------------------------------------------------
# FEDERAL UNIVERSITY OF UBERLANDIA
# Faculty of Electrical Engineering
# Biomedical Engineering Lab
#------------------------------------------------------------------------------
# Author: Italo G S Fernandes; Andrei Nakagawa
# Contact: italogsfernandes@gmail.com; nakagawa.andrei@gmail.com
# URL: www.biolab.eletrica.ufu.br
# Git: www.github.com/italogfernandes/; www.github.com/andreinakagawa
#------------------------------------------------------------------------------
# Decription: Aqui estão implementados metodos de comunicacao via serial
# entre o computador e o arduino em q o nrf host esta conectado.
#------------------------------------------------------------------------------
from struct import unpack
from ctypes import c_short, c_ubyte
from Queue import Queue
from serial import Serial
from threading import Timer
from threadhandler import ThreadHandler
from time import sleep

class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

#------------------------------------------------------------------------------
#NRF constants
#Constants for handling serial communication
class NRFConsts():
	UART_START_SIGNAL = 0x53
	CMD_OK = 0x00  # Ack - Uart Command
	CMD_ERROR = 0x01  # Error flag - Uart Command
	CMD_START = 0x02  # Start Measuring - Uart Command
	CMD_STOP = 0x03  # Stop Measuring - Uart Command
	CMD_CONNECTION = 0x04  # Teste Connection - Uart Command
	CMD_CALIBRATE = 0x05  # Calibrate Sensors Command
	CMD_DISCONNECT = 0x06  # Some sensor has gone disconected
	CMD_READ = 0x07  # Request a packet of readings
	CMD_SET_PACKET_TYPE = 0x08
	CMD_GET_ACTIVE_SENSORS = 0x09  # Retorna a variavel do host active sensors
	CMD_SET_ACTIVE_SENSORS = 0x0A  # Retorna a variavel do host active sensors
	CMD_TEST_RF_CONNECTION = 0x0B
	CMD_LIGHT_UP_LED = 0x0C
	CMD_TURN_OFF_LED = 0x0D
	#TODO: implement in devices:
	CMD_TIMEOUT = 0x03			#Board response timed out

#------------------------------------------------------------------------------
class SerialHandler():
  def __init__(self,_port='ttyACM0',_baud=38400,_timeout=0.5):
	self.port = _port
	self.baud = _baud
	self.timeout = _timeout
	self.waiting = False
	self.serialPort = None

  def open(self):
	try:
	  self.serialPort = Serial(self.port,self.baud,timeout=self.timeout)
	  if self.serialPort.is_open:
		self.serialPort.flushInput()
		self.serialPort.flushOutput()
		return True
	  else:
		return False

	except:
	  return False

  def close(self):
	try:
	  self.serialPort.flushInput()
	  self.serialPort.flushOutput()
	  self.serialPort.close()
	  if self.serialPort.is_open:
		return False
	  else:
		return True
	except:
	  return False

  def waitBytes(self,_numBytes):
	try:
	  self.waiting = True
	  t = Timer(self.timeout,self.getTimeout)
	  t.start()
	  numWaiting = 0
	  while True:
		if(self.serialPort.is_open):
		  numWaiting = self.serialPort.in_waiting
		  if numWaiting < _numBytes and self.waiting is True:
			pass
		  else:
			break
		else:
		  break
	  if numWaiting >= _numBytes:
		t.cancel()
		return True
	  else:
		return False
	except:
	  return False

  def getTimeout(self):
	self.waiting = False

  def to_int16(self,_MSB,_LSB):
	return c_short((_MSB<<8) + _LSB).value

  #TODO: perguntar andrei oq esta acontecendo aqui
  def to_float(self,_byteVector):
	binF = ''.join(char(i) for i in _byteVector)
	return unpack('f',binF)[0]

  #NOTE: teste this funcion, created by italo
  def checksum_calc(self,_num):
	complemet_of_two = c_ubyte(~_num + 1).value
	return complement_of_two

  def getquatfromstr(self,values_raw):
    start_index = values_raw.rfind('S')
    packet_len = ord(values_raw[start_index+1])
    sensor_id = ord(values_raw[start_index+2])
    wquat = self.to_int16(ord(values_raw[start_index+3]), ord(values_raw[start_index+4]))
    xquat = self.to_int16(ord(values_raw[start_index+5]), ord(values_raw[start_index+6]))
    yquat = self.to_int16(ord(values_raw[start_index+7]), ord(values_raw[start_index+8]))
    zquat = self.to_int16(ord(values_raw[start_index+9]), ord(values_raw[start_index+10]))
    print("packet len: %d, sensor_id: %d" %(packet_len,sensor_id))
    quat = [wquat,xquat,yquat,zquat]
    return quat
#------------------------------------------------------------------------------
#Definition of the nrf class
#Inherits from SerialHandler for using serial communication
#Serial data is stored in a queue that can be accessed for
#later processing

class nrf(SerialHandler):
	#Constructor
	def __init__(self,_port='dev/ttyACM0',_baud=38400):
		#TODO: Fix Thread
		#FIX: Thread
		#self.acqThread = ThreadHandler(self.readPackage)
		self.isConnected = False
		self.dataQueue = Queue()
		SerialHandler.__init__(self,_port,_baud)

	def sendcmd(self, cmd_2_send):
		self.serialPort.write([NRFConsts.UART_START_SIGNAL, cmd_2_send])

	def sendcmd_with_arg(self, cmd_2_send, arg_2_send):
		self.serialPort.write([NRFConsts.UART_START_SIGNAL, cmd_2_send, arg_2_send])

	#Starts data acquisition
	#TODO: perguntar andrei, "Needs return?"
	def start(self):
		self.sendcmd(NRFConsts.CMD_START)
		self.dataQueue = Queue()

	#Stops data acquisition
	#Needs return?
	def stop(self):
		self.sendcmd(NRFConsts.CMD_STOP)

	def calibration(self):
		aux = self.timeout
		self.timeout = 10
		self.sendcmd(NRFConsts.CMD_CALIBRATE)
		ret = self.waitBytes(1)
		self.timeout = aux
		if ret:
			resp = ord(self.serialPort.read())
			if resp is NRFConsts.CMD_OK:
				print 'ok'
				return NRFConsts.CMD_OK
			else:
				return NRFConsts.CMD_ERROR
		else:
			return NRFConsts.CMD_TIMEOUT


#------------------------------------------------------------------------------
#Tests acquisition
if __name__ == "__main__":
	due_host = nrf('/dev/ttyACM0',115200)
	due_host.open()
	print '-------------------------------'
	sleep(1)
	print(bcolors.OKBLUE + "Leitura da porta serial:" + bcolors.ENDC)
	print(due_host.serialPort.read(due_host.serialPort.in_waiting))
	while(True):
		print '-------------------------------'*2
		print 'Biomedical Engineering Lab'
		print 'MPU 6050 Data Acquisition Using nrf24LE1 Wireless Transponder'
		print '-------------------------------'*2
		print 'Menu'
		print '1 - New acquisition'
		print '2 - Stop acquisition'
		print '3 - Calibrate'
		print '4 - Connection'
		print '5 - Set Packet Type'
		print '6 - Get Active Sensors'
		print '7 - Set Active Sensors'
		print '8 - Test RF Connection'
		print '9 - Light up LED'
		print '10 - Turn off LED'
		print '11 - Read a value'
		print '12 - Read Serial Port'
		print '13 - Exit'
		print '-------------------------------'
		strkey = raw_input()
		if strkey == '1':
			due_host.start()
			print(due_host.serialPort.read(1))
			due_host.acqThread.start()
		elif strkey == '2':
			due_host.stop()
			print(due_host.serialPort.read(1))
			due_host.acqThread.kill()
		elif strkey == '3':
			due_host.calibration()
			print(bcolors.OKBLUE + "Implement" + bcolors.ENDC)
		elif strkey == '4':
			due_host.sendcmd(NRFConsts.CMD_CONNECTION)
			print(bcolors.OKBLUE + "implement" + bcolors.ENDC)
		elif strkey == '5':
			#TODO: complement
			print(bcolors.OKBLUE + "Digite o codigo do tipo:" + bcolors.ENDC)
			strkey = int(raw_input())
			due_host.sendcmd_with_arg(NRFConsts.CMD_SET_PACKET_TYPE,strkey)
		elif strkey == '6':
			print(bcolors.OKBLUE + "Leitura da porta serial:" + bcolors.ENDC)
			print(due_host.serialPort.read(due_host.serialPort.in_waiting))
			due_host.sendcmd(NRFConsts.CMD_GET_ACTIVE_SENSORS)
			print(bcolors.OKBLUE + "Active Sensors variable:" + bcolors.ENDC)
			ret = due_host.waitBytes(1)
			if ret:
				receivedByte = ord(due_host.serialPort.read(1))
				print(bin(receivedByte))
			else:
				print(bcolors.OKBLUE + "Resposta nao Recebida, timeout ou erro." + bcolors.ENDC)
		elif strkey == '7':
			#TODO: complement
			print(bcolors.OKBLUE + "Digite o valor da variavel binario (8bits):" + bcolors.ENDC)
			strkey = int(raw_input(),2)
			print(bcolors.OKBLUE + "Voce inseriu: %d -- %s -- %s"%(strkey,hex(strkey),bin(strkey)+ bcolors.ENDC))
			due_host.sendcmd_with_arg(NRFConsts.CMD_SET_ACTIVE_SENSORS,strkey)
		elif strkey == '8':
			#TODO: complement
			print(bcolors.OKBLUE + "Digite endereco do sensor em hex:" + bcolors.ENDC)
			strkey = int(raw_input(),16)
			print(bcolors.OKBLUE + "Voce inseriu: %d -- %s -- %s"%(strkey,hex(strkey),bin(strkey)+ bcolors.ENDC))
			due_host.sendcmd_with_arg(NRFConsts.CMD_TEST_RF_CONNECTION,strkey)
		elif strkey == '9':
			print(bcolors.OKBLUE + "Acender LED" + bcolors.ENDC)
			print(bcolors.OKBLUE + "Digite endereco do sensor em hex:" + bcolors.ENDC)
			strkey = int(raw_input(),16)
			print(bcolors.OKBLUE + "Voce inseriu: %d -- %s -- %s"%(strkey,hex(strkey),bin(strkey)+ bcolors.ENDC))
			due_host.sendcmd_with_arg(NRFConsts.CMD_LIGHT_UP_LED,strkey)
		elif strkey == '10':
			print(bcolors.OKBLUE + "Apagar LED" + bcolors.ENDC)
			print(bcolors.OKBLUE + "Digite endereco do sensor em hex:" + bcolors.ENDC)
			strkey = int(raw_input(),16)
			print(bcolors.OKBLUE + "Voce inseriu: %d -- %s -- %s"%(strkey,hex(strkey),bin(strkey)+ bcolors.ENDC))
			due_host.sendcmd_with_arg(NRFConsts.CMD_TURN_OFF_LED,strkey)

		elif strkey == '11':
			due_host.sendcmd(NRFConsts.CMD_READ)
			sleep(0.5)
			ret = due_host.waitBytes(1)
			if ret:
			    receivedstr = due_host.serialPort.read(due_host.serialPort.in_waiting)
			    print(receivedstr)
			    quat = due_host.getquatfromstr(receivedstr)
			    print([quat[0]/18384.00,quat[1]/18384.00,quat[2]/18384.00,quat[3]/18384.00])
			    print(quat)
			    receivedstr = receivedstr[0:len(receivedstr)-12]
			    quat = due_host.getquatfromstr(receivedstr)
			    print([quat[0]/18384.00,quat[1]/18384.00,quat[2]/18384.00,quat[3]/18384.00])
			    print(quat)
			    print(bcolors.OKBLUE + "Leitura da porta serial em HEX:" + bcolors.ENDC)
			    stroutput = ""
			    for valor in receivedstr:
			        stroutput += (bcolors.OKGREEN + ("%s "%(hex(ord(valor)))) + bcolors.OKBLUE + "- " + bcolors.ENDC)
			    print(stroutput)
			else:
				print(bcolors.OKBLUE + "Resposta nao Recebida, timeout ou erro." + bcolors.ENDC)
		elif strkey == '12':
			print(bcolors.OKBLUE + "Leitura da porta serial:" + bcolors.ENDC)
			str_leitura = due_host.serialPort.read(due_host.serialPort.in_waiting)
			print(bcolors.OKGREEN + str_leitura + bcolors.ENDC)
			print(bcolors.OKBLUE + "Leitura da porta serial em HEX:" + bcolors.ENDC)
			stroutput = ""
			for valor in str_leitura:
			    stroutput += (bcolors.OKGREEN + ("%s "%(hex(ord(valor)))) + bcolors.OKBLUE + "- " + bcolors.ENDC)
			if len(stroutput) < 10000:
			    print(stroutput)

		elif strkey == '13':
			due_host.close()
			break

#------------------------------------------------------------------------------
