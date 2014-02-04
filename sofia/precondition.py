# precondition.py
#
# known bugs:
# - masking and boxcar at the some time results in blank cube, as boxcar smoothing routine doesn't handle NaNs

# import required modules

from __future__ import division  # make all division floating point. Otherwise is integer division
import string                    # for split, strip string functions
import os                        # for unix/linux system calls (eg 'ls')
import math                      # for maths commands (fabs)
import sys                       # system (sys.exit)
import re                        # for regular expressions
import random                    # for random numbers
import pyfits
import scipy
import numpy
import ast

from numpy import *
from scipy import ndimage

#---------------------
# subroutines
#---------------------

def readfits(fnamein):
    """
    read fits file, returning hdu

    """
    f1=pyfits.open(fnamein)
    return f1

def writefits(fnameout, data, header):
    """
    Create fits file from given filename, data and header

    """
    hdu = pyfits.PrimaryHDU()
    hdu.data = data
    hdu.header = header
    
    fitsobj = pyfits.HDUList()
    fitsobj.append(hdu)
    fitsobj.writeto(fnameout)

def smooth(indata, type, kern_px, kern_py, kern_pz):
    """
    Smooth image array

    type = g: gaussian
    type = b: boxcar (average)
    #type = m: boxcar (median), nb kernal must be 3D as currently written
    
    kern_px,y,z: sigma of gaussian kernal/box size
    """

    if type == "g":
        outdata=ndimage.gaussian_filter(indata, (kern_pz, kern_px, kern_py))
    elif type == "b":
        outdata=ndimage.uniform_filter(indata, (kern_pz, kern_px, kern_py))
    #elif type == "m":
    #    outdata=ndimage.filters.median_filter(input=indata, size=(kern_pz, kern_px, kern_py))
    else:
        print "FATAL: smoothing type not recognized"
        sys.exit()
    
    return outdata

#---------------------
# main
#---------------------

# specify preconditioning parameters

pc_pars = dict(imfile="test_in.fits")
pc_pars['smooth_type'] = "m"
pc_pars['kern_nx'] = 3
pc_pars['kern_ny'] = 3
pc_pars['kern_nz'] = 3
pc_pars['xchanmask'] = '(0,5)'
pc_pars['ychanmask'] = '(0,5),(-5,0)'
pc_pars['zchanmask'] = ''
   
# read in image data

image = readfits(pc_pars['imfile'])
imdata = image[0].data
imheader = image[0].header

# apply masks

if pc_pars['xchanmask']:    
    xchanmask_tuples = ast.literal_eval(pc_pars['xchanmask'])
    imdata[:,:,r_[xchanmask_tuples] ] = nan
if pc_pars['ychanmask']:
    ychanmask_tuples = ast.literal_eval(pc_pars['ychanmask'])
    imdata[:,r_[ychanmask_tuples],:] = nan
if pc_pars['zchanmask']:
    zchanmask_tuples = ast.literal_eval(pc_pars['zchanmask'])
    imdata[r_[zchanmask_tuples],:,: ] = nan

# smooth image

imdata_smooth = smooth(imdata, pc_pars['smooth_type'], pc_pars['kern_nx'], pc_pars['kern_ny'], pc_pars['kern_nz'])

# write output

writefits('test_out.fits', imdata_smooth, imheader)

# and that's it

sys.exit()
