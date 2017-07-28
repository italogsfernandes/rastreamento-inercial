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
import webbrowser
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
from time import sleep
from shutil import copyfile
import serial.tools.list_ports as serial_tools
from datetime import datetime
#------------------------------------------------------------------------------
Ui_MainWindow, QMainWindow = loadUiType('mainwindow.ui')
#------------------------------------------------------------------------------
class Main(QMainWindow, Ui_MainWindow):
	def __init__(self):
		super(Main,self).__init__()
		self.setupUi(self)

		self.plotLock = Lock()

		self.skeleton = Skeleton()
		self.shdist = 20.0
		self.eldist = 28.0
		self.wrdist = 23.0
		self.criarSkeleton(40.0,28.0,23.0)

		self.cbJointNames.addItem('InterSegmentos-Right')
		self.lbPhi.setText('Inclinacao do Torso:')
		self.lbTheta.setText('Ombro-Braco: ')
		self.lbPsi.setText('Braco-Antebraco: ')
		self.interseginuse = True
		self.cbJointNames.addItem('InterSegmentos-Left')

		for segs in self.skeleton.segments:
			self.cbJointNames.addItem(segs)

		for serial_port in serial_tools.comports():
			self.cbSerialPort.addItem(serial_port.device)

		if len(serial_tools.comports()) == 0:
			self.show_error_msg("Nenhuma porta Serial Disponivel")
			self.cbSerialPort.setEnabled(False)
			self.btnAnimation.setEnabled(False)
		else:
			self.cbSerialPort.setCurrentIndex(len(serial_tools.comports())-1)
			self.cbSerialChanged(len(serial_tools.comports())-1)

		self.connect(self.cbJointNames,SIGNAL('currentIndexChanged(int)'),self.cbChanged)
		self.connect(self.cbSerialPort,SIGNAL('currentIndexChanged(int)'),self.cbSerialChanged)
		self.connect(self.cbMarcas,SIGNAL('currentIndexChanged(int)'),self.cbMarcasChanged)
		self.slPhi.valueChanged.connect(self.degChanged)
		self.slTheta.valueChanged.connect(self.degChanged)
		self.slPsi.valueChanged.connect(self.degChanged)
		self.sliderColeta.valueChanged.connect(self.updateRotfromColeta)
		self.btnRotate.clicked.connect(self.doRotate)
		self.btnReset.clicked.connect(self.doReset)
		self.btnMedidas.clicked.connect(self.doMedidas)
		self.arqColeta = None
		self.arqColetaBruta = None
		self.marcanumero = 0
		self.btnMarcar.setText("Marcar " + str(self.marcanumero + 1))

		self.actionAbrir_Coleta.triggered.connect(self.doOpenColeta)
		self.actionFechar_Coleta.triggered.connect(self.doCloseColeta)
		self.actionHelp.triggered.connect(self.doHelp)
		self.actionMedidas_da_Pessoa.triggered.connect(self.doMenuRotacao)

		self.actionAbrir_Postura.triggered.connect(self.doOpenPosture)
		self.actionSalvar_Postura.triggered.connect(self.doSavePosture)
		self.actionMenu_de_Rota_o.triggered.connect(self.doMenuRotacao)

		self.groupRotation.setVisible(False)

		self.actionPosi_o.triggered.connect(self.doGraphPosition)
		self.actionQuaternion.triggered.connect(self.doGraphQuaternion)
		self.action_rea_do_Tri_ngulo.triggered.connect(self.doGraphTriangulo)

		self.btnAnimation.clicked.connect(self.doAnimation)
		self.btnColeta.setText("Iniciar Coleta")
		self.btnSetOffset.setText("Set Offset")
		self.btnSetOffset.clicked.connect(self.doOffset)
		self.btnColeta.clicked.connect(self.doColeta)
		self.btnColeta.setVisible(False)
		self.statusColetaRunning = False
		self.offsetpending = False

		self.btnMarcar.clicked.connect(self.doMarcar)
		self.btnMarcar.setVisible(False)
		self.marcacaoPending = False
		self.cbMarcas.setVisible(False)
		self.groupDadosColeta.setVisible(False)
		self.sliderColeta.setVisible(False)
		#Queue for data acquisition from RF sensors
		self.dataQueue = Queue()

		#Create the skeleton chart
		#matplotlib
		self.addmpl()

		self.linereg = [3.925e-5, 3.0504e-5, 3.197e-5, 3.774e-5, 4.7458e-5]
		self.biasreg = [-0.00664, 0.0179, -0.0163, -0.00613, -0.00614]
		self.time = 0.0
		self.sampfreq = 20.0

	def criarSkeleton(self,distShoulders,distShouderElbow,distElbowWrist):
			self.shdist = distShoulders/2.0
			self.eldist = distShouderElbow
			self.wrdist = distElbowWrist

			#Limb positions
			torso = np.array([0,0,0])
			phead = np.array([0,0,self.shdist/2.0])
			pwaist = np.array([0,0,-self.shdist*1.75])
			#Upper limbs
			#Right
			rsh = np.array([self.shdist,0,0])
			rel = np.array([self.shdist+self.eldist,0,0])
			rwr = np.array([rel[0]+self.wrdist,0,0])
			#Left
			lsh = np.array([-rsh[0],0,0])
			lel = np.array([lsh[0]-self.eldist,0,0])
			lwr = np.array([lel[0]-self.wrdist,0,0])

			#Lower limbs
			#Right
			rhi = np.array([self.shdist*0.66,0,pwaist[2]])
			rkn = np.array([rhi[0],self.eldist,pwaist[2]])
			ran = np.array([rkn[0],rkn[1],pwaist[2]-self.eldist])
			#Left
			lhi = np.array([-rhi[0],0,pwaist[2]])
			lkn = np.array([-rkn[0],rkn[1],pwaist[2]])
			lan = np.array([-ran[0],ran[1],ran[2]])

			#Remove all existing joints
			self.skeleton.remove(BodyJoints.UNILAT,BodyJoints.HEAD)
			self.skeleton.remove(BodyJoints.RIGHT,BodyJoints.SHOULDER)
			self.skeleton.remove(BodyJoints.RIGHT,BodyJoints.ELBOW)
			self.skeleton.remove(BodyJoints.RIGHT,BodyJoints.WRIST)
			self.skeleton.remove(BodyJoints.LEFT,BodyJoints.SHOULDER)
			self.skeleton.remove(BodyJoints.LEFT,BodyJoints.ELBOW)
			self.skeleton.remove(BodyJoints.LEFT,BodyJoints.WRIST)

			self.skeleton.remove(BodyJoints.UNILAT,BodyJoints.WAIST)
			self.skeleton.remove(BodyJoints.RIGHT,BodyJoints.HIP)
			self.skeleton.remove(BodyJoints.RIGHT,BodyJoints.KNEE)
			self.skeleton.remove(BodyJoints.RIGHT,BodyJoints.ANKLE)
			self.skeleton.remove(BodyJoints.LEFT,BodyJoints.HIP)
			self.skeleton.remove(BodyJoints.LEFT,BodyJoints.KNEE)
			self.skeleton.remove(BodyJoints.LEFT,BodyJoints.ANKLE)

			#Then add the new joints
			self.skeleton.add(BodyJoints.UNILAT,BodyJoints.HEAD,phead)
			self.skeleton.add(BodyJoints.RIGHT,BodyJoints.SHOULDER,rsh)
			self.skeleton.add(BodyJoints.RIGHT,BodyJoints.ELBOW,rel)
			self.skeleton.add(BodyJoints.RIGHT,BodyJoints.WRIST,rwr)
			self.skeleton.add(BodyJoints.LEFT,BodyJoints.SHOULDER,lsh)
			self.skeleton.add(BodyJoints.LEFT,BodyJoints.ELBOW,lel)
			self.skeleton.add(BodyJoints.LEFT,BodyJoints.WRIST,lwr)

			self.skeleton.add(BodyJoints.UNILAT,BodyJoints.WAIST,pwaist)
			#self.skeleton.add(BodyJoints.RIGHT,BodyJoints.HIP,rhi)
			#self.skeleton.add(BodyJoints.RIGHT,BodyJoints.KNEE,rkn)
			#self.skeleton.add(BodyJoints.RIGHT,BodyJoints.ANKLE,ran)
			#self.skeleton.add(BodyJoints.LEFT,BodyJoints.HIP,lhi)
			#self.skeleton.add(BodyJoints.LEFT,BodyJoints.KNEE,lkn)
			#self.skeleton.add(BodyJoints.LEFT,BodyJoints.ANKLE,lan)



	def doHelp(self):
		webbrowser.open('http://www.github.com/italogfernandes/rastreamento-inercial', autoraise=True)
		#webbrowser.open('http://www.gogle.com', autoraise=True)
		#webbrowser.open('http://lmgtfy.com/?q=Como+medir+angulos%3F', autoraise=True)

	def doOffset(self):
		self.offsetpending = True

	def doMedidas(self):
		shouldershoulder = float(self.editShoulderShoulder.text())
		shoulderelbow = float(self.editShoulderElbow.text())
		elbowwritst = float(self.editElbowWrist.text())
		self.criarSkeleton(shouldershoulder,shoulderelbow,elbowwritst)
		self.plot()
		self.doMenuRotacao()

	def closeEvent(self,event):
		if self.imu.acqThread.isAlive:
			self.imu.acqThread.kill()
		if self.plotTh.isAlive:
			self.plotTh.kill()
		if self.dataProc.isAlive:
			self.dataProc.kill()
		print 'closing...'

	def addmpl(self):
		#Plot parameters
		axesLim = [-100,100]
		self.fig = plt.figure()
		self.ax = self.fig.gca(projection='3d')
		self.ax.set_xlim(axesLim)
		self.ax.set_ylim(axesLim)
		self.ax.set_zlim(axesLim)
		self.ax.set_title('Body model')
		self.ax.set_xlabel('x-axis (cm)')
		self.ax.set_ylabel('y-axis (cm)')
		self.ax.set_zlabel('z-axis (cm)')

		for item in [self.fig, self.ax]:
			item.patch.set_visible(False)

		self.resize(800,600)
		self.canvas = FigureCanvas(self.fig)
		self.horizontalLayout_3.addWidget(self.canvas)
		self.canvas.draw()
		Axes3D.mouse_init(self.ax)
		self.toolbar = NavigationToolbar(self.canvas,self,coordinates=True)
		self.verticalLayout_6.addWidget(self.toolbar)

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
		lel = self.skeleton.getJoint(BodyJoints.LEFT,BodyJoints.ELBOW).position
		lwr = self.skeleton.getJoint(BodyJoints.LEFT,BodyJoints.WRIST).position
		#an = self.skeleton.getJoint(BodyJoints.RIGHT,BodyJoints.ANKLE).position

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

		#quiver for orientation of the left elbow
		rot = quat.toRotMat(self.skeleton.getJoint(BodyJoints.LEFT,BodyJoints.ELBOW).quaternion)
		lux = np.array([rot[0,0],rot[0,1],rot[0,2]])
		luy = np.array([rot[1,0],rot[1,1],rot[1,2]])
		luz = np.array([rot[2,0],rot[2,1],rot[2,2]])

		#quiver for orientation of the left wrist
		rot = quat.toRotMat(self.skeleton.getJoint(BodyJoints.LEFT,BodyJoints.WRIST).quaternion)
		lwx = np.array([rot[0,0],rot[0,1],rot[0,2]])
		lwy = np.array([rot[1,0],rot[1,1],rot[1,2]])
		lwz = np.array([rot[2,0],rot[2,1],rot[2,2]])


		#quiver for orientation of the ankle
		#rot = quat.toRotMat(self.skeleton.getJoint(BodyJoints.RIGHT,BodyJoints.ANKLE).quaternion)
		#ax = np.array([rot[0,0],rot[0,1],rot[0,2]])
		#ay = np.array([rot[1,0],rot[1,1],rot[1,2]])
		#az = np.array([rot[2,0],rot[2,1],rot[2,2]])


		quiverSize = 7.5
		#Orientation of elbow sensor
		self.ax.quiver(el[0],el[1],el[2],ux[0],ux[1],ux[2],length=quiverSize, pivot='tail', color='red')
		self.ax.quiver(el[0],el[1],el[2],uy[0],uy[1],uy[2],length=quiverSize, pivot='tail', color='green')
		self.ax.quiver(el[0],el[1],el[2],uz[0],uz[1],uz[2],length=quiverSize, pivot='tail', color='blue')
		#Orientation of wrist sensor
		self.ax.quiver(wr[0],wr[1],wr[2],wx[0],wx[1],wx[2],length=quiverSize, pivot='tail', color='red')
		self.ax.quiver(wr[0],wr[1],wr[2],wy[0],wy[1],wy[2],length=quiverSize, pivot='tail', color='green')
		self.ax.quiver(wr[0],wr[1],wr[2],wz[0],wz[1],wz[2],length=quiverSize, pivot='tail', color='blue')
		#Orientation of torso sensor
		self.ax.quiver(to[0],to[1],to[2],tx[0],tx[1],tx[2],length=quiverSize, pivot='tail', color='red')
		self.ax.quiver(to[0],to[1],to[2],ty[0],ty[1],ty[2],length=quiverSize, pivot='tail', color='green')
		self.ax.quiver(to[0],to[1],to[2],tz[0],tz[1],tz[2],length=quiverSize, pivot='tail', color='blue')
		#Orientation of left elbow sensor
		self.ax.quiver(lel[0],lel[1],lel[2],lux[0],lux[1],lux[2],length=quiverSize, pivot='tail', color='red')
		self.ax.quiver(lel[0],lel[1],lel[2],luy[0],luy[1],luy[2],length=quiverSize, pivot='tail', color='green')
		self.ax.quiver(lel[0],lel[1],lel[2],luz[0],luz[1],luz[2],length=quiverSize, pivot='tail', color='blue')
		#Orientation of left wrist sensor
		self.ax.quiver(lwr[0],lwr[1],lwr[2],lwx[0],lwx[1],lwx[2],length=quiverSize, pivot='tail', color='red')
		self.ax.quiver(lwr[0],lwr[1],lwr[2],lwy[0],lwy[1],lwy[2],length=quiverSize, pivot='tail', color='green')
		self.ax.quiver(lwr[0],lwr[1],lwr[2],lwz[0],lwz[1],lwz[2],length=quiverSize, pivot='tail', color='blue')

		#Orientation of head sensor
		#self.ax.quiver(he[0],he[1],he[2],kx[0],kx[1],kx[2],length=quiverSize, pivot='tail', color='red')
		#self.ax.quiver(he[0],he[1],he[2],ky[0],ky[1],ky[2],length=quiverSize, pivot='tail', color='green')
		#self.ax.quiver(he[0],he[1],he[2],kz[0],kz[1],kz[2],length=quiverSize, pivot='tail', color='blue')
		##Orientation of right ankle sensor
		#self.ax.quiver(an[0],an[1],an[2],ax[0],ay[0],az[0],length=quiverSize, pivot='tail', color='red')
		#self.ax.quiver(an[0],an[1],an[2],ax[1],ay[1],az[1],length=quiverSize, pivot='tail', color='green')
		#self.ax.quiver(an[0],an[1],an[2],ax[2],ay[2],az[2],length=quiverSize, pivot='tail', color='blue')

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
		print "Do rotate: "
		print q1
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


	def doGraphTriangulo(self):
		self.show_error_msg("Nao implementada")

	def doGraphQuaternion(self):
		self.show_error_msg("Nao implementada")

	def doGraphPosition(self):
		self.show_error_msg("Nao implementada")


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
		self.updateQuaternions(rel,qr)
		self.updateQuaternions(lel,ql)
		self.skeleton.rotate()
		self.plot()
		jointName = self.cbJointNames.itemText(self.cbJointNames.currentIndex())
		joint = self.skeleton.getJoint(None,None,str(jointName))
		euler = quat.toEuler(joint.quaternion)
		self.slPhi.setValue(int(np.round(euler[0])))
		self.slTheta.setValue(int(np.round(euler[1])))
		self.slPsi.setValue(int(np.round(euler[2])))

	def doOpenPosture(self):
		self.doReset()
		dlg = QtGui.QFileDialog( self )
		dlg.setWindowTitle( 'Secione a posição que deseja abrir.' )
		dlg.setViewMode( QtGui.QFileDialog.Detail )
		dlg.setNameFilters( [self.tr('Arquivos de Posição (*.position)'), self.tr('All Files (*)')] )
		dlg.setDefaultSuffix( '.position' )
		file_name = dlg.getOpenFileName(self,'Open File')
		print file_name
		file_to_open = open(file_name, 'r')
		file_to_open.readline() == "joint_name\t[position]\t[origin]\t[quaternion]\n"
		linhas_read = file_to_open.readlines()
		for line in linhas_read:
			joint_values = line.split('\t')
			joint_name = joint_values[0]
			joint_position = [float(v) for v in joint_values[1].split(',')]
			joint_origin = [float(v) for v in joint_values[2].split(',')]
			joint_quaternion =  [float(v) for v in joint_values[3].split(',')]
			joint = self.skeleton.getJoint(None,None,joint_name)
			print()
			print(str(joint_name) + '\t'+ str(joint_quaternion))
			self.updateQuaternions(joint,joint_quaternion)
			self.skeleton.rotate()


		print("Fim da leitura" + "*"*50)
		self.updateSlideBars()
		self.plot()
		file_to_open.close()

	def doMenuRotacao(self):
		self.groupRotation.setVisible(not self.groupRotation.isVisible())
		self.groupMedidas.setVisible(not self.groupMedidas.isVisible())

	def doSavePosture(self):
		dlg = QtGui.QFileDialog( self )
		dlg.setWindowTitle( 'Secione o local para salvar a posição.' )
		dlg.setViewMode( QtGui.QFileDialog.Detail )
		dlg.setNameFilters( [self.tr('Arquivos de Posição (*.position)'), self.tr('All Files (*)')] )
		dlg.setDefaultSuffix( '.position' )
		file_name = dlg.getSaveFileName(self,'Save File')
		if ".position" not in file_name:
			file_name = file_name + ".position"
		print "*"*len(file_name)
		print file_name
		#Montar arquivo de saida
		joints_to_save = []
		joints_to_save.append(self.skeleton.getJoint(BodyJoints.UNILAT,BodyJoints.HEAD))
		joints_to_save.append(self.skeleton.getJoint(BodyJoints.UNILAT,BodyJoints.TORSO))
		joints_to_save.append(self.skeleton.getJoint(BodyJoints.UNILAT,BodyJoints.WAIST))
		joints_to_save.append(self.skeleton.getJoint(BodyJoints.RIGHT,BodyJoints.SHOULDER))
		joints_to_save.append(self.skeleton.getJoint(BodyJoints.RIGHT,BodyJoints.ELBOW))
		joints_to_save.append(self.skeleton.getJoint(BodyJoints.RIGHT,BodyJoints.WRIST))
		joints_to_save.append(self.skeleton.getJoint(BodyJoints.LEFT,BodyJoints.SHOULDER))
		joints_to_save.append(self.skeleton.getJoint(BodyJoints.LEFT,BodyJoints.ELBOW))
		joints_to_save.append(self.skeleton.getJoint(BodyJoints.LEFT,BodyJoints.WRIST))

		file_to_save = open(file_name, 'w')
		print("joint_name\t[position]\t[origin]\t[quaternion]\n")
		file_to_save.write("joint_name\t[position]\t[origin]\t[quaternion]\n")
		for _j in joints_to_save:
			joint_dados = "%s\t%.4f,%.4f,%.4f\t%.4f,%.4f,%.4f\t%.6f,%.6f,%.6f,%.6f\n" % (_j.name,
				 _j.position[0],  _j.position[1],  _j.position[2],
				 _j.origin[0], _j.origin[1], _j.origin[2],
				 _j.quaternion[0], _j.quaternion[1], _j.quaternion[2],_j.quaternion[3])
			#joint_dados = str(_j.name) + "\t" + str(_j.position) + "\t" + str(_j.origin) + "\t" + str(_j.quaternion) + "\n"
			print(joint_dados)
			file_to_save.write(joint_dados)
		file_to_save.close()
		print "*"*len(file_name)

	def doMarcar(self):
		if self.statusColetaRunning:
			self.marcanumero = self.marcanumero + 1
			self.marcacaoPending = True
			self.btnMarcar.setText("Marcar " + str(self.marcanumero + 1))
		else:
			self.show_error_msg("É necessário estar em uma coleta para marcar algo.")

	def show_error_msg(self,msg_to_show):
		msg = QMessageBox()
		msg.setIcon(QMessageBox.Warning)
		msg.setText(msg_to_show)
		msg.setWindowTitle("Erro")
		retval = msg.exec_()

	def show_info_msg(self,msg_to_show):
		msg = QMessageBox()
		msg.setIcon(QMessageBox.Information)
		msg.setText(msg_to_show)
		msg.setWindowTitle("Mensagem Info")
		retval = msg.exec_()

	def doColeta(self):
		if self.statusColetaRunning:
			#Stops a coleta
			self.statusColetaRunning = False
			self.arqColeta.close()
			self.arqColetaBruta.close()
			msg = QMessageBox()
			msg.setIcon(QMessageBox.Question)
			msg.setText("Deseja salvar a coleta?")
			msg.setWindowTitle("Coleta Finalizada")
			msg.setStandardButtons(QMessageBox.Save | QMessageBox.Cancel)
			retval = msg.exec_()
   			self.saveColeta()
   			self.btnColeta.setText("Iniciar Coleta")
			self.btnMarcar.setVisible(False)
		elif not self.imu.acqThread.isAlive:
			self.show_error_msg("Coleta nao iniciada pois o sensor nao esta conectado.")
		else:
			#inicia a coleta
			self.arqColeta = open("arq_temporario.temp", 'w')
			self.arqColetaBruta = open("arq_temporario_bruto.temp", 'w')
			self.write_coleta_header(self.arqColeta)
			self.write_coleta_header(self.arqColetaBruta)
			self.btnColeta.setText("Finalizar Coleta")
			self.statusColetaRunning = True
			self.btnMarcar.setVisible(True)

	def write_coleta_header(self,file2write):
		file2write.write("#Coleta - " + str(datetime.now()) + "\n")
		file2write.write("#Dimencoes(shdist-eldist-wrdist):\t%.2f\t%.2f\t%.2f\n" % (self.shdist, self.eldist, self.wrdist))
		file2write.write("#Quaternions das seguintes Joints: \n")
		file2write.write("#q_w\tq_x\tq_y\tq_z\n")
		file2write.write("#RIGHT WRIST\tRIGHT ELBOW\tUNILAT TORSO\tLEFT ELBOW\tLEFT WRIST\n")
		file2write.write("#" + "*" * 36 + " DADOS: " + "*" * 36 + "\n")


	def saveColeta(self):
		dlg = QtGui.QFileDialog( self )
		dlg.setWindowTitle( 'Secione o local para salvar a coleta.' )
		dlg.setViewMode( QtGui.QFileDialog.Detail )
		dlg.setNameFilters( [self.tr('Text Files (*.coleta)'), self.tr('All Files (*)')] )
		dlg.setDefaultSuffix( '.coleta' )
		file_name = dlg.getSaveFileName(self,'Save File')
		if ".coleta" not in file_name:
			file_name = file_name + ".coleta"
		print "*"*len(file_name)
		print file_name
		copyfile("arq_temporario.temp", file_name)
		copyfile("arq_temporario_bruto.temp", file_name.replace('.coleta','_dadosbrutos.coleta'))
		print "*"*len(file_name)

	def doOpenColeta(self):
		self.doReset()
		dlg = QtGui.QFileDialog( self )
		dlg.setWindowTitle( 'Secione a coleta que deseja abrir.' )
		dlg.setViewMode( QtGui.QFileDialog.Detail )
		dlg.setNameFilters( [self.tr('Arquivos de Posição (*.coleta)'), self.tr('All Files (*)')] )
		dlg.setDefaultSuffix( '.coleta' )
		file_name = dlg.getOpenFileName(self,'Open File')
		print file_name
		self.opened_coleta_file_name = file_name
		self.marca_quats = []
		self.populateCBMarcas()

		self.cbMarcas.setVisible(True)
		self.groupDadosColeta.setVisible(True)
		self.sliderColeta.setVisible(True)
		print "TODO"

	def create_file_coleta(self):
		joints_to_save = []
		joints_to_save.append(self.skeleton.getJoint(BodyJoints.RIGHT,BodyJoints.WRIST))
		joints_to_save.append(self.skeleton.getJoint(BodyJoints.RIGHT,BodyJoints.ELBOW))
		joints_to_save.append(self.skeleton.getJoint(BodyJoints.UNILAT,BodyJoints.TORSO))
		joints_to_save.append(self.skeleton.getJoint(BodyJoints.LEFT,BodyJoints.WRIST))
		joints_to_save.append(self.skeleton.getJoint(BodyJoints.LEFT,BodyJoints.ELBOW))
		file_to_save = open('coleta_file.temp', 'w')
		file_to_save.write("MARCACAO\n")
		file_to_save.write("\t\tPosicao\t\t\tEulerAngles\n")
		file_to_save.write("joint_name\tX\tY\tZ\tX\tY\tZ\n")
		for _j in joints_to_save:
			j_euler = quaternion.toEuler(_j.quaternion)
			joint_dados = ("%s\t%.4f\t%.4f\t%.4f\t%.4f\t%.4f\t%.4f\n")% (_j.name,
				 _j.position[0],  _j.position[1],  _j.position[2],
				 j_euler[0], j_euler[1], j_euler[2])
			file_to_save.write(joint_dados)

		file_to_save.write("Angulos entre os segmentos\n")

		right_ombro_euler = quaternion.toEuler(
			self.skeleton.getJoint(BodyJoints.RIGHT,BodyJoints.ELBOW).quaternion)
		left_ombro_euler = quaternion.toEuler(
			self.skeleton.getJoint(BodyJoints.LEFT,BodyJoints.ELBOW).quaternion)

		file_to_save.write("\tDireito\tEsquerdo\n")
		file_to_save.write("Ombro-Braco (Plano coronal)\t%.4f\t%.4f\n" %
			(right_ombro_euler[1], left_ombro_euler[1]))
		file_to_save.write("Ombro-Braco (Plano transversal)\t%.4f\t%.4f\n" %
			(right_ombro_euler[2], left_ombro_euler[2]))

		rvectorbraco = self.skeleton.getJoint(BodyJoints.RIGHT,BodyJoints.WRIST).position - self.skeleton.getJoint(BodyJoints.RIGHT,BodyJoints.ELBOW).position
		rvectorantebraco = self.skeleton.getJoint(BodyJoints.RIGHT,BodyJoints.ELBOW).position - self.skeleton.getJoint(BodyJoints.RIGHT,BodyJoints.SHOULDER).position
		lvectorbraco = self.skeleton.getJoint(BodyJoints.LEFT,BodyJoints.WRIST).position - self.skeleton.getJoint(BodyJoints.LEFT,BodyJoints.ELBOW).position
		lvectorantebraco = self.skeleton.getJoint(BodyJoints.LEFT,BodyJoints.ELBOW).position - self.skeleton.getJoint(BodyJoints.LEFT,BodyJoints.SHOULDER).position

		file_to_save.write("Braco-Antebraco\t%.4f\t%.4f\n" % (
			geometry.degBetweenVectors(rvectorbraco,rvectorantebraco),
			geometry.degBetweenVectors(lvectorbraco,lvectorantebraco)))

		file_to_save.write("Posicao Final da Mão\n")
		file_to_save.write("Direita\t\t\tEsquerda\n")
		file_to_save.write("X\tY\tZ\tX\tY\tZ\n")
		file_to_save.write("%.4f\t%.4f\t%.4f\t%.4f\t%.4f\t%.4f\n" % (
			self.skeleton.getJoint(BodyJoints.RIGHT,BodyJoints.WRIST).position[0],
			self.skeleton.getJoint(BodyJoints.RIGHT,BodyJoints.WRIST).position[1],
			self.skeleton.getJoint(BodyJoints.RIGHT,BodyJoints.WRIST).position[2],
			self.skeleton.getJoint(BodyJoints.LEFT,BodyJoints.WRIST).position[0],
			self.skeleton.getJoint(BodyJoints.LEFT,BodyJoints.WRIST).position[1],
			self.skeleton.getJoint(BodyJoints.LEFT,BodyJoints.WRIST).position[2]))


		mao_direita = self.skeleton.getJoint(BodyJoints.RIGHT,BodyJoints.WRIST).position
		mao_esquerda = self.skeleton.getJoint(BodyJoints.LEFT,BodyJoints.WRIST).position
		mao_esquerda[0] = -mao_esquerda[0]
		vector_maos = mao_direita - mao_esquerda
		dist_maos = np.sqrt(vector_maos[0]**2+vector_maos[1]**2+vector_maos[2]**2)
		file_to_save.write("Diferença da Posicao Final das 2 Mãos%.4f\n" % dist_maos)
		#file_to_save.write("Area do triangulo Mao-Cotovelo-Ombro")
		#file_to_save.write("Direito\tEsquerdo\n")
		#file_to_save.write("%.4f\t%.4f\n")

		file_to_save.close()

	def populateCBMarcas(self):
		self.cbMarcas.clear()
		self.quats_coleta = []
		marca_achada = False
		with open(self.opened_coleta_file_name, 'r') as openfileobject:
			for line in openfileobject:
				if marca_achada:
					marca_achada = False
					self.marca_quats.append(line)
					print line
				if 'MARCA' in line:
					self.cbMarcas.addItem(line.split(' ')[1])
					marca_achada = True
				elif '#' not in line and '*' not in line:
					self.quats_coleta.append(line)
		self.sliderColeta.setMaximum(len(self.quats_coleta)-1)
		self.cbMarcasChanged(0)

	def updateRotfromColeta(self):
		#peg index no slider
		#colocar a rota do index do slider
		idx = self.sliderColeta.value()
		print idx
		print self.quats_coleta[idx]
		data = [float(s) for s in self.quats_coleta[idx].split()]
		if len(data) == 20:
			self.rotate_joints_from_data_vector(data)


	def doCloseColeta(self):
		self.cbMarcas.setVisible(False)
		self.groupDadosColeta.setVisible(False)
		self.sliderColeta.setVisible(False)

	def cbMarcasChanged(self, idx):
		actual_position = self.marca_quats[idx]
		data = [float(s) for s in actual_position.split()]
		self.rotate_joints_from_data_vector(data)
		self.create_file_coleta()

	def rotate_joints_from_data_vector(self, data):
		self.print_pacote(data)
		joint = self.skeleton.getJoint(BodyJoints.RIGHT,BodyJoints.WRIST)
		joint.quaternion_offset = [1.,0.,0.,0.]
		joint.quaternion = data[0:4]
		print 'data'
		print data[0:4]
		print 'quat'
		print self.skeleton.getJoint(BodyJoints.RIGHT,BodyJoints.WRIST).quaternion
		#self.updateQuaternions(joint,data[0:4])
		joint = self.skeleton.getJoint(BodyJoints.RIGHT, BodyJoints.ELBOW)
		joint.quaternion_offset = [1.,0.,0.,0.]
		joint.quaternion = data[4:8]
		#self.updateQuaternions(joint,data[4:8])
		joint = self.skeleton.getJoint(BodyJoints.UNILAT,BodyJoints.TORSO)
		joint.quaternion_offset = [1.,0.,0.,0.]
		joint.quaternion = data[8:12]
		joint = self.skeleton.getJoint(BodyJoints.RIGHT,BodyJoints.SHOULDER)
		joint.quaternion_offset = [1.,0.,0.,0.]
		joint.quaternion = data[8:12]
		joint = self.skeleton.getJoint(BodyJoints.LEFT,BodyJoints.SHOULDER)
		joint.quaternion_offset = [1.,0.,0.,0.]
		joint.quaternion = data[8:12]
		#self.updateQuaternions(joint,data[8:12])
		joint = self.skeleton.getJoint(BodyJoints.LEFT,BodyJoints.ELBOW)
		joint.quaternion_offset = [1.,0.,0.,0.]
		joint.quaternion = data[12:16]
		#self.updateQuaternions(joint,data[12:16])
		joint = self.skeleton.getJoint(BodyJoints.LEFT,BodyJoints.WRIST)
		joint.quaternion_offset = [1.,0.,0.,0.]
		joint.quaternion = data[16:20]
		#self.updateQuaternions(joint,data[16:20])

		self.skeleton.rotate()
		self.plot()

	def cbSerialChanged(self, idx):
		try:
			self.imu = MPU6050(self.cbSerialPort.itemText(self.cbSerialPort.currentIndex()))
			if self.imu.open():
				self.show_info_msg("Porta serial %s aberta com sucesso!" % (self.imu.port))
		except Exception as e:
			show_error_msg("Erro ao abrir porta serial")

	def cbChanged(self,idx):
		self.updateSlideBars()

	def updateSlideBars(self):
		jointName = self.cbJointNames.itemText(self.cbJointNames.currentIndex())
		if jointName == 'InterSegmentos-Right':
			self.interseginuse = True
			intersegAngulos = self.calculateInterSegAngles('right')
			self.slPhi.setValue(int(np.round(intersegAngulos[0])))
			self.slTheta.setValue(int(np.round(intersegAngulos[1])))
			self.slPsi.setValue(int(np.round(intersegAngulos[2])))
		elif jointName == 'InterSegmentos-Left':
			self.interseginuse = True
			intersegAngulos = self.calculateInterSegAngles('left')
			self.slPhi.setValue(int(np.round(intersegAngulos[0])))
			self.slTheta.setValue(int(np.round(intersegAngulos[1])))
			self.slPsi.setValue(int(np.round(intersegAngulos[2])))
		else:
			self.interseginuse = False
			joint = self.skeleton.getJoint(None,None,str(jointName))
			euler = quat.toEuler(joint.quaternion)
			self.slPhi.setValue(int(np.round(euler[0])))
			self.slTheta.setValue(int(np.round(euler[1])))
			self.slPsi.setValue(int(np.round(euler[2])))

	def degChanged(self):
		if not self.interseginuse:
			self.lbPhi.setText('Phi: '+  str(self.slPhi.value()) + ' graus')
			self.lbTheta.setText('Theta: '+  str(self.slTheta.value()) + ' graus')
			self.lbPsi.setText('Psi: ' + str(self.slPsi.value()) + ' graus')
		else:
			self.lbPhi.setText('Inclinacao do Torso: '+  str(self.slPhi.value()) + ' graus')
			self.lbTheta.setText('Ombro-Braco: '+  str(self.slTheta.value()) + ' graus')
			self.lbPsi.setText('Braco-Antebraco: ' +  str(self.slPsi.value()) + ' graus')


	''' Return a vector containing the following angles:
			interSegAngles[0] = angulo entre torso e o eixo longitudinal.
			interSegAngles[1] = angulo entre o ombro e o braco.
			interSegAngles[2] = angulo entre o braco e o antebraco.'''
	def calculateInterSegAngles(self, side):
		if 'right' in side:
			jsh = self.skeleton.getJoint(BodyJoints.RIGHT,BodyJoints.SHOULDER)
			jel = self.skeleton.getJoint(BodyJoints.RIGHT, BodyJoints.ELBOW)
			jwr = self.skeleton.getJoint(BodyJoints.RIGHT,BodyJoints.WRIST)

			vsh = jsh.position
			refsh = [1.0,0.0,0.0]
			dsh = geometry.degBetweenVectors(jsh.position,refsh)
			vel = jel.position - jsh.position
			refel = vsh
			dshel = geometry.degBetweenVectors(vel,refel)
			vwr = jwr.position - jel.position
			refwr = vel
			delwr = geometry.degBetweenVectors(vwr,refwr)

			return [dsh,dshel,delwr]

		elif 'left' in side:
			jsh = self.skeleton.getJoint(BodyJoints.LEFT,BodyJoints.SHOULDER)
			jel = self.skeleton.getJoint(BodyJoints.LEFT, BodyJoints.ELBOW)
			jwr = self.skeleton.getJoint(BodyJoints.LEFT,BodyJoints.WRIST)

			vsh = jsh.position
			refsh = [-1.0,0.0,0.0]
			dsh = geometry.degBetweenVectors(jsh.position,refsh)
			vel = jel.position - jsh.position
			refel = vsh
			dshel = geometry.degBetweenVectors(vel,refel)
			vwr = jwr.position - jel.position
			refwr = vel
			delwr = geometry.degBetweenVectors(vwr,refwr)

			return [dsh,dshel,delwr]


	#Triggered when the button "Animation" is clicked
	def doAnimation(self):
		if not self.imu.acqThread.isAlive:
			try:
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
				self.btnAnimation.setText("Stop Animation")
				self.btnColeta.setVisible(True)
				self.cbSerialPort.setEnabled(False)
			except Exception as e:
				self.show_error_msg("Erro ao iniciar animacao.\nError Log: " + str(e))
		else:
			try:
				self.doStop()
				self.btnAnimation.setText("Start Animation")
				self.btnAnimation.setEnabled(False)
				self.btnColeta.setVisible(False)
				self.cbSerialPort.setEnabled(True)
			except Exception as e:
				self.show_error_msg("Erro ao finalizar animacao.\nError Log: " + str(e))


	#Triggered when the "Stop" button is clicked
	def doStop(self):
		self.imu.stop()
		time.sleep(1)
		#Kill the threads
		self.imu.acqThread.kill()
		self.dataProc.kill()
		self.plotTh.kill()

	#Function that updates the chart
	def runPlot(self):
		if self.plotcounter == 100:
			self.plotLock.acquire()
			self.plot()
			self.plotLock.release()
			self.plotcounter = 0
		else:
			self.plotcounter += 1

	#Function that handles the data acquisition
	def runAnimation(self):
		self.plotLock.acquire()
		#self.imu.acqThread.lock.acquire()
		if self.imu.dataQueue.qsize() > 0:
			self.disassemblePacket()
			#print joint.quaternion, joint.rotquaternion, joint.position
			#print joint.position
			self.skeleton.rotate()
		self.plotLock.release()
		#print "[%.2f\t%.2f\t%.2f\t]" % (joint.position[0],joint.position[1],joint.position[2])
		#self.plot()
		#self.imu.acqThread.lock.release()
		#time.sleep(0.005) #sampling frequency 100 Hz

	def disassemblePacket(self):
		#print 'DONE'
		n = self.imu.dataQueue.qsize()
		for i in range(n):
			data = self.imu.dataQueue.get()
			#print 'qntsensor: %d' % (len(data)/4)

			if self.statusColetaRunning:
				self.write_joints_quat()
				self.write_dados_brutos_quat(data)
			if self.marcacaoPending:
				self.arqColeta.write("%s MARCA-%d %s\n" % ("*"*30,self.marcanumero,"*"*30))
				self.arqColetaBruta.write("%s MARCA-%d %s\n" % ("*"*30,self.marcanumero,"*"*30))
				self.marcacaoPending = False

			if self.offsetpending:
				self.offsetpending = False
				print "*******************DOING OFFSETS********************"
				joint = self.skeleton.getJoint(BodyJoints.RIGHT,BodyJoints.WRIST)
				joint.setQuaternionOffset(data[0:4])
				joint = self.skeleton.getJoint(BodyJoints.RIGHT, BodyJoints.ELBOW)
				joint.setQuaternionOffset(data[4:8])
				joint = self.skeleton.getJoint(BodyJoints.UNILAT,BodyJoints.TORSO)
				joint.setQuaternionOffset(data[8:12])
				joint = self.skeleton.getJoint(BodyJoints.LEFT,BodyJoints.SHOULDER)
				joint.setQuaternionOffset(data[8:12])
				joint = self.skeleton.getJoint(BodyJoints.RIGHT,BodyJoints.SHOULDER)
				joint.setQuaternionOffset(data[8:12])
				joint = self.skeleton.getJoint(BodyJoints.UNILAT,BodyJoints.HEAD)
				joint.setQuaternionOffset(data[8:12])
				joint = self.skeleton.getJoint(BodyJoints.LEFT,BodyJoints.ELBOW)
				joint.setQuaternionOffset(data[12:16])
				joint = self.skeleton.getJoint(BodyJoints.LEFT,BodyJoints.WRIST)
				joint.setQuaternionOffset(data[16:20])


			if len(data) >= 4:
				quat = data[0:4]
				zn = quat[3]
				estz = self.linereg[0]*self.time + self.biasreg[0]
				quat[3] -= estz
				modim = np.sqrt(quat[1]*quat[1]+quat[2]*quat[2]+quat[3]*quat[3])
				quat[0] = np.cos(np.arcsin(modim))
				joint = self.skeleton.getJoint(BodyJoints.RIGHT,BodyJoints.WRIST)
				joint.setQuaternion(quat)
			if len(data) >= 8:
				quat = data[4:8]
				estz = self.linereg[1]*self.time + self.biasreg[1]
				quat[3] -= estz
				modim = np.sqrt(quat[1]*quat[1]+quat[2]*quat[2]+quat[3]*quat[3])
				quat[0] = np.cos(np.arcsin(modim))
				joint = self.skeleton.getJoint(BodyJoints.RIGHT, BodyJoints.ELBOW)
				joint.setQuaternion(quat)
			if len(data) >= 12:
				quat = data[8:12]
				estz = self.linereg[2]*self.time + self.biasreg[2]
				quat[3] -= estz
				modim = np.sqrt(quat[1]*quat[1]+quat[2]*quat[2]+quat[3]*quat[3])
				quat[0] = np.cos(np.arcsin(modim))
				joint = self.skeleton.getJoint(BodyJoints.UNILAT,BodyJoints.TORSO)
				joint.setQuaternion(quat)
				joint = self.skeleton.getJoint(BodyJoints.LEFT,BodyJoints.SHOULDER)
				joint.setQuaternion(quat)
				joint = self.skeleton.getJoint(BodyJoints.RIGHT,BodyJoints.SHOULDER)
				joint.setQuaternion(quat)
				joint = self.skeleton.getJoint(BodyJoints.UNILAT,BodyJoints.HEAD)
				joint.setQuaternion(quat)
			if len(data) >= 16:
				quat = data[12:16]
				estz = self.linereg[3]*self.time + self.biasreg[3]
				quat[3] -= estz
				modim = np.sqrt(quat[1]*quat[1]+quat[2]*quat[2]+quat[3]*quat[3])
				quat[0] = np.cos(np.arcsin(modim))
				joint = self.skeleton.getJoint(BodyJoints.LEFT,BodyJoints.ELBOW)
				joint.setQuaternion(quat)
			if len(data) >= 20:
				quat = data[16:20]
				estz = self.linereg[4]*self.time + self.biasreg[4]
				quat[3] -= estz
				modim = np.sqrt(quat[1]*quat[1]+quat[2]*quat[2]+quat[3]*quat[3])
				quat[0] = np.cos(np.arcsin(modim))
				joint = self.skeleton.getJoint(BodyJoints.LEFT,BodyJoints.WRIST)
				joint.setQuaternion(quat)

			self.time += 1.0/self.sampfreq
			#self.updateQuaternions(joint,quat)
			#self.print_pacote(data)
			#self.print_joints_quat()
			self.updateSlideBars()

	def print_pacote(self,_data):
		print "Pacote: "
		print "%.4f\t%.4f\t%.4f\t%.4f" % (_data[0], _data[1], _data[2], _data[3])
		print "%.4f\t%.4f\t%.4f\t%.4f" % (_data[0+4], _data[1+4], _data[2+4], _data[3+4])
		print "%.4f\t%.4f\t%.4f\t%.4f" % (_data[0+8], _data[1+8], _data[2+8], _data[3+8])
		print "%.4f\t%.4f\t%.4f\t%.4f" % (_data[0+12], _data[1+12], _data[2+12], _data[3+12])
		print "%.4f\t%.4f\t%.4f\t%.4f" % (_data[0+16], _data[1+16], _data[2+16], _data[3+16])

	def print_joints_quat(self):
		print "Joint Quats: "
		q_joint = self.skeleton.getJoint(BodyJoints.RIGHT,BodyJoints.WRIST).quaternion
		print "RIGHT\tWRIST: %.2f\t%.2f\t%.2f\t%.2f" % (q_joint[0], q_joint[1], q_joint[2], q_joint[3])

		q_joint = self.skeleton.getJoint(BodyJoints.RIGHT,BodyJoints.ELBOW).quaternion
		print "RIGHT\tELBOW: %.2f\t%.2f\t%.2f\t%.2f" % (q_joint[0], q_joint[1], q_joint[2], q_joint[3])

		q_joint = self.skeleton.getJoint(BodyJoints.UNILAT,BodyJoints.TORSO).quaternion
		print "UNILAT\tTORSO: %.2f\t%.2f\t%.2f\t%.2f" % (q_joint[0], q_joint[1], q_joint[2], q_joint[3])

		q_joint = self.skeleton.getJoint(BodyJoints.LEFT,BodyJoints.ELBOW).quaternion
		print "LEFT\tELBOW: %.2f\t%.2f\t%.2f\t%.2f" % (q_joint[0], q_joint[1], q_joint[2], q_joint[3])

		q_joint = self.skeleton.getJoint(BodyJoints.LEFT,BodyJoints.WRIST).quaternion
		print "LEFT\tWRIST: %.2f\t%.2f\t%.2f\t%.2f" % (q_joint[0], q_joint[1], q_joint[2], q_joint[3])

	def write_joints_quat(self):
		q_joint = self.skeleton.getJoint(BodyJoints.RIGHT,BodyJoints.WRIST).quaternion
		self.arqColeta.write("%f\t%f\t%f\t%f\t" % (q_joint[0], q_joint[1], q_joint[2], q_joint[3]))

		q_joint = self.skeleton.getJoint(BodyJoints.RIGHT,BodyJoints.ELBOW).quaternion
		self.arqColeta.write("%f\t%f\t%f\t%f\t" % (q_joint[0], q_joint[1], q_joint[2], q_joint[3]))

		q_joint = self.skeleton.getJoint(BodyJoints.UNILAT,BodyJoints.TORSO).quaternion
		self.arqColeta.write("%f\t%f\t%f\t%f\t" % (q_joint[0], q_joint[1], q_joint[2], q_joint[3]))

		q_joint = self.skeleton.getJoint(BodyJoints.LEFT,BodyJoints.ELBOW).quaternion
		self.arqColeta.write("%f\t%f\t%f\t%f\t" % (q_joint[0], q_joint[1], q_joint[2], q_joint[3]))

		q_joint = self.skeleton.getJoint(BodyJoints.LEFT,BodyJoints.WRIST).quaternion
		self.arqColeta.write("%f\t%f\t%f\t%f" % (q_joint[0], q_joint[1], q_joint[2], q_joint[3]))
		self.arqColeta.write("\n")

	def write_dados_brutos_quat(self,quats):
		for valor in quats:
			self.arqColetaBruta.write('%f\t')
		self.arqColetaBruta.write("\n")





if __name__ == '__main__':
	import sys
	from PyQt4 import QtGui

	app = QtGui.QApplication(sys.argv)
	main = Main()
	main.plot()
	main.show()
	sys.exit(app.exec_())
