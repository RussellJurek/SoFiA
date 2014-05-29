#! /usr/bin/env python

# import default python libraries
import pyfits
import os
#import numpy as np
from numpy import *
import re


#def read_data(inFile,weightsFile,maskFile):
def read_data(inFile, weightsFile, maskFile, weightsFunction = None):
	# import the fits file into an numpy array for the cube and a dictionary for the header:

	if os.path.isfile(inFile) == False:
		print 'FATAL ERROR: The specified data cube does not exist.'
		print 'Cannot find: ' + inFile
		raise SystemExit(1)

	print 'Loading cube: ' , inFile 
	f = pyfits.open(inFile)
	np_Cube = f[0].data
	dict_Header = f[0].header
	f.close


	# check whether the number of dimensions is acceptable
	print
	# the default is three axis
	if dict_Header['NAXIS'] == 3:
		print 'The input cube has 3 axes:'
		print 'type: ', dict_Header['CTYPE1'], dict_Header['CTYPE2'], dict_Header['CTYPE3'] 
		print 'dimensions: ', dict_Header['NAXIS1'], dict_Header['NAXIS2'], dict_Header['NAXIS3'] 
	elif dict_Header['NAXIS'] == 4:
		if dict_Header['NAXIS4'] != 1:
			print 'type: ', dict_Header['CTYPE1'], dict_Header['CTYPE2'], dict_Header['CTYPE3'], dict_Header['CTYPE4']  
			print 'dimensions: ', dict_Header['NAXIS1'], dict_Header['NAXIS2'], dict_Header['NAXIS3'], dict_Header['NAXIS4']  
			print 'ERROR: The 4th dimension has more than 1 value.'
			raise SystemExit(1)
		else: np_Cube=np_Cube[0]
	elif dict_Header['NAXIS'] == 2:
		print 'WARNING: The input cube has 2 axes, third axis added.'
		print 'type: ', dict_Header['CTYPE1'], dict_Header['CTYPE2']
		print 'dimensions: ', dict_Header['NAXIS1'], dict_Header['NAXIS2']
		np_Cube = [np_Cube]
	elif dict_Header['NAXIS'] == 1:
		print 'ERROR: The input has 1 axis, this is probably a spectrum instead of an 2D/3D image.'
		print 'type: ', dict_Header['CTYPE1'] 
		print 'dimensions: ', dict_Header['NAXIS1']
		raise SystemExit(1)
	else:
		print 'ERROR: The file has less than 1 or more than 4 dimensions.'
		raise SystemExit(1)
		
	
	# check whether the axis are in the expected order:
	#if dict_Header['ctype1'][0:2] != 'RA' or dict_Header['ctype2'][0:3] != 'DEC':
	#	print 'WARNING: the dimensions are not in the expected order'
	


	# the data cube has been loaded
	print 'The data cube has been loaded'


	# apply weights if a weights file exists:
	if weightsFile:
		# check whether the weights cube exists:
		if os.path.isfile(weightsFile) == False:
			print 'FATAL ERROR: The defined weights cube does not exist.'
			print 'Cannot find: ' + weightsFile
			raise SystemExit(1)

		else:
			# Scale the input cube with a weights cube
			# load the weights cube
			print 'Loading weights cube: ' , weightsFile
			f = pyfits.open(weightsFile)
			np_Weights_cube = f[0].data
			f.close()
			print 'Weights cube loaded.'
		
			# check the number of axis, and add dimensions if required
			if len(shape(np_Weights_cube)) == 2:
				np_Weights_cube = np_Weights_cube[newaxis,:,:]
		
			print 'Generating weighted cube.'
			dimensions1 = np_Cube.shape
			dimensions2 = np_Weights_cube.shape
			# check whether spatial dimensions are similar
			if (dimensions1[1:3] != dimensions2[1:3]):
				print 'FATAL ERROR: dimensions of input cube and weights cube differ!'
				print 'dimensions input cube  : ', dimensions1
				print 'dimensions weights cube: ', dimensions2
				raise SystemExit(1)
			else:
				np_Weighted_cube = np_Cube * np_Weights_cube
		
				# the input cube is replaced by the weighted cube, 			
				np_Cube = np_Weighted_cube
				del np_Weighted_cube
		
				print 'Weighted cube created.'
				print
	elif weightsFunction:
		# WARNING: This entire implementation is currently seriously flawed for the reasons given further down!
		# WARNING: I'm not sure if there is a safe way to properly implement multiplication of a data array 
		# WARNING: with a user-specified function in Python without the need for a whitelist, nested loops, 
		# WARNING: or the creation of multiple copies of the cube.
		print "Evaluating function: %s"%weightsFunction
		
		# Define whitelist of allowed character sequences:
		whitelist = ["x", "y", "z", "e", "E", "sin", "cos", "tan", "arcsin", "arccos", "arctan", "arctan2", "sinh", "cosh", "tanh", "arcsinh", "arccosh", "arctanh", "exp", "log", "sqrt", "square", "power", "absolute", "fabs", "sign"]
		
		# Search for all keywords consisting of consecutive sequences of alphabetical characters:
		# NOTE: Explicit conversion to string is required unless readoptions.py is modified!
		keywordsFound = filter(None, re.split("[^a-zA-Z]+", str(weightsFunction)))
		
		# Check for non-whitelisted sequences:
		for keyword in keywordsFound:
			if keyword not in whitelist:
				print "ERROR: Unsupported keyword/function found in weights function:"
				print "       %s"%weightsFunction
				print "       Please check your input."
				raise SystemExit(1)
		
		z, y, x = indices(np_Cube.shape)
		# WARNING: This is crazy, as it will create three additional copies of the entire cube in memory!!!
		#          In C you would have three nested loops, but that doesn't work in Python because it's too slow... :-(
		
		try:
			# NOTE: eval() should be safe now as we don't allow for non-whitelisted sequences...
			np_Cube *= eval(str(weightsFunction))
			# WARNING: There is no check here whether the expression to be evaluated is actually valid,
			#          e.g. SoFiA will crash if the weights function is sqrt(-1). 'try' doesn't catch this!
			#          Even if we set np.seterr(all='raise'), we still run into problems with expressions 
			#          that are valid but not floating-point numbers, e.g. sqrt((1,2)).
		except:
			print "ERROR: Failed to evaluate weights function:"
			print "       %s"%weightsFunction
			print "       Please check your input."
			raise SystemExit(1)
		
		print "Function-weighted cube created.\n"

	if maskFile:
		# check whether the mask cube exists:
		if os.path.isfile(maskFile) == False:
			print 'FATAL ERROR: The specified mask cube does not exist.'
			print 'Cannot find: ' + maskFile
			#print 'WARNING: Program continues, without using input mask'
			#mask=zeros(np_Cube.shape)
			raise SystemExit(1)

		else:	
			# Scale the input cube with a weights cube
			# load the weights cube
			print 'Loading mask cube: ' , maskFile
			g = pyfits.open(maskFile)
			mask = g[0].data
			mask[mask>0]=1
			g.close()
			print 'Mask cube loaded.'
	else: mask = zeros(np_Cube.shape)

	# The original data is replaced with the Weighted cube!
	# If weighting is being used, the data should be read in again during parameterisation.
	return np_Cube, dict_Header, mask
