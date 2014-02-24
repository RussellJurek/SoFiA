#! /usr/bin/env python

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
		rms=scipy.stats.nanmedian(abs(cube[z0:z1,y0:y1,x0:x1]-scipy.stats.nanmedian(cube[z0:z1,y0:y1,x0:x1],axis=None)),axis=None)/0.6745
	elif rmsMode=='std':
		rms=scipy.stats.nanstd(cube[z0:z1,y0:y1,x0:x1],axis=None)
	if verbose: print '    ... %s rms = %.2e (data units)'%(rmsMode,rms)
	return rms

def SizeFilter(mskt,sfx,sfy,sfz,sbx,sby,sbz,zt,sizeFilter,edgeMode='constant',verbose=0):
	mskt=nd.filters.gaussian_filter(mskt,[0,mt.sqrt(sfy**2+sby**2)/2.355,mt.sqrt(sfx**2+sbx**2)/2.355],mode=edgeMode)
	if zt=='b': mskt=nd.filters.uniform_filter1d(mskt,max(sbz,sfz),axis=0,mode=edgeMode)
	elif zt=='g': mskt=nd.filters.gaussian_filter1d(mskt,max(sbz,sfz/2.355),axis=0,mode=edgeMode)
	mskt[mskt< sizeFilter]=0
	mskt[mskt>=sizeFilter]=1
	return mskt

def MaskedCube(incube,msk,replace_value):
	maskedcube=np.copy(incube)
	maskedcube[msk]=np.sign(incube[msk])*np.minimum(abs(incube[msk]),replace_value)
	# this only decreases the absolute value of voxels already in the mask, or leaves it unchanged
	# if already lower than replace_value; the sign is unchanged
	return maskedcube

def SortKernels(kernels):
	# Sorting kernels
	uniquesky=[]
	velsmooth=[]
	velfshape=[]
	for jj in np.array(kernels):
		if list(jj[:2].astype(float)) not in uniquesky: uniquesky.append(list(jj[:2].astype(float)))
	uniquesky=[kk[1] for kk in sorted([(float(jj[0]),jj) for jj in uniquesky])]

	for jj in uniquesky:
		velsmooth.append([])
		velfshape.append([])
		for ii in np.array(kernels):
			if list(ii[:2].astype(float))==jj:
				velsmooth[-1].append(int(ii[2]))
				velfshape[-1].append(ii[3])
	return uniquesky,velsmooth,velfshape

def SCfinder(cube,header,kernels=[[0,0,0,'b'],],threshold=3.5,sizeFilter=0,maskScaleXY=2.,maskScaleZ=2.,kernelUnit='pixel',edgeMode='constant',rmsMode='negative',verbose=0):
	# Create binary mask array
	msk=np.zeros(cube.shape,'bool')
	found_nan=np.isnan(cube).sum()

	# Measure noise in original cube
	rms=GetRMS(cube,rmsMode=rmsMode,zoomx=1,zoomy=1,zoomz=1,verbose=verbose)

	# Sort kernels
	uniquesky,velsmooth,velfshape=SortKernels(kernels)

	# Loop over all xy kernels
	for jj in range(len(uniquesky)):
		[kx,ky]=uniquesky[jj]
		if kernelUnit=='world' or kernelUnit=='w':
			kx=abs(float(kx)/header['cdelt1']/3600)
			ky=abs(float(ky)/header['cdelt2']/3600)
		if verbose:  print '    Filter %2.0f %2.0f %2.0f %s ...'%(kx,ky,0,'-')

		mskxy=np.zeros(cube.shape,'bool')

		# Get previous xy kernel
		if jj: [kxold,kyold]=uniquesky[jj-1]
		else: [kxold,kyold]=[0,0]

		# Gaussian angular smoothing of *clipped* cube if needed
		if kx+ky:
			# smooth starting from the latest xy cube (clipped to the detection threshold times maskScaleXY)
			cubexy=nd.filters.gaussian_filter(np.clip(cubexy,-maskScaleXY*threshold*rmsxy,maskScaleXY*threshold*rmsxy),[0,mt.sqrt(ky**2-kyold**2)/2.355,mt.sqrt(kx**2-kxold**2)/2.355],mode=edgeMode)
			if found_nan: cubexy[np.isnan(cube)]=np.nan
			rmsxy=GetRMS(cubexy,rmsMode=rmsMode,zoomx=1,zoomy=1,zoomz=1,verbose=verbose)
			if found_nan: cubexy=np.nan_to_num(cubexy)
		elif found_nan: cubexy,rmsxy=np.nan_to_num(cube),rms
		else: cubexy,rmsxy=np.copy(cube),rms

		# Loop over all z kernels
		for ii in range(len(velsmooth[jj])):
			kz=velsmooth[jj][ii]
			kt=velfshape[jj][ii]
			if kernelUnit=='world' or kernelUnit=='w': kz=abs(float(kz)/header['cdelt3']*1000)

			# Velocity smoothing of *clipped* cube if needed
			if kz:
				if verbose:  print '    Filter %2.0f %2.0f %2.0f %s ...'%(kx,ky,kz,kt)
				if not ii:
					rmsxyz=rmsxy/mt.sqrt(kz)
					print '!!!'
				# smooth starting from the latest xy cube
				# before smoothing voxels already detected at the current angular resolution are brought down to +/-maskScaleZ*rmsxyz*threshold
				if kt=='b': cubexyz=nd.filters.uniform_filter1d(MaskedCube(cubexy,mskxy,maskScaleZ*rmsxyz*threshold),kz,axis=0,mode=edgeMode)
				elif kt=='g': cubexyz=nd.filters.gaussian_filter1d(MaskedCube(cubexy,mskxy,maskScaleZ*rmsxyz*threshold),kz/2.355,axis=0,mode=edgeMode)
				if found_nan: cubexyz[np.isnan(cube)]=np.nan
				rmsxyz=GetRMS(cubexyz,rmsMode=rmsMode,zoomx=1,zoomy=1,zoomz=1,verbose=verbose)
				if found_nan: cubexyz=np.nan_to_num(cubexyz)
			else: cubexyz,rmsxyz=cubexy,rmsxy

			# Add detected voxels to mask
			if sizeFilter:
				if verbose: print '      Adding detected voxels to xy mask with size filtering'
				# Get beam FWHM in pixels
				if 'BMAJ' in header.keys(): bmaj=header['bmaj']/abs(header['cdelt2']) # assumed to be along y axis
				else: bmaj=0
				if 'BMIN' in header.keys(): bmin=header['bmin']/abs(header['cdelt1']) # assumed to be along x axis
				else: bmin=bmaj
				mskxy=mskxy+SizeFilter(((cubexyz>=threshold*rmsxyz)+(cubexyz<=-threshold*rmsxyz)).astype('float32'),kx,ky,kz,bmin,bmaj,3,kt,sizeFilter,edgeMode=edgeMode,verbose=0).astype('bool')
			else:
				if verbose: print '      Adding detected voxels directly to xy mask ...'
				mskxy=mskxy+(cubexyz>=threshold*rmsxyz)+(cubexyz<=-threshold*rmsxyz)

		if verbose: print '    Adding xy mask to final mask ...'
		msk=msk+mskxy

	msk[np.isnan(cube)]=False
	return msk

import pyfits as pf
import numpy as np
import math as mt
import scipy
from scipy import optimize
import scipy.ndimage as nd
from sys import argv
import string
from os import path

