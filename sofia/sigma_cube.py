#! /usr/bin/env python

# import default python libraries
import numpy
import scipy
import matplotlib
import pyfits
import os
from numpy import *
from pylab import *
from scipy import *
import os

# function to read in a cube and generate a cube with sigma levels using 'MAD' statistic
# or simple standard deviation
# MAD is Median Absolute Deviation:

# this script is usefull to correct for variation in noise as function of frequency, noisy edges of cubes and channels with strong rfi

def sigma_scale(np_Cube,edgeX,edgeY,edgeZ,statistic):
	# np_cube: the input cube
	# edgeX,Y,Z: the edges of the cube that can be trimmed of (default 0,0,0)
	# statistic: mad or std (default mad)
	
	print 'Start generating Sigma-value cube'	
	
	edge_x1 = edgeX # the edge of the cube which is trimmed off when calculating the statistics
	edge_x2 = edgeX
	edge_y1 = edgeY
	edge_y2 = edgeY
	edge_z1 = edgeZ
	edge_z2 = edgeZ
	
	# check the dimensions of the cube (could be obtained from header information)
	dimensions = shape(np_Cube)
	np_Sigma_cube = np_Cube
	
	
	# calculated rms as function of x,y,z
	z_rms = zeros(dimensions[0])
	y_rms = zeros(dimensions[1])
	x_rms = zeros(dimensions[2])

	# define the range over which stats are calculated
	z1 = edge_z1
	z2 = len(z_rms) - edge_z2
	y1 = edge_y1
	y2 = len(y_rms) - edge_y2
	x1 = edge_x1
	x2 = len(x_rms) - edge_x2


	
	if statistic == 'mad':
		# apply 'MAD' statistic
		print 'Apply Median Absolute Deviation (MAD) statistic'	
		for i in range(len(z_rms)):
			z_rms[i] = 1.4826 * median(abs(np_Sigma_cube[i,y1:y2,x1:x2] - median(np_Sigma_cube[i,y1:y2,x1:x2])))
		# scale the cube by the rms
		for i in range(len(z_rms)):
			if z_rms[i] > 0:
				np_Sigma_cube[i,:,:] = np_Sigma_cube[i,:,:]/z_rms[i] 

		for i in range(len(y_rms)):
			y_rms[i] = 1.4826 * median(abs(np_Sigma_cube[z1:z2,i,x1:x2] - median(np_Sigma_cube[z1:z2,i,x1:x2])))
		for i in range(len(y_rms)):
			if y_rms[i] > 0:
				np_Sigma_cube[:,i,:] = np_Sigma_cube[:,i,:]/y_rms[i] 
    
		for i in range(len(x_rms)):
			x_rms[i] = 1.4826 * median(abs(np_Sigma_cube[z1:z2,y1:y2,i] - median(np_Sigma_cube[z1:z2,y1:y2,i])))    
		for i in range(len(x_rms)):
			if x_rms[i] > 0:
				np_Sigma_cube[:,:,i] = np_Sigma_cube[:,:,i]/x_rms[i] 

	if statistic == 'std':
		# apply the standard deviation as statistic
		print 'Apply Standard Deviation (STD) statistic'
		for i in range(len(z_rms)):
			z_rms[i] = std(np_Sigma_cube[i,y1:y2,x1:x2])
		# scale the cube by the rms
		for i in range(len(z_rms)):
			if z_rms[i] > 0:
				np_Sigma_cube[i,:,:] = np_Sigma_cube[i,:,:]/z_rms[i] 

		for i in range(len(y_rms)):
			y_rms[i] = std(np_Sigma_cube[z1:z2,i,x1:x2])    
		for i in range(len(y_rms)):
			if y_rms[i] > 0:
				np_Sigma_cube[:,i,:] = np_Sigma_cube[:,i,:]/y_rms[i] 
    
		for i in range(len(x_rms)):
			x_rms[i] = std(np_Sigma_cube[z1:z2,y1:y2,i])
		for i in range(len(x_rms)):
			if x_rms[i] > 0:
				np_Sigma_cube[:,:,i] = np_Sigma_cube[:,:,i]/x_rms[i] 
	
	# the input cube is replaced by the sigma cube
	np_Cube = np_Sigma_cube
	del np_Sigma_cube	
	print 'Sigma cube is created'
	print
	return np_Cube
