# -*- coding: utf-8 -*-
'''
#------------------------------------------------------------------------------
# FEDERAL UNIVERSITY OF UBERLANDIA
# Faculty of Electrical Engineering
# Biomedical Engineering Lab
# Uberlândia, Brazil
#------------------------------------------------------------------------------
# Author: Andrei Nakagawa, MSc
# Contact: nakagawa.andrei@gmail.com
# URL: www.biolab.eletrica.ufu.br
# Git: https://github.com/BIOLAB-UFU-BRAZIL
#------------------------------------------------------------------------------
# Decription:
#------------------------------------------------------------------------------
'''
#------------------------------------------------------------------------------
# LIBRARIES
#------------------------------------------------------------------------------
import numpy as np
from enum import Enum
from sets import Set
from copy import copy
import geometry #Provides methods for rotating points in space
import quaternion #Provides methods that handles quaternions
#------------------------------------------------------------------------------
# CLASSES
#------------------------------------------------------------------------------
'''
Name: BodyJoints
This class contains the variables for acessing the joints
UNILAT, RIGHT and LEFT identify joints according to their hemisphere
TORSO, WAIST and HEAD identify the unilateral body segments
SHOUDLER, ELBOW, WRIST and HAND identify the upper joints
HIP, KNEE, ANKLE and FOOT identify the lower joints
'''
#------------------------------------------------------------------------------
class BodyJoints(Enum):
	#Body hemisphere
	UNILAT = ''
	RIGHT = 'right_'
	LEFT = 'left_'
	#Unilateral body segments
	TORSO = 'torso'
	WAIST = 'waist'
	HEAD = 'head'
	#Upper joints
	SHOULDER = 'shoulder'
	ELBOW = 'elbow'
	WRIST = 'wrist'
	HAND = 'hand'
	#Lower joints
	HIP = 'hip'
	KNEE = 'knee'
	ANKLE = 'ankle'
	FOOT = 'foot'
#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
'''
Name: Joint
This class provides the basic structure of every joint that composes the
complete skeleton.
The joint class provides a node of a hierarchical tree that is built according
to the kinematic chain of the human body. The TORSO is the origin of the body
reference frame and is the root of the tree. Every node can be linked to
three other nodes that are acessed according to their hemisphere: UNILAT, LEFT
or RIGHT. For example: The TORSO can be connected to the HEAD (UNILAT) and to the
LEFT and RIGHT SHOULDERs.
Every joint also has a name, a position (location in 3D space in respect to the TORSO)
and a quaternion that represents the orientation of the IMU sensor of that joint
(this orientation is given in respect to the world frame)
The method "setQuaternion" should be used to update the quaternion value of
the joint. Never assign the quaternion value directly because the direct quaternion
does not provide the necessary rotation of the joint according to its current
location in space.
'''
#------------------------------------------------------------------------------
class Joint():
	#Default constructor
	def __init__(self,_side,_segment,_position=None):
		#Name
		self.name = _side.value + _segment.value
		#Position
		self.position = _position
		#Origin (the fist location)
		self.origin = _position
		#Quaternion
		self.quaternion = None
		self.rotquaternion = [1.,0.,0.,0.]
		#The joint does not have nodes when it is created. The nodes are assigned
		#by the "Skeleton" class.
		self.links = [None,None,None]
		self.parent = []
	#Updates the quaternion of the joint
	#The rotation of the joint is given by the product between the new quaternion
	#and the conjugate of the current quaternion
	#The "rotquaternion" property is the one that will be used for rotating the
	#joint in space, while the "quaternion" property will serve to be used
	#when the new quaternion will be updated.
	def setQuaternion(self,_quaternion):
		newRot = quaternion.product(_quaternion,quaternion.conjugate(self.quaternion))
		self.quaternion = _quaternion
		self.rotquaternion = self.quaternion
		'''
		for i in range(len(self.rotquaternion)):
			if self.rotquaternion[i] < -1.0:
				self.rotquaternion[i] = -1.0
			elif self.rotquaternion[i] > 1.0:
				self.rotquaternion[i] = 1.0
	 	'''
#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
'''
Name: Skeleton
This class provides the human body skeleton and all the necessary methods for
performing movement reconstruction using quaternions acquired by a set of IMUs
located in specific joints of the body.
The 'body' property points to the first node of the hierarchical tree that
represents the kinematic chain of the human body.
The current implementation takes into consideration that the skeleton has two
root nodes (joints). The TORSO is the root of the upper limbs and the WAIST
is the root of the lower limb, with the TORSO being the origin of the body
reference frame. From the TORSO it is possible to navigate the kinematic chain
of the upper limbs with it having three links (HEAD, LEFT SHOULDER and RIGHT SHOULDER).
The WAIST can be used to naviate through the lower limbs with it having two links:
LEFT HIP and RIGHT HIP.
Thus, the current implementation has segmented the human body into upper and
lower joints.
When an object of the skeleton class, the complete model of the human body
(as a hierarchical tree), is built. Therefore, all the joints are already
connected as they should. It is not possible to create a new joint and add it
to the skeleton from outside. For example, you can't create an unilateral elbow
and add it to the skeleton.
The class keeps track of a set called "segments" that indicates what joints
are going to be used for body reconstruction. In this sense, the following
methods "add" and "remove" do not really erase a joint, but rather they only
indicate whether the join will be used or not. When a given joint is not being
used, it will have a null position and a null quaternion. Otherwise, when the
"add" method is called, the joint will receive an identity quaternion (1,0,0,0)
and its current location.
Pay attention that the joint when created should have a position that is
compatible with the identity quaternion. In general, for easieness, you can
assume that the default position of the human body are:
Acessing any joint of the body can be acessed directly by using the
method "getJoint". This is achieved by having a dictionary that stores the
object representing each joint of the skeleton according to their names.
**
About the rotation algorithm:
Rotation of any joint is performed following the hierarchical structure.
Whenever the "rotate" method is called, every joint will be visited and
rotated according to its property "rotquaternion". For upper limbs, we start
with the TORSO and for lower limbs we start with the WAIST.
Do remember that whenever a new quaternion is obtained, the "setQuaternion" method
should be called to assign the new value.
Rotation of higher-level nodes also rotate their lower-level linked segments.
Therefore, the algorithm works as follows:
- For every child node connected to the parent node, do:
- 1st step: Rotate the parent node according to its "rotquaternion" property.
The method "rotate" from the "geometry" library can perform this task. To rotate
it is necessary to perform a translation of the point to the origin, followed by
the rotation and another translation from the origin.
- 2nd step: For every joint (child node) connected to the parent, apply the
rotation given by the "rotquaternion" property of the parent, considering that
the parent position is the origin for the rotations. Some rotation might still
be present in this joint, therefore, it is necessary to estimate what rotation
is still left to be performed that was caused by a different joint rather than
the parent. Thus, the new quaternion should be estimated by finding the product
between the quaternion of the child node and the conjugate of the parent's
quaternion. Then, the "rotquaternion" property of the child node should be updated
with the new value.
- 3rd step: The parent is updated to be the next node in the hierarchy until there
is no more nodes to be visited
'''
class Skeleton():
	def __init__(self):

		#Set containing which body segments have been instantiated
		self.segments = Set()

		#Dictionary that will provide direct acess to the joints (objects from the Joint class)
		self.joints = {}

		#Dictionary that provides acess to the link segments of the body
		self.sideId = {BodyJoints.UNILAT:0, BodyJoints.LEFT:1, BodyJoints.RIGHT:2}

		#Creating the whole body
		#Unilateral segments
		torso = Joint(BodyJoints.UNILAT, BodyJoints.TORSO)
		head = Joint(BodyJoints.UNILAT, BodyJoints.HEAD)
		waist = Joint(BodyJoints.UNILAT, BodyJoints.WAIST)
		#Adding the joints to the dictionary
		self.joints[torso.name] = torso
		self.joints[head.name] = head
		self.joints[waist.name] = waist
		torso.parent = None

		#Right segments of the body
		#Upper limbs
		rshoulder = Joint(BodyJoints.RIGHT, BodyJoints.SHOULDER)
		relbow = Joint(BodyJoints.RIGHT, BodyJoints.ELBOW)
		rwrist = Joint(BodyJoints.RIGHT, BodyJoints.WRIST)
		rhand = Joint(BodyJoints.RIGHT, BodyJoints.HAND)
		rshoulder.links[self.sideId[BodyJoints.RIGHT]] = relbow
		relbow.links[self.sideId[BodyJoints.RIGHT]] = rwrist
		rwrist.links[self.sideId[BodyJoints.RIGHT]] = rhand
		#Defining the parents
		rshoulder.parent = torso
		relbow.parent = rshoulder
		rwrist.parent = relbow
		rhand.parent = rwrist
		#Adding the joints to the dictionary
		self.joints[rshoulder.name] = rshoulder
		self.joints[relbow.name] = relbow
		self.joints[rwrist.name] = rwrist
		self.joints[rhand.name] = rhand
		#Lower limbs
		rhip = Joint(BodyJoints.RIGHT, BodyJoints.HIP)
		rknee = Joint(BodyJoints.RIGHT, BodyJoints.KNEE)
		rankle = Joint(BodyJoints.RIGHT, BodyJoints.ANKLE)
		rfoot = Joint(BodyJoints.RIGHT, BodyJoints.FOOT)
		rhip.links[self.sideId[BodyJoints.RIGHT]] = rknee
		rknee.links[self.sideId[BodyJoints.RIGHT]] = rankle
		rankle.links[self.sideId[BodyJoints.RIGHT]] = rfoot
		#Adding the joints to the dictionary
		self.joints[rhip.name] = rhip
		self.joints[rknee.name] = rknee
		self.joints[rankle.name] = rankle
		self.joints[rfoot.name] = rfoot

		#Left segments of the body
		lshoulder = Joint(BodyJoints.LEFT, BodyJoints.SHOULDER)
		lelbow = Joint(BodyJoints.LEFT, BodyJoints.ELBOW)
		lwrist = Joint(BodyJoints.LEFT, BodyJoints.WRIST)
		lhand = Joint(BodyJoints.LEFT, BodyJoints.HAND)
		lshoulder.links[self.sideId[BodyJoints.LEFT]] = lelbow
		lelbow.links[self.sideId[BodyJoints.LEFT]] = lwrist
		lwrist.links[self.sideId[BodyJoints.LEFT]] = lhand
		#Adding the joints to the dictionary
		self.joints[lshoulder.name] = lshoulder
		self.joints[lelbow.name] = lelbow
		self.joints[lwrist.name] = lwrist
		self.joints[lhand.name] = lhand
		#Defining the parents
		lshoulder.parent = torso
		lelbow.parent = lshoulder
		lwrist.parent = lelbow
		lhand.parent = lwrist
		#Lower limbs
		lhip = Joint(BodyJoints.LEFT, BodyJoints.HIP)
		lknee = Joint(BodyJoints.LEFT, BodyJoints.KNEE)
		lankle = Joint(BodyJoints.LEFT, BodyJoints.ANKLE)
		lfoot = Joint(BodyJoints.LEFT, BodyJoints.FOOT)
		lhip.links[self.sideId[BodyJoints.LEFT]] = lknee
		lknee.links[self.sideId[BodyJoints.LEFT]] = lankle
		lankle.links[self.sideId[BodyJoints.LEFT]] = lfoot
		#Adding the joints to the dictionary
		self.joints[lhip.name] = lhip
		self.joints[lknee.name] = lknee
		self.joints[lankle.name] = lankle
		self.joints[lfoot.name] = lfoot

		#Adding body links to the torso
		torso.links[self.sideId[BodyJoints.UNILAT]] = head
		torso.links[self.sideId[BodyJoints.LEFT]] = lshoulder
		torso.links[self.sideId[BodyJoints.RIGHT]] = rshoulder

		#Adding body links to the waist
		waist.links[self.sideId[BodyJoints.UNILAT]] = None
		waist.links[self.sideId[BodyJoints.LEFT]] = lhip
		waist.links[self.sideId[BodyJoints.RIGHT]] = rhip

		#Add the origin of the body which is the torso
		self.add(BodyJoints.UNILAT, BodyJoints.TORSO, [0,0,0])

	#Checks if the informed side and joint are valid according to the enum
	def isvalid(self,_side,_joint):
		if(isinstance(_side,Enum) and isinstance(_joint,Enum)):
			if _side.name in BodyJoints.__members__ and _joint.name in BodyJoints.__members__:
				return True
			else:
				return False
		else:
			return False

	#Checks if the specified joint has already been instantiated
	def exist(self,_side,_joint,_name=None):
		if _name is not None:
			return _name in self.segments

		if(self.isvalid(_side,_joint)):
			if _side.value + _joint.value in self.segments:
				return True
			else:
				return False
		else:
			return False

	#Returns the desired joint if it is a valid one
	#Acessing joints are O(1) with dictionary despite the
	#data structure being a tree
	def getJoint(self,_side,_joint,_name=None):
		if _name is not None and _name in self.segments:
			return self.joints[_name]

		#Check if it is valid
		if self.isvalid(_side,_joint):
			if _side.value+_joint.value in self.segments:
				return self.joints[_side.value+_joint.value]
			else:
				return False
		else:
			return False

	#Adds a new joint to the body -> which means that a joint
	#will be instantiated and used to form the skeleton
	#If the joint is already instantiated, the method returns False
	def add(self,_side,_joint, _position):
		#Check if the joint already exists in body composition
		if(not self.exist(_side,_joint)):
			#Add the new segment to the list of created joints
			self.segments.add(_side.value + _joint.value)
			#Find the given joint in the body
			joint = self.getJoint(_side,_joint)
			#If found, updates its position and origin
			if joint is not False:
				#Updating the position
				joint.position = np.array(_position)
				#Updating the origin
				#The origin will only be altered in this method
				joint.origin = np.array(_position)
				#Quaternion at identity
				joint.quaternion = np.array([1,0,0,0])
				joint.setQuaternion(joint.quaternion)
				return True
			else:
				return False
		else:
			return False

	#Removes the specified joint from the set of body segments
	def remove(self,_side,_joint):
		if(self.exist(_side,_joint)):
			self.segments.remove(_side.value+_joint.value)
			joint = self.getJoint(_side,_joint)
			joint.position = None
			joint.origin = None
		else:
			return False

	#Returns the position of the specified joint
	def position(self,_side,_joint):
		if(self.exist(_side,_joint)):
			return self.getJoint(_side,_joint).position
		else:
			return False

	#Returns the origin of the specified joint
	def origin(self,_side,_joint):
		if(self.exist(_side,_joint)):
			return self.getJoint(_side,_joint).origin
		else:
			return False

	#Creates the proper array that links two joints
	def link(self,_joint1,_joint2):
		x = [_joint1.position[0],_joint2.position[0]]
		y = [_joint1.position[1],_joint2.position[1]]
		z = [_joint1.position[2],_joint2.position[2]]
		return x,y,z

	#Returns all the links between the joints according to hieararchy
	#This method returns an array that can be easily used for drawing the
	#skeleton in 3D using matplotlib
	def bodyLinks(self):
		body = []
		bodyx = []
		bodyy = []
		bodyz = []
		colors = ['black','blue','red']
		color = []

		#Finding the links from upper limbs
		for i in range(len(self.sideId)):
			aux = self.getJoint(BodyJoints.UNILAT,BodyJoints.TORSO)
			while aux.links[i] is not None:
				if aux.name in self.segments and aux.links[i].name in self.segments:
					link = self.link(aux,aux.links[i])
					bodyx.append(link[0])
					bodyy.append(link[1])
					bodyz.append(link[2])
					color.append(colors[i])
				aux = aux.links[i]
			aux = self.getJoint(BodyJoints.UNILAT,BodyJoints.TORSO)

		#If the waist segment exists
		if self.exist(BodyJoints.UNILAT,BodyJoints.WAIST):
			link = self.link(self.getJoint(BodyJoints.UNILAT,BodyJoints.TORSO),self.getJoint(BodyJoints.UNILAT,BodyJoints.WAIST))
			bodyx.append(link[0])
			bodyy.append(link[1])
			bodyz.append(link[2])
			color.append(colors[0])

		#Finding the links from lower limbs
		for i in range(len(self.sideId)-1):
			aux = self.getJoint(BodyJoints.UNILAT,BodyJoints.WAIST)
			idx = i+1
			while aux.links[idx] is not None:
				if aux.name in self.segments and aux.links[idx].name in self.segments:
					link = self.link(aux,aux.links[idx])
					bodyx.append(link[0])
					bodyy.append(link[1])
					bodyz.append(link[2])
					color.append(colors[idx])
				aux = aux.links[idx]

		body.append(bodyx)
		body.append(bodyy)
		body.append(bodyz)
		return body,color

	#This method returns a collection of the position of all the instantiated
	#body joints existing in the skeleton
	#This method returns an array that can be easily used for drawing the
	#position of the joints in 3D using matplotlib
	def bodyPoints(self):
		body = []
		bodyx = []
		bodyy = []
		bodyz = []
		for seg in self.segments:
			joint = self.getJoint(None,None,seg)
			bodyx.append(joint.position[0])
			bodyy.append(joint.position[1])
			bodyz.append(joint.position[2])
		body.append(bodyx)
		body.append(bodyy)
		body.append(bodyz)
		return body

	def rotate3(self):
		to = self.getJoint(BodyJoints.UNILAT, BodyJoints.TORSO)
		sh = self.getJoint(BodyJoints.RIGHT, BodyJoints.SHOULDER)
		el = self.getJoint(BodyJoints.RIGHT, BodyJoints.ELBOW)
		wr = self.getJoint(BodyJoints.RIGHT, BodyJoints.WRIST)

		rot = quaternion.toRotMat(to.rotquaternion)
		sh.position = geometry.rotate(sh.origin,to.origin,rot)
		el.position = geometry.rotate(el.origin,to.origin,rot)
		wr.position = geometry.rotate(wr.origin,to.origin,rot)

		sh.rotquaternion = quaternion.product(sh.quaternion, quaternion.conjugate(to.quaternion))
		el.rotquaternion = quaternion.product(el.quaternion, quaternion.conjugate(to.quaternion))
		wr.rotquaternion = quaternion.product(wr.quaternion, quaternion.conjugate(to.quaternion))
		rot = quaternion.toRotMat(sh.rotquaternion)
		sh.position = geometry.rotate(sh.position,to.position,rot)
		el.position = geometry.rotate(el.position,to.position,rot)
		wr.position = geometry.rotate(wr.position,to.position,rot)

		el.rotquaternion = quaternion.product(el.rotquaternion, quaternion.conjugate(sh.rotquaternion))
		wr.rotquaternion = quaternion.product(wr.rotquaternion, quaternion.conjugate(sh.rotquaternion))
		rot = quaternion.toRotMat(el.rotquaternion)
		el.position = geometry.rotate(el.position,sh.position,rot)
		wr.position = geometry.rotate(wr.position,sh.position,rot)

		wr.rotquaternion = quaternion.product(wr.rotquaternion, quaternion.conjugate(el.rotquaternion))
		rot = quaternion.toRotMat(wr.rotquaternion)
		wr.position = geometry.rotate(wr.position,el.position,rot)

	#Rotates the right and left arms
	def rotateArms(self):
		#Right side
		sh = self.getJoint(BodyJoints.RIGHT, BodyJoints.SHOULDER)
		el = self.getJoint(BodyJoints.RIGHT, BodyJoints.ELBOW)
		wr = self.getJoint(BodyJoints.RIGHT, BodyJoints.WRIST)
		rot = quaternion.toRotMat(el.rotquaternion)
		el.position = geometry.rotate(el.origin,sh.origin,rot)
		wr.position = geometry.rotate(wr.origin,sh.origin,rot)
		wr.rotquaternion = quaternion.product(wr.quaternion,quaternion.conjugate(el.quaternion))
		rot = quaternion.toRotMat(wr.rotquaternion)
		wr.position = geometry.rotate(wr.position,el.position,rot)

		#Left side
		lsh = self.getJoint(BodyJoints.LEFT, BodyJoints.SHOULDER)
		lel = self.getJoint(BodyJoints.LEFT, BodyJoints.ELBOW)
		lwr = self.getJoint(BodyJoints.LEFT, BodyJoints.WRIST)
		rot = quaternion.toRotMat(lel.rotquaternion)
		lel.position = geometry.rotate(lel.origin,lsh.origin,rot)
		lwr.position = geometry.rotate(lwr.origin,lsh.origin,rot)
		lwr.rotquaternion = quaternion.product(lwr.quaternion,quaternion.conjugate(lel.quaternion))
		rot = quaternion.toRotMat(lwr.rotquaternion)
		lwr.position = geometry.rotate(lwr.position,lel.position,rot)

	def rotate(self):
		#Rotating upper-limbs
		for i in range(len(self.sideId)):
			idx = i
			parent = self.getJoint(BodyJoints.UNILAT,BodyJoints.TORSO)
			p0 = parent.origin
			node = parent.links[idx]
			while True:
				if self.exist(None,None,parent.name):
					#Rotation of the parent itself
					#Rotation matrix from quaternion
					rot = quaternion.toRotMat(parent.rotquaternion)

					if parent.name == 'torso':
						parent.position = geometry.rotate(parent.origin,p0,rot)
					else:
						parent.position = geometry.rotate(parent.position,p0,rot)

					#Rotation of the linked segments
					while True:
						if node is not None and self.exist(None,None,node.name):

							if parent.name == 'torso':
								node.position = geometry.rotate(node.origin,p0,rot)
								node.rotquaternion = quaternion.product(node.quaternion,quaternion.conjugate(parent.quaternion))
							else:
								node.position = geometry.rotate(node.position,p0,rot)
								node.rotquaternion = quaternion.product(node.rotquaternion,quaternion.conjugate(parent.rotquaternion))

							if node.links[idx] is not None:
								node = node.links[idx]
							else:
								break
						else:
							break
					#Since the rotation have been applied, we can set the
					#"rotquaternion" of the parent to be the identity quaternion
					#parent.rotquaternion = np.array([1,0,0,0])
					#If exists another node in the hierarchy
					if parent.links[idx] is not None:
						#The origin for translations is the parent position
						p0 = parent.position
						#The parent becomes the next joint in the tree
						parent = parent.links[idx]
						#The first node is the next joint in the tree
						node = parent.links[idx]
					else:
						break
				else:
					break

		#Rotating lower-limbs
		for i in range(2): #Both sides
			idx = i+1
			parent = self.getJoint(BodyJoints.UNILAT,BodyJoints.WAIST)
			node = parent.links[idx]
			p0 = parent.origin
			while True:
				if self.exist(None,None,parent.name):
					rot = quaternion.toRotMat(parent.rotquaternion)
					#Rotation of the parent itself
					parent.position = geometry.rotate(parent.position,p0,rot)
					#Rotation of the linked segments
					while True:
						if node is not None and self.exist(None,None,node.name):
							node.position = geometry.rotate(node.position,p0,rot)
							node.rotquaternion = quaternion.product(node.rotquaternion,quaternion.conjugate(parent.rotquaternion))
							if node.name == 'right_ankle':
								print 'estou mudando o right ankle!'
								print 'olha o valor', node.rotquaternion
							if node.links[idx] is not None:
								node = node.links[idx]
							else:
								break
						else:
							break
					#Since the rotation have been applied, we can set the
					#"rotquaternion" of the parent to be the identity quaternion
					parent.rotquaternion = np.array([1,0,0,0])
					if parent.links[idx] is not None:
						p0 = parent.position
						parent = parent.links[idx]
						node = parent.links[idx]
					else:
						break
				else:
					break
#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
# MAIN
if __name__ == '__main__':
	s = Skeleton()
	s.add(BodyJoints.UNILAT, BodyJoints.HEAD, [0,0,10])
	s.add(BodyJoints.RIGHT, BodyJoints.SHOULDER, [3,0,0])
	s.add(BodyJoints.RIGHT, BodyJoints.ELBOW, [6,0,0])
	s.add(BodyJoints.RIGHT, BodyJoints.WRIST, [9,0,0])
	s.add(BodyJoints.RIGHT, BodyJoints.HAND, [10,0,0])
	s.getJoint(BodyJoints.UNILAT,BodyJoints.TORSO).setQuaternion([1,0,0,0])
	s.getJoint(BodyJoints.RIGHT,BodyJoints.SHOULDER).setQuaternion([0.966,0,0.259,0])
	s.getJoint(BodyJoints.RIGHT,BodyJoints.ELBOW).setQuaternion([0.966,0,0.259,0])
	s.getJoint(BodyJoints.RIGHT,BodyJoints.WRIST).setQuaternion([0.966,0,0.259,0])
	s.getJoint(BodyJoints.RIGHT,BodyJoints.HAND).setQuaternion([0.966,0,0.259,0])
	#print s.segments
	#s.rotate()
	#print 'Positions'
	#print 'Torso:', s.getJoint(BodyJoints.UNILAT,BodyJoints.TORSO).position
	#print 'Shoulder:', s.getJoint(BodyJoints.RIGHT,BodyJoints.SHOULDER).position
	#print 'Elbow:', s.getJoint(BodyJoints.RIGHT,BodyJoints.ELBOW).position
	#print 'Wrist:', s.getJoint(BodyJoints.RIGHT,BodyJoints.WRIST).position
	#print 'Hand:', s.getJoint(BodyJoints.RIGHT,BodyJoints.HAND).position
	s.rotateJoint(s.getJoint(BodyJoints.RIGHT,BodyJoints.WRIST),s.sideId[BodyJoints.RIGHT])
#------------------------------------------------------------------------------
