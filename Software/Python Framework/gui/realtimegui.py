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
# Decription: Adding real-time control of a single joint with MPU6050
#------------------------------------------------------------------------------
import os, sys
sys.path.append('../')
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4.uic import loadUiType
import matplotlib
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from mpl_toolkits.mplot3d import axes3d
from mpl_toolkits.mplot3d import Axes3D
from matplotlib.backends.backend_qt4agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.backends.backend_qt4agg import NavigationToolbar2QT as NavigationToolbar
import time
from threading import Thread, Lock, Timer
from Queue import Queue
from datetime import datetime
from sets import Set
import numpy as np
from skeleton import *
import geometry
import quaternion as quat
from threadhandler import ThreadHandler
from mpu6050handler import *
import math
#------------------------------------------------------------------------------
Ui_MainWindow, QMainWindow = loadUiType('mainwindow.ui')
#------------------------------------------------------------------------------
class Main(QMainWindow, Ui_MainWindow):
	def __init__(self):
		super(Main,self).__init__()
		self.setupUi(self)

		#mpu6050
		self.imu = MPU6050('/dev/ttyACM0')
		self.imu.open()

		#Limb positions
		torso = np.array([0,0,0])
		phead = np.array([0,0,2])
		pwaist = np.array([0,0,-4])
		#Upper limbs
		#Right
		rsh = np.array([3,0,0])
		rel = np.array([5,0,0])
		rwr = np.array([7,0,0])
		#Left
		lsh = np.array([-3,0,0])
		lel = np.array([-5,0,0])
		lwr = np.array([-7,0,0])
		#Lower limbs
		#Right
		rhi = np.array([1.5,0,-4])
		rkn = np.array([4.5,0,-4])
		ran = np.array([7.5,0,-4])
		#Left
		lhi = np.array([-1.5,0,-4])
		lkn = np.array([-4.5,0,-4])
		lan = np.array([-7.5,0,-4])

		self.skeleton = Skeleton()
		self.skeleton.add(BodyJoints.UNILAT,BodyJoints.HEAD,phead)
		self.skeleton.add(BodyJoints.RIGHT,BodyJoints.SHOULDER,rsh)
		self.skeleton.add(BodyJoints.RIGHT,BodyJoints.ELBOW,rel)
		self.skeleton.add(BodyJoints.RIGHT,BodyJoints.WRIST,rwr)
		self.skeleton.add(BodyJoints.LEFT,BodyJoints.SHOULDER,lsh)
		self.skeleton.add(BodyJoints.LEFT,BodyJoints.ELBOW,lel)
		self.skeleton.add(BodyJoints.LEFT,BodyJoints.WRIST,lwr)

		self.skeleton.add(BodyJoints.UNILAT,BodyJoints.WAIST,pwaist)
		self.skeleton.add(BodyJoints.RIGHT,BodyJoints.HIP,rhi)
		self.skeleton.add(BodyJoints.RIGHT,BodyJoints.KNEE,rkn)
		self.skeleton.add(BodyJoints.RIGHT,BodyJoints.ANKLE,ran)
		self.skeleton.add(BodyJoints.LEFT,BodyJoints.HIP,lhi)
		self.skeleton.add(BodyJoints.LEFT,BodyJoints.KNEE,lkn)
		self.skeleton.add(BodyJoints.LEFT,BodyJoints.ANKLE,lan)

		#self.skeleton.getJoint(BodyJoints.UNILAT,BodyJoints.HEAD).setQuaternion([0.707,0.707,0,0])

		for segs in self.skeleton.segments:
			self.cbJointNames.addItem(segs)

		self.connect(self.cbJointNames,SIGNAL('currentIndexChanged(int)'),self.cbChanged)
		self.slPhi.valueChanged.connect(self.degChanged)
		self.slTheta.valueChanged.connect(self.degChanged)
		self.slPsi.valueChanged.connect(self.degChanged)
		self.btnRotate.clicked.connect(self.doRotate)
		self.btnReset.clicked.connect(self.doReset)
		self.btnStanding.clicked.connect(self.doStanding)
		self.btnSentado.clicked.connect(self.doSentado)
		self.btnAnimation.clicked.connect(self.doAnimation)
		self.btnStop.clicked.connect(self.doStop)
		#Queue for data acquisition from RF sensors
		self.dataQueue = Queue()

		#Create the skeleton chart
		#matplotlib
		self.addmpl()

	def addmpl(self):
		#Plot parameters
		axesLim = [-10,10]
		self.fig = plt.figure()
		self.ax = self.fig.gca(projection='3d')
		self.ax.set_xlim(axesLim)
		self.ax.set_ylim(axesLim)
		self.ax.set_zlim(axesLim)
		self.ax.set_title('Body model')
		self.ax.set_xlabel('x-axis')
		self.ax.set_ylabel('y-axis')
		self.ax.set_zlabel('z-axis')

		for item in [self.fig, self.ax]:
			item.patch.set_visible(False)

		self.canvas = FigureCanvas(self.fig)
		self.mbly.addWidget(self.canvas)
		self.canvas.draw()
		Axes3D.mouse_init(self.ax)
		#self.toolbar = NavigationToolbar(self.canvas,self,coordinates=True)
		#self.mbly.addWidget(self.toolbar)

	def plot(self):
		#self.ax.cla()
		#self.ax.clear()
		#Clearing the chart
		if self.ax.collections:
			for i in range(len(self.ax.collections)):
				self.ax.collections[0].remove()
			for i in range(len(self.ax.lines)):
				self.ax.lines[0].remove()

		to = self.skeleton.getJoint(BodyJoints.UNILAT,BodyJoints.TORSO).position
		he = self.skeleton.getJoint(BodyJoints.UNILAT,BodyJoints.HEAD).position
		sh = self.skeleton.getJoint(BodyJoints.RIGHT,BodyJoints.SHOULDER).position
		el = self.skeleton.getJoint(BodyJoints.RIGHT,BodyJoints.ELBOW).position
		wr = self.skeleton.getJoint(BodyJoints.RIGHT,BodyJoints.WRIST).position
		an = self.skeleton.getJoint(BodyJoints.RIGHT,BodyJoints.ANKLE).position

		#quiver for orientation of the torso
		rot = quat.toRotMat(self.skeleton.getJoint(BodyJoints.UNILAT,BodyJoints.TORSO).quaternion)
		tx = np.array([rot[0,0],rot[0,1],rot[0,2]])
		ty = np.array([rot[1,0],rot[1,1],rot[1,2]])
		tz = np.array([rot[2,0],rot[2,1],rot[2,2]])

		#quiver for orientation of the head
		rot = quat.toRotMat(self.skeleton.getJoint(BodyJoints.UNILAT,BodyJoints.HEAD).quaternion)
		kx = np.array([rot[0,0],rot[0,1],rot[0,2]])
		ky = np.array([rot[1,0],rot[1,1],rot[1,2]])
		kz = np.array([rot[2,0],rot[2,1],rot[2,2]])

		#quiver for orientation of the elbow
		rot = quat.toRotMat(self.skeleton.getJoint(BodyJoints.RIGHT,BodyJoints.ELBOW).quaternion)
		ux = np.array([rot[0,0],rot[0,1],rot[0,2]])
		uy = np.array([rot[1,0],rot[1,1],rot[1,2]])
		uz = np.array([rot[2,0],rot[2,1],rot[2,2]])

		#quiver for orientation of the wrist
		rot = quat.toRotMat(self.skeleton.getJoint(BodyJoints.RIGHT,BodyJoints.WRIST).quaternion)
		wx = np.array([rot[0,0],rot[0,1],rot[0,2]])
		wy = np.array([rot[1,0],rot[1,1],rot[1,2]])
		wz = np.array([rot[2,0],rot[2,1],rot[2,2]])

		#quiver for orientation of the wrist
		rot = quat.toRotMat(self.skeleton.getJoint(BodyJoints.RIGHT,BodyJoints.ANKLE).quaternion)
		ax = np.array([rot[0,0],rot[0,1],rot[0,2]])
		ay = np.array([rot[1,0],rot[1,1],rot[1,2]])
		az = np.array([rot[2,0],rot[2,1],rot[2,2]])


		quiverSize = 1.5
		#Orientation of elbow sensor
		self.ax.quiver(el[0],el[1],el[2],ux[0],ux[1],ux[2],length=quiverSize, pivot='tail', color='red')
		self.ax.quiver(el[0],el[1],el[2],uy[0],uy[1],uy[2],length=quiverSize, pivot='tail', color='green')
		self.ax.quiver(el[0],el[1],el[2],uz[0],uz[1],uz[2],length=quiverSize, pivot='tail', color='blue')
		#Orientation of wrist sensor
		self.ax.quiver(wr[0],wr[1],wr[2],wx[0],wx[1],wx[2],length=quiverSize, pivot='tail', color='red')
		self.ax.quiver(wr[0],wr[1],wr[2],wy[0],wy[1],wy[2],length=quiverSize, pivot='tail', color='green')
		self.ax.quiver(wr[0],wr[1],wr[2],wz[0],wz[1],wz[2],length=quiverSize, pivot='tail', color='blue')
		#Orientation of head sensor
		self.ax.quiver(he[0],he[1],he[2],kx[0],kx[1],kx[2],length=quiverSize, pivot='tail', color='red')
		self.ax.quiver(he[0],he[1],he[2],ky[0],ky[1],ky[2],length=quiverSize, pivot='tail', color='green')
		self.ax.quiver(he[0],he[1],he[2],kz[0],kz[1],kz[2],length=quiverSize, pivot='tail', color='blue')
		#Orientation of torso sensor
		#self.ax.quiver(to[0],to[1],to[2],tx[0],tx[1],tx[2],length=quiverSize, pivot='tail', color='red')
		#self.ax.quiver(to[0],to[1],to[2],ty[0],ty[1],ty[2],length=quiverSize, pivot='tail', color='green')
		#self.ax.quiver(to[0],to[1],to[2],tz[0],tz[1],tz[2],length=quiverSize, pivot='tail', color='blue')
		##Orientation of right ankle sensor
		self.ax.quiver(an[0],an[1],an[2],ax[0],ay[0],az[0],length=quiverSize, pivot='tail', color='red')
		self.ax.quiver(an[0],an[1],an[2],ax[1],ay[1],az[1],length=quiverSize, pivot='tail', color='green')
		self.ax.quiver(an[0],an[1],an[2],ax[2],ay[2],az[2],length=quiverSize, pivot='tail', color='blue')

		points = self.skeleton.bodyPoints()
		links,colors = self.skeleton.bodyLinks()
		for i in range(len(self.skeleton.segments)):
			self.ax.scatter(points[0][i],points[1][i],points[2][i],c='gray',s=60)
		for i in range(len(self.skeleton.segments)-1):
			self.ax.plot(links[0][i],links[1][i],links[2][i],c=colors[i],linewidth=2)
		self.canvas.draw()

	def doRotate(self):
		jointName = self.cbJointNames.itemText(self.cbJointNames.currentIndex())
		parent = self.skeleton.getJoint(None,None,str(jointName))
		q1 = quat.fromEuler(self.slPhi.value(),self.slTheta.value(),self.slPsi.value())
		self.updateQuaternions(parent,q1)
		self.skeleton.rotate()
		self.plot()

	def updateQuaternions(self,_joint,_quat):
		for i in range(3):
			node = _joint.links[i]
			while node is not None:
				if node.name in self.skeleton.segments:
					print node.name
					auxq = quat.product(quat.conjugate(_joint.quaternion),node.quaternion)
					node.setQuaternion(quat.product(_quat,auxq))
					node = node.links[i]
				else:
					break

		_joint.setQuaternion(_quat)
		if(_joint.name == 'right_ankle'):
			print 'entrei malandro'
			print _joint.rotquaternion
		print _joint.name, _joint.quaternion


	def doReset(self):
		for seg in self.skeleton.segments:
			joint = self.skeleton.getJoint(None,None,seg)
			joint.setQuaternion(np.array([1,0,0,0]))

		self.slPhi.setValue(0)
		self.slTheta.setValue(0)
		self.slPsi.setValue(0)

		self.skeleton.rotate()
		self.plot()

	def doStanding(self):
		self.doReset()
		qr = [math.sqrt(2)/2,0,math.sqrt(2)/2,0]
		ql = [math.sqrt(2)/2,0,-math.sqrt(2)/2,0]
		rel = self.skeleton.getJoint(BodyJoints.RIGHT,BodyJoints.ELBOW)
		lel = self.skeleton.getJoint(BodyJoints.LEFT,BodyJoints.ELBOW)
		rkn = self.skeleton.getJoint(BodyJoints.RIGHT,BodyJoints.KNEE)
		lkn = self.skeleton.getJoint(BodyJoints.LEFT,BodyJoints.KNEE)
		self.updateQuaternions(rel,qr)
		self.updateQuaternions(lel,ql)
		self.updateQuaternions(rkn,qr)
		self.updateQuaternions(lkn,ql)
		self.skeleton.rotate()
		self.plot()

	def doSentado(self):
		self.doStanding()
		#qright_knee = [0.707,0,0,0.707]
		#qright_ankle = [0.707,0,0,0.707]
		#rkn = self.skeleton.getJoint(BodyJoints.RIGHT,BodyJoints.KNEE)
		#ran = self.skeleton.getJoint(BodyJoints.RIGHT,BodyJoints.ANKLE)
		#self.updateQuaternions(rkn,qright_knee)
		#self.updateQuaternions(ran,qright_ankle)
		#self.skeleton.rotate()
		#self.plot()
		#print 'entrando nos motherfucker'
		#print ran.rotquaternion
		#print ran.quaternion
		qright_knee = [0.5,0,-0.5,0] #quat.fromEuler(-90,-90,0)
		qleft_knee = [0.5,0,0.5,0] #quat.fromEuler(90,90,180)
		#qright_ankle = [0.707,0.707,0,0] #quat.fromEuler(0,-90,0)
		#qleft_ankle = [0.707,0.707,0,0] #quat.fromEuler(180,90,180)
		ran = self.skeleton.getJoint(BodyJoints.RIGHT,BodyJoints.ANKLE)
		lan = self.skeleton.getJoint(BodyJoints.LEFT,BodyJoints.ANKLE)
		rkn = self.skeleton.getJoint(BodyJoints.RIGHT,BodyJoints.KNEE)
		lkn = self.skeleton.getJoint(BodyJoints.LEFT,BodyJoints.KNEE)
		self.updateQuaternions(rkn,qright_knee)
		self.updateQuaternions(lkn,qleft_knee)
		#self.updateQuaternions(ran,qright_ankle)
		#self.updateQuaternions(lan,qleft_ankle)
		self.skeleton.rotate()
		self.plot()

	def cbChanged(self,idx):
		jointName = self.cbJointNames.itemText(self.cbJointNames.currentIndex())
		joint = self.skeleton.getJoint(None,None,str(jointName))
		euler = quat.toEuler(joint.quaternion)
		self.slPhi.setValue(int(np.round(euler[0])))
		self.slTheta.setValue(int(np.round(euler[1])))
		self.slPsi.setValue(int(np.round(euler[2])))

	def degChanged(self):
		self.lbPhi.setText('Phi: ' + str(self.slPhi.value()))
		self.lbTheta.setText('Theta: ' + str(self.slTheta.value()))
		self.lbPsi.setText('Psi: ' + str(self.slPsi.value()))

	#Triggered when the button "Animation" is clicked
	def doAnimation(self):
		self.imu.start()

		self.plotcounter = 0
		#Thread that handles the data acquisition
		self.dataProc = ThreadHandler(self.runAnimation)
		#Thread that handles the chart
		self.plotTh = ThreadHandler(self.runPlot)

		#Start the threads
		self.imu.acqThread.start()
		self.dataProc.start()
		self.plotTh.start()

		#self.plotTh.start()
		self.drawLock = Lock()

	#Triggered when the "Stop" button is clicked
	def doStop(self):
		self.imu.pause()
		self.imu.stop()
		time.sleep(1)
		#Kill the threads
		self.imu.acqThread.Kill()
		self.dataProc.Kill()
		self.plotTh.Kill()

	#Function that updates the chart
	def runPlot(self):
		if self.plotcounter == 100:
			self.plot()
			self.plotcounter = 0
		else:
			self.plotcounter += 1

	#Function that handles the data acquisition
	def runAnimation(self):
		#self.imu.acqThread.lock.acquire()
		if self.imu.dataQueue.qsize() > 0:
			n = self.imu.dataQueue.qsize()
			for i in range(n):
				data = self.imu.dataQueue.get()
				print 'qntsensor: %d' % (len(data)/4)
				if len(data) >= 4:
					quat = data[0:4]
					print '[%.2f,%.2f,%.2f,%.2f]'% (quat[0],quat[1],quat[2],quat[3])
					joint = self.skeleton.getJoint(BodyJoints.RIGHT,BodyJoints.WRIST)
					joint.setQuaternion(quat)
				if len(data) >= 8:
					quat = data[4:8]
					print '[%.2f,%.2f,%.2f,%.2f]'% (quat[0],quat[1],quat[2],quat[3])
					joint = self.skeleton.getJoint(BodyJoints.RIGHT,BodyJoints.ELBOW)
					joint.setQuaternion(quat)
				if len(data) >= 12:
					quat = data[8:12]
					print '[%.2f,%.2f,%.2f,%.2f]'% (quat[0],quat[1],quat[2],quat[3])
					joint = self.skeleton.getJoint(BodyJoints.UNILAT,BodyJoints.TORSO)
					joint.setQuaternion(quat)
				#self.updateQuaternions(joint,quat)
		self.skeleton.rotate()
		#self.plot()
		#self.imu.acqThread.lock.release()
		#time.sleep(0.005) #sampling frequency 100 Hz

if __name__ == '__main__':
	import sys
	from PyQt4 import QtGui

	app = QtGui.QApplication(sys.argv)
	main = Main()
	main.plot()
	main.show()
	sys.exit(app.exec_())
