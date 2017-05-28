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
        MainWindow.resize(280, 600)
        self.centralwidget = QtGui.QWidget(MainWindow)
        self.centralwidget.setObjectName(_fromUtf8("centralwidget"))
        self.horizontalLayout = QtGui.QHBoxLayout(self.centralwidget)
        self.horizontalLayout.setObjectName(_fromUtf8("horizontalLayout"))
        self.mbly = QtGui.QHBoxLayout()
        self.mbly.setObjectName(_fromUtf8("mbly"))
        self.verticalLayout_2 = QtGui.QVBoxLayout()
        self.verticalLayout_2.setObjectName(_fromUtf8("verticalLayout_2"))
        self.ColetaGroup = QtGui.QGroupBox(self.centralwidget)
        self.ColetaGroup.setObjectName(_fromUtf8("ColetaGroup"))
        self.verticalLayout_5 = QtGui.QVBoxLayout(self.ColetaGroup)
        self.verticalLayout_5.setObjectName(_fromUtf8("verticalLayout_5"))
        self.btnAnimation = QtGui.QPushButton(self.ColetaGroup)
        self.btnAnimation.setObjectName(_fromUtf8("btnAnimation"))
        self.verticalLayout_5.addWidget(self.btnAnimation)
        self.btnColeta = QtGui.QPushButton(self.ColetaGroup)
        self.btnColeta.setObjectName(_fromUtf8("btnColeta"))
        self.verticalLayout_5.addWidget(self.btnColeta)
        self.btnMarcar = QtGui.QPushButton(self.ColetaGroup)
        self.btnMarcar.setObjectName(_fromUtf8("btnMarcar"))
        self.verticalLayout_5.addWidget(self.btnMarcar)
        spacerItem = QtGui.QSpacerItem(20, 40, QtGui.QSizePolicy.Minimum, QtGui.QSizePolicy.Expanding)
        self.verticalLayout_5.addItem(spacerItem)
        self.verticalLayout_2.addWidget(self.ColetaGroup)
        self.groupBox = QtGui.QGroupBox(self.centralwidget)
        self.groupBox.setMaximumSize(QtCore.QSize(200, 16777215))
        self.groupBox.setObjectName(_fromUtf8("groupBox"))
        self.verticalLayout = QtGui.QVBoxLayout(self.groupBox)
        self.verticalLayout.setObjectName(_fromUtf8("verticalLayout"))
        self.cbJointNames = QtGui.QComboBox(self.groupBox)
        self.cbJointNames.setMaximumSize(QtCore.QSize(200, 16777215))
        self.cbJointNames.setObjectName(_fromUtf8("cbJointNames"))
        self.verticalLayout.addWidget(self.cbJointNames)
        self.lbPhi = QtGui.QLabel(self.groupBox)
        self.lbPhi.setObjectName(_fromUtf8("lbPhi"))
        self.verticalLayout.addWidget(self.lbPhi)
        self.slPhi = QtGui.QSlider(self.groupBox)
        self.slPhi.setMinimum(-180)
        self.slPhi.setMaximum(180)
        self.slPhi.setOrientation(QtCore.Qt.Horizontal)
        self.slPhi.setObjectName(_fromUtf8("slPhi"))
        self.verticalLayout.addWidget(self.slPhi)
        self.lbTheta = QtGui.QLabel(self.groupBox)
        self.lbTheta.setObjectName(_fromUtf8("lbTheta"))
        self.verticalLayout.addWidget(self.lbTheta)
        self.slTheta = QtGui.QSlider(self.groupBox)
        self.slTheta.setMinimum(-180)
        self.slTheta.setMaximum(180)
        self.slTheta.setOrientation(QtCore.Qt.Horizontal)
        self.slTheta.setObjectName(_fromUtf8("slTheta"))
        self.verticalLayout.addWidget(self.slTheta)
        self.lbPsi = QtGui.QLabel(self.groupBox)
        self.lbPsi.setObjectName(_fromUtf8("lbPsi"))
        self.verticalLayout.addWidget(self.lbPsi)
        self.slPsi = QtGui.QSlider(self.groupBox)
        self.slPsi.setMinimum(-180)
        self.slPsi.setMaximum(180)
        self.slPsi.setProperty("value", 0)
        self.slPsi.setOrientation(QtCore.Qt.Horizontal)
        self.slPsi.setObjectName(_fromUtf8("slPsi"))
        self.verticalLayout.addWidget(self.slPsi)
        self.btnRotate = QtGui.QPushButton(self.groupBox)
        self.btnRotate.setMaximumSize(QtCore.QSize(200, 16777215))
        self.btnRotate.setObjectName(_fromUtf8("btnRotate"))
        self.verticalLayout.addWidget(self.btnRotate)
        self.btnReset = QtGui.QPushButton(self.groupBox)
        self.btnReset.setMaximumSize(QtCore.QSize(200, 16777215))
        self.btnReset.setObjectName(_fromUtf8("btnReset"))
        self.verticalLayout.addWidget(self.btnReset)
        self.verticalLayout_2.addWidget(self.groupBox)
        self.mbly.addLayout(self.verticalLayout_2)
        self.horizontalLayout.addLayout(self.mbly)
        MainWindow.setCentralWidget(self.centralwidget)
        self.menubar = QtGui.QMenuBar(MainWindow)
        self.menubar.setGeometry(QtCore.QRect(0, 0, 280, 28))
        self.menubar.setObjectName(_fromUtf8("menubar"))
        self.menuArquivo = QtGui.QMenu(self.menubar)
        self.menuArquivo.setObjectName(_fromUtf8("menuArquivo"))
        MainWindow.setMenuBar(self.menubar)
        self.statusbar = QtGui.QStatusBar(MainWindow)
        self.statusbar.setObjectName(_fromUtf8("statusbar"))
        MainWindow.setStatusBar(self.statusbar)
        self.actionAbrir_Postura = QtGui.QAction(MainWindow)
        self.actionAbrir_Postura.setObjectName(_fromUtf8("actionAbrir_Postura"))
        self.actionSalvar_Postura = QtGui.QAction(MainWindow)
        self.actionSalvar_Postura.setObjectName(_fromUtf8("actionSalvar_Postura"))
        self.menuArquivo.addSeparator()
        self.menuArquivo.addAction(self.actionAbrir_Postura)
        self.menuArquivo.addAction(self.actionSalvar_Postura)
        self.menubar.addAction(self.menuArquivo.menuAction())

        self.retranslateUi(MainWindow)
        QtCore.QMetaObject.connectSlotsByName(MainWindow)

    def retranslateUi(self, MainWindow):
        MainWindow.setWindowTitle(_translate("MainWindow", "MainWindow", None))
        self.ColetaGroup.setTitle(_translate("MainWindow", "Coleta:", None))
        self.btnAnimation.setText(_translate("MainWindow", "Iniciar Animação", None))
        self.btnColeta.setText(_translate("MainWindow", "Iniciar Coleta", None))
        self.btnMarcar.setText(_translate("MainWindow", "Marcar", None))
        self.groupBox.setTitle(_translate("MainWindow", "Rotação:", None))
        self.lbPhi.setText(_translate("MainWindow", "Phi", None))
        self.lbTheta.setText(_translate("MainWindow", "Theta", None))
        self.lbPsi.setText(_translate("MainWindow", "Psi", None))
        self.btnRotate.setText(_translate("MainWindow", "Rotate", None))
        self.btnReset.setText(_translate("MainWindow", "Reset", None))
        self.menuArquivo.setTitle(_translate("MainWindow", "Arquivo", None))
        self.actionAbrir_Postura.setText(_translate("MainWindow", "Abrir Postura", None))
        self.actionSalvar_Postura.setText(_translate("MainWindow", "Salvar Postura", None))

