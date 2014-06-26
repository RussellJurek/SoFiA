#! /usr/bin/env python

# import default python libraries
from numpy import *
from pylab import *
from scipy import *
import numpy as np
import math as mt
from scipy import optimize
import scipy.ndimage as nd
import os
from functions import *

# a function to a simple threshold filter and write out a mask:

def filter(mask,cube,header,clipMethod,threshold,rmsMode,verbose):
	if clipMethod == 'relative':
		# determine the clip level
		# Measure noise in original cube
		# rms = GetRMS(cube,rmsmode=rmsMode,zoomx=1,zoomy=1,zoomz=100000,verb=verbose,nrbins=100000)
		rms = GetRMS(cube,rmsMode=rmsMode,zoomx=1,zoomy=1,zoomz=1,verbose=verbose)
		print 'Estimated rms = ', rms
		clip = threshold * rms
	if clipMethod == 'absolute':
		clip = threshold
	print 'using clip threshold: ', clip
	#return ((cube >= clip)+(cube <= -1*clip))
	np.logical_or(mask, (np.greater_equal(cube, clip) + np.less_equal(cube, -clip)), mask)
	
	return 
