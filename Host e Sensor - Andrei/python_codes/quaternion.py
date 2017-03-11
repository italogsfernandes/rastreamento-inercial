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
Acknowledgements: All the methods contained in this file were based on the code
available from xioTechnologies on GitHub, implemented in MATLAB.
-----------------------------------------------------------------------------
'''
#------------------------------------------------------------------------------
# LIBRARIES
#------------------------------------------------------------------------------
import numpy as np
#------------------------------------------------------------------------------
def toEuler(quat):
	#from paper: "Adaptive Filter for a Miniature MEMS Based Attitude and
    #Heading Reference System" by Wang et al, IEEE.
    R = np.zeros([5,1])
    R[0] = 2. * np.power(quat[0],2) - 1 + 2. * np.power(quat[1],2)
    R[1] = 2. * (quat[1]*quat[2] - quat[0]*quat[3])
    R[2] = 2. * (quat[1]*quat[3] + quat[0]*quat[2])
    R[3] = 2. * (quat[2]*quat[3] - quat[0]*quat[1])
    R[4] = 2. * np.power(quat[0],2) - 1 + 2. * np.power(quat[3],2)
    phi = np.arctan2(R[3],R[4])
    theta = -np.arctan(R[2] / np.sqrt(1 - np.power(R[2],2)))
    psi = np.arctan2(R[1],R[0])
    phi = np.rad2deg(phi)
    theta = np.rad2deg(theta)
    psi = np.rad2deg(psi)
    return [phi[0],theta[0],psi[0]]
#------------------------------------------------------------------------------
def toRotMat(quat):
	R = np.zeros((3,3))
	R[0,0] = 2. * np.power(quat[0],2)-1 + 2 * np.power(quat[1],2);
	R[0,1] = 2. * (quat[1]*quat[2]+quat[0]*quat[3]);
	R[0,2] = 2. * (quat[1]*quat[3]-quat[0]*quat[2]);
	R[1,0] = 2. * (quat[1]*quat[2]-quat[0]*quat[3]);
	R[1,1] = 2. * np.power(quat[0],2)-1 + 2*np.power(quat[2],2);
	R[1,2] = 2. * (quat[2]*quat[3]+quat[0]*quat[1]);
	R[2,0] = 2. * (quat[1]*quat[3]+quat[0]*quat[2]);
	R[2,1] = 2. * (quat[2]*quat[3]-quat[0]*quat[1]);
	R[2,2] = 2. * np.power(quat[0],2)-1 + 2*np.power(quat[3],2);
	return R
#------------------------------------------------------------------------------
def toGravity(quat):
	x = 2. * (quat[1] * quat[3] - quat[0]*quat[2])
	y = 2. * (quat[0] * quat[1] + quat[2]*quat[3])
	z = quat[0]*quat[0] - quat[1]*quat[1] - quat[2]*quat[2] + quat[3]*quat[3]
	return [x,y,z]
#------------------------------------------------------------------------------
def conjugate(quat):
	return [ quat[0], -quat[1], -quat[2], -quat[3] ]
#------------------------------------------------------------------------------
def product(a,b):
	ab = np.zeros(np.shape(a))
	ab[0] = a[0]*b[0] - a[1]*b[1] - a[2]*b[2] - a[3]*b[3]
	ab[1] = a[0]*b[1] + a[1]*b[0] + a[2]*b[3] - a[3]*b[2]
	ab[2] = a[0]*b[2] - a[1]*b[3] + a[2]*b[0] + a[3]*b[1]
	ab[3] = a[0]*b[3] + a[1]*b[2] - a[2]*b[1] + a[3]*b[0]
	return ab
#------------------------------------------------------------------------------
def rotate(v,q):
	rows,cols = np.shape(v)
	zeroAux = np.zeros([rows,5])
	zeroAux[:,1:6] = v
	v0XYZ = quaternProd(quaternProd(q,zeroAux),quaternConj(q))
	return v0XYZ[:,1:4]
#------------------------------------------------------------------------------
def euler2rotMat(phi,theta,psi):
	phi = np.deg2rad(phi)
	theta = np.deg2rad(theta)
	psi = np.deg2rad(psi)
	rot = np.zeros([3,3])
	rot[0,0] = np.cos(psi) * np.cos(theta)
	rot[0,1] = -np.sin(psi)*np.cos(phi) + np.cos(psi)*np.sin(theta)*np.sin(phi)
	rot[0,2] = np.sin(psi)*np.sin(phi) + np.cos(psi)*np.sin(theta)*np.cos(phi)
	rot[1,0] = np.sin(psi)*np.cos(theta)
	rot[1,1] = np.cos(psi)*np.cos(phi) + np.sin(psi)*np.sin(theta)*np.sin(phi)
	rot[1,2] = -np.cos(psi)*np.sin(phi) + np.sin(psi)*np.sin(theta)*np.cos(phi)
	rot[2,0] = -np.sin(theta)
	rot[2,1] = np.cos(theta)*np.sin(phi)
	rot[2,2] = np.cos(theta)*np.cos(phi)
	return rot
#------------------------------------------------------------------------------
def fromRotMat(rot):
	k = np.zeros([4,4])
	respQuat = np.zeros([4,1])
	k[0,0] = (1./3.) * (rot[0,0] - rot[1,1] - rot[2,2])
	k[0,1] = (1./3.) * (rot[1,0] + rot[0,1])
	k[0,2] = (1./3.) * (rot[2,0] + rot[0,2])
	k[0,3] = (1./3.) * (rot[1,2] - rot[2,1])
	k[1,0] = (1./3.) * (rot[1,0] + rot[0,1])
	k[1,1] = (1./3.) * (rot[1,1] - rot[0,0] - rot[2,2])
	k[1,2] = (1./3.) * (rot[2,1] + rot[1,2])
	k[1,3] = (1./3.) * (rot[2,0] - rot[0,2])
	k[2,0] = (1./3.) * (rot[2,0] + rot[0,2])
	k[2,1] = (1./3.) * (rot[2,1] + rot[1,2])
	k[2,2] = (1./3.) * (rot[2,2] - rot[0,0] - rot[1,1])
	k[2,3] = (1./3.) * (rot[2,0] - rot[0,2])
	k[3,0] = (1./3.) * (rot[1,2] - rot[2,1])
	k[3,1] = (1./3.) * (rot[2,0] - rot[0,2])
	k[3,2] = (1./3.) * (rot[0,1] - rot[1,0])
	k[3,3] = (1./3.) * (rot[0,0] + rot[1,1] + rot[2,2])
	[d,v] = np.linalg.eigh(k)
	return [v[3,3],v[0,3],v[1,3],v[2,3]]
#------------------------------------------------------------------------------
def fromEuler(phi,theta,psi):
	rot = euler2rotMat(phi,theta,psi)
	return fromRotMat(rot)
#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
#Use for testing the library
if __name__ == "__main__":
	import numpy as np
	q = np.array([1,0,0,0])
	#print toRotMat(q)
	#print toGravity([0.707,0.707,0,0])
	print fromEuler(30,0,0)
