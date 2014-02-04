#! /usr/bin/env python

import numpy as np
import math as mt
from scipy import optimize
import scipy.ndimage as nd


def GaussianNoise(F,N0,s0):
    return N0*np.exp(-F**2/2/s0**2)

def GetRMS(cube,rmsmode='negative',zoomx=1,zoomy=1,zoomz=10000,nrbins=100000,verb=0):
	sh=cube.shape
	x0,x1=int(mt.ceil((1-1./zoomx)*sh[2]/2)),int(mt.floor((1+1./zoomx)*sh[2]/2))+1
	y0,y1=int(mt.ceil((1-1./zoomy)*sh[1]/2)),int(mt.floor((1+1./zoomy)*sh[1]/2))+1
	z0,z1=int(mt.ceil((1-1./zoomz)*sh[0]/2)),int(mt.floor((1+1./zoomz)*sh[0]/2))+1
	if verb: print '    Estimating rms on subcube (x,y,z zoom = %.0f,%.0f,%.0f) ...'%(zoomx,zoomy,zoomz)
	if verb: print '    ... Subcube shape is',cube[z0:z1,y0:y1,x0:x1].shape,'...'

	if rmsmode=='negative':
		bins=np.arange(cube.min(),abs(cube.min())/nrbins,abs(cube.min())/nrbins)
		fluxval=(bins[:-1]+bins[1:])/2
		rms=abs(optimize.curve_fit(GaussianNoise,fluxval,np.histogram(cube[z0:z1,y0:y1,x0:x1],bins=bins)[0],p0=[1e+3,1e-3])[0][1])
	elif rmsmode=='robust':
		rms=np.median(abs(cube[z0:z1,y0:y1,x0:x1]-np.median(cube[z0:z1,y0:y1,x0:x1])))/0.6745
	elif rmsmode=='naive':
		rms=nd.standard_deviation(cube[z0:z1,y0:y1,x0:x1])
	if verb: print '    ... %s rms = %.2e (arbitrary units)'%(rmsmode,rms)
	return rms
