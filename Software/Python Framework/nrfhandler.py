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

#------------------------------------------------------------------------------
#Definition of the nrf class
#Inherits from SerialHandler for using serial communication
#Serial data is stored in a queue that can be accessed for
#later processing

class nrf(SerialHandler):
  #Constructor
  def __init__(self,_port='dev/ttyACM0',_baud=38400):
    self.acqThread = ThreadHandler(self.readPackage)
    self.isConnected = False
    self.dataQueue = Queue()
    SerialHandler.__init__(self,_port,_baud)
    
	def sendcmd(self, cmd_2_send):
		self.porta.write([NRFConsts.UART_START_SIGNAL, cmd_2_send])

	def sendcmd_with_arg(self, cmd_2_send, arg_2_send):
		self.porta.write([NRFConsts.UART_START_SIGNAL, cmd_2_send, arg_2_send])

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
		
		
	def readPackage(self):
		dataVector = []
		cont = 0
		try:
			ret = self.waitSTByte(MPUConsts.UART_ST)
			if ret:
				ret = self.waitBytes(2)
				if ret:
					#self.pckgType = ord(self.serialPort.read())
					self.pckgLen = ord(self.serialPort.read())
					ret = self.waitBytes(self.pckgLen)
					if ret:
						data = self.serialPort.read(self.pckgLen)
						data = map(ord,data)
						while True:
							if cont < 8:
								dataVector.append(self.to_int16((data[cont]),(data[cont+1]))/16384.)
							else:
								dataVector.append(self.to_int16((data[cont]),(data[cont+1])))
							cont += 2
							if cont >= self.pckgLen:
								break
						ret = self.waitBytes(1)
						if ret:
							endByte = ord(self.serialPort.read())
							if endByte == MPUConsts.UART_ET:
								self.acqThread.lock.acquire()
								self.dataQueue.put(dataVector)
								#print dataVector
								self.acqThread.lock.release()
							else:
								print ' package error!'
		except:
			print 'read error!'
    
    
    
#------------------------------------------------------------------------------
#Tests acquisition
if __name__ == "__main__":
	imu = nrf('/dev/ttyACM0')
	imu.open()
	while(True):
		print '-------------------------------'
		print 'Biomedical Engineering Lab'
		print 'MPU 6050 Data Acquisition Using nrf24LE1 Wireless Transponder'
		print '-------------------------------'
		print 'Menu'
		print '1 - New acquisition'
		print '2 - Stop acquisition'
		print '3 - Calibrate'
    print '4 - Other Options'
		print '5 - Exit'
		print '-------------------------------'
		strkey = raw_input()
		if strkey == '1':
			imu.start()
			imu.acqThread.start()
		elif strkey == '2':
			imu.stop()
			imu.acqThread.kill()
		elif strkey == '3':
			imu.calibration()
    elif strkey == '5':
			imu.close()
			break
    elif strkey == '4':
      print '-------------------------------'
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
      print '12 - Exit'
      print '-------------------------------'
      strkey = raw_input()
      if strkey == '1':
        imu.start()
        imu.acqThread.start()
      elif strkey == '2':
        imu.stop()
        imu.acqThread.kill()
      elif strkey == '3':
        imu.calibration()
      elif strkey == '4':
				imu.sendcmd(NRFConsts.CMD_CONNECTION)
			elif strkey == '5':
				#TODO: complement
				#imu.sendcmd_with_arg(NRFConsts.CMD_SET_PACKET_TYPE)
			elif strkey == '6':
				imu.sendcmd(NRFConsts.CMD_GET_ACTIVE_SENSORS)
			elif strkey == '7':
				#TODO: complement
				#imu.sendcmd_with_arg(NRFConsts.CMD_SET_ACTIVE_SENSORS)
			elif strkey == '8':
				imu.sendcmd(NRFConsts.CMD_TEST_RF_CONNECTION)
			elif strkey == '9':
				imu.sendcmd(NRFConsts.CMD_LIGHT_UP_LED)
			elif strkey == '10':
				imu.sendcmd(NRFConsts.CMD_TURN_OFF_LED)
			elif strkey == '11':
				imu.sendcmd(NRFConsts.CMD_READ)
      elif strkey == '12':
        imu.close()
        break
        
        

#------------------------------------------------------------------------------
    
######################################################################
import serial
from time import sleep



class nrf:
    '''Comandos reconhecidos pelo host'''


    porta = serial.Serial()

    def __init__(self, porta, rate):
        self.porta = serial.Serial(porta, rate)

    def readall(self):
        return self.porta.read(self.porta.inWaiting())


    def led(self, estado_led):
        if estado_led == 1:
            return self.sendcmd(self.CMD_LIGHT_UP_LED)
        else:
            return self.sendcmd(self.CMD_TURN_OFF_LED)

    def readsomething(self):
        return self.sendcmd(self.CMD_READ)
