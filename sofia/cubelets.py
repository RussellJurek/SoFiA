#! /usr/bin/env python
import numpy as np
import pyfits
import os
from sofia import writemoment
import math
from scipy.ndimage import map_coordinates


def writeSubcube(cube,header,mask,objects,cathead,outroot):
    
    # strip path variable to get the file name and the directory separately
    splitroot = outroot.split('/')
    cubename  = splitroot[-1]
    if len(splitroot)>1:
      outputDir = '/'.join(splitroot[:-1])+'/objects/'
    else:
      outputDir = './objects/'
      
    # chek if output directory exists and create it if not
    if os.path.exists(outputDir)==False:
      os.system('mkdir '+outputDir)

    
    # read all important information (central pixels & values, increments) from the header
    dX    = header['CDELT1']
    dY    = header['CDELT2']
    dZ    = header['CDELT3']
    cValX = header['CRVAL1']
    cValY = header['CRVAL2']
    cValZ = header['CRVAL3']
    cPixX = header['CRPIX1']-1
    cPixY = header['CRPIX2']-1
    cPixZ = header['CRPIX3']-1
    #specTypeX = header['CTYPE3']
    #specTypeY = header['CTYPE3']
    #specUnitY = header['BUNIT']
    #specUnitX = header['CUNIT3']
    cubeDim = cube.shape
    
    for obj in objects:
      
	# centers and bounding boxes 
	#obj = np.array(objects[rr - 1])
	Xc = obj[cathead=='Xm'][0]
	Yc = obj[cathead=='Ym'][0]
	Zc = obj[cathead=='Zm'][0]
	Xmin = obj[cathead=='Xmin'][0]
	Ymin = obj[cathead=='Ymin'][0]
	Zmin = obj[cathead=='Zmin'][0]
	Xmax = obj[cathead=='Xmax'][0]
	Ymax = obj[cathead=='Ymax'][0]
	Zmax = obj[cathead=='Zmax'][0]
	
	# if center of mass estimation is wrong replace by geometric center
	if Xc < 0 or Xc > cubeDim[2]-1:  Xc = obj[cathead=='Xg'][0]
	if Yc < 0 or Yc > cubeDim[1]-1:  Yc = obj[cathead=='Yg'][0]
	if Zc < 0 or Zc > cubeDim[0]-1:  Zc = obj[cathead=='Zg'][0]
    
        cPixXNew = int(Xc)
        cPixYNew = int(Yc)
        cPixZNew = int(Zc)
	
	# largest distance of source limits from the center
	maxX = 2*max(abs(cPixXNew-Xmin),abs(cPixXNew-Xmax))
	maxY = 2*max(abs(cPixYNew-Ymin),abs(cPixYNew-Ymax))
	maxZ = 2*max(abs(cPixZNew-Zmin),abs(cPixZNew-Zmax))
	
	# calculate the new bounding box for the mass centered cube
	XminNew = cPixXNew - maxX
	if XminNew < 0: XminNew = 0
	YminNew = cPixYNew - maxY
	if YminNew < 0: YminNew = 0
	ZminNew = cPixZNew - maxZ
	if ZminNew < 0: ZminNew = 0
	XmaxNew = cPixXNew + maxX
	if XmaxNew > cubeDim[2]-1: XmaxNew = cubeDim[2]-1
	YmaxNew = cPixYNew + maxY
	if YmaxNew > cubeDim[1]-1: YmaxNew = cubeDim[1]-1
	ZmaxNew = cPixZNew + maxZ
	if ZmaxNew > cubeDim[0]-1: ZmaxNew = cubeDim[0]-1
	
	# calculate the center with respect to the cutout cube
	cPixXCut = cPixX - XminNew
	cPixYCut = cPixY - YminNew
	cPixZCut = cPixZ - ZminNew
	
	# update header keywords:
	header['CRPIX1'] = cPixXCut+1
	header['CRPIX2'] = cPixYCut+1
	header['CRPIX3'] = cPixZCut+1
	
	# write the cubelet
	subcube = cube[ZminNew:ZmaxNew+1,YminNew:YmaxNew+1,XminNew:XmaxNew+1]
	hdu = pyfits.PrimaryHDU(data=subcube,header=header)
	hdulist = pyfits.HDUList([hdu])
	name = outputDir+cubename+'_'+str(int(obj[0]))+'.fits'
	hdulist.writeto(name,clobber=True)
	hdulist.close()
	
	# make PV diagram
	if 'ELL_PA' in cathead:
		pv_sampling=10
		pv_r=np.arange(-max(subcube.shape[1:]),max(subcube.shape[1:])-1+1./pv_sampling,1./pv_sampling)
		pv_y=Yc-YminNew+pv_r*math.cos(float(obj[cathead=='ELL_PA'][0])/180*math.pi)
		pv_x=Xc-XminNew-pv_r*math.sin(float(obj[cathead=='ELL_PA'][0])/180*math.pi)
		pv_x,pv_y=pv_x[(pv_x>=0)*(pv_x<=subcube.shape[2]-1)],pv_y[(pv_x>=0)*(pv_x<=subcube.shape[2]-1)]
		pv_x,pv_y=pv_x[(pv_y>=0)*(pv_y<=subcube.shape[1]-1)],pv_y[(pv_y>=0)*(pv_y<=subcube.shape[1]-1)]
		pv_x.resize((1,pv_x.shape[0]))
		pv_y.resize((pv_x.shape))
		pv_coords=np.concatenate((pv_y,pv_x),axis=0)
		pv_array=[]
		for jj in range(subcube.shape[0]):
			plane=map_coordinates(subcube[jj],pv_coords)
			plane=[plane[ii::pv_sampling] for ii in range(pv_sampling)]
			plane=np.array([ii[:plane[-1].shape[0]] for ii in plane])
			pv_array.append(plane.mean(axis=0))
		pv_array=np.array(pv_array)
		hdu = pyfits.PrimaryHDU(data=pv_array,header=header)
		hdulist = pyfits.HDUList([hdu])
		hdulist[0].header['CTYPE1']='PV--DIST'
		hdulist[0].header['CDELT1']=hdulist[0].header['CDELT2']
		hdulist[0].header['CRVAL1']=0
		hdulist[0].header['CRPIX1']=pv_array.shape[1]/2
		hdulist[0].header['CTYPE2']=hdulist[0].header['CTYPE3']
		hdulist[0].header['CDELT2']=hdulist[0].header['CDELT3']
		hdulist[0].header['CRVAL2']=hdulist[0].header['CRVAL3']
		hdulist[0].header['CRPIX2']=hdulist[0].header['CRPIX3']
		del hdulist[0].header['CTYPE3']
		del hdulist[0].header['CDELT3']
		del hdulist[0].header['CRVAL3']
		del hdulist[0].header['CRPIX3']
		name = outputDir+cubename+'_'+str(int(obj[0]))+'_pv.fits'
		hdulist.writeto(name,clobber=True)
		hdulist.close()

	# remove all other sources from the mask
	submask = mask[ZminNew:ZmaxNew+1,YminNew:YmaxNew+1,XminNew:XmaxNew+1].astype('int')
	submask[submask!=obj[0]] = 0
	submask[submask==obj[0]] = 1
	
	# write mask
	hdu = pyfits.PrimaryHDU(data=submask.astype('int16'),header=header)
	hdulist = pyfits.HDUList([hdu])
	name = outputDir+cubename+'_'+str(int(obj[0]))+'_mask.fits'
	hdulist.writeto(name,clobber=True)
	hdulist.close()
	


	# moment 0
	if 'vopt' in header['ctype3'].lower() or 'vrad' in header['ctype3'].lower() or 'velo' in header['ctype3'].lower() or 'felo' in header['ctype3'].lower():
        	if not 'cunit3' in header: dkms=abs(header['cdelt3'])/1e+3 # assuming m/s
        	elif header['cunit3'].lower()=='km/s': dkms=abs(header['cdelt3'])
        elif 'freq' in header['ctype3'].lower():
        	if not 'cunit3' in header or header['cunit3'].lower()=='hz': dkms=abs(header['cdelt3'])/1.42040575177e+9*2.99792458e+5 # assuming Hz
        	elif header['cunit3'].lower()=='khz': dkms=abs(header['cdelt3'])/1.42040575177e+6*2.99792458e+5
        else: dkms=1. # no scaling, avoids crashing
	m0=np.nan_to_num(subcube*submask.astype(bool)).sum(axis=0)
	m0*=dkms
	hdu = pyfits.PrimaryHDU(data=m0,header=header)
	hdu.header['bunit']+='.km/s'
	#hdu.header['datamin']=m0.min()
	#hdu.header['datamax']=m0.max()
	hdu.header['datamin']=np.nanmin(m0)
	hdu.header['datamax']=np.nanmax(m0)
	del(hdu.header['crpix3'])
	del(hdu.header['crval3'])
	del(hdu.header['cdelt3'])
	del(hdu.header['ctype3'])
	name = outputDir+cubename+'_'+str(int(obj[0]))+'_mom0.fits'
	hdu.writeto(name,clobber=True)
	
	
	# moment 1
	m1=(np.arange(subcube.shape[0]).reshape((subcube.shape[0],1,1))*np.ones(subcube.shape)-header['crpix3']+1)*header['cdelt3']+header['crval3']
        if 'vopt' in header['ctype3'].lower() or 'vrad' in header['ctype3'].lower() or 'velo' in header['ctype3'].lower() or 'felo' in header['ctype3'].lower():
        	if not 'cunit3' in header: m1/=1e+3 # assuming m/s
        	elif header['cunit3'].lower()=='km/s': pass
        elif 'freq' in header['ctype3'].lower():
        	if not 'cunit3' in header or header['cunit3'].lower()=='hz': m1*=2.99792458e+5/1.42040575177e+9 # assuming Hz
        	elif header['cunit3'].lower()=='khz': m1*=2.99792458e+5/1.42040575177e+6
        m0[m0==0]=np.nan
        m0/=dkms
        m1=np.divide(np.array(np.nan_to_num(m1*subcube*submask.astype('bool')).sum(axis=0)),m0)
	hdu = pyfits.PrimaryHDU(data=m1,header=header)
	hdu.header['bunit']='km/s'
	hdu.header['datamin']=np.nanmin(m1)
	hdu.header['datamax']=np.nanmax(m1)
	del(hdu.header['crpix3'])
	del(hdu.header['crval3'])
	del(hdu.header['cdelt3'])
	del(hdu.header['ctype3'])
	name = outputDir+cubename+'_'+str(int(obj[0]))+'_mom1.fits'
	hdu.writeto(name,clobber=True)
	
	
	# moment 2
	m2=(np.arange(subcube.shape[0]).reshape((subcube.shape[0],1,1))*np.ones(subcube.shape)-header['crpix3']+1)*header['cdelt3']+header['crval3']
	if 'vopt' in header['ctype3'].lower() or 'vrad' in header['ctype3'].lower() or 'velo' in header['ctype3'].lower() or 'felo' in header['ctype3'].lower():
        	if not 'cunit3' in header: m2/=1e+3 # assuming m/s
        	elif header['cunit3'].lower()=='km/s': pass
        elif 'freq' in header['ctype3'].lower():
        	if not 'cunit3' in header or header['cunit3'].lower()=='hz': m2*=2.99792458e+5/1.42040575177e+9 # assuming Hz
        	elif header['cunit3'].lower()=='khz': m2*=2.99792458e+5/1.42040575177e+6
	m2 = m2**2
	m2=np.divide(np.array(np.nan_to_num(m2*subcube*submask.astype('bool')).sum(axis=0)),m0)
	m2-=m1*m1
	m2=np.sqrt(m2)
	hdu = pyfits.PrimaryHDU(data=m2,header=header)
        hdu.header['bunit']='km/s.km/s'
        hdu.header['datamin']=np.nanmin(m1)
        hdu.header['datamax']=np.nanmax(m1)
        del(hdu.header['crpix3'])
        del(hdu.header['crval3'])
        del(hdu.header['cdelt3'])
        del(hdu.header['ctype3'])
        name = outputDir+cubename+'_'+str(int(obj[0]))+'_mom2.fits'
        hdu.writeto(name,clobber=True)
	
	# spectra
	spec = np.nansum(subcube*submask,axis=(1,2))
	f = open(outputDir+cubename+'_'+str(int(obj[0]))+'_spec.txt', 'w')
	#f.write('# '+specTypeX+' ('+specUnitX+')'+'  '+specTypeY+' ('+specUnitY+')\n')
	for i in range(0,len(spec)):
	  xspec = cValZ + (i+ZminNew-cPixZ) * dZ
	  f.write('%9.0f %15.6e %15.6e\n'%(i+ZminNew,xspec,spec[i]))
	f.close()

        
        
        
	  
