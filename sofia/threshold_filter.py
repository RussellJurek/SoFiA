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

def filter(cube,header,clipMethod,threshold,rmsMode,verbose):
	
	mask = zeros(cube.shape)
	if clipMethod == 'relative':
		# determine the clip level
		# Measure noise in original cube
		rms=GetRMS(cube,rmsmode=rmsMode,zoomx=1,zoomy=1,zoomz=100000,verb=verbose,nrbins=100000)
		print 'Estimated rms = ', rms
		clip = threshold * rms
	if clipMethod == 'absolute':
		clip = threshold
	print 'using clip threshold: ', clip

	mask = mask + (cube >= clip)
	mask = mask + (cube <= -1*clip)
	
	return mask