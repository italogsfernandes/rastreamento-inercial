# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'mainwindow.ui'
#
# Created by: PyQt4 UI code generator 4.11.4
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    def _fromUtf8(s):
        return s

try:
    _encoding = QtGui.QApplication.UnicodeUTF8
    def _translate(context, text, disambig):
        return QtGui.QApplication.translate(context, text, disambig, _encoding)
except AttributeError:
    def _translate(context, text, disambig):
        return QtGui.QApplication.translate(context, text, disambig)

class Ui_MainWindow(object):
    def setupUi(self, MainWindow):
        MainWindow.setObjectName(_fromUtf8("MainWindow"))
        MainWindow.resize(800, 600)
        self.centralwidget = QtGui.QWidget(MainWindow)
        self.centralwidget.setObjectName(_fromUtf8("centralwidget"))
        self.horizontalLayout = QtGui.QHBoxLayout(self.centralwidget)
        self.horizontalLayout.setObjectName(_fromUtf8("horizontalLayout"))
        self.verticalLayout_2 = QtGui.QVBoxLayout()
        self.verticalLayout_2.setObjectName(_fromUtf8("verticalLayout_2"))
        self.cbJointNames = QtGui.QComboBox(self.centralwidget)
        self.cbJointNames.setMaximumSize(QtCore.QSize(200, 16777215))
        self.cbJointNames.setObjectName(_fromUtf8("cbJointNames"))
        self.verticalLayout_2.addWidget(self.cbJointNames)
        self.btnColeta = QtGui.QPushButton(self.centralwidget)
        self.btnColeta.setObjectName(_fromUtf8("btnColeta"))
        self.verticalLayout_2.addWidget(self.btnColeta)
        self.btnMarcar = QtGui.QPushButton(self.centralwidget)
        self.btnMarcar.setObjectName(_fromUtf8("btnMarcar"))
        self.verticalLayout_2.addWidget(self.btnMarcar)
        self.groupBox = QtGui.QGroupBox(self.centralwidget)
        self.groupBox.setMaximumSize(QtCore.QSize(200, 16777215))
        self.groupBox.setObjectName(_fromUtf8("groupBox"))
        self.lbPhi = QtGui.QLabel(self.groupBox)
        self.lbPhi.setGeometry(QtCore.QRect(10, 40, 67, 17))
        self.lbPhi.setObjectName(_fromUtf8("lbPhi"))
        self.lbTheta = QtGui.QLabel(self.groupBox)
        self.lbTheta.setGeometry(QtCore.QRect(10, 110, 121, 17))
        self.lbTheta.setObjectName(_fromUtf8("lbTheta"))
        self.lbPsi = QtGui.QLabel(self.groupBox)
        self.lbPsi.setGeometry(QtCore.QRect(10, 170, 67, 17))
        self.lbPsi.setObjectName(_fromUtf8("lbPsi"))
        self.slPhi = QtGui.QSlider(self.groupBox)
        self.slPhi.setGeometry(QtCore.QRect(10, 70, 121, 31))
        self.slPhi.setMinimum(-180)
        self.slPhi.setMaximum(180)
        self.slPhi.setOrientation(QtCore.Qt.Horizontal)
        self.slPhi.setObjectName(_fromUtf8("slPhi"))
        self.slTheta = QtGui.QSlider(self.groupBox)
        self.slTheta.setGeometry(QtCore.QRect(10, 130, 121, 31))
        self.slTheta.setMinimum(-180)
        self.slTheta.setMaximum(180)
        self.slTheta.setOrientation(QtCore.Qt.Horizontal)
        self.slTheta.setObjectName(_fromUtf8("slTheta"))
        self.slPsi = QtGui.QSlider(self.groupBox)
        self.slPsi.setGeometry(QtCore.QRect(10, 200, 121, 31))
        self.slPsi.setMinimum(-180)
        self.slPsi.setMaximum(180)
        self.slPsi.setProperty("value", 0)
        self.slPsi.setOrientation(QtCore.Qt.Horizontal)
        self.slPsi.setObjectName(_fromUtf8("slPsi"))
        self.btnRotate = QtGui.QPushButton(self.groupBox)
        self.btnRotate.setGeometry(QtCore.QRect(0, 240, 100, 27))
        self.btnRotate.setMaximumSize(QtCore.QSize(200, 16777215))
        self.btnRotate.setObjectName(_fromUtf8("btnRotate"))
        self.btnReset = QtGui.QPushButton(self.groupBox)
        self.btnReset.setGeometry(QtCore.QRect(0, 370, 100, 27))
        self.btnReset.setMaximumSize(QtCore.QSize(200, 16777215))
        self.btnReset.setObjectName(_fromUtf8("btnReset"))
        self.btnStanding = QtGui.QPushButton(self.groupBox)
        self.btnStanding.setGeometry(QtCore.QRect(0, 270, 100, 27))
        self.btnStanding.setMaximumSize(QtCore.QSize(200, 16777215))
        self.btnStanding.setObjectName(_fromUtf8("btnStanding"))
        self.btnAnimation = QtGui.QPushButton(self.groupBox)
        self.btnAnimation.setGeometry(QtCore.QRect(0, 400, 100, 27))
        self.btnAnimation.setMaximumSize(QtCore.QSize(200, 16777215))
        self.btnAnimation.setObjectName(_fromUtf8("btnAnimation"))
        self.btnStop = QtGui.QPushButton(self.groupBox)
        self.btnStop.setGeometry(QtCore.QRect(0, 420, 100, 27))
        self.btnStop.setMaximumSize(QtCore.QSize(200, 16777215))
        self.btnStop.setObjectName(_fromUtf8("btnStop"))
        self.btnSavePostura = QtGui.QPushButton(self.groupBox)
        self.btnSavePostura.setGeometry(QtCore.QRect(0, 300, 101, 28))
        self.btnSavePostura.setObjectName(_fromUtf8("btnSavePostura"))
        self.verticalLayout_2.addWidget(self.groupBox)
        self.horizontalLayout.addLayout(self.verticalLayout_2)
        self.mbly = QtGui.QVBoxLayout()
        self.mbly.setObjectName(_fromUtf8("mbly"))
        self.horizontalLayout.addLayout(self.mbly)
        MainWindow.setCentralWidget(self.centralwidget)
        self.menubar = QtGui.QMenuBar(MainWindow)
        self.menubar.setGeometry(QtCore.QRect(0, 0, 800, 28))
        self.menubar.setObjectName(_fromUtf8("menubar"))
        MainWindow.setMenuBar(self.menubar)
        self.statusbar = QtGui.QStatusBar(MainWindow)
        self.statusbar.setObjectName(_fromUtf8("statusbar"))
        MainWindow.setStatusBar(self.statusbar)

        self.retranslateUi(MainWindow)
        QtCore.QMetaObject.connectSlotsByName(MainWindow)

    def retranslateUi(self, MainWindow):
        MainWindow.setWindowTitle(_translate("MainWindow", "MainWindow", None))
        self.btnColeta.setText(_translate("MainWindow", "Iniciar Coleta", None))
        self.btnMarcar.setText(_translate("MainWindow", "Marcar Posição de Repouso", None))
        self.groupBox.setTitle(_translate("MainWindow", "Degrees", None))
        self.lbPhi.setText(_translate("MainWindow", "Phi", None))
        self.lbTheta.setText(_translate("MainWindow", "Theta", None))
        self.lbPsi.setText(_translate("MainWindow", "Psi", None))
        self.btnRotate.setText(_translate("MainWindow", "Rotate", None))
        self.btnReset.setText(_translate("MainWindow", "Reset", None))
        self.btnStanding.setText(_translate("MainWindow", "Repouso", None))
        self.btnAnimation.setText(_translate("MainWindow", "Animation", None))
        self.btnStop.setText(_translate("MainWindow", "Stop", None))
        self.btnSavePostura.setText(_translate("MainWindow", "Salvar Postura", None))

