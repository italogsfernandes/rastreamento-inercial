# -*- coding: utf-8 -*-
'''
------------------------------------------------------------------------------
FEDERAL UNIVERSITY OF UBERLANDIA
Faculty of Electrical Engineering
Biomedical Engineering Lab
Uberlândia, Brazil
-----------------------------------------------------------------------------
Author: Andrei Nakagawa, MSc
Contact: nakagawa.andrei@gmail.com
URL: www.biolab.eletrica.ufu.br
Git: https://github.com/BIOLAB-UFU-BRAZIL
-----------------------------------------------------------------------------
Decription:
-----------------------------------------------------------------------------
'''
#------------------------------------------------------------------------------
# LIBRARIES
#------------------------------------------------------------------------------
import numpy as np
#------------------------------------------------------------------------------
def translate2Origin(_point,_origin):
	_point = np.array(_point)
	_origin = np.array(_origin)
	return _point - _origin
#------------------------------------------------------------------------------
def translateFromOrigin(_point,_origin):
	_point = np.array(_point)
	_origin = np.array(_origin)
	return _point + _origin
#------------------------------------------------------------------------------
def rotate(_point,_origin,_rotationMatrix):
	_point = np.array(_point)
	_origin = np.array(_origin)
	trPoint = translate2Origin(_point,_origin)
	rotPoint = np.dot(trPoint,_rotationMatrix)
	respPoint = translateFromOrigin(rotPoint,_origin)
	return respPoint
#------------------------------------------------------------------------------
def vectorNorm(_point,_origin):
	vector = _point - _origin
	return np.linalg.norm(vector)
#------------------------------------------------------------------------------
def deg2rotMat(_deg):
	rot = np.zeros([2,2])
	rot[0,0] = np.cos(np.deg2rad(_deg))
	rot[0,1] = -np.sin(np.deg2rad(_deg))
	rot[1,0] = np.sin(np.deg2rad(_deg))
	rot[1,1] = np.cos(np.deg2rad(_deg))
	return rot
#------------------------------------------------------------------------------
def euler2rotMat(_phi,_theta,_psi):
	rot = np.zeros([3,3])
	_phi = np.deg2rad(_phi)
	_theta = np.deg2rad(_theta)
	_psi = np.deg2rad(_psi)

	rot[0,0] = np.cos(_psi)*np.cos(_theta)
	rot[0,1] = -np.sin(_psi)*np.cos(_phi) + np.cos(_psi)*np.sin(_theta)*np.sin(_phi)
	rot[0,2] = np.sin(_psi)*np.sin(_phi) + np.cos(_psi)*np.sin(_theta)*np.cos(_phi)

	rot[1,0] = np.sin(_psi)*np.cos(_theta)
	rot[1,1] = np.cos(_psi)*np.cos(_phi) + np.sin(_psi)*np.sin(_theta)*np.sin(_phi)
	rot[1,2] = -np.cos(_psi)*np.sin(_phi) + np.sin(_psi	)*np.sin(_theta)*np.cos(_phi)

	rot[2,0] = -np.sin(_theta)
	rot[2,1] = np.cos(_theta)*np.sin(_phi)
	rot[2,2] = np.cos(_theta)*np.cos(_phi)

	return rot
#------------------------------------------------------------------------------
def rotmat2euler(_rotationMatrix):
	phi = np.arctan2(_rotationMatrix[2,1],_rotationMatrix[2,2])
	theta = -np.arctan(_rotationMatrix[2,0] / np.sqrt(1 - np.power(_rotationMatrix[2,0],2)))
	psi = np.arctan2(_rotationMatrix[1,0],_rotationMatrix[0,0])
	return phi,theta,psi
#------------------------------------------------------------------------------
def degBetweenVectors(_vectorA,_vectorB):
	dotp = np.dot(_vectorA,_vectorB)
	dotpn = np.dot(np.linalg.norm(_vectorA),np.linalg.norm(_vectorB))
	return np.rad2deg(np.arccos(dotp/dotpn))
#------------------------------------------------------------------------------
if __name__ == '__main__':
	v1 = [3,0,0]
	v2 = [6,0,0]
	rot = euler2rotMat(0,0,30)
	print rot
	print rotate(v2,v1,rot)
#------------------------------------------------------------------------------
