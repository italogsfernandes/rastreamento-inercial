# -*- coding: utf-8 -*-
#------------------------------------------------------------------------------
# FEDERAL UNIVERSITY OF UBERLANDIA
# Faculty of Electrical Engineering
# Biomedical Engineering Lab
#------------------------------------------------------------------------------
# Author: Andrei Nakagawa, MSc
# Contact: nakagawa.andrei@gmail.com
# URL: www.biolab.eletrica.ufu.br
# Git: www.github.com/andreinakagawa
#------------------------------------------------------------------------------
# Decription:
#------------------------------------------------------------------------------
from struct import unpack
from ctypes import c_short
from Queue import Queue
from serial import Serial
from threading import Timer
from threadhandler import ThreadHandler
import numpy as np
import scipy as sp
import os, sys
sys.path.append('../')
import quaternion as quat
#------------------------------------------------------------------------------
#MPU constants
#Constants for handling serial communication
class MPUConsts():
	UART_ST = 0x7F  			#Start transmission
	UART_ET = 0x7E				#End transmission
	UART_PT1 = 0x21 			#Package Type I: Motion and Quaternion (in float)
	UART_PT2 = 0x22 			#Package Type I: Motion and Quaternion (in int16)
	UART_PT3 = 0x23 			#Package Type I: Motion and Quaternion (in int16)
	CMD_START = 'CMDSTART'		#Command that starts acquisition
	CMD_STOP = 'CMDSTOP'  		#Command that stops acquisition
	CMD_CONN = 'CMDCONN'  		#Command that checks the connection with the MPU6050
	CMD_CALIB = 'CMDCALIB'		#Command for calibrating the MPU6050
	CMD_OK = 0x01 				#Command was successful
	CMD_ERROR = 0x02 			#Command was unsuccessful
	CMD_TIMEOUT = 0x03			#Board response timed out
	CAL_ACCEL_TOL = 8			#Tolerated error for accelerometer
	CAL_GYRO_TOL = 1			#Tolerated error for gyroscope
	CAL_TIMEOUT = 10			#Maximum duration of calibration (in seconds)
#------------------------------------------------------------------------------
class SerialHandler():
	def __init__(self,_port='ttyACM0',_baud=115200,_timeout=0.5):
		self.port = _port
		self.baud = _baud
		self.timeout = _timeout
		self.waiting = False
		self.serialPort = None

	def open(self):
		try:
			#self.serialPort = Serial(self.port,self.baud,timeout=self.timeout)
			self.serialPort = Serial('/dev/ttyUSB0',self.baud,timeout=self.timeout)
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

	def waitSTByte(self,_startByte):
		receivedByte = 0
		while True:
			ret = self.waitBytes(1)
			if ret:
				receivedByte = ord(self.serialPort.read())
				if receivedByte == _startByte:
					return True
				else:
					return False
			else:
				return False

	def to_int16(self,_MSB,_LSB):
		return c_short((_MSB<<8) + _LSB).value

	def to_float(self,_byteVector):
		binF = ''.join(chr(i) for i in _byteVector)
		return unpack('f',binF)[0]
#------------------------------------------------------------------------------
#Definition of the MPU6050 class
#Inherits from SerialHandler for using serial communication
#Serial data is stored in a queue that can be accessed for
#later processing
class MPU6050(SerialHandler):
	#Constructor
	def __init__(self,_port='/dev/ttyACM0',_baud=115200):
		self.acqThread = ThreadHandler(self.readPackage)
		self.isConnected = False
		self.dataQueue = Queue()
		self.flagAcq = False
		SerialHandler.__init__(self,_port,_baud)

	#Stars data acquisition
	#Needs return?
	def start(self):
		self.serialPort.write(MPUConsts.CMD_START)
		self.serialPort.flushInput()
		self.serialPort.flushOutput()
		self.dataQueue = Queue()
		twait = Timer(5,self.waitTimeout)
		twait.start()

	#Stops data acquisition
	#Needs return?
	def stop(self):
		self.serialPort.write(MPUConsts.CMD_STOP)

	def waitTimeout(self):
		print 'liberado!'
		self.flagAcq = True

	'''
	Summary: This method serves to test whether
	communication has been established with the
	MPU6050 board
	Returns CMD_OK if MPU6050 is connected
	Returns CMD_ERROR if MPU6050 is not connected
	Returns CMD_TIMEOUT if the board did not respond in time
	'''
	def testConnection(self):
		aux = self.timeout
		self.timeout = 2
		self.serialPort.write(MPUConsts.CMD_CONN)
		ret = self.waitBytes(1)
		if(ret is False):
			self.timeout = aux
			return MPUConsts.CMD_TIMEOUT
		else:
			self.timeout = aux
			resp = ord(self.serialPort.read(1))
			return resp

	#Method that triggers the calibration process
	def calibration(self,_accelTol=MPUConsts.CAL_ACCEL_TOL,_gyroTol=MPUConsts.CAL_GYRO_TOL,_timeout=MPUConsts.CAL_TIMEOUT):
		aux = self.timeout
		self.timeout = 5
		self.serialPort.write(MPUConsts.CMD_CALIB)
		ret = self.waitBytes(1)
		self.timeout = aux
		if ret:
			resp = ord(self.serialPort.read())
			if resp is MPUConsts.CMD_OK:
				#self.serialPort.write(str(_accelTol) + '\n')
				#self.serialPort.write(str(_gyroTol) + '\n')
				#self.serialPort.write(str(_timeout) + '\n')
				print 'ok'
				return MPUConsts.CMD_OK
			else:
				return MPUConsts.CMD_ERROR
		else:
			return MPUConsts.CMD_TIMEOUT

	'''
	Summary: Method for reading packages
	Standard MPU6050 package contains:
	Sensor data is read from DMP
	1st byte: Start transmission = '$'
	Next 8 bytes, each measure is composed by two bytes (MSB first)
	Quaternion from sensor1 (w,x,y,z)
	...
	End Byte = '\n'
	'''
	def readPackage(self):
		#print 'readPackage called'
		dataVector = []
		cont = 0
		try:
			ret = self.waitSTByte(MPUConsts.UART_ST)
			if ret:
				#print 'ST received'
				for sensor_id in range(0,1):
					#dataVector.append(float(sensor_id));
					#print 'sensor_id: %d' % sensor_id
					ret = self.waitBytes(8)
					if ret:
						data = self.serialPort.read(8)
						data = map(ord,data)
						for cont in range(0,8,2):
							dataVector.append(self.to_int16((data[cont]),(data[cont+1]))/16384.00)
							#print dataVector[cont/2]
				ret = self.waitBytes(1)
				if ret:
					endByte = ord(self.serialPort.read())
					if endByte == MPUConsts.UART_ET:
						normq = []
						cont = 0
						for i in range(0,1):
							normq.append(np.sqrt(dataVector[cont]**2+dataVector[cont+1]**2+dataVector[cont+2]**2+dataVector[cont+3]**2))
							cont += 4
						#Should check if every quaternion value is less than 1
						#quat = [q if -1 < q < 1 else [] for q in dataVector[0:4]]
						normTest = lambda(val): True if 0 < val < 1.1 else False
						vetNorms = map(normTest,normq)
						#print normq
						if not False in vetNorms:
							#print "%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t\t\t" % (dataVector[0], dataVector[1], dataVector[2], dataVector[3], normq)
							#print self.serialPort.in_waiting
							#print self.serialPort.in_waiting
							#floatfix = lambda(x): float("{0:.4f}".format(x))
							#dataVector = map(floatfix,dataVector)
							if self.flagAcq is True:
								self.acqThread.lock.acquire()
								self.dataQueue.put(dataVector)
								self.acqThread.lock.release()
						else:
							print "NAO: %.2f\t%.2f\t%.2f\t%.2f\t%.2f" % (dataVector[0], dataVector[1], dataVector[2], dataVector[3], normq)
					else:
						print dataVector
						print ' package error!'
		except:
			print data
			print 'read error!'
#------------------------------------------------------------------------------
#Tests acquisition
if __name__ == "__main__":
	imu = MPU6050('/dev/ttyACM0')
	imu.open()
	while(True):
		print '-------------------------------'
		print 'Biomedical Engineering Lab'
		print 'MPU 6050 Data Acquisition'
		print '-------------------------------'
		print 'Menu'
		print '1 - New acquisition'
		print '2 - Stop acquisition'
		print '3 - Calibration'
		print '4 - Exit'
		print '-------------------------------'
		strkey = raw_input()
		if strkey == '1':
			#print("Thread Started.")
			imu.start()
			imu.acqThread.start()
		elif strkey == '2':
			imu.stop()
			imu.acqThread.kill()
		elif strkey == '3':
			imu.calibration()
		elif strkey == '4':
			imu.close()
			break
#------------------------------------------------------------------------------
