from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4.uic import loadUiType
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import axes3d
from mpl_toolkits.mplot3d import Axes3D
from matplotlib.backends.backend_qt4agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.backends.backend_qt4agg import NavigationToolbar2QT as NavigationToolbar
import numpy as np

Ui_MainWindow, QMainWindow = loadUiType('window.ui')

class Main(QMainWindow, Ui_MainWindow):
	def __init__(self):
		super(Main,self).__init__()
		self.setupUi(self)		

		#This is how we connect signals and slots
		self.connect(self.btnPlot,SIGNAL('clicked()'),self.carai)
	
	def addmpl(self,fig,ax):   
		self.ax = ax
		self.canvas = FigureCanvas(fig)
		self.mplvl.addWidget(self.canvas)
		self.canvas.draw()
		Axes3D.mouse_init(ax)
		self.toolbar = NavigationToolbar(self.canvas,self,coordinates=True)
		self.mplvl.addWidget(self.toolbar)

	def plot(self,ax,point):
		ax.scatter(point[0],point[1],point[2],c='gray',s=60)
		self.canvas.draw()

	def carai(self):
		self.plot(self.ax,[0,0,5])
		print 'ok'


if __name__ == '__main__':
	import sys
	from PyQt4 import QtGui

	#Limb positions
	torso = np.array([0,0,0])
	phead = np.array([0,5,0])
	rsh = np.array([2,0,0])
	rel = np.array([5,0,0])
	rwr = np.array([7,0,0])
	
	#Plot parameters
	axesLim = [-15,15]
	fig = plt.figure()
	ax = fig.gca(projection='3d')
	ax.set_xlim(axesLim)
	ax.set_ylim(axesLim)
	ax.set_zlim(axesLim)
	ax.set_title('Body model')
	ax.set_xlabel('x-axis')
	ax.set_ylabel('y-axis')
	ax.set_zlabel('z-axis')

	for item in [fig, ax]:
		item.patch.set_visible(False)

	app = QtGui.QApplication(sys.argv)
	main = Main()
	main.addmpl(fig,ax)	
	main.plot(ax,rsh)
	main.plot(ax,rel)
	main.plot(ax,rwr)
	main.show()
	sys.exit(app.exec_())
