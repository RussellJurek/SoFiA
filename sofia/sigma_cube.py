#! /usr/bin/env python

# import default python libraries
import numpy as np
import scipy as sp
from functions import *

# function to read in a cube and generate a cube with sigma levels using 'MAD' statistic
# or simple standard deviation
# MAD is Median Absolute Deviation:

# this script is usefull to correct for variation in noise as function of frequency, noisy edges of cubes and channels with strong rfi

def sigma_scale(cube,edgeX,edgeY,edgeZ,statistic):
	verbose = 0


	# sigma scaling only works for 3D cubes, as it is mainyl designed to correct for differences in frequency
	
	
	
	# cube: the input cube
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
	dimensions = np.shape(cube)
	
	
	# calculated rms as function of x,y,z
	z_rms = np.zeros(dimensions[0])
	y_rms = np.zeros(dimensions[1])
	x_rms = np.zeros(dimensions[2])

	# define the range over which stats are calculated
	z1 = edge_z1
	z2 = len(z_rms) - edge_z2
	y1 = edge_y1
	y2 = len(y_rms) - edge_y2
	x1 = edge_x1
	x2 = len(x_rms) - edge_x2

	if statistic == 'mad':
		print 'Apply Median Absolute Deviation (MAD) statistic'
	if statistic == 'std':
		print 'Apply Standard Deviation (STD) statistic'
	if statistic == 'negative':
		print 'Apply Negatice statistic'


	
	for i in range(len(z_rms)):
		#z_rms[i] = GetRMS(cube[i,y1:y2,x1:x2],rmsMode=statistic,zoomx=1,zoomy=1,zoomz=1,verbose=verbose)
		z_rms[i] = GetRMS(cube[i,y1:y2,x1:x2],rmsMode=statistic,zoomx=1,zoomy=1,zoomz=1,verbose=verbose)
		#print 'z_rms[i] = ', z_rms[i]
	# scale the cube by the rms
	for i in range(len(z_rms)):
		if z_rms[i] > 0:
			cube[i,:,:] = cube[i,:,:]/z_rms[i] 

	for i in range(len(y_rms)):
		y_rms[i] = GetRMS(cube[z1:z2,i,x1:x2],rmsMode=statistic,zoomx=1,zoomy=1,zoomz=1,verbose=verbose)
	# scale the cube by the rms
	for i in range(len(y_rms)):
		if y_rms[i] > 0:
			cube[:,i,:] = cube[:,i,:]/y_rms[i] 
    
	for i in range(len(x_rms)):
		x_rms[i] = GetRMS(cube[z1:z2,y1:y2,i],rmsMode=statistic,zoomx=1,zoomy=1,zoomz=1,verbose=verbose)
	# scale the cube by the rms
	for i in range(len(x_rms)):
		if x_rms[i] > 0:
			cube[:,:,i] = cube[:,:,i]/x_rms[i] 


	
	print 'Sigma cube is created'
	print
	return cube
