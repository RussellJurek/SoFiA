#! /usr/bin/env python

import numpy as np
import math as mt
from scipy import optimize
import scipy.ndimage as nd


def GaussianNoise(F,N0,s0):
    return N0*np.exp(-F**2/2/s0**2)

def GetRMS(cube,rmsMode='negative',zoomx=1,zoomy=1,zoomz=10000,nrbins=10000,verbose=0,min_hist_peak=0.05):
	sh=cube.shape
	x0,x1=int(mt.ceil((1-1./zoomx)*sh[2]/2)),int(mt.floor((1+1./zoomx)*sh[2]/2))+1
	y0,y1=int(mt.ceil((1-1./zoomy)*sh[1]/2)),int(mt.floor((1+1./zoomy)*sh[1]/2))+1
	z0,z1=int(mt.ceil((1-1./zoomz)*sh[0]/2)),int(mt.floor((1+1./zoomz)*sh[0]/2))+1
	if verbose: print '    Estimating rms on subcube (x,y,z zoom = %.0f,%.0f,%.0f) ...'%(zoomx,zoomy,zoomz)
	if verbose: print '    ... Subcube shape is',cube[z0:z1,y0:y1,x0:x1].shape,'...'

	if rmsMode=='negative':
		cubemin=np.nanmin(cube)
		bins=np.arange(cubemin,abs(cubemin)/nrbins-1e-12,abs(cubemin)/nrbins)
		fluxval=(bins[:-1]+bins[1:])/2
		rmshisto=np.histogram(cube[z0:z1,y0:y1,x0:x1],bins=bins)[0]

		nrsummedbins=0
		while rmshisto[-nrsummedbins-1:].sum()<min_hist_peak*rmshisto.sum():
			nrsummedbins+=1
		if nrsummedbins:
			if verbose: print '    ... adjusting bin size to get a fraction of voxels in central bin >=',min_hist_peak
			nrbins/=nrsummedbins
			bins=np.arange(cubemin,abs(cubemin)/nrbins-1e-12,abs(cubemin)/nrbins)
			fluxval=(bins[:-1]+bins[1:])/2
			rmshisto=np.histogram(cube[z0:z1,y0:y1,x0:x1],bins=bins)[0]

		rms=abs(optimize.curve_fit(GaussianNoise,fluxval,rmshisto,p0=[rmshisto.max(),-fluxval[rmshisto<rmshisto.max()/2].max()*2/2.355])[0][1])
	elif rmsMode=='mad':
                # normal median does not deal with "NaN"
		# rms=np.median(abs(cube[z0:z1,y0:y1,x0:x1]-np.median(cube[z0:z1,y0:y1,x0:x1])))/0.6745
                # a[~np.isnan(a)] gives array "a" without "NaN"
                rms=np.median(abs(cube[z0:z1,y0:y1,x0:x1][~np.isnan(cube[z0:z1,y0:y1,x0:x1])]-np.median(cube[z0:z1,y0:y1,x0:x1][~np.isnan(cube[z0:z1,y0:y1,x0:x1])])))/0.6745
            
	elif rmsMode=='std':
                #rms=np.nanstd(cube[z0:z1,y0:y1,x0:x1])
                # nanstd crashes as it is not defined in numpy 1.7.1
                # normal std does not deal with "NaN"
                #rms=np.std(cube[z0:z1,y0:y1,x0:x1])
                # a[~np.isnan(a)] gives array "a" without "NaN"
                rms = np.std(cube[z0:z1,y0:y1,x0:x1][~np.isnan(cube[z0:z1,y0:y1,x0:x1])])
	if verbose: print '    ... %s rms = %.2e (data units)'%(rmsMode,rms)
	return rms
