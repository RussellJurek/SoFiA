#! /usr/bin/env python

# import default python libraries
import pyfits
import os
import numpy as np


def read_data(inFile,weightsFile,maskFile):
	# import the fits file into an numpy array for the cube and a dictionary for the header:

	if os.path.isfile(inFile) == False:
		print 'FATAL ERROR: The defined data cube does not exist'
		print 'Cannot find: ' + inFile
		exit()

	print 'start loading cube: ' , inFile 
	f = pyfits.open(inFile)
	np_Cube = f[0].data
	dict_Header = f[0].header
	f.close


	# check whether the number of dimensions is acceptable
	print
	# the default is three axis
	if dict_Header['NAXIS'] == 3:
		print 'The input cube has 3 axis:'
		print 'type: ', dict_Header['CTYPE1'], dict_Header['CTYPE2'], dict_Header['CTYPE3'] 
		print 'dimensions: ', dict_Header['NAXIS1'],dict_Header['NAXIS2'],dict_Header['NAXIS3'] 


	# if there are more than 4 dimensions it will complain and later on probably crash
	if dict_Header['NAXIS'] > 4 or dict_Header['NAXIS'] < 1:
		print 'ERROR: The file has less than 1 or more than 4 dimensions'
	if dict_Header['NAXIS'] == 4:
		if dict_Header['NAXIS4'] != 1:
			print 'type: ', dict_Header['CTYPE1'], dict_Header['CTYPE2'], dict_Header['CTYPE3'], dict_Header['CTYPE4']  
			print 'dimensions: ', dict_Header['NAXIS1'],dict_Header['NAXIS2'],dict_Header['NAXIS3'],dict_Header['NAXIS4']  
			print 'ERROR: The 4th dimension has more than 1 value'
		else: np_Cube=np_Cube[0]
	
	if dict_Header['NAXIS'] == 2:
		print 'WARNING: The input cube has 2 axis, third axis added'
		print 'type: ', dict_Header['CTYPE1'], dict_Header['CTYPE2']
		print 'dimensions: ', dict_Header['NAXIS1'],dict_Header['NAXIS2']
		np_Cube = [np_Cube]

		
	if dict_Header['NAXIS'] == 1:
		print 'ERROR: The input has 1 axis, this is probably a spectrum instead of an 2D/3D image'
		print 'type: ', dict_Header['CTYPE1'] 
		print 'dimensions: ', dict_Header['NAXIS1']
		
	
	# check whether the axis are in the exepcted order:
	if dict_Header['ctype1'][0:2] != 'RA' or dict_Header['ctype2'][0:3] != 'DEC':
		print 'WARNING: the dimensions are not in the expected order'
	


	# the data cube has been loaded
	print 'The data cube has been loaded'


	# apply weights if a weightsfile exists:
	if weightsFile:
		# check whether the weightscube exists:
		if os.path.isfile(weightsFile) == False:
			print 'FATAL ERROR: The defined weighting cube does not exist'
			print 'Cannot find: ' + weightsFile
			exit()

		else:	
			# Scale the input cube with a weights cube
			# load the weights cube
			print 'start loading weights cube: ' , weightsFile
			f = pyfits.open(weightsFile)
			np_Weights_cube = f[0].data
			f.close()
			print 'Weights cube loaded'
		
			# check the number of axis, and add dimensions if required
			if len(np.shape(np_Weights_cube)) == 2:
				np_Weights_cube = np_Weights_cube[np.newaxis,:,:]
		
			print 'Start generating weighted cube'
			dimensions1 = np_Cube.shape
			dimensions2 = np_Weights_cube.shape
			# check whether spatial dimensions are similar
			if (dimensions1[1:3] != dimensions2[1:3]):
				print 'FATAL ERROR: dimensions of input cube and weights cube are not similar!'
				print 'dimensions input cube  : ', dimensions1
				print 'dimensions weights cube: ', dimensions2
				exit()
			else:
				np_Weighted_cube = np_Cube * np_Weights_cube
		
				# the input cube is replaced by the weighted cube, 			
				np_Cube = np_Weighted_cube
				del np_Weighted_cube
		
				print 'Weighted cube created'
				print
		
	if maskFile:
		# check whether the mask cube exists:
		if os.path.isfile(maskFile) == False:
			print 'FATAL ERROR: The defined mask cube does not exist'
			print 'Cannot find: ' + maskFile
			#print 'WARNING: Program continues, without using input mask'
			#mask=zeros(np_Cube.shape)
			exit()

		else:	
			# Scale the input cube with a weights cube
			# load the weights cube
			print 'start loading mask cube: ' , maskFile
			g = pyfits.open(maskFile)
			mask = g[0].data
			mask[mask>0]=1
			g.close()
			print 'Mask cube loaded'
	else: mask=np.zeros(np_Cube.shape)

	# The original data is replaced with the Weighted cube!
	# if weighting is being used, the data should be read in again, during the parameterisation.
 	f[0].data=np_Cube
	return np_Cube, dict_Header, mask
