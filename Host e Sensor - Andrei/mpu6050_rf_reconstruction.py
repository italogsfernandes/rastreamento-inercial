# -*- coding: utf-8 -*-
'''
#------------------------------------------------------------------------------
# FEDERAL UNIVERSITY OF UBERLANDIA
# Faculty of Electrical Engineering 
# Biomedical Engineering Lab
#------------------------------------------------------------------------------
# Author: Andrei Nakagawa, MSc
# Contact: nakagawa.andrei@gmail.com
# Decription:
#------------------------------------------------------------------------------
# Algorithm for 3D limb tracking
# Given the placement of the sensor in each joint: Consider that a standard
# initial position will have the sensor faced up with the x-axis parallel
# to the limb. For example: Arms open in T
# In this case, the position of each sensor in the x-axis will correspond to
# its length. Therefore, the rotation of joints will be given by this same axis
# since the origin will only have a component in x.
#------------------------------------------------------------------------------
'''
import serial
import threadinggit
from threading import Lock, Timer
from time import time,sleep
from datetime import datetime
from datetime import timedelta
from Queue import Queue
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import numpy as np
from mpl_toolkits.mplot3d import axes3d
import matplotlib
import matplotlib.pyplot as plt
import MadgwickAHRS as imu
import Quaternion as quat
#from pymouse import PyMouse
import copy
import struct

#Sensor readings with offsets:   -1807   -108    17042   0   4   -3
#Your offsets:   -2884   -695    1213    82  -25 2

#Start signal | Packet Type | Packet Length | ... | data | ... | End signal
UART_START_SIGNAL = 0x53
UART_END_SIGNAL = 0x04

#mouseController = PyMouse()
dataList = []
timeList = []
sampfreq = 100
flagWait = True
flagAcq = True
flagPause = True
flagTimer = True
flagKey = True
keyPressed = False
lostPackages = 0
timerPeriod = 0.1 #100 ms
dataQueue = Queue()
lock = Lock()
plotLock = Lock()

fig = plt.figure()
ax = fig.gca(projection='3d')
matplotlib.interactive(True)

axisLimits = [-10,10]
quiverSize = 3

#limits
ax.set_xlim(axisLimits)
ax.set_ylim(axisLimits)
ax.set_zlim(axisLimits)

#labels
ax.set_title('3D reconstruction')
ax.set_xlabel('x-axis')
ax.set_ylabel('y-axis')
ax.set_zlabel('z-axis')

#Points
p0 = [0.,0.,0.] #Chest position
p1 = [3.,0.,0.] #Shoulder position
p2 = np.array([6.,0.,0.]) #Arm position --> Initial position
p3 = np.array([0.,0.,0.]) #Arm position --> Initial position
#Vector
#Origin to Shoulder
v1x = [p0[0],p1[0]]
v1y = [p0[1],p1[1]]
v1z = [p0[2],p1[2]]
#Shoulder to hand
v2x = [p1[0],p2[0]]
v2y = [p1[1],p2[1]]
v2z = [p1[2],p2[2]]

#Origin for quiver is the hand point
ox = p2[0]
oy = p2[1]
oz = p2[2]

ux = 1
vx = 0
wx = 0

uy = 0
vy = 1
wy = 0

uz = 0
vz = 0
wz = 1

chartData = []

beta = 0.9
g =  9.81 #Standard gravity value
nbits = 16
accelFactor = (np.power(2,nbits)) / (2*2*g);
gyrFactor = (np.power(2,nbits)) / (250*2);
imuProc = imu.MadgwickAHRS(sampfreq,beta)

def wait_serial_bytes(how_many):
    while serialPort.inWaiting()<how_many:
        if(flagWait == False):
            break
        pass

def wait_start_signal():
    start_signal = 0
    while start_signal  != UART_START_SIGNAL:    #waiting start signal
        wait_serial_bytes(1)
        start_signal = ord(serialPort.read())
        print start_signal

def inverter(y):
    x = bin(y)
    return int(x.replace('1','2').replace('0','1').replace('2','0').replace('1b','0b'),2)

def to_signed(num):
    return -(inverter(num & 0x7fff))-1 if num & 0x8000 else num & 0x7fff

def to_uint16(MSB,LSB):
    return (MSB << 8 | LSB)

def to_int16(MSB,LSB):
    return to_signed(MSB << 8 | LSB)

def to_float(byteVector):
    binF = ''.join(chr(i) for i in byteVector)
    return struct.unpack('f',binF)[0]

def StartAcq():
    serialPort.write([0x53,0x02])

def PauseAcq():
    serialPort.write('CMDSTOP'.encode('utf-8'))

def ResumeTh():
    global flagWait, flagPause
    flagWait = True
    flagPause = False

def PauseTh():
    global flagWait, flagPause
    flagPause = True
    flagWait = False

def KillTh():
    global flagWait, flagPause, flagAcq
    flagWait = False
    flagPause = True
    flagAcq = False

def Save():
    f = open(filename,'w')
    for samples in dataList:
        for i in range(len(samples)):
            f.write(str(samples[i]))
            f.write('\t')
        f.write('\n')

def convScale(xval,xmin,xmax,ymin,ymax):
    return float((xval-xmin)*(ymax-ymin) / float(xmax-xmin)) + float(ymin)

def worker():
    global dataList, lostPackages, lock, dataQueue, imuProc
    while(flagAcq):
        if(flagPause == False):
            wait_start_signal()
            wait_serial_bytes(1)
            packet_len = ord(serialPort.read())
            wait_serial_bytes(packet_len)
            sensor_id = ord(serialPort.read())
            packet_type = ord(serialPort.read())
            data = serialPort.read(packet_len-2)
            print data
            cont = 1
            dataVector = []
            for i in range(4):
                val = to_int16(ord(data[cont]),ord(data[cont+1]))
                val = val / 16384.
                dataVector.append(val)
                cont = cont + 2
            #endByte = ord(serialPort.read());
            #if(endByte == UART_END_SIGNAL):
            dataVector.append(str(datetime.now()))
            dataList.append(dataVector)
            #Acessing the dataQueue
            lock.acquire()
            dataQueue.put(dataVector)
            print dataVector
            lock.release()
            #else:
            #    lostPackages = lostPackages + 1;
    return

def timerTick():
    global ax,qx,qy,qz,accelFactor,gyrFactor,g,imuProc,plotLock,ux,vx,wx,uy,vy,wy,uz,vz,wz,chartData,p3,v2x,v2y,v2z,mouseController
    while(flagTimer):
        sleep(timerPeriod)
        lock.acquire()
        numbSamples = dataQueue.qsize()
        dataacq = []
        if(numbSamples > 0):
            for i in range(numbSamples):
                dataacq.append(dataQueue.get())
            lock.release()

            for samples in dataacq:
                imuQuaternion = samples[0:4]

            plotLock.acquire()
            rotmat = quat.quatern2rotMat(imuQuaternion)
            ux = rotmat[0,0]
            vx = rotmat[0,1]
            wx = rotmat[0,2]
            uy = rotmat[1,0]
            vy = rotmat[1,1]
            wy = rotmat[1,2]
            uz = rotmat[2,0]
            vz = rotmat[2,1]
            wz = rotmat[2,2]
            pt = p2 - p1
            p3 = np.dot(pt,rotmat)
            p3 += p1
            v2x = [p1[0],p3[0]]
            v2y = [p1[1],p3[1]]
            v2z = [p1[2],p3[2]]
            #print 'Quaternion'
            #print imuQuaternion
            #print 'Rotation Matrix'
            #print ux,' ', uy, ' ',   uz
            #print vx,' ', vy, ' ', vz
            #print wx,' ', wy, ' ', wz
            #print str(datetime.now())
            #mouseController.move(convScale(p3[0],3,6,800,1500),convScale(p3[2],-6,6,1024,0))
            #print 'P3:', p3[0],' ',p3[1],' ',p3[2]
            #print 'P4:', p4[0],' ',p4[1],' ',p4[2]
            #print p3[0],' ',p3[1],' ',p3[2]
            plotLock.release()

        else:
            lock.release()

key = ''
def keyListener():
    global key
    while(flagKey):
        key = raw_input()

filename = 'mpu6050data.txt'
portName = '/dev/ttyACM0'
baud = 9600
tout = 1
serialPort = serial.Serial(portName,baud,timeout=tout)
print serialPort
if(serialPort.is_open == False):
    serialPort.open()
serialPort.flushInput()
serialPort.flushOutput()
threadAcq = threading.Thread(target=worker)
threadAcq.start()
threadTimer = threading.Thread(target=timerTick)
threadKey = threading.Thread(target=keyListener)

while(1):
    print '-------------------------------'
    print 'Biomedical Engineering Lab'
    print 'MPU 6050 Data Acquisition'
    print '-------------------------------'
    print 'Menu'
    print '1 - New acquisition'
    print '2 - Calibration'
    print '3 - Exit'
    print '-------------------------------'
    strkey = raw_input()
    if(strkey == '1'):
        #Quivers at the hand
        qx = ax.quiver(ox,oy,oz,ux,vx,wx, length=quiverSize, pivot='tail', color='red')
        qy = ax.quiver(ox,oy,oz,uy,vy,wy, length=quiverSize, pivot='tail', color='green')
        qz = ax.quiver(ox,oy,oz,uz,vz,wz, length=quiverSize, pivot='tail', color='blue')
        #Points
        #Origin
        p0sct = ax.scatter(p0[0],p0[1],p0[2],color='black')
        #Shoulder
        p1sct = ax.scatter(p1[0],p1[1],p1[2],color='blue')
        #Hand
        p2sct = ax.scatter(p2[0],p2[1],p2[2],color='green')
        #Lines
        #From chest (origin) to shoulder
        line1 = ax.plot(v1x,v1y,v1z,color='black')
        #From shoulder to hand
        line2 = ax.plot(v2x,v2y,v2z,color='black')
        #From chest to foot
        line3 = ax.plot([p0[0],p0[0]],[p0[1],p0[1]],[p0[2],-10],color='black')
        StartAcq()
        ResumeTh()
        threadTimer.start()
        #threadKey.start()
        print 'Acquisition in process...'
        print 'Stop? (y)'
        while(1):
            plotLock.acquire()
            ax.collections[0].remove()
            ax.collections[0].remove()
            ax.collections[0].remove()
            ax.collections[0].remove()
            ax.collections[0].remove()
            ax.collections[0].remove()
            ax.axes.lines[0].remove()
            ax.axes.lines[0].remove()
            ax.axes.lines[0].remove()
            #Quivers at the hand
            qx = ax.quiver(p3[0],p3[1],p3[2],ux,vx,wx, length=quiverSize, pivot='tail', color='red')
            qy = ax.quiver(p3[0],p3[1],p3[2],uy,vy,wy, length=quiverSize, pivot='tail', color='green')
            qz = ax.quiver(p3[0],p3[1],p3[2],uz,vz,wz, length=quiverSize, pivot='tail', color='blue')
            #Points
            #Origin
            p0sct = ax.scatter(p0[0],p0[1],p0[2],color='black')
            #Shoulder
            p1sct = ax.scatter(p1[0],p1[1],p1[2],color='blue')
            #Hand
            p2sct = ax.scatter(p3[0],p3[1],p3[2],color='green')
            #Lines
            #From chest (origin) to shoulder
            line1 = ax.plot(v1x,v1y,v1z,color='black')
            #From shoulder to hand
            line2 = ax.plot(v2x,v2y,v2z,color='black')
            #From chest to foot
            line3 = ax.plot([p0[0],p0[0]],[p0[1],p0[1]],[p0[2],-10],color='black')
            plotLock.release()
            plt.draw()
            plt.pause(0.00001)
            #strkey = raw_input()
            if(strkey == 'y'):
                PauseTh()
                PauseAcq()
                print 'Lost packages:', lostPackages
                print 'Saving file...'
                Save()
                del dataList[:]
                del timeList[:]
                flagTimer = False
                flagKey = False
                break
    elif(strkey == '2'):
        #serialPort.write([0x53,0x03])
        serialPort.write([0x53,0x0b])
        #serialPort.write(0x05)

    elif(strkey == '3'):
        KillTh()
        serialPort.close()
        break
